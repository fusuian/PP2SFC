#ifndef _PRECISIONPRO_
#define def _PRECISIONPRO_

#include <SPI.h>

// SCK  D13 // Game port 2 // pull up
//#define PIN_TRIGGER 3 // Game port 3
// MISO D12 // NC
// MOSI D11 // Game port 7 // pull up
// SS   D10 // PIN_CLEAR

//#define PIN_MONITOR  5 

// data structure as sent from my sidewinder device, 48 bits total
// this datatype is accessable via two ways: an integer-array and a struct
// the former is used to used to manipulate the data at a bit-level
// while the latter is used to access individual pieces of data
typedef union
{
  // the struct consisting of the various data-fields sent by the sidewinder device
  struct
  {
    unsigned int btn_fire:1;      // bit  1

    unsigned int btn_top:1;       // bit  2
    unsigned int btn_top_up:1;    // bit  3
    unsigned int btn_top_down:1;  // bit  4

    unsigned int btn_a:1;         // bit  5
    unsigned int btn_b:1;         // bit  6
    unsigned int btn_c:1;         // bit  7
    unsigned int btn_d:1;         // bit  8
    
    unsigned int btn_shift:1;     // bit  9

    unsigned int x:10;            // bits 10-19
    unsigned int y:10;            // bits 20-29
    unsigned int m:7;             // bits 30-36
    unsigned int r:6;             // bits 37-42

    unsigned int head:4;          // bits 43-46

    unsigned int reserved:1;      // bit 47
    unsigned int parity:1;        // bit 48

    unsigned short blank;
  };

  // the ints are used to access the struct-data at bit-level
  unsigned char buf[6];
} sw_data_t;


volatile sw_data_t sw_data;
volatile byte pos;


// SPI割り込み
ISR (SPI_STC_vect)
{
//  PORTD |= _BV(PIN_MONITOR);
  sw_data.buf[pos++] = SPDR;
//  PORTD &= ~_BV(PIN_MONITOR);
}


class PrecisionPro
{
  private:
    int MOSI;
    int SCK;
    int SS;
    int PIN_TRIGGER;
    int PIN_CLEAR;

  public:
  
  PrecisionPro(int mosi, int sck, int ss, int pin_trigger, int pin_clear)
  {
    MOSI = mosi;
    SCK = sck;
    SS = ss;
    PIN_CLEAR = pin_clear;
    PIN_TRIGGER = pin_trigger;
    
    pinMode(PIN_TRIGGER, OUTPUT);
    //pinMode(PIN_MONITOR, OUTPUT);
    pinMode(PIN_CLEAR,   OUTPUT);
    digitalWrite(PIN_CLEAR, LOW);
    
    // SPI スレーブモード設定
    pinMode(MOSI, INPUT);
    pinMode(SCK, INPUT);
    pinMode(SS,  INPUT);

    SPCR |= _BV(SPE); // set slave mode
    SPI.setBitOrder(LSBFIRST); 
    SPI.attachInterrupt();
  }


  void update()
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
  }

};

#endif
