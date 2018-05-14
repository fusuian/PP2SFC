#include "PrecisionPro.h"

#define PIN_REVERSE 8

#define PIN_TRIGGER 2
#define PIN_CLEAR   7

#define portCOn(p)      (PORTC |= _BV(p))
#define portCOff(p)     (PORTC &= ~_BV(p))
#define portDOn(p)      (PORTD |= _BV(p))
#define portDOff(p)     (PORTD &= ~_BV(p))

#define isPortB(p)      ((PORTB & _BV(p)) != 0)

PrecisionPro * pp;

// SPI割り込み
ISR (SPI_STC_vect)
{
  pp->add_buf(SPDR);
}


const int threshold = 200;
const int up_pin = 6;
const int down_pin = 5;
const int right_pin = 3;
const int left_pin = 4;

const int a_pin = 0; // A0
const int b_pin = 1; // A1
const int select_pin = 2; // A2
const int start_pin = 3; // A3

void setup (void)
{
  Serial.begin (115200);   // debugging

  pinMode(up_pin, OUTPUT);
  pinMode(down_pin, OUTPUT);
  pinMode(left_pin, OUTPUT);
  pinMode(right_pin, OUTPUT);

  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);

  portDOn(up_pin);
  portDOn(down_pin);
  portDOn(left_pin);
  portDOn(right_pin);

  portCOn(a_pin);
  portCOn(b_pin);
  portCOn(select_pin);
  portCOn(start_pin);


  pp = new PrecisionPro(MOSI, SCK, SS, PIN_TRIGGER, PIN_CLEAR);
}

int p_down, p_up;

void loop (void)
{
  pp->update();
  delayMicroseconds(1000); // ここで待機する間に割り込みでSPI受信

  volatile sw_data_t & sw_data = pp->data();

  int x = pp->x();
  int y = pp->y();

//  if (isPortB(PIN_REVERSE)) {
    p_up = down_pin;
    p_down = up_pin;  
//  } else {
//    p_up = up_pin;
//    p_down = down_pin;    
//  }
  
  if (y > threshold) {
    portDOff(p_down);
  } else {
    portDOn(p_down);
  }
  
  if (y < -threshold) {
    portDOff(p_up);
  } else {
    portDOn(p_up);
  }

  if (x > threshold) {
    portDOff(right_pin);
  } else {
    portDOn(right_pin);
  }
  if (x < -threshold) {
    portDOff(left_pin);
  } else {
    portDOn(left_pin);
  }

  if (pp->fire()) {
    //Serial.println("B");
    portCOff(b_pin);
  } else {
    portCOn(b_pin);
  }

  if (pp->top()) {
    //Serial.println("A");
    portCOff(a_pin);
  } else {
    portCOn(a_pin);
  }
  
  if (pp->top_up()) {
    //Serial.println("SELECT");
    portCOff(select_pin);
  } else {
    portCOn(select_pin);
  }
  
  if (pp->top_down()) {
    //Serial.println("START");
    portCOff(start_pin);
  } else {
    portCOn(start_pin);
  }
  
  delay(15);
}
