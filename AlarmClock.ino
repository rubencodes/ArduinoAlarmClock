/* Ruben Martinez Jr.
 * Bowdoin College
 * 1 December 2012
 * Easy-Wake Alaem System
 */
#include <Wire.h>
#include <SoftwareSerial.h>
#include "RTClib.h"

/* PINS */
//receiver
#define rcRxPin 0  // receiver data pin
#define rcTxPin 1  // unused, but SoftwareSerial needs it...
//segment display 
#define sgRxPin 2  // unused, but SoftwareSerial needs it...
#define sgTxPin 3  // segment display data pin
//alarm
#define setAlarmPin 5  // triggers alarm set function
#define alarmHourPin 6  // change alarm hour
#define alarmMinutePin 7  // change alarm minute
//lights
#define ledOne 9  // an LED row
#define ledTwo 10  // an LED row
#define ledThree 11  // an LED row
/* END PINS */

/* SETUP */
//receiver
SoftwareSerial transmitter = SoftwareSerial(rcRxPin,rcTxPin);  // software serial for receiver
//segment display
SoftwareSerial segdis = SoftwareSerial(sgRxPin, sgTxPin);  // software serial for display
int digits[10] = {
  0x00, 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09}; //display digits
int displayHour;  // 12-hour format variable
//real time clock
RTC_DS1307 RTC;  // real time clock
DateTime now;  // time now
//alarm
int alarmHour = 10;  // hour of alarm
int alarmMinute = 10;  // minute of alarm
int hoursUntilAlarm;  // how many hours until the alarm rings?
int minutesUntilAlarm;  // how many minutes until the alarm rings?
boolean wake = false;  // is it time to wake?
boolean snooze = false;  // has the snooze button been hit?
boolean blink = false;  // should they really get up now?
int delayTime = 4000;  // how long between blinks?
//lights
int brightness = 0;  // how bright the LED is 

void setup()  {
  //starting engines
  segdis.begin(9600);   // display serial
  transmitter.begin(52600);
  Wire.begin();  // begin data transmission
  RTC.begin();  // start real time clock

  //starting leds and display
  pinMode(ledOne, OUTPUT);  // LEDs
  pinMode(ledTwo, OUTPUT);  // LEDs
  pinMode(ledThree, OUTPUT);  // LEDs
  pinMode(sgTxPin, OUTPUT);  // display output 

  //starting buttons
  pinMode(setAlarmPin, INPUT);  // button turns alarm on
  pinMode(alarmHourPin, INPUT);  // button ups alarm hour
  pinMode(alarmMinutePin, INPUT);  // button ups alarm minute
  //setting pins to high for pullup resistors
  digitalWrite(setAlarmPin, HIGH);
  digitalWrite(alarmHourPin, HIGH);
  digitalWrite(alarmMinutePin, HIGH);

  //resetting display
  segdis.write(0x76); // reset the display
  segdis.write(0x7A); // command byte
  segdis.write(0x01); // display brightness (FE for dim)
  segdis.write(0x77); // command byte for decimal points
  segdis.write(0x10); // turn them off for this example

  //fail-safe for real time clock
  if (!RTC.isrunning())
    RTC.adjust(DateTime(__DATE__, __TIME__)); // sets RTC to date,time sketch was compiled
}

void loop()  {  //call print time and call check alarm
  now = RTC.now();  // the time is NOW
  checkAlarm();  // check alarm
  //is the alarm being set?
  if (digitalRead(setAlarmPin) == LOW) { // if so, 
    if (brightness == 0) setAlarm();  // if alarm isn't on, enter set alarm mode
    else snooze = true;  // if alarm IS on, function as a snooze button
  }
  else displayTime();  // display the time
}

