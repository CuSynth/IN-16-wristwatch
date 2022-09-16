#include "RTClib.h"
#include <Wire.h>
#include "LowPower.h"
#include <GyverButton.h>
#include <avr/power.h>

RTC_DS3231 rtc;

#define PIN 2
#define KEY0  A2
#define KEY1  A3
#define LED_H  A0
#define LED_M  A1
#define mos0  12
#define mos1  6
#define mos2  13
#define mos3  7
#define mos4  10
#define mos5  4
#define mos6  3
#define mos7  5
#define mos8  8
#define mos9  11

GButton butt1(PIN);
int volts;
int digits[1];
int mode = 0;
int regim = 1;
int pwm_duty = 100;
int flag = 0;
int8_t hrs = 10, mins = 10, secs;
int stb_time = 2800;
unsigned long standby_timer, mode_timer ;
boolean sleep_flag = true;
int randnum0;
int randnum1;

void setup() {
  Wire.begin();
  TCCR1B=TCCR1B&0b11111000|0x01; 
  Serial.begin(9600);
  pinMode(9, OUTPUT);
  pinMode(KEY0, OUTPUT);
  pinMode(KEY1, OUTPUT);
  pinMode(LED_H, OUTPUT);
  pinMode(LED_M, OUTPUT);
  pinMode(mos0, OUTPUT);
  pinMode(mos1, OUTPUT);
  pinMode(mos2, OUTPUT);
  pinMode(mos3, OUTPUT);
  pinMode(mos4, OUTPUT);
  pinMode(mos5, OUTPUT);
  pinMode(mos6, OUTPUT);
  pinMode(mos7, OUTPUT);
  pinMode(mos8, OUTPUT);
  pinMode(mos9, OUTPUT);
  rtc.begin();
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  DateTime now = rtc.now();
  secs = now.second();
  mins = now.minute();
  hrs = now.hour();

  butt1.setDebounce(50);        
  butt1.setTimeout(500);       
  butt1.setClickTimeout(300);  
  butt1.setType(LOW_PULL);
  butt1.setDirection(NORM_OPEN);
  butt1.setTickMode(AUTO);
  
  attachInterrupt(0, wake_up, RISING);
  digitalWrite(9, LOW);
  delay(100);
  good_night();
  
}

void showTime(){
      if (mode == 0) { 
       randnum0++;
       randnum1++;
       if(randnum0 > 9){randnum0 = 0;}
       if(randnum1 > 9){randnum1 = 0;}
      digits[0] = randnum0;
      digits[1] = randnum1;
      delay(5);    
      if (mode == 0 && (millis() -  mode_timer > 200)) {
      mode = 1;
      mode_timer = millis();
         }
      } else if (mode == 1) {
      digitalWrite(LED_H, HIGH);
      digitalWrite(LED_M, LOW);
      digits[0] = hrs/10;
      digits[1] = hrs%10;
      
      if (mode == 1 && (millis() -  mode_timer > 1200)) {
      mode = 2;
      mode_timer = millis();
         }
      }else if (mode == 2) {
        digitalWrite(LED_H, LOW);
       randnum0++;
       randnum1++;
       if(randnum0 > 9){randnum0 = 0;}
       if(randnum1 > 9){randnum1 = 0;}
      digits[0] = randnum0;
      digits[1] = randnum1;  
      delay(5);
      if (mode == 2 && (millis() -  mode_timer > 200)) {
      mode = 3;
      mode_timer = millis();
         }
      }else if (mode == 3) {
      
      digitalWrite(LED_M, HIGH);
      digits[0] = mins/10;
      digits[1] = mins%10;
      }
  show(digits);
  if (millis() - standby_timer > stb_time) {
  good_night();
    }
}
  


void showVcc(){
  digitalWrite(LED_M, LOW);
  digitalWrite(LED_H, LOW);
        digits[0] = volts/10;
        digits[1] = volts%10;
         show(digits);
         
  if (millis() - standby_timer > 1000) {
  good_night();
  }
}

