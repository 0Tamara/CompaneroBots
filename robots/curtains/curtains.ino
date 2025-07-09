#define MOTOR_1 12
#define MOTOR_2 13

void setup()
{
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  
  pinMode(MOTOR_1, OUTPUT);
  digitalWrite(MOTOR_1, LOW);
  pinMode(MOTOR_2, OUTPUT);
  digitalWrite(MOTOR_2, LOW);
}

void loop()
{
  digitalWrite(MOTOR_1, HIGH);
  digitalWrite(MOTOR_2, LOW);
  delay(4000);
  digitalWrite(MOTOR_1, LOW);
  digitalWrite(MOTOR_2, LOW);
  delay(1000);
  digitalWrite(MOTOR_1, LOW);
  digitalWrite(MOTOR_2, HIGH);
  delay(4000);
  digitalWrite(MOTOR_1, LOW);
  digitalWrite(MOTOR_2, LOW);
  delay(1000);
}
