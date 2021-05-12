#include <ELECHOUSE_CC1101.h>
#define size 1
int pot = A0;

byte TX_buffer[size]={0};
byte i;

void setup()
{
  pinMode(pot,INPUT);
  Serial.begin(9600);
  ELECHOUSE_cc1101.Init();
  for(i=0;i<size;i++)
  {
     TX_buffer[i]=i;
  }
}

void loop()
{
  int val = map(analogRead(pot),0,1024,0,255);
  TX_buffer[0] = val;
  Serial.println(val);
  ELECHOUSE_cc1101.SendData(TX_buffer,size);
  delay(100);
}
