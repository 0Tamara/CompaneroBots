import cv2
import numpy as np

# Kamera
cap = cv2.VideoCapture(0)

# Funkcia na detekciu farby
def detekuj_farbu(hsv_img, farba="oranžová"):
    # Nastavenie rozsahu pre oranžovú a zelenú farbu
    if farba == "oranžová":
        lower_color = np.array([5, 150, 150])
        upper_color = np.array([15, 255, 255])
    elif farba == "zelená":
        lower_color = np.array([35, 100, 100])
        upper_color = np.array([85, 255, 255])
    
    mask = cv2.inRange(hsv_img, lower_color, upper_color)
    return mask

# Hlavný cyklus
while True:
    success, img = cap.read()
    if not success:
        break

    # Prevod obrazu na RGB a HSV
    hsv_img = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)

    # Detekcia oranžovej farby
    mask_orange = detekuj_farbu(hsv_img, "oranžová")
    result_orange = cv2.bitwise_and(img, img, mask=mask_orange)

    # Detekcia zelenej farby
    mask_green = detekuj_farbu(hsv_img, "zelená")
    result_green = cv2.bitwise_and(img, img, mask=mask_green)

    # Zobrazenie rámčeka
    cv2.rectangle(img, (220, 150), (420, 250), (0, 255, 0), 2)

    # Skontrolujeme, či je oranžová farba v rámci
    if np.sum(mask_orange[150:250, 220:420]) > 0:  # Ak je oranžová farba v rámci
        cv2.putText(img, 'Detekovany Bubennik', (150, 130), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
    
    # Skontrolujeme, či je zelená farba v rámci
    if np.sum(mask_green[150:250, 220:420]) > 0:  # Ak je zelená farba v rámci
        cv2.putText(img, 'Detekovany Tanecnik', (150, 130), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)

    # Zobrazenie textu
    cv2.putText(img, 'Ukazte farbu pre detekciu robota!', (50, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)

    # Zobrazenie výsledku
    cv2.imshow("Detekcia Farby", img)

    # Ukončenie programu pri stlačení klávesy 'q'
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Ukončenie
cap.release()
cv2.destroyAllWindows()
