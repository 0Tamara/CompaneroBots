void setup()
{
  Serial.begin(115200);
  int array[] = {
  0b10000001, //osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10000001, //osm
  0b00000000,
  0b00000000, //sest pomlcka
  0b10000001, //osm
  0b00000000,
  0b10000001, //stv
  0b00000000, 
  0b00000000, 
  0b00000000, 
  0b10000001, //stv
  0b00000000, 
  0b00000000, 
  0b00000000,
  };

  for(int i=0; i<16; i++)
  {
    Serial.printf("%X,", array[i]);
  }
}

void loop()
{

}