import cv2
import mediapipe as mp
import pygame
import numpy as np

# Inicializácia Pygame
pygame.init()
# Načítanie rozmerov obrazu z kamery
cap = cv2.VideoCapture(0)
width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

# Nastavenie Pygame okna podľa kamery
screen = pygame.display.set_mode((width, height))
pygame.display.set_caption("Test Gest")
clock = pygame.time.Clock()

# Mediapipe pre detekciu rúk
mp_hands = mp.solutions.hands
hands = mp_hands.Hands(max_num_hands=1)  # Zmena na max_num_hands=1 pre jednu ruku
mp_draw = mp.solutions.drawing_utils

# Kamera
cap = cv2.VideoCapture(0)

# Funkcia na spočítanie prstov
def spocitaj_prsty(hand_landmarks):
    prsty = []
    tip_ids = [4, 8, 12, 16, 20]

    # Palec
    if hand_landmarks.landmark[tip_ids[0]].x < hand_landmarks.landmark[tip_ids[0] - 1].x:
        prsty.append(1)
    else:
        prsty.append(0)

    # Ostatné prsty
    for id in range(1, 5):
        if hand_landmarks.landmark[tip_ids[id]].y < hand_landmarks.landmark[tip_ids[id] - 2].y:
            prsty.append(1)
        else:
            prsty.append(0)
    return prsty

# Funkcia na zobrazenie akcie (napr. hranie melódie)
def spust_akciu(akcia):
    if akcia == "bubnuj":
        print("Bubnujem! (Akcia spustená)")
    elif akcia == "rock":
        print("Rock gesto! (Akcia spustená)")

# Hlavný cyklus
running = True
while running:
    success, img = cap.read()
    if not success:
        break

    #img = cv2.flip(img, 1)  # Zrkadlenie
    img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    results = hands.process(img_rgb)

    # Detekcia rúk a gest
    prsty = []
    if results.multi_hand_landmarks:
        for hand_landmarks in results.multi_hand_landmarks:
            prsty = spocitaj_prsty(hand_landmarks)

            # Detekcia päsťe (žiadny prst)
            if prsty == [0, 0, 0, 0, 0]:
                print("Päsť detekovaná!")
                spust_akciu("bubnuj")  # Simulácia bubnovania

            # Detekcia rock gestom (palec, ukazovák, malíček)
            elif prsty == [1, 1, 0, 0, 1]:
                print("Rock gesto detekované!")
                spust_akciu("rock")  # Simulácia rockového gesta

    # Spracovanie udalostí
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    # Zobrazenie videa z kamery v Pygame
    img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    img_surface = pygame.surfarray.make_surface(np.rot90(img_rgb))
    screen.blit(img_surface, (0, 0))

    pygame.display.flip()
    clock.tick(30)

# Ukončenie
cap.release()
pygame.quit()