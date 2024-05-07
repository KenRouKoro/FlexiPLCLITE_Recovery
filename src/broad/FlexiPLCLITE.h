//
// Created by Koro on 2024/3/11.
//
#pragma once

#include <Arduino.h>
#include <Wire.h>
/**
 * ********************引脚定义********************
 */
#define OUT0 PA4
#define OUT1 PA5
#define OUT2 PA6
#define OUT3 PA7
#define OUT4 PB0
#define OUT5 PB1
#define OUT6 PB2
#define OUT7 PB12
#define OUT8 PA11
#define OUT9 PA12
#define OUT10 PA15
#define OUT11 PD0

#define sw1 PB15
#define sw2 PA8

#define IN0 PB3
#define IN1 PD3
#define IN2 PD2
#define IN3 PD1

#define BTN_UP PB9
#define BTN_DOWN PB8
#define BTN_OK PB5
#define BTN_CANCEL PB4

#define TX1 PB6
#define RX1 PB7

#define TX2 PA2
#define RX2 PA3

#define TX3 PB10
#define RX3 PB11

#define TX4 PA0
#define RX4 PA1

#define SCL1 PA9
#define SDA1 PA10

#define SCL2 PB13
#define SDA2 PB14

/**
 * ********************全局外设资源********************
 */

/**
 * ********************方法声明********************
 */

void initBSP();//初始化板上IO
void initOut();
void initIN();
void initBtn();
void initSW();

void closeAll();

void outputRelay(short num,bool status);
bool readRelay(short num);
bool readINPUT(short num);

void onClickOK();
void onClickCANCEL();
void onClickUP();
void onClickDOWN();

void onIn0();
void onIn1();
void onIn2();
void onIn3();

void onSW1();
void onSW2();

void onInput(short pinNum);

/**
 * ********************方法实现********************
 */
void inline initBSP() {
 initBtn();
 initOut();
 initIN();
 closeAll();
 initSW();

 //初始化Wire到OLED
 Wire.setSCL(SCL1);
 Wire.setSDA(SDA1);
}

void inline initBtn() {
 pinMode(BTN_OK, INPUT_PULLUP);
 pinMode(BTN_CANCEL, INPUT_PULLUP);
 pinMode(BTN_UP, INPUT_PULLUP);
 pinMode(BTN_DOWN, INPUT_PULLUP);
 attachInterrupt(digitalPinToInterrupt(BTN_OK), onClickOK, LOW);
 attachInterrupt(digitalPinToInterrupt(BTN_CANCEL), onClickCANCEL, LOW);
 attachInterrupt(digitalPinToInterrupt(BTN_UP), onClickUP, LOW);
 attachInterrupt(digitalPinToInterrupt(BTN_DOWN), onClickDOWN, LOW);
}

void inline initOut() {
 pinMode(OUT0, OUTPUT);
 pinMode(OUT1, OUTPUT);
 pinMode(OUT2, OUTPUT);
 pinMode(OUT3, OUTPUT);
 pinMode(OUT4, OUTPUT);
 pinMode(OUT5, OUTPUT);
 pinMode(OUT6, OUTPUT);
 pinMode(OUT7, OUTPUT);
 pinMode(OUT8, OUTPUT);
 pinMode(OUT9, OUTPUT);
 pinMode(OUT10, OUTPUT);
 pinMode(OUT11, OUTPUT);
}

void inline initIN() {
 pinMode(IN0, INPUT_PULLDOWN);
 pinMode(IN1, INPUT_PULLDOWN);
 pinMode(IN2, INPUT_PULLDOWN);
 pinMode(IN3, INPUT_PULLDOWN);
 attachInterrupt(digitalPinToInterrupt(IN0), onIn0, LOW);
 attachInterrupt(digitalPinToInterrupt(IN1), onIn1, LOW);
 attachInterrupt(digitalPinToInterrupt(IN2), onIn2, LOW);
 attachInterrupt(digitalPinToInterrupt(IN3), onIn3, LOW);
}

void inline initSW() {
 pinMode(sw1, INPUT_PULLDOWN);
 pinMode(sw2, INPUT_PULLDOWN);
 attachInterrupt(digitalPinToInterrupt(sw1), onSW1, LOW);
 attachInterrupt(digitalPinToInterrupt(sw2), onSW2, LOW);
}

void inline closeAll() {
 digitalWrite(OUT0, LOW);
 digitalWrite(OUT1, LOW);
 digitalWrite(OUT2, LOW);
 digitalWrite(OUT3, LOW);
 digitalWrite(OUT4, LOW);
 digitalWrite(OUT5, LOW);
 digitalWrite(OUT6, LOW);
 digitalWrite(OUT7, LOW);
 digitalWrite(OUT8, LOW);
 digitalWrite(OUT9, LOW);
 digitalWrite(OUT10, LOW);
 digitalWrite(OUT11, LOW);
}