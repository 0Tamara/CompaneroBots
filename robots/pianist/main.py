#!/usr/bin/env pybricks-micropython
from pybricks.hubs import EV3Brick
from pybricks.ev3devices import (Motor, TouchSensor, ColorSensor,
                                 InfraredSensor, UltrasonicSensor, GyroSensor)
from pybricks.parameters import Port, Stop, Direction, Button, Color
from pybricks.tools import wait, StopWatch, DataLog
from pybricks.robotics import DriveBase
from pybricks.media.ev3dev import SoundFile, ImageFile
import threading
ev3 = EV3Brick()

# Inicializácia farebného senzora na porte S1
color_sensor = ColorSensor(Port.S1)


print("Spúšťam hlavný program!")
wait(10000)
# hlavny program
motorA = Motor(Port.A)
motorB = Motor(Port.B)
motorC = Motor(Port.C)
motorD = Motor(Port.D)

# dajake tempo priblizne podobne ako v pesnicke
tempo = int(3870) # uz sa menit nebude
tempo_freedom = int(6085) # priblizne, mozno ze sa bude menit
# pocet opakovani cyklu funkcie
pocet = 1
# noty
def osminova_nota(klavesa, servo):
    servo.stlac()
    print("klavesa " + klavesa + " je stlacena")
    wait(tempo / 8)
    #print("klavesa " + klavesa + " je pustena")
    servo.pusti()

def osminova_nota_freedom(klavesa, servo):
    servo.stlac()
    print("klavesa " + klavesa + " je stlacena")
    wait(tempo_freedom / 8)
    #print("klavesa " + klavesa + " je pustena")
    servo.pusti()
# pomlcka
def osminova_pomlcka():
    print("pomlcka")
    wait(tempo / 8)
#casy jednotlivycho not bez stlacania tych not
def osminova_pomlcka_freedom():
    print("pomlcka")
    wait(tempo_freedom / 8)

cas_osminovej_freedom = int(tempo_freedom / 8)

cas_osminovej = int(tempo / 8)

#toto je specialne iba za bodku za notou
def bodka_za_notou_alebo_pomlckou(nota_alebo_pomlcka):
    print(".")
    wait(nota_alebo_pomlcka / 2)


# tu bude servo a pohyb so servom
class Servo:
    def __init__(self, Motor, uhol_stlacenia):
        self.Motor = Motor
        self.uhol_stlacenia = uhol_stlacenia
    def stlac(self):
        self.Motor.run_angle(1000, self.uhol_stlacenia, wait= False)        
    def pusti(self):
        self.Motor.run_angle(1000, -(self.uhol_stlacenia), wait= False)        

#oznacovanie not na klaviature
f = Servo(motorA, 70)
g = Servo(motorA, -70)
a = Servo(motorB, 70)
h = Servo(motorB, -80)
c = Servo(motorC, 70)
d = Servo(motorC, -70)
e = Servo(motorD, 80)
f1 = Servo(motorD, -80)
#samotna skladba

#freedom na tempo: 6085

print("Čakám na bielu farbu...")
while color_sensor.ambient() < 20:
    wait(10)
wait(6000)
print("IDZECE")
for _ in range(2):
    #stvrty takt
    threading.Thread(target=osminova_nota_freedom, args=('e', e)).start()
    osminova_nota_freedom("c", c)
    bodka_za_notou_alebo_pomlckou( cas_osminovej_freedom)
    threading.Thread(target=osminova_nota_freedom, args=('e', e)).start()
    osminova_nota_freedom("c", c)
    #prvy takt
    bodka_za_notou_alebo_pomlckou(cas_osminovej_freedom)
    osminova_nota_freedom("g", g)
    bodka_za_notou_alebo_pomlckou(cas_osminovej_freedom)
    osminova_nota_freedom("d", d)
    bodka_za_notou_alebo_pomlckou(cas_osminovej_freedom)
    #druhy takt
    threading.Thread(target=osminova_nota_freedom, args=('e', e)).start()
    osminova_nota_freedom("c", c)
    bodka_za_notou_alebo_pomlckou( cas_osminovej_freedom)
    osminova_nota_freedom("d", d)
    bodka_za_notou_alebo_pomlckou( cas_osminovej_freedom)
    #treti takt
    threading.Thread(target=osminova_nota_freedom, args=('e', e)).start()
    osminova_nota_freedom("c", c)
    bodka_za_notou_alebo_pomlckou(cas_osminovej_freedom)
    osminova_nota_freedom('c', c)
    bodka_za_notou_alebo_pomlckou(cas_osminovej_freedom)
    #stvrty takt
    threading.Thread(target=osminova_nota_freedom, args=('f', f)).start() # mozem zmenit na basic e , c
    osminova_nota_freedom("c", c)
    bodka_za_notou_alebo_pomlckou( cas_osminovej_freedom)
    osminova_nota_freedom("d", d)
    bodka_za_notou_alebo_pomlckou( cas_osminovej_freedom)
    #prvy takt
    threading.Thread(target=osminova_nota_freedom, args=('e', e)).start()
    osminova_nota_freedom("c", c)
    bodka_za_notou_alebo_pomlckou( cas_osminovej_freedom)
    osminova_nota_freedom("d", d)
    bodka_za_notou_alebo_pomlckou( cas_osminovej_freedom)
    #druhy takt
    threading.Thread(target=osminova_nota_freedom, args=('e', e)).start()
    osminova_nota_freedom("c", c)
    bodka_za_notou_alebo_pomlckou(cas_osminovej_freedom)
    osminova_nota_freedom("c", c)
    bodka_za_notou_alebo_pomlckou(cas_osminovej_freedom)
    #treti takt
    threading.Thread(target=osminova_nota_freedom, args=('e', e)).start()
    osminova_nota_freedom("h", h)
    bodka_za_notou_alebo_pomlckou(cas_osminovej_freedom)
    threading.Thread(target=osminova_nota_freedom, args=('e', e)).start()
    osminova_nota_freedom("h", h)
    bodka_za_notou_alebo_pomlckou(cas_osminovej_freedom)

#fireball na tempo: 3875
# Čakaj, kým sa nedetekuje biela farba

print("Čakám na bielu farbu...")
while color_sensor.ambient() < 20:
    wait(10)
wait(1000)
for _ in range(4):
    osminova_nota("e", e)
    osminova_pomlcka()
    osminova_nota("f", f)
    osminova_pomlcka()
for _ in range(4): 
    osminova_nota("c", c)
    osminova_pomlcka() 
    osminova_nota("f", f)
    osminova_pomlcka()
for _ in range(2):
    osminova_nota("c", c)
    osminova_pomlcka()
    osminova_nota("c", c)
    osminova_pomlcka()
    osminova_nota("f1", f1) 
    osminova_pomlcka()   
    osminova_nota("f1", f1)
    osminova_pomlcka()
    osminova_nota("e", e)
    osminova_pomlcka()
    osminova_nota("c", c)
    osminova_pomlcka()
    osminova_nota("f", f)
    osminova_pomlcka()
    osminova_nota("f", f)
    osminova_pomlcka()
for _ in range(3):
    osminova_nota("c", c)
    osminova_pomlcka()
    osminova_nota("f1", f1)
    osminova_pomlcka()
    osminova_nota("c", c)
    osminova_pomlcka()
    osminova_nota("f", f)
    osminova_pomlcka()
osminova_nota("c", c)
osminova_pomlcka()
osminova_nota("f1", f1)
osminova_pomlcka()