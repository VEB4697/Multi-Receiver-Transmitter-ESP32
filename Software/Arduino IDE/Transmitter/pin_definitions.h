#ifndef PIN_DEFINITIONS_H
#define PIN_DEFINITIONS_H

// NRF24L01 Pins
#define CE_PIN 4
#define CSN_PIN 5

// Joystick Pins
#define THROTTLE_PIN 34    // ADC1_CH6
#define PITCH_PIN 35       // ADC1_CH7  
#define ROLL_PIN 32        // ADC1_CH4
#define YAW_PIN 33         // ADC1_CH5

// Potentiometer Pins
#define AUX1_PIN 25        // ADC2_CH8
#define AUX2_PIN 26        // ADC2_CH9

// 2-way Toggle Switches
#define AUX3_PIN 15
#define AUX4_PIN 2
#define AUX5_PIN 0
#define AUX6_PIN 16

// 3-way Toggle Switches (using 2 pins each)
#define AUX7_PIN1 17
#define AUX7_PIN2 18
#define AUX8_PIN1 19
#define AUX8_PIN2 21

// Trim Buttons
#define PITCH_UP_PIN 13
#define PITCH_DOWN_PIN 12
#define ROLL_UP_PIN 14
#define ROLL_DOWN_PIN 27
#define YAW_UP_PIN 22
#define YAW_DOWN_PIN 23

// Menu Navigation Buttons
#define BTN_UP_PIN 39
#define BTN_DOWN_PIN 36
#define BTN_SELECT_PIN 3

// OLED Display (I2C)
#define OLED_SDA 21
#define OLED_SCL 22

// PCF8575 (I2C) - Same bus as OLED
#define PCF8575_ADDRESS 0x20

#endif