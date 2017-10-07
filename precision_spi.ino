#include <SPI.h>

// SCK  D13 // Game port 2 // pull up
#define PIN_TRIGGER 3 // Game port 3
// MISO D12 // NC
// MOSI D11 // Game port 7 // pull up
// SS   D10 // PIN_CLEAR

#define PIN_CLEAR    6
#define PIN_MONITOR  5 

volatile unsigned char buf [6];
volatile byte pos;

void setup (void)
{
  Serial.begin (115200);   // debugging

  pinMode(PIN_TRIGGER, OUTPUT);
  pinMode(PIN_MONITOR, OUTPUT);
  pinMode(PIN_CLEAR,   OUTPUT);
  digitalWrite(PIN_CLEAR, LOW);
  
  // SPI スレーブモード設定
  pinMode(MOSI, INPUT);
  pinMode(SCK, INPUT);
  pinMode(SS,  INPUT);
  SPCR |= _BV(SPE);
  SPI.attachInterrupt();
}


// SPI割り込み
ISR (SPI_STC_vect)
{
  PORTD |= _BV(PIN_MONITOR);
  buf[pos++] = SPDR;
  PORTD &= ~_BV(PIN_MONITOR);
}



void loop (void)
{
  PORTD &= ~_BV(PIN_TRIGGER);
  delayMicroseconds(500);
  // SSをHIGHにしてSPIのシフトレジスタをクリア
  PORTD |=  _BV(PIN_CLEAR);
  delayMicroseconds(100);
  PORTD &= ~_BV(PIN_CLEAR);
  
  delayMicroseconds(400);
  
  pos = 0;
  PORTD |= _BV(PIN_TRIGGER);
  delayMicroseconds(1000); // ここで待機する間に割り込みでSPI受信

  for (int i=0; i < 6; i++){  
      Serial.print(buf[i], HEX);
      Serial.print(":");
  }
  Serial.println("");

  delay(50);
}
