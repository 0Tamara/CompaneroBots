#include <ESP32Servo.h>
#include <FastAccelStepper.h>

#define numServos 8
#define stepsPerNote 123
#define stepsPerOctave 855

const int leftHandStepPin = 14; 
const int leftHandDirPin = 12;
const int rightHandStepPin = 18;
const int rightHandDirPin = 19;

//AccelStepper stepperLeft(AccelStepper::DRIVER, leftHandStepPin, leftHandDirPin);
//AccelStepper stepperRight(AccelStepper::DRIVER, rightHandStepPin, rightHandDirPin);

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepperLeft = NULL;
FastAccelStepper *stepperRight = NULL;

struct Hand {
  Servo servos[numServos];
  int servoPins[numServos];
  int currentOctave;
  int currentNote;
  FastAccelStepper& stepper;
  unsigned long timeFromMoving;
  unsigned long lastTime;
};

Hand leftHand = {
  .servoPins = {2, 4, 5, 16, 13, 17, 15, 25},
  .currentOctave = 0,
  .currentNote = 0,
  .timeFromMoving = 0,
  .lastTime = 0,
  .stepper = stepperLeft
};
Hand rightHand = {
  .servoPins = {26, 27, 32, 33, 21, 22, 23, 4},
  .currentOctave = 0,
  .currentNote = 0,
  .timeFromMoving = 0,
  .lastTime = 0,
  .stepper = stepperRight
};

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < numServos; i++) {
    if (!leftHand.servos[i].attach(leftHand.servoPins[i])) {
      Serial.printf("Servo %d na pine %d (lava ruka) nepripojeno\n", i, leftHand.servoPins[i]);
    }
    if (!rightHand.servos[i].attach(rightHand.servoPins[i])) {
      Serial.printf("Servo %d na pine %d (prava ruka) nepripojeno\n", i, rightHand.servoPins[i]);
    }
    leftHand.servos[i].write(0);
    rightHand.servos[i].write(0);
  }
  stepperLeft.setMaxSpeed(1000);
  stepperLeft.setAcceleration(500);
  stepperLeft.setCurrentPosition(0);
  stepperRight.setMaxSpeed(1000);
  stepperRight.setAcceleration(500);
  stepperRight.setCurrentPosition(0);

  // Test krokových motorov
  unsigned long start = millis();
  lavy_doprava(stepsPerOctave);
  Serial.printf("Čas pohybu: %lu ms\n", millis() - start);
  delay(1000);
  start = millis();
  lavy_dolava(stepsPerOctave);
  Serial.printf("Čas pohybu: %lu ms\n", millis() - start);
  delay(1000);
  start = millis();
  pravy_doprava(stepsPerOctave);
  Serial.printf("Čas pohybu: %lu ms\n", millis() - start);
  delay(1000);
  start = millis();
  pravy_dolava(stepsPerOctave);
  Serial.printf("Čas pohybu: %lu ms\n", millis() - start);
  delay(1000);
  start = millis();
  lavy_doprava(stepsPerNote);
  Serial.printf("Čas pohybu: %lu ms\n", millis() - start);
  delay(1000);
  start = millis();
  lavy_dolava(stepsPerNote);
  Serial.printf("Čas pohybu: %lu ms\n", millis() - start);
  delay(1000);
  start = millis();
  pravy_doprava(stepsPerNote);
  Serial.printf("Čas pohybu: %lu ms\n", millis() - start);
  delay(1000);
  start = millis();
  pravy_dolava(stepsPerNote);
  Serial.printf("Čas pohybu: %lu ms\n", millis() - start);
  delay(1000);

  // Test serv
  Serial.println("a teraz serva");
  for (int i = 0; i < numServos; i++) {
    Serial.printf("servo %d na lavej ruke\n", i);
    unsigned long servoStart = millis();
    leftHand.servos[i].write(90);
    delay(500); // Čas na stlačenie
    unsigned long pressTime = millis() - servoStart;
    Serial.printf("Čas stlačenia: %lu ms\n", pressTime);
    servoStart = millis();
    leftHand.servos[i].write(0);
    delay(500); // Čas na pustenie
    Serial.printf("Čas pustenia: %lu ms\n", millis() - servoStart);
  }
  for (int i = 0; i < numServos; i++) {
    Serial.printf("Testujem servo %d na pravej ruke\n", i);
    unsigned long servoStart = millis();
    rightHand.servos[i].write(90);
    delay(500); // Čas na stlačenie
    unsigned long pressTime = millis() - servoStart;
    Serial.printf("Čas stlačenia: %lu ms\n", pressTime);
    servoStart = millis();
    rightHand.servos[i].write(0);
    delay(500); // Čas na pustenie
    Serial.printf("Čas pustenia: %lu ms\n", millis() - servoStart);
  }

  // Test simultánneho pohybu
  Serial.println("Testujem simultánny pohyb");
  xTaskCreatePinnedToCore(
    [](void *) {
      Serial.println("Ľavá ruka: Pohyb doprava o oktávu");
      unsigned long start = millis();
      lavy_doprava(stepsPerOctave);
      Serial.printf("Ľavá ruka: Čas pohybu doprava: %lu ms\n", millis() - start);
      start = millis();
      lavy_dolava(stepsPerOctave);
      Serial.printf("Ľavá ruka: Čas pohybu doľava: %lu ms\n", millis() - start);
      vTaskDelete(NULL);
    }, "LeftHandTask", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(
    [](void *) {
      Serial.println("Pravá ruka: Pohyb doprava o oktávu");
      unsigned long start = millis();
      pravy_doprava(stepsPerOctave);
      Serial.printf("Pravá ruka: Čas pohybu doprava: %lu ms\n", millis() - start);
      start = millis();
      pravy_dolava(stepsPerOctave);
      Serial.printf("Pravá ruka: Čas pohybu doľava: %lu ms\n", millis() - start);
      vTaskDelete(NULL);
    }, "RightHandTask", 4096, NULL, 1, NULL, 1);
}

void loop() {
  // Prázdne
}

void lavy_doprava(int steps) {
  stepperLeft.move(steps);
  while (stepperLeft.distanceToGo() != 0) {
    stepperLeft.run();
  }
}

void lavy_dolava(int steps) {
  stepperLeft.move(-steps);
  while (stepperLeft.distanceToGo() != 0) {
    stepperLeft.run();
  }
}

void pravy_doprava(int steps) {
  stepperRight.move(steps);
  while (stepperRight.distanceToGo() != 0) {
    stepperRight.run();
  }
}

void pravy_dolava(int steps) {
  stepperRight.move(-steps);
  while (stepperRight.distanceToGo() != 0) {
    stepperRight.run();
  }
}