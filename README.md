# README #

BBB GPIO入力割り込みプログラム
GTK+ のglib-2.0を使いGPIO入力イベントドリブンプログラム

標準入力(cin)でテストする場合、GIOConditionにG_IO_INを使用する
リリース時のGPIOピン入力の場合、G_IO_PRIを使用
（G_IO_INを使うとg_main_loop_unrefでアサートエラー）
g_main_loop_unref: assertion 'g_atomic_int_get (&loop->ref_count) > 0' failed

func[]でイベント発生時の実行イベントを指定する

timer_create()で割り込みタイマーを生成し、チャタリング防止にソフトディレイ 50msを入れている

ハードディレイ　10kΩ x 0.1~0.84μF　今回は0.47μFを使用 CR時定数　5mSec
ハードディレイを入れない場合、８〜１０回のwaitingが発生する

GPIO60番(P9_12)GPIO1[28] を入力用、立ち下がりエッジで使用

-------------------------------------------------------
# apt-get install libglib2.0-dev pkg-config
$ make

pkg-config glib-2.0 --cflags --libs
-I/usr/include/glib-2.0 -I/usr/lib/arm-linux-gnueabihf/glib-2.0/include -lglib-2.0　-lrt

g++ gioevent.cpp -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -lglib-2.0 -lrt -o gioevent
或いは
g++ gioevent.cpp `pkg-config glib-2.0 --cflags --libs` -lrt -o gioevent
ただし、C++14に対応するため-std=c++1yが必要
