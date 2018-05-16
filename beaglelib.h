// beaglebone用　GPIOサポートユーティリティ
// リリース時、-DNDEBUG

#ifndef _BEAGLE_H
#define _BEAGLE_H

#include <iostream>
#include <fstream>
#include <string>
#include <assert.h>
using namespace std;

bool gpio_export( int n );					// n番のgpioを生成する
void gpio_unexport( int n );				// n番のgpioを削除する
void gpio_init_out( int n, int val);	// n番のgpioを出力モード、値をvalに
void gpio_init_in( int n, int edge);	//　edge=1:riding 2:falling 3:both 4:none
void gpio_data_set( int n, int val); // n番のgpioの値をvalに
int gpio_data_get( int n );				// n番のgpioの値を読みだし返す
#endif
