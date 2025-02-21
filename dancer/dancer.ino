#include <ESP32Servo.h>

Servo rd, rv, lv, ld, r, l;

void setup() {
  Serial.begin(9600);
  rd.attach(13);
  rv.attach(12);
  lv.attach(23);
  ld.attach(22);
  r.attach(15);
  l.attach(21);

  vyrovnanieDole();
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
}

void vyrovnanieHore() {
  rv.write(0);
  rd.write(70);
  lv.write(0);
  ld.write(40);
}

void vyrovnanieDole() {
  rv.write(180);
  rd.write(70);
  lv.write(110);
  ld.write(40);
}

void vyrovnanieVodorovne() {
  rv.write(90);
  rd.write(70);
  lv.write(50);
  ld.write(40);
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
  rd.write(160);
  lv.write(0);
  ld.write(60);
  delay(500);
  ld.write(10);
  delay(500);
}

void sikmo() {
  rv.write(50);
  rd.write(70);
  lv.write(80);
  ld.write(40);
}

void sikmoNegovane() {
  rv.write(150);
  rd.write(70);
  lv.write(20);
  ld.write(40);
}

void mavanie1(){
  rv.write(90);
  rd.write(70);
  lv.write(20);
  ld.write(90);
}

void mavanie2(){
  rv.write(30);
  rd.write(0);
  lv.write(45);
  ld.write(40);
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