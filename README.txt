Arduino Alarm Clock
By Ruben Martinez Jr.
Last Modified: 12/1/2012

This code is designed for an Arduino Uno. It runs a simple alarm clock that awakens the user with LED lights. It is designed with the following hardware in mind:

 - Arduino Uno
 - LEDs
 - RF Chipset (utilized for an extension of the project)
 - RTC Real Time Clock Module
 - A set of 3 push buttons

The Alarm Clock is designed with the following functionality in mind: A user will set an alarm by pressing down a "Set" button and alternating between the "Hour" and "Minute" buttons. The current time will be displayed on the segment display, and the alarm time will be displayed when holding down the "Set" button. When the alarm time comes, the clock will slowly fade the LEDs on to wake the user. Pressing the "Set" button will snooze the alarm clock, and it will turn itself off automatically after a code-set amount of time.