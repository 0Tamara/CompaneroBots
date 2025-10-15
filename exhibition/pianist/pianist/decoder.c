#include <stdio.h>
#include <stdlib.h>

int array[] =
{
    0b00010000,
    0b00000000,
    0b00010000,
    0b00000000,
    0b00001000,
    0b00100000,
    0b01010000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b01000000, //e
    0b00010000,
};
void main()
{
  for(int i=0; i<12; i++)
  {
    printf("%X,", array[i]);
  }
}