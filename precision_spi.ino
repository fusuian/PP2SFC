#include "PrecisionPro.h"

#define PIN_TRIGGER 3 // Game port 3
#define PIN_CLEAR   6 

PrecisionPro * pp;

void setup (void)
{
  Serial.begin (115200);   // debugging

  pp = new PrecisionPro(MOSI, SCK, SS, PIN_TRIGGER, PIN_CLEAR);
}


void loop (void)
{
  pp->update();
  delayMicroseconds(1000); // ここで待機する間に割り込みでSPI受信

  for (int i=0; i < 6; i++){  
      Serial.print(sw_data.buf[i], HEX);
      Serial.print(":");
  }
  Serial.println("");

    Serial.print("sw_data: ");
    Serial.print(sw_data.btn_fire);
    Serial.print("; ");
    Serial.print(sw_data.btn_top);
    Serial.print("; ");
    Serial.print(sw_data.btn_top_up);
    Serial.print("; ");
    Serial.print(sw_data.btn_top_down);
    Serial.print("; x=");
    Serial.print(sw_data.x - 512);
    Serial.print("; y=");
    Serial.print(sw_data.y - 512);
    Serial.print("; r=");
    Serial.print(sw_data.r - 32);
    Serial.print("; m=");
    Serial.print(sw_data.m);
    Serial.println();

  delay(50);
}
