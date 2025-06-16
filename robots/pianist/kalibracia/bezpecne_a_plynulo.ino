#include <Servo.h>

struct ServoState {
  int currentAngle;
  float x;
  bool moving; 
  unsigned long lastUpdate;
};

Servo servoPravaRuka; 
ServoState statePravaRuka = {0, 0, false, 0};
Servo servoLavaRuka;
ServoState stateLavaRuka = {0, 0, false, 0};

const int MAX_ANGLE = 360; 

void setup() {
  servoPravaRuka.attach(1);
  servoLavaRuka.attach(2);
  servoPravaRuka.write(statePravaRuka.currentAngle); 
  servoLavaRuka.write(stateLavaRuka.currentAngle); 
}
// pojde to? 
void moveToAngle(int targetAngle, Servo& currentServo, ServoState& state) {
  const float maxX = PI; 
  float interval; 

  
  if (targetAngle < 0 || targetAngle > MAX_ANGLE) {
    return; 
  }

  if (!state.moving) {
    state.x = 0; 
    state.moving = true; 
    state.lastUpdate = millis(); 
  }

  while (state.x < maxX && state.moving) {
    unsigned long currentTime = millis();
    interval = (1 - sin(state.x)) * 15 + 5; //mozes pridavat aj tu, ale tu zasa cim vecsie cislo tym pomalsie pojde

    if (currentTime - state.lastUpdate >= interval) {
      float y = (cos(state.x + PI) + 1) / 2; 
      int newAngle = state.currentAngle + y * (targetAngle - state.currentAngle); 
      currentServo.write(newAngle); 

      state.x += 0.1; //ked chces, aby islo rychlejsie, tomu pridaj hodnoty
      state.lastUpdate = currentTime; 


      if (state.x >= maxX) {
        currentServo.write(targetAngle); 
        state.currentAngle = targetAngle;
        state.moving = false; 
      }
    }
  }
}

void loop() {
  moveToAngle(90, servoPravaRuka, statePravaRuka);
  while (statePravaRuka.moving) {} 
  moveToAngle(45, servoLavaRuka, stateLavaRuka); 
  while (stateLavaRuka.moving) {} 
}