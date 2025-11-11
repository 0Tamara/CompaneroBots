#include <stdio.h>
#include <stdlib.h>

int array[] =
{
  0b00001000,//stv pomcka
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,//osm pomlcka
  0b00000000,
  0b00000000,//sest 
  0b00000000,//sest
  0b00000000,//stv
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,//stv
  0b00000000,
  0b00000000,
  0b00000000,
};
void main()
{
  for(int i=0; i<16; i++)
  {
    printf("%X,", array[i]);
  }
}