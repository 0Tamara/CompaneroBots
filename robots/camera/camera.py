import cv2
import mediapipe as mp
import math
from collections import deque

# Inicializácia Mediapipe
mp_pose = mp.solutions.pose
pose = mp_pose.Pose(min_detection_confidence=0.7, min_tracking_confidence=0.5)
mp_drawing = mp.solutions.drawing_utils

# Definícia spojení (pravá a ľavá ruka)
custom_connections = [
    (mp_pose.PoseLandmark.RIGHT_SHOULDER.value, mp_pose.PoseLandmark.RIGHT_ELBOW.value),
    (mp_pose.PoseLandmark.RIGHT_ELBOW.value, mp_pose.PoseLandmark.RIGHT_WRIST.value),
    (mp_pose.PoseLandmark.LEFT_SHOULDER.value, mp_pose.PoseLandmark.LEFT_ELBOW.value),
    (mp_pose.PoseLandmark.LEFT_ELBOW.value, mp_pose.PoseLandmark.LEFT_WRIST.value),
]

# Vyhladzovacie fronty
right_shoulder_angles = deque(maxlen=25)
right_elbow_angles = deque(maxlen=25)
left_shoulder_angles = deque(maxlen=25)
left_elbow_angles = deque(maxlen=25)
x_pixel_history = deque(maxlen=50)
x_pixel_changes = deque(maxlen=25)

# Funkcia na výpočet uhla ramena
def calculate_shoulder_angle(shoulder, elbow):
    vector = [elbow.x - shoulder.x, elbow.y - shoulder.y]
    ref_vector = [0, 1]
    cosine_angle = (vector[0] * ref_vector[0] + vector[1] * ref_vector[1]) / (
        math.sqrt(vector[0]**2 + vector[1]**2) * math.sqrt(ref_vector[0]**2 + ref_vector[1]**2) + 1e-10
    )
    angle = math.acos(max(min(cosine_angle, 1), -1))
    angle_deg = math.degrees(angle)
    if elbow.y < shoulder.y:
        angle_deg = 180 - angle_deg
    return max(0, min(180, int(angle_deg)))

# Funkcia na výpočet uhla lakťa
# sipky od lakta v oboch smeroch, pocita uhol, kosinusova veta
def calculate_elbow_angle(shoulder, elbow, wrist):
    ba = [shoulder.x - elbow.x, shoulder.y - elbow.y]
    bc = [wrist.x - elbow.x, wrist.y - elbow.y]
    cosine_angle = (ba[0] * bc[0] + ba[1] * bc[1]) / (
        math.sqrt(ba[0]**2 + ba[1]**2) * math.sqrt(bc[0]**2 + bc[1]**2) + 1e-10
    )
    angle = math.acos(max(min(cosine_angle, 1), -1))
    angle_deg = math.degrees(angle)
    return max(0, min(180, int(angle_deg)))

# Otvorenie kamery
cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("Chyba: Kamera nie je pripojená.")
    exit()

# Nastavenie rozlíšenia
width, height = 1280, 720
cap.set(cv2.CAP_PROP_FRAME_WIDTH, width)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, height)

# Premenné predvolene
prev_x_pixel = None
lateral_state = "Žiadny pohyb"
cumulative_x_change = 0
last_lateral_direction = None
tolerance_window = 0
last_valid_lateral = "Žiadny pohyb"

