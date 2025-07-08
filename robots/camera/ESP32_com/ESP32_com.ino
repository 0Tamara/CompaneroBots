#define RXD2 16
#define TXD2 17
#define CAM_BAUD 115200

//messages = right up, right down, left up, left down

int recvData;

// Create an instance of the HardwareSerial class for Serial 2
HardwareSerial camSerial(2);

void setup(){
  // Serial Monitor
  Serial.begin(115200);
  
  // Start Serial 2 with the defined RX and TX pins and a baud rate of 9600
  camSerial.begin(CAM_BAUD, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial 2 started at 115200 baud rate");
  pinMode(2, OUTPUT);
}

void loop(){
  if(camSerial.available())
  {
    while (camSerial.available()){
      digitalWrite(2, HIGH);
      // get the byte data from the GPS
      recvData = camSerial.read();
      Serial.print(recvData);
      Serial.print(" ");
    }
  Serial.println();
  }
  delay(100);
  digitalWrite(2, LOW);
}