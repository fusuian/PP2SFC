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

const int sfc_a_pin = 4; // A4 
const int sfc_b_pin = a_pin; // A5 
const int sfc_x_pin = 5; // A4 
const int sfc_y_pin = b_pin; // A5 
const int sfc_l_pin = 6; // A6 
const int sfc_r_pin = 7; // A7

#if 0
const int fire_pin = b_pin;
const int top_pin = a_pin;
const int top_up_pin = select_pin;
const int top_down_pin = start_pin;
#else
const int fire_pin = sfc_y_pin;
const int top_pin = sfc_a_pin;
const int top_up_pin = sfc_x_pin;
const int top_down_pin = sfc_b_pin;
#endif


void setup (void)
{
  Serial.begin (115200);   // debugging

  pinMode(up_pin, OUTPUT);
  pinMode(down_pin, OUTPUT);
  pinMode(left_pin, OUTPUT);
  pinMode(right_pin, OUTPUT);

  for (int i = A0; i <= A5; i++) { 
    pinMode(i, OUTPUT);
  }

  portDOn(up_pin);
  portDOn(down_pin);
  portDOn(left_pin);
  portDOn(right_pin);

  for (int i = 0; i < 6; i++) {
    portCOn(i);
  }

  pp = new PrecisionPro(MOSI, SCK, SS, PIN_TRIGGER, PIN_CLEAR);
}

int p_down, p_up;
int rapid_counter = 1;
const int rapid_interval = 6;

int double_counter = -1;
const int double_interval = 15;

int cnt=0;

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
//  Serial.print(cnt++);
//  Serial.print("; m: ");
//  Serial.print(pp->m());
//  Serial.print("; r:");
//  Serial.print(pp->r());
//  Serial.print("; head:");
//  Serial.println(pp->head());
  
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
    y = 0;
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
    x = 0;
  }

  if (x == 0 && y == 0) {
    switch (pp->head()) {
    case 0:
        break;
    case 1:
        analogWrite(up_pin, 0);
        break;
    case 2:
        analogWrite(up_pin, 0);
        analogWrite(right_pin, 0);
        break;
    case 3:
        analogWrite(right_pin, 0);
        break;
    case 4:
        analogWrite(down_pin, 0);
        analogWrite(right_pin, 0);
        break;
    case 5:
        analogWrite(down_pin, 0);
        break;
    case 6:
        analogWrite(down_pin, 0);
        analogWrite(left_pin, 0);
        break;
    case 7:
        analogWrite(left_pin, 0);
        break;
    case 8:
        analogWrite(up_pin, 0);
        analogWrite(left_pin, 0);
        break;

    default:
        break;
    }
  }


  if (pp->fire()) {
    //Serial.println("B");
    if (--rapid_counter == 0) {
      portCOff(fire_pin);
      rapid_counter = rapid_interval;
    } else {
      portCOn(fire_pin);
    }
  } else if (pp->b()) {
    portCOff(fire_pin);
    rapid_counter = 1;
  } else {
    portCOn(fire_pin);
    rapid_counter = 1;
  }
  
  if (pp->top() || pp->a()) {
    //Serial.println("A");
    portCOff(top_pin);
  } else {
    portCOn(top_pin);
  }
  
  if (pp->top_up() || pp->c()) {
    portCOff(top_up_pin);
  } else {
    portCOn(top_up_pin);
  }
   
  if (pp->top_down() || pp->d()) {
    portCOff(top_down_pin);
  } else {
    portCOn(top_down_pin);
  }

  if (double_counter >= 0) {
    double_counter--;
  }
  // shiftキー一瞬押しでSTART, 長押しでSELECT
  if (pp->shift()) {
    if (double_counter < 0) {
      double_counter = double_interval;
    } else if (double_counter == 0) {
      Serial.println("SELECT");
      portCOff(select_pin);
    }
  } else {
    if (double_counter > 0) {
      Serial.println("START");
      portCOff(start_pin);
      double_counter = -1;
    } else {
      portCOn(select_pin);
      portCOn(start_pin);
      double_counter = -1;
    }
  }
  
  delay(15);
}
