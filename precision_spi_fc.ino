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


const int threshold = 32;
const int up_pin = 6;
const int down_pin = 5;
const int right_pin = 3;
const int left_pin = 9;

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
int rapid_counter = 1;
const int rapid_interval = 6;

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

//  Serial.print(x);
//  Serial.print(", ");
//  Serial.println(y);

  x/=2;
  y/=2;
  if (y > threshold) {
    //Serial.println(255-y);
    analogWrite(p_down, 255-y);
  } else {
    analogWrite(p_down, 255);
  }
  
  if (y < -threshold) {
//    Serial.println(256+y);
    analogWrite(p_up, 256+y);
  } else {
    analogWrite(p_up, 255);
  }

  if (x > threshold) {
    analogWrite(right_pin, 255-x);
  } else {
    analogWrite(right_pin, 255);
  }
  if (x < -threshold) {
    analogWrite(left_pin, 256+x);
  } else {
    analogWrite(left_pin, 255);
  }

  if (pp->fire()) {
    //Serial.println("B");
    if (--rapid_counter == 0) {
      portCOff(b_pin);
      rapid_counter = rapid_interval;
    } else {
      portCOn(b_pin);
    }
  } else if (pp->b()) {
    portCOff(b_pin);
    rapid_counter = 1;
  } else {
    portCOn(b_pin);
    rapid_counter = 1;
  }
  
  if (pp->top() || pp->a()) {
    //Serial.println("A");
    portCOff(a_pin);
  } else {
    portCOn(a_pin);
  }
  
  if (pp->top_up() || pp->c()) {
    //Serial.println("SELECT");
    portCOff(select_pin);
  } else {
    portCOn(select_pin);
  }
  
  if (pp->top_down() || pp->d()) {
    //Serial.println("START");
    portCOff(start_pin);
  } else {
    portCOn(start_pin);
  }
  
  delay(15);
}
