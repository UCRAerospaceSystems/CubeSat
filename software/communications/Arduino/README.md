# Arduino/CC1101

## Description

This folder contains the [ELECHOUSE](https://electronoobs.com/eng_arduino_tut98.php) library for the Arduino and CC1101. The header file for the CC1101 is similar to the ones MSP-430 uses but this library is **only** compatible with Arduino IDE. The example Tx/Rx code basically reads the data from A2 on the transmitter arduino and transmit that data to the receiver arduino. To run the program, place the ELECHOUSE_CC1101 folder into the Arduino library folder. Then place the Tx/Rx folders into the Arduino sketch folder. The code was tested with a button and an LED and works as intended. This folder serves as a starting point for the communications system incase we need to reimplement the entire process. Please refer to the two pinouts below.

## 8-pin CC1101 Pinout
![CPinout](/../images/CC1101Pinout.png)

## Arduino Pinout
![APinout](/../images/arduinoCC1101Pinout.jpg)
