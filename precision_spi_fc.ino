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
#include "portmacro.h"

#define PIN_TRIGGER 2 // Precision Proのトリガーとつなぐデジタルピン
#define PIN_CLEAR   7 // Precision Proとの通信開始（レジスタクリア）のデジタルピン

bool mode_rapid_fire; // fireキー連射モード
bool mode_reverse;    // 上下反転モード
bool mode_super;      // スーパーファミコンモード(トップアップ/ダウンキーがXYボタンになり、スティックのひねりがLRボタンになる)



PrecisionPro * pp;

// SPI割り込み
ISR (SPI_STC_vect)
{
  pp->add_buf(SPDR);
}


const int threshold = 100;

// キー/ボタンに対応するデジタルピン

// 上下左右はPWM対応のピンであること（疑似アナログジョイスティック）
const int up_pin = 6;
const int down_pin = 5;
const int right_pin = 9;
const int left_pin = 3;

const int a_pin = 8;
const int sfc_a_pin = 4;
const int sfc_b_pin = a_pin;
const int sfc_y_pin = b_pin;



const int sfc_x_pin = A0;
const int sfc_l_pin = A1;
const int sfc_r_pin = A2;
const int b_pin = A3;
const int select_pin = A4;
const int start_pin =  A5;


int fire_pin;
int top_pin;
int top_up_pin;
int top_down_pin;

int p_down, p_up;             // mode_reverseのために、アップダウンのピン番号を保持
int rapid_counter = 1;        // 連射間隔を制御するカウンタ
const int rapid_interval = 6; // 連射間隔

unsigned int cnt = 0;
unsigned int cnt_shift;

// ファミコン/スーファミによって、操縦桿の頭のファイア/トップ/
// トップアップ/トップダウンボタンの割り振りを変更する
void set_super(bool state)
{
  mode_super = state;
  if (state) {
    // スーファミの場合 Y/A/X/B
    // セレクト/スタートは、シフトキー + トップアップ/ダウン
    fire_pin = sfc_y_pin;
    top_pin = sfc_a_pin;
    top_up_pin = sfc_x_pin;
    top_down_pin = sfc_b_pin;
  } else {
    // ファミコンの場合 B/A/Select/Start
    fire_pin = b_pin;
    top_pin = a_pin;
    top_up_pin = select_pin;
    top_down_pin = start_pin;
  }
}

// 操縦桿の上下反転モードを設定する
void set_reverse(bool state)
{
  mode_reverse = state;
  // 上下反転
  if (state) {
    p_up = down_pin;
    p_down = up_pin;
  } else {
    p_up = up_pin;
    p_down = down_pin;
  }
}

void setup (void)
{
  Serial.begin(57600);   // debugging

  pinMode(up_pin, OUTPUT);
  pinMode(down_pin, OUTPUT);
  pinMode(left_pin, OUTPUT);
  pinMode(right_pin, OUTPUT);

  pinMode(select_pin, OUTPUT);
  pinMode(start_pin, OUTPUT);

  pinMode(sfc_a_pin, OUTPUT);
  pinMode(sfc_b_pin, OUTPUT);
  pinMode(sfc_x_pin, OUTPUT);
  pinMode(sfc_y_pin, OUTPUT);
  pinMode(sfc_l_pin, OUTPUT);
  pinMode(sfc_r_pin, OUTPUT);

  portOn(up_pin);
  portOn(down_pin);
  portOn(left_pin);
  portOn(right_pin);
  portOn(select_pin);
  portOn(start_pin);

  portOn(sfc_a_pin);
  portOn(sfc_b_pin);
  portOn(sfc_x_pin);
  portOn(sfc_y_pin);
  portOn(sfc_l_pin);
  portOn(sfc_r_pin);

  pp = new PrecisionPro(MOSI, SCK, SS, PIN_TRIGGER, PIN_CLEAR);
  set_super(false);
  set_reverse(true);
  mode_rapid_fire = false;
}

