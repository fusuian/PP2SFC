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

int initialize_wait = 3*60;   // 初期化待機時間
int count_threshold = 90;     // 初期化待機中に特定のボタンが一定カウント押された場合、ボタンに対応するモードをセットする。

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
}

int p_down, p_up;             // mode_reverseのために、アップダウンのピン番号を保持
int rapid_counter = 1;        // 連射間隔を制御するカウンタ
const int rapid_interval = 6; // 連射間隔

int double_counter = -1;        // SELECT/START押し分けのためのカウンタ
const int double_interval = 60; // SELECT/START押し分けのタイミング

int cnt=0;

// 設定用カウンタ
int c_rapid_fire = 0;
int c_reverse = 0;
int c_super = 0;

// 初期化待機中、特定のボタンを押し続けると動作モードが切り替わる
void wait()
{
//    Serial.print("wait: ");
//    Serial.print(pp->fire());
//    Serial.print(pp->y());
//    Serial.println(pp->shift());

    // ファイアボタン→連射
    if (pp->fire()) {
      c_rapid_fire++;
    }
    // トップボタン→上下反転
    if (pp->top()) {
      c_reverse++;
    }
    // シフトボタン→スーファミモード
    if (pp->shift()) {
      c_super++;
    }
}


// 初期化待機中のコマンドに応じて設定を変更する
void initialize()
{
  Serial.println("initialize");
  mode_rapid_fire = (c_rapid_fire > count_threshold);
  mode_reverse = (c_reverse > count_threshold);
  mode_super = (c_super > count_threshold);

  if (mode_super) {
    // スーファミの場合のファイア・トップボタンの割り振り
    fire_pin = sfc_y_pin;
    top_pin = sfc_a_pin;
    top_up_pin = sfc_x_pin;
    top_down_pin = sfc_b_pin;
  }

  // 上下反転
  if (mode_reverse) {
    p_up = down_pin;
    p_down = up_pin;
  } else {
    p_up = up_pin;
    p_down = down_pin;
  }
}


void loop (void)
{
  pp->update();
  delayMicroseconds(1000); // ここで待機する間に割り込みでSPI受信

  // 初期化待機
  if (initialize_wait > 0) {
    --initialize_wait;
    wait();
    delay(16);
    return;
  } else if (initialize_wait == 0) {
    --initialize_wait;
    initialize();
    delay(16);
    return;
  }

  int x = pp->x();
  int y = pp->y();
  char * buf = pp->data().buf;

#if 0
  Serial.print(++cnt);
  for (int i=0; i < 6; i++){  
      Serial.print(":");
    Serial.print(*buf++, HEX);
  }

  Serial.print(": (");
  Serial.print(x);
  Serial.print(", ");
  Serial.print(y);
  Serial.print(")");

  Serial.print(" top_up: ");
  Serial.print(top_up_pin);

  Serial.print(" top_down: ");
  Serial.print(top_down_pin);

//  Serial.print("; m: ");
//  Serial.print(pp->m());
//  Serial.print("; r:");
//  Serial.print(pp->r());
//  Serial.print("; head:");
//  Serial.print(pp->head());
  Serial.println();
#endif

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
  // スロットル→X,Bボタン
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

    // シフト+top_up/top_down → SELECT, START
    if (pp->shift()) {
      if (pp->top_up() || pp->c()) {
        portOff(select_pin);
      } else {
        portOn(select_pin);
      }

      if (pp->top_down() || pp->d()) {
        portOff(start_pin);
      } else {
        portOn(start_pin);
      }
    }
  }

  delay(15);
}