void checkAlarm() {
  //calculation of max minutes until alarm should go off
  if (now.hour() <= alarmHour) hoursUntilAlarm = alarmHour-now.hour();
  else hoursUntilAlarm = (24-now.hour())+alarmHour;
  if (now.minute() <= alarmMinute) minutesUntilAlarm = minutesUntilAlarm-now.minute();
  else minutesUntilAlarm = (60-now.minute())+alarmMinute;
  int timeUntilAlarm = (hoursUntilAlarm*60) + (minutesUntilAlarm);
  
  /* if we get the "wake" signal,
   * and it's within 30 minutes of the alarm,
   * wake the user up!
   */
  if(transmitter.read() == '#' && snooze == false && timeUntilAlarm <= 30) wake = true;

  //if it's alarm time, wake the user up!
  if (timeUntilAlarm == 0) wake = true;

  //turn light off 15 minutes after turning light on
  if (timeUntilAlarm >= ((23*60)+45)) wake = false;

  /* if we're waking up,
   * slowly turn up the brightness,
   * until max brightness
   */
  if(wake == true) {
    // set the brightness of the LED pins
    analogWrite(ledOne, brightness);  
    analogWrite(ledTwo, brightness);
    analogWrite(ledThree, brightness);  

    // if we can get brighter, do it
    if (brightness < 255) {
      brightness++;
      delay(250);
    }
    
    /* if we're at max brightness,
     * start blinking!
     */
    if (blink == true && delayTime >= 500) delayTime = delayTime-50;
    if (brightness == 255) {
      delay(delayTime);
      blink = true;
      analogWrite(ledOne, 0);  
      analogWrite(ledTwo, 0);
      analogWrite(ledThree, 0);  
    }
    if (brightness == 0 && blink == true) {
      delay(delayTime);
      blink = false;
      analogWrite(ledOne, 255);  
      analogWrite(ledTwo, 255);
      analogWrite(ledThree, 255);
    }

    /* if snooze button is hit after alarm is turned on,
     * set alarm to turn back on in 5 minutes,
     * do not wake for now
     */
    if (snooze = true) {
      brightness = 0;
      alarmHour = now.hour();
      alarmMinute = now.minute()+5;
      wake = false;
      snooze = false;
    }
  }
}

void checkButtons() {
  static boolean minPressed = false, hourPressed = false; //track button state
  if (digitalRead(alarmHourPin) == LOW && hourPressed == false) {  // if hour button is being held
    //constrain hours from 0 to 24
    if (alarmHour < 23)
      alarmHour++;  // increment alarm hour
    if (alarmHour == 23)
      alarmHour = 0;
    hourPressed = true;
  }
  if (digitalRead(alarmHourPin) == HIGH) hourPressed = false;
  if (digitalRead(alarmMinutePin) == LOW && minPressed == false) {  // if minute button is being held
    //constrain minutes from 0 to 59
    if (alarmMinute < 59)
      alarmMinute++;  // increment alarm minute
    if (alarmMinute == 59)
      alarmMinute = 0;
    minPressed = true;
  }
  if (digitalRead(alarmMinutePin) == HIGH) minPressed = false;
}

void displayTime() {  // display time on seg display
  //conversion to 12-hour time
  if (now.hour() > 12) 
    displayHour = now.hour()-12;
  else if (now.hour() == 0)
    displayHour = 12;
  else 
    displayHour = now.hour();

  //separate digit by digit
  int tensHour = displayHour/10;  // ten's place hour
  int onesHour = displayHour%10;  // one's place hour
  int tensMinute = now.minute()/10;  // ten's place minute
  int onesMinute = now.minute()%10;  // one's place minute
  segdis.print(digits[tensHour]);  // print first digit of the hour
  segdis.print(digits[onesHour]);  // print second digit of the hour
  segdis.print(digits[tensMinute]);  // print first digit of minutes
  segdis.print(digits[onesMinute]);  // print second digit of minutes
  if (now.hour() >= 12) {
    segdis.write(0x77); // command byte for decimal points
    segdis.write(0x30); // turn them off for this example
  }
  else {
    segdis.write(0x77); // command byte for decimal points
    segdis.write(0x10); // turn them off for this example
  }
}

void setAlarm() {
  //check hour and minute buttons
  checkButtons();
  //conversion to 12-hour time
  if (alarmHour > 12)
    displayHour = alarmHour-12;
  else if (alarmHour == 0)
    displayHour = 12;
  else 
    displayHour = alarmHour;

  //seperate digit by digit
  int alarmTensHour = displayHour/10;  // ten's place hour
  int alarmOnesHour = displayHour%10;  // one's place hour
  int alarmTensMinute = alarmMinute/10;  // ten's place minute
  int alarmOnesMinute = alarmMinute%10;  // one's place minute
  segdis.print(digits[alarmTensHour]);  // print first digit of the hour
  segdis.print(digits[alarmOnesHour]);  // print second digit of the hour
  segdis.print(digits[alarmTensMinute]);  // print first digit of minutes
  segdis.print(digits[alarmOnesMinute]);  // print second digit of minutes
  if (alarmHour >= 12) {
    segdis.write(0x77); // command byte for decimal points
    segdis.write(0x30); // turn them off for this example
  }
  else {
    segdis.write(0x77); // command byte for decimal points
    segdis.write(0x10); // turn them off for this example
  }
}
