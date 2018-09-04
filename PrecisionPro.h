#ifndef _PRECISIONPRO_
#define _PRECISIONPRO_

#include <SPI.h>
#include "portmacro.h"

// SPI pins (Arduino Nano)
// SCK  D13 // -> Game port 2 // pull up
// MISO D12 // NC
// MOSI D11 // -> Game port 7 // pull up
// SS   D10 // <- clear_pin (Any GPIO pin)
// trigger_pin (Any GPIO pin) -> Game port 3

// sw_data_t from https://github.com/MaZderMind/SidewinderInterface
//
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





class PrecisionPro
{
  private:
    int mosi_pin;
    int sck_pin;
    int ss_pin;
    int trigger_pin;
    int clear_pin;

    volatile sw_data_t sw_data;
    volatile byte pos;

  public:

  PrecisionPro(int pin_trigger, int pin_clear, int mosi=MOSI, int sck=SCK, int ss=SS)
  {
    mosi_pin = mosi;
    sck_pin = sck;
    ss_pin = ss;
    clear_pin = pin_clear;
    trigger_pin = pin_trigger;
  }


  void init()
  {
    pinMode(mosi_pin, INPUT_PULLUP);
    pinMode(sck_pin, INPUT_PULLUP);
    pinMode(trigger_pin, OUTPUT);
    pinMode(clear_pin,   OUTPUT);
    digitalWrite(clear_pin, LOW);

    // SPI スレーブモード設定
    pinMode(mosi_pin, INPUT);
    pinMode(sck_pin, INPUT);
    pinMode(ss_pin,  INPUT);

    SPCR |= _BV(SPE); // set slave mode
    SPI.setBitOrder(LSBFIRST);
    SPI.attachInterrupt();
  }


  void update()
  {
    portOff(trigger_pin);
    delayMicroseconds(500);
    // clear_pinを使ってSSをHIGHにして、SPIのシフトレジスタをクリア
    // （これをしないと入力信号がずれていく）
    // （SSは入力ピンになっているので、SS自体を直接HIGH/LOWにはできない！）
    portOn(clear_pin);
    delayMicroseconds(100);
    portOff(clear_pin);

    delayMicroseconds(400);

    pos = 0;
    portOn(trigger_pin);

    // trigger_pinをHIGHにしたあと、呼び出し側で1000us待つと結果がPrecisionPro::dataに反映する
  }


  void add_buf(uint8_t spdr) {
    sw_data.buf[pos++] = spdr;
  }

  volatile sw_data_t & data() {
    return sw_data;
  }

  bool fire() {
    return sw_data.btn_fire == 0;
  }

  bool top() {
    return sw_data.btn_top == 0;
  }

  bool top_up() {
    return sw_data.btn_top_up == 0;
  }

  bool top_down() {
    return sw_data.btn_top_down == 0;
  }

  bool shift() {
    return sw_data.btn_shift == 0;
  }

  bool a() {
    return sw_data.btn_a == 0;
  }

  bool b() {
    return sw_data.btn_b == 0;
  }

  bool c() {
    return sw_data.btn_c == 0;
  }

  bool d() {
    return sw_data.btn_d == 0;
  }


  int hat_switch() {
    return sw_data.head;
  }


  int x() {
    return sw_data.x - 512;
  }

  int y() {
    return sw_data.y - 512;
  }

  int throttle() {
    return sw_data.m;
  }

  int rudder() {
    return sw_data.r - 32;
  }




};

#endif
