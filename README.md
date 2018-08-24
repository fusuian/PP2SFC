# Precision Pro to SFC Adapter

マイクロソフト製フライトスティック SideWinder Precision Pro を、任天堂スーパーファミコン（およびファミリーコンピュータ）に接続するアダプタです。
PWM出力を使って、アナログスティックの傾きを移動量に反映させています。


## ハードウェア

アダプタ本体となるマイコンボードと電子部品をブレッドボードに組み付けます。
ブレッドボードの配線図は、PP2SFC_BreadBoard.pdfをご覧ください。

 - ブレッドボード ハーフサイズ x 2 (または中央で絶縁されたフルサイズ x 1)
 - マイコンボード Arduino Nano
 - シフトレジスタIC TC4021BP x 2
 - カーボン抵抗 56kΩ x 16
 - ダイオード 1N4007 x 1
 - ジャンパワイヤ 適宜

 - Dサブ15ピン メスコネクタ (to Precision Pro)
 − ゲーム機接続ケーブル (以下の3種のうち、手持ちのゲーム機に合わせて)
    - Dサブ9ピン オスコネクタ (to FC compatible)
    - ファミコンコントローラ オスコネクタ
    - ニューファミコンコントローラ オスコネクタ
    - スーパーファミコンコントローラ オスコネクタ
 - USBバッテリ (アダプタ電源)



## ソフトウェア

Arduino IDEを使用して、PP2SFC スケッチをArduino Nanoに書き込みます。

 - PP2SFC.ino 	スケッチ本体
 - PrecisionPro.h	PrecisionProクラス
 - portmacro.h	 digitalWrite に相当する、portOn/portOffマクロを定義



## 使用方法

アダプタボードに Precision Proとゲーム機を接続したら、
USBバッテリをArduino Nanoに接続して、ゲーム機に電源を入れます。

起動直後はファミコン配列、スティック上下反転、連射なしのモードになります。


### ファミコン配列

電源を入れた直後はこのモードになります。
ファミコンソフト『スターラスター』（ナムコ）向けの設定です。

 - スティック 十字キー（疑似アナログ・反転モードあり）
 - ラダー（スティックひねり）	なし
 - スロットル	なし

 - Fireボタン			Bボタン
 - Topボタン			Aボタン
 - Top Upボタン		SELECTボタン
 - Top Downボタン		STARTボタン
 - HATスイッチ		十字キー（反転モードなし）

 - Aボタン	Aボタン
 - Bボタン	Bボタン
 - Cボタン	SELECTボタン
 - Dボタン	STARTボタン

#### モード切り替えボタン

Shiftボタンと特定のボタンを同時に押すと、いつでもモード変更ができます。

 - Shift + Topボタン		ファミコン配列／スーパーファミコン配列の切り替え
 - Shift + Fireボタン	 	Fireボタンの連射のオン・オフ
 - Shift + HATスイッチ上	スティックの上下を十字キーの上下に割り当てる
 - Shift + HATスイッチ下	スティックの上下を十字キーの下上に割り当てる


### スーパーファミコン配列


スーパーファミコンソフト『スターフォックス』（任天堂）向けの設定です。
スティックの根元のスロットルにX（ブレーキ）とY（ブースト）を割り当てていますが、L/Rボタンを二度押すローリングは出しにくいです。

 - スティック 十字キー（疑似アナログ）
 - ラダー（スティックひねり）左	Lボタン
 - ラダー（スティックひねり）右	Rボタン
 - スロットル上	Xボタン
 - スロットル中	なし
 - スロットル下	Yボタン

 - Fireボタン		Bボタン
 - Topボタン			Aボタン
 - Top Upボタン		Xボタン
 - Top Downボタン		Yボタン
 - Shift + Top Upボタン		SELECTボタン
 - Shift + Top Downボタン	 	STARTボタン
 - HATスイッチ	十字キー（反転モードなし）

 - Aボタン	Aボタン
 - Bボタン	Bボタン
 - Cボタン	Xボタン
 - Dボタン	Yボタン


## 参考ウェブサイト

1. [MaZderMind/SidewinderInterface](https://github.com/MaZderMind/SidewinderInterface/tree/master/software) Precision Proの通信プロトコルについて、オシロスコープ画像つきで詳細に説明されています。また、union sw_data_t の定義を引用しています。
2. [SFCコントローラーをパラレル風の信号に変換](http://d-tomo.net/pic2.html) スーファミコントローラの通信プロトコルについて、詳細に説明されています。
3. [SNESkey Arcade Interface](http://arcadecontrols.com/arcade_sneskey.html)4021を二つ使った、スーファミコントローラの回路図が紹介されています。
4. [Arduino SPI マスターとスレーブでデータ通信](https://ogapsan.com/archives/343) ArduinoをSPIスレーブとして動作させるソースコードが紹介されています。