# Hlavná slučka
while True:
    ret, frame = cap.read()
    if not ret:
        print("Chyba: Nepodarilo sa načítať obraz.")
        break

    frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    results = pose.process(frame_rgb)

    if results.pose_landmarks:
        # Vykreslenie kľúčových bodov a spojení
        for connection in custom_connections:
            start = results.pose_landmarks.landmark[connection[0]]
            end = results.pose_landmarks.landmark[connection[1]]
            if start.visibility > 0.5 and end.visibility > 0.5:
                start_point = (int(start.x * frame.shape[1]), int(start.y * frame.shape[0]))
                end_point = (int(end.x * frame.shape[1]), int(end.y * frame.shape[0]))
                cv2.line(frame, start_point, end_point, (0, 255, 0), 2)
                cv2.circle(frame, start_point, 4, (0, 0, 255), -1)
                cv2.circle(frame, end_point, 4, (0, 0, 255), -1)

        # Získanie súradníc
        landmarks = results.pose_landmarks.landmark
        right_shoulder = landmarks[mp_pose.PoseLandmark.RIGHT_SHOULDER.value]
        right_elbow = landmarks[mp_pose.PoseLandmark.RIGHT_ELBOW.value]
        right_wrist = landmarks[mp_pose.PoseLandmark.RIGHT_WRIST.value]
        left_shoulder = landmarks[mp_pose.PoseLandmark.LEFT_SHOULDER.value]
        left_elbow = landmarks[mp_pose.PoseLandmark.LEFT_ELBOW.value]
        left_wrist = landmarks[mp_pose.PoseLandmark.LEFT_WRIST.value]

        # Kontrola istoty
        if all(p.visibility > 0.6 for p in [right_shoulder, right_elbow, right_wrist, left_shoulder, left_elbow, left_wrist]):
            # Uhly
            right_shoulder_angle = calculate_shoulder_angle(right_shoulder, right_elbow)
            right_elbow_angle = calculate_elbow_angle(right_shoulder, right_elbow, right_wrist)
            left_shoulder_angle = calculate_shoulder_angle(left_shoulder, left_elbow)
            left_elbow_angle = calculate_elbow_angle(left_shoulder, left_elbow, left_wrist)

            # Vyhladzovanie uhlov koli sumu aby mi neskakali stale hodnoty
            right_shoulder_angles.append(right_shoulder_angle)
            right_elbow_angles.append(right_elbow_angle)
            left_shoulder_angles.append(left_shoulder_angle)
            left_elbow_angles.append(left_elbow_angle)
            
            smoothed_right_shoulder = int(sum(right_shoulder_angles) / len(right_shoulder_angles)) if right_shoulder_angles else right_shoulder_angle
            smoothed_right_elbow = int(sum(right_elbow_angles) / len(right_elbow_angles)) if right_elbow_angles else right_elbow_angle
            smoothed_left_shoulder = int(sum(left_shoulder_angles) / len(left_shoulder_angles)) if left_shoulder_angles else left_shoulder_angle
            smoothed_left_elbow = int(sum(left_elbow_angles) / len(left_elbow_angles)) if left_elbow_angles else left_elbow_angle

            # Detekcia laterálneho pohybu
            if right_shoulder.visibility > 0.6 and left_shoulder.visibility > 0.6:
                # X-súradnica
                current_x = (right_shoulder.x + left_shoulder.x) / 2
                current_x_pixel = current_x * frame.shape[1]
                x_pixel_history.append(current_x_pixel)

                if prev_x_pixel is not None:
                    # Vpravo/Vľavo
                    x_pixel_change = current_x_pixel - prev_x_pixel
                    x_pixel_changes.append(x_pixel_change)
                    avg_x_change = sum(x_pixel_changes) / len(x_pixel_changes) if x_pixel_changes else x_pixel_change
                    threshold_x_pixel = 1.0

                    if tolerance_window > 0:
                        tolerance_window -= 1
                        lateral_state = "Žiadny pohyb"
                    else:
                        if avg_x_change > threshold_x_pixel:
                            lateral_state = "Vpravo"
                            current_direction = "Vpravo"
                        elif avg_x_change < -threshold_x_pixel:
                            lateral_state = "Vľavo"
                            current_direction = "Vľavo"
                        else:
                            lateral_state = "Žiadny pohyb"
                            current_direction = None

                    # Kumulatívna detekcia
                    if len(x_pixel_history) == x_pixel_history.maxlen:
                        cumulative_x_change = x_pixel_history[-1] - x_pixel_history[0]
                        if last_lateral_direction and current_direction and last_lateral_direction != current_direction:
                            tolerance_window = 5
                        cumulative_threshold = 10
                        if cumulative_x_change > cumulative_threshold:
                            lateral_state = "Vpravo"
                            current_direction = "Vpravo"
                        elif cumulative_x_change < -cumulative_threshold:
                            lateral_state = "Vľavo"
                            current_direction = "Vľavo"

                    last_lateral_direction = current_direction

                    # Debug výpis, ptm to nejako upravim aby sa to posielalo
                    print(f"X-súradnica: {current_x:.3f}, Zmena: {(current_x - (prev_x_pixel / frame.shape[1])):.3f}")
                    print(f"X-pixel: {current_x_pixel:.1f}px, Zmena: {x_pixel_change:.1f}px, Kumulatívna: {cumulative_x_change:.1f}px")
                    print(f"Pravé rameno: {smoothed_right_shoulder}°, Pravý lakeť: {smoothed_right_elbow}°")
                    print(f"Ľavé rameno: {smoothed_left_shoulder}°, Ľavý lakeť: {smoothed_left_elbow}°")
                    print(f"Pohyb: {lateral_state}")

                    last_valid_lateral = lateral_state

                prev_x_pixel = current_x_pixel

                # Zobrazenie pohybu
                cv2.rectangle(frame, (5, 0), (300, 150), (255, 255, 255), -1)
                cv2.rectangle(frame, (5, 0), (300, 150), (0, 0, 0), 1)

                font = cv2.FONT_HERSHEY_DUPLEX
                cv2.putText(frame, f"Pravé rameno: {smoothed_right_shoulder}°", (10, 30), font, 0.6, (255, 0, 0), 1)
                cv2.putText(frame, f"Pravý lakeť: {smoothed_right_elbow}°", (10, 50), font, 0.6, (255, 0, 0), 1)
                cv2.putText(frame, f"Ľavé rameno: {smoothed_left_shoulder}°", (10, 70), font, 0.6, (255, 0, 0), 1)
                cv2.putText(frame, f"Ľavý lakeť: {smoothed_left_elbow}°", (10, 90), font, 0.6, (255, 0, 0), 1)
                cv2.putText(frame, f"Pohyb: {lateral_state}", (10, 110), font, 0.6, (255, 0, 0), 1)

            else:
                print("Niektoré kľúčové body nemajú dostatočnú istotu.")
                lateral_state = last_valid_lateral

                cv2.rectangle(frame, (5, 0), (300, 150), (255, 255, 255), -1)
                cv2.rectangle(frame, (5, 0), (300, 150), (0, 0, 0), 1)
                font = cv2.FONT_HERSHEY_DUPLEX
                cv2.putText(frame, f"Pravé rameno: N/A", (10, 30), font, 0.6, (255, 0, 0), 1)
                cv2.putText(frame, f"Pravý lakeť: N/A", (10, 50), font, 0.6, (255, 0, 0), 1)
                cv2.putText(frame, f"Ľavé rameno: N/A", (10, 70), font, 0.6, (255, 0, 0), 1)
                cv2.putText(frame, f"Ľavý lakeť: N/A", (10, 90), font, 0.6, (255, 0, 0), 1)
                cv2.putText(frame, f"Pohyb: {lateral_state}", (10, 110), font, 0.6, (255, 0, 0), 1)

        cv2.imshow('Detekcia pohybu', frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

cap.release()
cv2.destroyAllWindows()
pose.close()