void changeH(){
  if (regim == 4 && butt1.isRelease())
      {
        hrs++;
        if (hrs > 23)
        {
          hrs = 0;
        } 
      }
      
      digitalWrite(LED_H, HIGH);
      digitalWrite(LED_M, LOW);
      digits[0] = hrs/10;
      digits[1] = hrs%10;
      show(digits);
  if (millis() - standby_timer > 5000) {
  rtc.adjust(DateTime(2014, 1, 21, hrs, mins, 0));
  good_night();
  }
}

void changeM(){
  if (regim == 3 && butt1.isRelease())
      {
        mins++;
        if (mins > 59)
        {
          mins = 0;
        } 
      }
      Serial.println(mins);
      digitalWrite(LED_H, LOW);
      digitalWrite(LED_M, HIGH);
      digits[0] = mins/10;
      digits[1] = mins%10;
      show(digits);
  if (millis() - standby_timer > 5000) {
  rtc.adjust(DateTime(2014, 1, 21, hrs, mins, 0));
  good_night();
  }
}

void lowbattery(){
      digitalWrite(LED_H, HIGH);
      digitalWrite(LED_M, HIGH);
      delay(1000);
      good_night();
}

void loop(){
  if (sleep_flag) {  
    sleep_flag = false;
    volts = readVcc();
    volts = map(volts, 3300, 4250, 0, 99);
    if(volts < 02){regim = 5;}
    if(volts > 90){pwm_duty = 90;}
    if(volts < 90){pwm_duty = 100;}
    if(volts < 70){pwm_duty = 110;}
    if(volts < 50){pwm_duty = 130;}
    if(volts < 30){pwm_duty = 140;}
    if(volts < 15){pwm_duty = 160;}

    if(volts > 02){
    analogWrite(9, pwm_duty);
    randnum0 = random(0, 9);
    randnum1 = random(0, 9);
    DateTime now = rtc.now();
    mins = now.minute();
    hrs = now.hour();
    regim = 1;
    mode = 0;
    delay(100);
    standby_timer = millis();  // и обнуляем таймер
    mode_timer = millis();
    }
  }   
if(regim == 1){showTime();}
if(regim == 2){showVcc();}
if(regim == 3){
  changeM();
}

 if(regim == 4){
  changeH();
}

 if(regim == 5){
  lowbattery();
}

  if (regim == 1 && butt1.isDouble()) 
  {
    regim = 2;
  }
  
    if (regim == 2 && butt1.isDouble()) 
  {
    regim = 3;
  }
  
  if (regim == 3 && butt1.isHolded()) 
  {
    regim = 4;
    }
}
 
void good_night() {
  digitalWrite(9, 0);
  digitalWrite(KEY0, 0);
  digitalWrite(KEY1, 0);
  digitalWrite(LED_H, 0);
  digitalWrite(LED_M, 0);
  digitalWrite(mos0, 0);
  digitalWrite(mos1, 0);
  digitalWrite(mos2, 0);
  digitalWrite(mos3, 0);
  digitalWrite(mos4, 0);
  digitalWrite(mos5, 0);
  digitalWrite(mos6, 0);
  digitalWrite(mos7, 0);
  digitalWrite(mos8, 0);
  digitalWrite(mos9, 0);
  volts = 0;
  sleep_flag = true;
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}

void wake_up() {
  standby_timer = millis();  // и обнуляем таймер
  mode_timer = millis();
}

