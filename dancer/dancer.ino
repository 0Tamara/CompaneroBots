#include <ESP32Servo.h>

Servo rd, rv, lv, ld, r, l;

 //motors
  #define RR_EN 18  // Right rear enable pin
  const int RR_DIR[] = {17, 5}; // Right rear direction pins
  #define LR_EN 2  // Left rear enable pin
  const int LR_DIR[] = {16, 4}; // Left rear direction pins
  #define RF_EN 14   // Right front enable pin
  const int RF_DIR[] = {26, 27}; // Right front direction pins
  #define LF_EN 32   // Left front enable pin
  const int LF_DIR[] = {33, 25}; // Left front direction pins


//koncatiny

void setup() {
  Serial.begin(9600);
  rd.attach(13);
  rv.attach(12);
  lv.attach(23);
  ld.attach(22);
  r.attach(15);
  l.attach(21);

  vyrovnanieDole();

    for(int i=0; i<2; i++)
  {
    pinMode(RR_DIR[i], OUTPUT);
    pinMode(LR_DIR[i], OUTPUT);
    pinMode(RF_DIR[i], OUTPUT);
    pinMode(LF_DIR[i], OUTPUT);
  }

  ledcAttachChannel(RR_EN, 1000, 8, 0);
  ledcAttachChannel(LR_EN, 1000, 8, 0);
  ledcAttachChannel(RF_EN, 1000, 8, 0);
  ledcAttachChannel(LF_EN, 1000, 8, 0);
}

void loop() {
  //vyrovnanieHore();
  //vyrovnanieDole();
  //vyrovnanieVodorovne();
  //vlna();
  //divaPose();
  //sikmo();
  //sikmoNegovane();
  //mavanie2();
  //mavanie1();
  //pnohaVpred();
  //lnohaVpred();
  //pnohaVzad();
  //lnohaVzad();
  //pnohaRovno();
  //lnohaRovno();
  //forward(100);
    //delay(500);
    //stop();
    //delay(500);
  //backward(200);
    //delay(500);
    //stop();
    //delay(500);
    
  //digitalWrite(RR_DIR[0], LOW);
  //digitalWrite(RR_DIR[1], HIGH);
  //ledcWrite(RR_EN, 100);
}

void vyrovnanieHore() {
  rv.write(0);
  rd.write(30);
  lv.write(0);
  ld.write(40);
  delay(1000);
}

void vyrovnanieDole() {
  rv.write(180);
  rd.write(20);
  lv.write(110);
  ld.write(40);
  delay(1000);
}

void vyrovnanieVodorovne() {
  rv.write(90);
  rd.write(10);
  lv.write(45);
  ld.write(50);
  delay(1000);
}

void vlna() {
  rv.write(120);  
  lv.write(70);
  delay(300);
  ld.write(10);
  rd.write(-10);
  delay(300); 
  lv.write(50);
  rv.write(70);
  delay(200); 
  ld.write(80);
  rd.write(80);
  delay(300);  
  lv.write(35);
  rv.write(120);
  delay(300);  
  ld.write(0);
  rd.write(-10);
  delay(0);
}

void divaPose() {
  rv.write(140);
  rd.write(110);
  lv.write(0);
  ld.write(60);
  delay(500);
  ld.write(10);
  delay(500);
}

void sikmo() {
  rv.write(50);
  rd.write(30);
  lv.write(80);
  ld.write(40);
  delay(1000);
}

void sikmoNegovane() {
  rv.write(150);
  rd.write(30);
  lv.write(20);
  ld.write(40);
  delay(1000);
}

void mavanie1(){
  rv.write(90);
  rd.write(10);
  lv.write(20);
  ld.write(90);
}

void mavanie2(){
  rv.write(30);
  rd.write(0);
  lv.write(45);
  ld.write(50);
}

void pnohaVpred(){
  r.write(20);
}

void lnohaVpred(){
  l.write(160);
}

void pnohaVzad(){
  r.write(130);
}

void lnohaVzad(){
  l.write(50);
}

void pnohaRovno(){
  r.write(100);
}

void lnohaRovno(){
  l.write(85);
}


//kolesa

void forward(byte speed)
  {
    digitalWrite(RF_DIR[0], LOW);
    digitalWrite(RF_DIR[1], HIGH);
    ledcWrite(RF_EN, speed);

    digitalWrite(LF_DIR[0], LOW);
    digitalWrite(LF_DIR[1], HIGH);
    ledcWrite(LF_EN, speed);

    digitalWrite(RR_DIR[0], LOW);
    digitalWrite(RR_DIR[1], HIGH);
    ledcWrite(RR_EN, speed);

    digitalWrite(LR_DIR[0], LOW);
    digitalWrite(LR_DIR[1], HIGH);
    ledcWrite(LR_EN, speed);
  }

  void backward(byte speed)
  {
    digitalWrite(RF_DIR[0], HIGH);
    digitalWrite(RF_DIR[1], LOW);
    ledcWrite(RF_EN, speed);

    digitalWrite(LF_DIR[0], HIGH);
    digitalWrite(LF_DIR[1], LOW);
    ledcWrite(LF_EN, speed);

    digitalWrite(RR_DIR[0], HIGH);
    digitalWrite(RR_DIR[1], LOW);
    ledcWrite(RR_EN, speed);

    digitalWrite(LR_DIR[0], HIGH);
    digitalWrite(LR_DIR[1], LOW);
    ledcWrite(LR_EN, speed);
  }

  void left(byte speed)
  {
    digitalWrite(RF_DIR[0], LOW);
    digitalWrite(RF_DIR[1], HIGH);
    ledcWrite(RF_EN, speed);

    digitalWrite(LF_DIR[0], HIGH);
    digitalWrite(LF_DIR[1], LOW);
    ledcWrite(LF_EN, speed);

    digitalWrite(RR_DIR[0], LOW);
    digitalWrite(RR_DIR[1], HIGH);
    ledcWrite(RR_EN, speed);

    digitalWrite(LR_DIR[0], HIGH);
    digitalWrite(LR_DIR[1], LOW);
    ledcWrite(LR_EN, speed);
  }

  void right(byte speed)
  {
    digitalWrite(RF_DIR[0], HIGH);
    digitalWrite(RF_DIR[1], LOW);
    ledcWrite(RF_EN, speed);

    digitalWrite(LF_DIR[0], LOW);
    digitalWrite(LF_DIR[1], HIGH);
    ledcWrite(LF_EN, speed);

    digitalWrite(RR_DIR[0], HIGH);
    digitalWrite(RR_DIR[1], LOW);
    ledcWrite(RR_EN, speed);

    digitalWrite(LR_DIR[0], LOW);
    digitalWrite(LR_DIR[1], HIGH);
    ledcWrite(LR_EN, speed);
  }

  void stop()
  {
    digitalWrite(RF_DIR[0], LOW);
    digitalWrite(RF_DIR[1], LOW);
    digitalWrite(LF_DIR[0], LOW);
    digitalWrite(LF_DIR[1], LOW);
    digitalWrite(RR_DIR[0], LOW);
    digitalWrite(RR_DIR[1], LOW);
    digitalWrite(LR_DIR[0], LOW);
    digitalWrite(LR_DIR[1], LOW);
  }
