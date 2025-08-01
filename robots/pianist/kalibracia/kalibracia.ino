
#include <FastAccelStepper.h>
#include <Adafruit_PWMServoDriver.h>
#include <Wire.h>


#define SERVOMIN  125
#define SERVOMAX  575
#define numServos 8
#define stepsPerNote 984
#define stepsPerOctave 6840
#define speedInHz 15000
#define acceleration 40000

const int leftHandStepPin = 5;
const int leftHandDirPin = 16;
const int leftHandEnPin = 18;
const int rightHandStepPin = 4;
const int rightHandDirPin = 17;
const int rightHandEnPin = 15;

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepperLeft = NULL;
FastAccelStepper *stepperRight = NULL;

Adafruit_PWMServoDriver pca9685right(0x40, Wire);
Adafruit_PWMServoDriver pca9685left(0x41, Wire);


struct Hand {
  int currentOctave;
  int currentNote;
  Adafruit_PWMServoDriver* pca9685; 
  unsigned long timeFromMoving;
  unsigned long lastTime;
};

Hand leftHand = {
  .currentOctave = 0,
  .currentNote = 0,
  .pca9685 = &pca9685left,
  .timeFromMoving = 0,
  .lastTime = 0,
};
Hand rightHand = {
  .currentOctave = 0,
  .currentNote = 0,
  .pca9685 = &pca9685right,
  .timeFromMoving = 0,
  .lastTime = 0,
};

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  
  pca9685right.begin();
  pca9685left.begin();

  pca9685right.setPWMFreq(50);
  pca9685left.setPWMFreq(50);

  for (int i = 8; i < numServos+8; i++) {
    rightHand.pca9685->setPWM(i, 0, SERVOMAX);
    leftHand.pca9685->setPWM(i, 0, SERVOMIN);
  }
  engine.init();
  stepperLeft = engine.stepperConnectToPin(leftHandStepPin);

  stepperLeft->setDirectionPin(leftHandDirPin);
  stepperLeft->setEnablePin(leftHandEnPin);
  stepperLeft->setAutoEnable(true);

  stepperLeft->setSpeedInHz(speedInHz);
  stepperLeft->setAcceleration(acceleration);
  stepperLeft->setCurrentPosition(0);
  
  stepperRight = engine.stepperConnectToPin(rightHandStepPin);

  stepperRight->setDirectionPin(rightHandDirPin);
  stepperRight->setEnablePin(rightHandEnPin);
  stepperRight->setAutoEnable(true);

  stepperRight->setSpeedInHz(speedInHz);
  stepperRight->setAcceleration(acceleration);
  stepperRight->setCurrentPosition(0);

  delay(3000);

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
  pravy_dolava(stepsPerOctave);
  Serial.printf("Čas pohybu: %lu ms\n", millis() - start);
  delay(1000);
  start = millis();
  pravy_doprava(stepsPerOctave);
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
  pravy_dolava(stepsPerNote);
  Serial.printf("Čas pohybu: %lu ms\n", millis() - start);
  delay(1000);
  start = millis();
  pravy_doprava(stepsPerNote);
  Serial.printf("Čas pohybu: %lu ms\n", millis() - start);
  delay(1000);

  // Test serv
  Serial.println("a teraz serva");
  for (int i = 8; i < numServos+8; i++) {
    Serial.printf("servo %d na lavej ruke\n", i);
    unsigned long servoStart = millis();
    leftHand.pca9685->setPWM(i, 0, SERVOMIN);
    delay(500); // Čas na stlačenie
    unsigned long pressTime = millis() - servoStart;
    Serial.printf("Čas stlačenia: %lu ms\n", pressTime);
    servoStart = millis();
    leftHand.pca9685->setPWM(i, 0, SERVOMIN + 100);
    delay(500); // Čas na pustenie
    Serial.printf("Čas pustenia: %lu ms\n", millis() - servoStart);
    leftHand.pca9685->setPWM(i, 0, SERVOMIN);
  }
  for (int i = 8; i < numServos+8; i++) {
    Serial.printf("Testujem servo %d na pravej ruke\n", i);
    unsigned long servoStart = millis();
    rightHand.pca9685->setPWM(i, 0, SERVOMAX);
    delay(500); // Čas na stlačenie
    unsigned long pressTime = millis() - servoStart;
    Serial.printf("Čas stlačenia: %lu ms\n", pressTime);
    servoStart = millis();
    rightHand.pca9685->setPWM(i, 0, SERVOMAX - 100);
    delay(500); // Čas na pustenie
    Serial.printf("Čas pustenia: %lu ms\n", millis() - servoStart);
    rightHand.pca9685->setPWM(i, 0, SERVOMAX);
  }

  for(int i=8; i<16; i++)
  {
    rightHand.pca9685->setPWM(i, 0, SERVOMAX - 100);
    leftHand.pca9685->setPWM(i, 0, SERVOMIN + 100);
  }
  delay(500);
  for(int i=8; i<16; i++)
  {
    rightHand.pca9685->setPWM(i, 0, SERVOMAX);
    leftHand.pca9685->setPWM(i, 0, SERVOMIN);
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
}

void lavy_doprava(int steps) {
  stepperLeft->move(steps);
  while (stepperLeft->isRunning());
}

void lavy_dolava(int steps) {
  stepperLeft->move(-steps);
  while (stepperLeft->isRunning());
}

void pravy_doprava(int steps) {
  stepperRight->move(steps);
  while (stepperRight->isRunning());
}

void pravy_dolava(int steps) {
  stepperRight->move(-steps);
  while (stepperRight->isRunning());
}