void print_status()
{
  int x = pp->x();
  int y = pp->y();
  char * buf = pp->data().buf;

  Serial.print(++cnt);
  Serial.print(": ");
  for (int i=0; i < 6; i++){
      unsigned char b = buf[i];
      if (b < 16) {
        Serial.print("0");
      }
      Serial.print(b, HEX);
      Serial.print(":");
  }

  Serial.print(" (");
  Serial.print(x);
  Serial.print(", ");
  Serial.print(y);
  Serial.print(")");

  Serial.print("; m: ");
  Serial.print(pp->m());
  Serial.print("; r:");
  Serial.print(pp->r());
  Serial.print("; head:");
  Serial.print(pp->head());
  Serial.println();
}


char pf, pt;

void shift()
{
  if (mode_super) {
    // top_up/top_down → SELECT, START
    if (pp->top_up()) {
      portOff(select_pin);
    } else {
      portOn(select_pin);
    }

    if (pp->top_down()) {
      portOff(start_pin);
    } else {
      portOn(start_pin);
    }
  }

  // ファイアボタンで連射のオン・オフを切り替え
  int f = pp->fire();
  if (f && pf == 0) {
    mode_rapid_fire = !mode_rapid_fire;
  }
  pf = f;

  // トップボタンでスーファミモードを切り替え
  int t = pp->top();
  if (t && pt == 0) {
    set_super(!mode_super);
  }
  pt = t;

  // HATスイッチの上下で操縦桿の上下を反転
  switch (pp->head()) {
  case 1:
    set_reverse(false);
    break;
  case 5:
    set_reverse(true);
    break;
  default:
    break;
  }
}


void arrow_key()
{
  int y = pp->y() / 2;
  if (y > threshold) {
    analogWrite(p_down, 255-y);
  } else {
    analogWrite(p_down, 255);
  }

  if (y < -threshold) {
    analogWrite(p_up, 256+y);
  } else {
    analogWrite(p_up, 255);
    y = 0;
  }

  int x = pp->x() / 2;
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
}


void buttons()
{
  // ファイアボタン
  if (mode_rapid_fire) {
    if (pp->fire()) {
      // 連射
      if (--rapid_counter == 0) {
        portOff(fire_pin);
        rapid_counter = rapid_interval;
      } else {
        portOn(fire_pin);
      }
    } else if (pp->b()) {
      portOff(fire_pin);
      rapid_counter = 1;
    } else {
      portOn(fire_pin);
      rapid_counter = 1;
    }
  } else {
    // 連射なし
    if (pp->fire() || pp->b()) {
      portOff(fire_pin);
    } else {
      portOn(fire_pin);
    }
  }

  // トップボタン
  if (pp->top() || pp->a()) {
    portOff(top_pin);
  } else {
    portOn(top_pin);
  }

  bool non_top_up = true;
  bool non_top_down = true;

  // トップ上ボタン
  if (pp->top_up() || pp->c()) {
    portOff(top_up_pin);
    non_top_up = false;
  } else {
    portOn(top_up_pin);
  }

  // トップ下ボタン
  if (pp->top_down() || pp->d()) {
    portOff(top_down_pin);
    non_top_down = false;
  } else {
    portOn(top_down_pin);
  }

  if (mode_super) {
  // スロットル→X,Bボタン（for STARFOX）
    if (non_top_down) {
      if (pp->m() < 16) {
        portOff(sfc_b_pin);
      } else {
        portOn(sfc_b_pin);
      }
    }

    if (non_top_up) {
      if (pp->m() > 104) {
        portOff(sfc_x_pin);
      } else {
        portOn(sfc_x_pin);
      }
    }

    // ツイスト→LR
    if (pp->r() <= -32) {
      portOff(sfc_l_pin);
    } else {
      portOn(sfc_l_pin);
    }

    if (pp->r() >= 20) {
      portOff(sfc_r_pin);
    } else {
      portOn(sfc_r_pin);
    }

  }
}


void loop (void)
{
  pp->update();
  delayMicroseconds(1000); // ここで待機する間に割り込みでSPI受信

  print_status();

  if (pp->shift()) {
    cnt_shift++;
    shift();
  } else {
    cnt_shift = 0;
    arrow_key();
    buttons();
  }
  delay(15);
}

