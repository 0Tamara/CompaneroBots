#include <ESP32Servo.h>

Servo r_arm, r_elbow, l_arm, l_elbow;

 //motors
#define RR_EN 18  // Right rear enable pin
const int RR_DIR[] = {17, 5}; // Right rear direction pins
#define LR_EN 2  // Left rear enable pin
const int LR_DIR[] = {16, 4}; // Left rear direction pins
#define RF_EN 14  // Right front enable pin
const int RF_DIR[] = {26, 27}; // Right front direction pins
#define LF_EN 32   // Left front enable pin
const int LF_DIR[] = {33, 25}; // Left front direction pins

void setup() {
  // put your setup code here, to run once:
 
  r_arm.attach(13);
  r_elbow.attach(14);
  l_arm.attach(12);
  l_elbow.attach(27);
  Serial.begin(115200);

  for(int i=0; i<2; i++) {
    pinMode(RR_DIR[i], OUTPUT);
    pinMode(LR_DIR[i], OUTPUT);
    pinMode(RF_DIR[i], OUTPUT);
    pinMode(LF_DIR[i], OUTPUT);
  }
  ledcAttachChannel(RR_EN, 1000, 8, 1);
  ledcAttachChannel(LR_EN, 1000, 8, 1);
  ledcAttachChannel(RF_EN, 1000, 8, 1);
  ledcAttachChannel(LF_EN, 1000, 8, 1);

}

void loop() {
  // put your main code here, to run repeatedly:
  waving();
}

void arms_down(){
  r_arm.write(180);
  r_elbow.write(70);
  l_arm.write(0);
  l_elbow.write(0);
}

void arms_horizontally(){
  r_arm.write(90);
  r_elbow.write(70);
  l_arm.write(90);
  l_elbow.write(0);
}

void arms_up(){
  r_arm.write(10);
  r_elbow.write(70);
  l_arm.write(150);
  l_elbow.write(0);
}

void wave(){
  r_arm.write(70);
  r_elbow.write(20);
  l_arm.write(90);
  l_elbow.write(0);
}

void wave2(){
  r_arm.write(90);
  r_elbow.write(70);
  l_arm.write(110);
  l_elbow.write(50);
}

void askew(){
  r_arm.write(160);
  r_elbow.write(70);
  l_arm.write(150);
  l_elbow.write(0);
}

void askew2(){
  r_arm.write(30);
  r_elbow.write(70);
  l_arm.write(20);
  l_elbow.write(0);
}

void right_hip(){
  r_arm.write(140);
  r_elbow.write(120);
  l_arm.write(150);
  l_elbow.write(0);
}

void left_hip(){
  r_arm.write(10);
  r_elbow.write(70);
  l_arm.write(40);
  l_elbow.write(50);
}

void head(){
  r_arm.write(60);
  r_elbow.write(20);
  l_arm.write(120);
  l_elbow.write(0);
}

void waving(){
  r_arm.write(50);
  r_elbow.write(70);
  l_arm.write(0);
  l_elbow.write(0);
  delay(800);
  r_elbow.write(30);
  delay(800);
  r_elbow.write(110);
}