#include <ESP32Servo.h>

Servo rd, rv, lv, ld, r, l;

 //motors
#define RR_EN 18  // Right rear enable pin
const int RR_DIR[] = {17, 5}; // Right rear direction pins
#define LR_EN 2  // Left rear enable pin
const int LR_DIR[] = {16, 4}; // Left rear direction pins
#define RF_EN 14  // Right front enable pin
const int RF_DIR[] = {26, 27}; // Right front direction pins
#define LF_EN 32   // Left front enable pin
const int LF_DIR[] = {33, 25}; // Left front direction pins

unsigned long timer_reset;

//koncatiny

//---main code---
void setup()
{

  rd.attach(13);
  rv.attach(12);
  lv.attach(23);
  ld.attach(22);
  r.attach(15);
  l.attach(21);
  Serial.begin(115200);
  //---pins---
  for(int i=0; i<2; i++)
  {
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
      //up();
      //down();
      //horizontal();
      //wave();
      //waving();
      //waving2();
      //askew();
      //askew2();
      //divaPose();
      //walking();
      //forward();
      //backward();
      //right();
      //left();





//prichod
   /* timer_reset = millis(); 
    while ((millis() - timer_reset) < 6000) {
       forward(255);  
       walking();    
       divaPose();   
       delay(300);    
}
    stop(); 

//otocenie
    delay(500);
    left(255);
    timer_reset = millis();
    while ((millis() - timer_reset) < 6000) {
      wave();
    }
    stop();

//kyvanie
    unsigned long startTime = millis();
    while (millis() - startTime < 5000) { 
      waving();
      delay(1000);
      waving2();
      delay(1000);
}

//phyb doprava + vlna
    timer_reset = millis();
    while ((millis() - timer_reset) < 5000) {
      to_right_side(255); 
      wave();          
      walking();          
      delay(300);        
}
    stop(); 

//phyb dolava + vlna
    timer_reset = millis();
    while ((millis() - timer_reset) < 5000) {
      to_left_side(255);  
      wave();            
      walking();         
      delay(300);      
}
    stop(); 

//tocenie doprava + kyvanie
    timer_reset = millis();
    while ((millis() - timer_reset) < 4000) {
      right(255);  
      waving();    
      delay(500);  
      waving2(); 
      delay(500);
}
    stop(); 

  delay(1000);

//tanec
   unsigned long timer_reset = millis();
  while (millis() - timer_reset < 10000) {  
    askew();
    delay(800);
    askew2();
    delay(800);
    askew();
    delay(800);
    up();
    delay(800);
    down();
    delay(800);
    up();
    delay(800);
    down();
    delay(800);
    delay(1000);
}
stop(); */

}




void up() {
  rv.write(0);
  rd.write(0);
  lv.write(10);
  ld.write(30);
}

void down() {
    rv.write(40);
    rd.write(180);
    lv.write(130);
    ld.write(30);
}

void horizontal() {
  rv.write(40);
  rd.write(75);
  lv.write(75);
  ld.write(30);
}

void wave() {
 rd.write(30);
 rv.write(120);
 delay(500);
 lv.write(150);
 ld.write(0);
 delay(500);
 rd.write(150);
 lv.write(30);
 delay(500);
 rv.write(0);
 ld.write(100); 
 delay(500);
}

void divaPose() {
  rv.write(0);
  rd.write(0);
  lv.write(110);
  ld.write(90);
}

void askew() {
  rv.write(50);
  rd.write(130);
  lv.write(30);
  ld.write(30);
}

void askew2() {
  rv.write(50);
  rd.write(30);
  lv.write(100);
  ld.write(40);
}

void waving(){
  rv.write(100);
  rd.write(30);
  lv.write(70);
  ld.write(30);
}

void waving2(){
  rv.write(40);
  rd.write(70);
  lv.write(30);
  ld.write(0);
}

void rlegforward(){
  r.write(20);
}

void llegforward(){
  l.write(160);
}

void rlegbackward(){
  r.write(130);
}

void llegbackward(){
  l.write(50);
}

void rlegstraight(){
  r.write(100);
}

void llegstraight(){
  l.write(85);
}

void walking(){
  r.write(20);
  l.write(50);
  delay(1000);
  l.write(160);
  r.write(130);
  delay(1000);

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



  void to_left_side(int speed) {
  digitalWrite(RF_DIR[0], LOW);
  digitalWrite(RF_DIR[1], HIGH);
  analogWrite(RF_EN, speed);

  digitalWrite(LF_DIR[0], HIGH);
  digitalWrite(LF_DIR[1], LOW);
  analogWrite(LF_EN, speed);

  digitalWrite(RR_DIR[0], HIGH);
  digitalWrite(RR_DIR[1], LOW);
  analogWrite(RR_EN, speed);

  digitalWrite(LR_DIR[0], LOW);
  digitalWrite(LR_DIR[1], HIGH);
  analogWrite(LR_EN, speed);
}

void to_right_side(int speed) {
  digitalWrite(RF_DIR[0], HIGH);
  digitalWrite(RF_DIR[1], LOW);
  analogWrite(RF_EN, speed);

  digitalWrite(LF_DIR[0], LOW);
  digitalWrite(LF_DIR[1], HIGH);
  analogWrite(LF_EN, speed);

  digitalWrite(RR_DIR[0], LOW);
  digitalWrite(RR_DIR[1], HIGH);
  analogWrite(RR_EN, speed);

  digitalWrite(LR_DIR[0], HIGH);
  digitalWrite(LR_DIR[1], LOW);
  analogWrite(LR_EN, speed);
}