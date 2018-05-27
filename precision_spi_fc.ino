/**
 * Sidewinder Precision Pro Adapter for Star Fox
 * 
 * MS製フライトスティック Sidewinder Precision Pro をスーパーファミコンに
 * 接続して、スターフォックスを遊ぶアダプタ。
 * 
 * コントロールモードAに対応
 * ローリング（LまたはRの連打による回転）は未対応
 * 
 */

#include "PrecisionPro.h"

#define PIN_TRIGGER 2 // Precision Proのトリガーとつなぐデジタルピン
#define PIN_CLEAR   7 // Precision Proとの通信開始（レジスタクリア）のデジタルピン

#define portBOn(p)      (PORTB |= _BV(p))
#define portBOff(p)     (PORTB &= ~_BV(p))
#define portCOn(p)      (PORTC |= _BV(p))
#define portCOff(p)     (PORTC &= ~_BV(p))
#define portDOn(p)      (PORTD |= _BV(p))
#define portDOff(p)     (PORTD &= ~_BV(p))

#define isPortB(p)      ((PORTB & _BV(p)) != 0)

bool mode_rapid_fire = false; // fireキー連射モード
bool mode_reverse = false;    // 上下反転モード
bool mode_super = true;       // スーパーファミコンモード



PrecisionPro * pp;

// SPI割り込み
ISR (SPI_STC_vect)
{
  pp->add_buf(SPDR);
}


const int threshold = 32;

// キー/ボタンに対応するデジタルピン

// 上下左右はPWM対応のピンであること（疑似アナログジョイスティック）
const int up_pin = 6;
const int down_pin = 5;
const int right_pin = 4;
const int left_pin = 9;

// ボタンは基本アナログピン(ポートC）を使用
const int a_pin = 0; // A0
const int b_pin = 1; // A1
const int select_pin = 2; // A2
const int start_pin = 3; // A3

const int sfc_a_pin = 4; // A4 
const int sfc_b_pin = a_pin;
const int sfc_x_pin = 5; // A5 
const int sfc_y_pin = b_pin;  

const int sfc_l_pin = 3; 
const int sfc_r_pin = 0; // ここだけD8(ポートB)ピン


// ファミコンの場合のファイア・トップボタンの割り振り
// トップアップ/ダウンキーにセレクト・スタートを割り振る
int fire_pin = b_pin;
int top_pin = a_pin;
int top_up_pin = select_pin;
int top_down_pin = start_pin;


void setup (void)
{
  Serial.begin (115200);   // debugging

  pinMode(up_pin, OUTPUT);
  pinMode(down_pin, OUTPUT);
  pinMode(left_pin, OUTPUT);
  pinMode(right_pin, OUTPUT);

  pinMode(sfc_l_pin, OUTPUT);
  pinMode(8, OUTPUT);

  for (int i = A0; i <= A5; i++) { 
    pinMode(i, OUTPUT);
  }

  portDOn(up_pin);
  portDOn(down_pin);
  portDOn(left_pin);
  portDOn(right_pin);

  portDOn(sfc_l_pin);
  portBOn(sfc_r_pin);

  for (int i = 0; i < 6; i++) {
    portCOn(i);
  }

  pp = new PrecisionPro(MOSI, SCK, SS, PIN_TRIGGER, PIN_CLEAR);
//  Serial.println("Waiting...");
//  for (int i = 0; i < 3; i++){
//    digitalWrite(LED_BUILTIN, HIGH);
//    delay(500);
//    digitalWrite(LED_BUILTIN, LOW);
//    delay(500);
//  }
//  Serial.println("Update");
//  pp->update();
//  delayMicroseconds(1000); // ここで待機する間に割り込みでSPI受信
//  volatile sw_data_t & sw_data = pp->data();
//  Serial.println(pp->b());
//
//  mode_rapid_fire = pp->b();

  if (mode_super) {
    // スーファミの場合のファイア・トップボタンの割り振り
    fire_pin = sfc_y_pin;
    top_pin = sfc_a_pin;
    top_up_pin = sfc_x_pin;
    top_down_pin = sfc_b_pin;
  }
}

int p_down, p_up;
int rapid_counter = 1;
const int rapid_interval = 6;

int double_counter = -1;
const int double_interval = 60;

int cnt=0;

void loop (void)
{
  pp->update();
  delayMicroseconds(1000); // ここで待機する間に割り込みでSPI受信

  volatile sw_data_t & sw_data = pp->data();

  int x = pp->x();
  int y = pp->y();

  if (mode_reverse) {
    p_up = down_pin;
    p_down = up_pin;  
  } else {
    p_up = up_pin;
    p_down = down_pin;    
  }

//  Serial.print(x);
//  Serial.print(", ");
//  Serial.println(y);
//  Serial.print(cnt++);
//  Serial.print("; m: ");
//  Serial.print(pp->m());
//  Serial.print("; r:");
//  Serial.print(pp->r());
//  Serial.print("; head:");
//  Serial.print(pp->head());
//  Serial.println();
  
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

  // HATスイッチ
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

  // ファイアボタン
  if (mode_rapid_fire) {
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
  } else {
    if (pp->fire() || pp->b()) {
      portCOff(fire_pin);
    } else {
      portCOn(fire_pin);
    }
  }

  // トップボタン
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

  // スロットル→X,Bボタン
  if (pp->m() < 16) {
    portCOff(sfc_b_pin);    
  } else {
    portCOn(sfc_b_pin);    
  }
  
  if (pp->m() > 104) {
    portCOff(sfc_x_pin);    
  } else {
    portCOn(sfc_x_pin);    
  }

  // ツイスト→LR
  if (pp->r() <= -16) {
    portDOff(sfc_l_pin);
  } else {
    portDOn(sfc_l_pin);
  }

  if (pp->r() >= 15) {
    portBOff(sfc_r_pin);
  } else {
    portBOn(sfc_r_pin);
  }
  
  delay(15);
}