void setNumber(int num) {
  switch (num)
  {
    case 0:
    digitalWrite (mos0,HIGH);
    digitalWrite (mos1,LOW);
    digitalWrite (mos2,LOW);
    digitalWrite (mos3,LOW);
    digitalWrite (mos4,LOW);
    digitalWrite (mos5,LOW);
    digitalWrite (mos6,LOW);
    digitalWrite (mos7,LOW);
    digitalWrite (mos8,LOW);
    digitalWrite (mos9,LOW);
    break;
    case 1:
    digitalWrite (mos0,LOW);
    digitalWrite (mos1,HIGH);
    digitalWrite (mos2,LOW);
    digitalWrite (mos3,LOW);
    digitalWrite (mos4,LOW);
    digitalWrite (mos5,LOW);
    digitalWrite (mos6,LOW);
    digitalWrite (mos7,LOW);
    digitalWrite (mos8,LOW);
    digitalWrite (mos9,LOW);
    break;
    case 2:
    digitalWrite (mos0,LOW);
    digitalWrite (mos1,LOW);
    digitalWrite (mos2,HIGH);
    digitalWrite (mos3,LOW);
    digitalWrite (mos4,LOW);
    digitalWrite (mos5,LOW);
    digitalWrite (mos6,LOW);
    digitalWrite (mos7,LOW);
    digitalWrite (mos8,LOW);
    digitalWrite (mos9,LOW);
    break;
    case 3:
    digitalWrite (mos0,LOW);
    digitalWrite (mos1,LOW);
    digitalWrite (mos2,LOW);
    digitalWrite (mos3,HIGH);
    digitalWrite (mos4,LOW);
    digitalWrite (mos5,LOW);
    digitalWrite (mos6,LOW);
    digitalWrite (mos7,LOW);
    digitalWrite (mos8,LOW);
    digitalWrite (mos9,LOW);
    break;
    case 4:
    digitalWrite (mos0,LOW);
    digitalWrite (mos1,LOW);
    digitalWrite (mos2,LOW);
    digitalWrite (mos3,LOW);
    digitalWrite (mos4,HIGH);
    digitalWrite (mos5,LOW);
    digitalWrite (mos6,LOW);
    digitalWrite (mos7,LOW);
    digitalWrite (mos8,LOW);
    digitalWrite (mos9,LOW);
    break;
    case 5:
    digitalWrite (mos0,LOW);
    digitalWrite (mos1,LOW);
    digitalWrite (mos2,LOW);
    digitalWrite (mos3,LOW);
    digitalWrite (mos4,LOW);
    digitalWrite (mos5,HIGH);
    digitalWrite (mos6,LOW);
    digitalWrite (mos7,LOW);
    digitalWrite (mos8,LOW);
    digitalWrite (mos9,LOW);
    break;
    case 6:
    digitalWrite (mos0,LOW);
    digitalWrite (mos1,LOW);
    digitalWrite (mos2,LOW);
    digitalWrite (mos3,LOW);
    digitalWrite (mos4,LOW);
    digitalWrite (mos5,LOW);
    digitalWrite (mos6,HIGH);
    digitalWrite (mos7,LOW);
    digitalWrite (mos8,LOW);
    digitalWrite (mos9,LOW);
    break;
    case 7:
    digitalWrite (mos0,LOW);
    digitalWrite (mos1,LOW);
    digitalWrite (mos2,LOW);
    digitalWrite (mos3,LOW);
    digitalWrite (mos4,LOW);
    digitalWrite (mos5,LOW);
    digitalWrite (mos6,LOW);
    digitalWrite (mos7,HIGH);
    digitalWrite (mos8,LOW);
    digitalWrite (mos9,LOW);
    break;
    case 8:
    digitalWrite (mos0,LOW);
    digitalWrite (mos1,LOW);
    digitalWrite (mos2,LOW);
    digitalWrite (mos3,LOW);
    digitalWrite (mos4,LOW);
    digitalWrite (mos5,LOW);
    digitalWrite (mos6,LOW);
    digitalWrite (mos7,LOW);
    digitalWrite (mos8,HIGH);
    digitalWrite (mos9,LOW);
    break;
    case 9:
    digitalWrite (mos0,LOW);
    digitalWrite (mos1,LOW);
    digitalWrite (mos2,LOW);
    digitalWrite (mos3,LOW);
    digitalWrite (mos4,LOW);
    digitalWrite (mos5,LOW);
    digitalWrite (mos6,LOW);
    digitalWrite (mos7,LOW);
    digitalWrite (mos8,LOW);
    digitalWrite (mos9,HIGH);
    break;
  }
}
void show(int a[]){
    setNumber(a[0]);
    digitalWrite(KEY0,HIGH);
    delay(1);
    digitalWrite(KEY0,LOW);
    delay(6);
    setNumber(a[1]);
    digitalWrite(KEY1,HIGH);
    delay(1);
    digitalWrite(KEY1,LOW);
    delay(6);
}
long readVcc() {
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

  delay(50); 
  ADCSRA |= _BV(ADSC); 
  while (bit_is_set(ADCSRA,ADSC)); 

  uint8_t low  = ADCL; 
  uint8_t high = ADCH; 

  long result = (high<<8) | low;

  result = 1125300L / result; 
  return result; 
}
