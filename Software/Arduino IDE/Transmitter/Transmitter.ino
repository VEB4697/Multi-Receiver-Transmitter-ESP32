#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PCF8575.h>
#include <EEPROM.h>

#include "pin_definitions.h"
#include "config.h"
#include "ui_controller.h"

// Global Objects
RF24 radio(CE_PIN, CSN_PIN);
PCF8575 pcf8575(PCF8575_ADDRESS);
UIController* ui_controller;

// System State
SystemSettings system_settings;
ChannelData channel_data;
bool settings_modified = false;

// Timing
unsigned long last_transmit_time = 0;
const unsigned long TRANSMIT_INTERVAL = 20; // 50Hz

void setup() {
  Serial.begin(115200);
  
  // Initialize EEPROM
  EEPROM.begin(512);
  
  // Load settings from EEPROM
  loadSettings();
  
  // Initialize pins
  initializePins();
  
  // Initialize radio
  if (!initializeRadio()) {
    Serial.println("Radio initialization failed!");
    while(1);
  }
  
  // Initialize OLED
  ui_controller = new UIController(&system_settings);
  if (!ui_controller->begin()) {
    Serial.println("OLED initialization failed!");
  }
  
  // Initialize PCF8575
  if (!pcf8575.begin()) {
    Serial.println("PCF8575 initialization failed!");
  }
  
  pcf8575.pinMode(P0, INPUT);
  pcf8575.pinMode(P1, INPUT);
  pcf8575.pinMode(P2, INPUT);
  pcf8575.pinMode(P3, INPUT);
  pcf8575.pinMode(P4, INPUT);
  pcf8575.pinMode(P5, INPUT);
  
  Serial.println("Transmitter initialized successfully!");
  ui_controller->update(false, false, false);
}

void loop() {
  // Read all inputs
  readInputs();
  
  // Update UI
  bool btn_up = digitalRead(BTN_UP_PIN);
  bool btn_down = digitalRead(BTN_DOWN_PIN);
  bool btn_select = digitalRead(BTN_SELECT_PIN);
  ui_controller->update(btn_up, btn_down, btn_select);
  
  // Transmit data
  if (millis() - last_transmit_time >= TRANSMIT_INTERVAL) {
    transmitData();
    last_transmit_time = millis();
  }
  
  // Save settings if modified
  if (settings_modified) {
    saveSettings();
    settings_modified = false;
  }
  
  delay(10);
}

void initializePins() {
  // Analog inputs
  pinMode(THROTTLE_PIN, INPUT);
  pinMode(PITCH_PIN, INPUT);
  pinMode(ROLL_PIN, INPUT);
  pinMode(YAW_PIN, INPUT);
  pinMode(AUX1_PIN, INPUT);
  pinMode(AUX2_PIN, INPUT);
  
  // Digital inputs with pullups
  pinMode(AUX3_PIN, INPUT_PULLUP);
  pinMode(AUX4_PIN, INPUT_PULLUP);
  pinMode(AUX5_PIN, INPUT_PULLUP);
  pinMode(AUX6_PIN, INPUT_PULLUP);
  
  pinMode(AUX7_PIN1, INPUT_PULLUP);
  pinMode(AUX7_PIN2, INPUT_PULLUP);
  pinMode(AUX8_PIN1, INPUT_PULLUP);
  pinMode(AUX8_PIN2, INPUT_PULLUP);
  
  // Menu buttons with pullups
  pinMode(BTN_UP_PIN, INPUT_PULLUP);
  pinMode(BTN_DOWN_PIN, INPUT_PULLUP);
  pinMode(BTN_SELECT_PIN, INPUT_PULLUP);
  
  // Trim buttons will be handled by PCF8575
}

bool initializeRadio() {
  if (!radio.begin()) {
    return false;
  }
  
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_2MBPS);
  radio.setChannel(76);
  radio.setRetries(3, 5);
  radio.setCRCLength(RF24_CRC_16);
  
  // Set initial pipe address
  setReceiverAddress(system_settings.current_receiver);
  
  radio.stopListening();
  return true;
}

void setReceiverAddress(uint8_t receiver_id) {
  if (receiver_id < MAX_RECEIVERS) {
    radio.openWritingPipe(BASE_PIPES[receiver_id]);
    system_settings.current_receiver = receiver_id;
    settings_modified = true;
  }
}

void readInputs() {
  // Read analog inputs
  channel_data.throttle = readAndMapAnalog(THROTTLE_PIN, system_settings.calibration.throttle_min, 
                                          system_settings.calibration.throttle_max, 
                                          system_settings.calibration.throttle_mid);
  channel_data.pitch = readAndMapAnalog(PITCH_PIN, system_settings.calibration.pitch_min,
                                       system_settings.calibration.pitch_max,
                                       system_settings.calibration.pitch_mid) + system_settings.trim.pitch_trim;
  channel_data.roll = readAndMapAnalog(ROLL_PIN, system_settings.calibration.roll_min,
                                      system_settings.calibration.roll_max,
                                      system_settings.calibration.roll_mid) + system_settings.trim.roll_trim;
  channel_data.yaw = readAndMapAnalog(YAW_PIN, system_settings.calibration.yaw_min,
                                     system_settings.calibration.yaw_max,
                                     system_settings.calibration.yaw_mid) + system_settings.trim.yaw_trim;
  channel_data.aux1 = readAndMapAnalog(AUX1_PIN, system_settings.calibration.aux1_min,
                                      system_settings.calibration.aux1_max,
                                      system_settings.calibration.aux1_mid);
  channel_data.aux2 = readAndMapAnalog(AUX2_PIN, system_settings.calibration.aux2_min,
                                      system_settings.calibration.aux2_max,
                                      system_settings.calibration.aux2_mid);
  
  // Read digital switches
  channel_data.aux3 = !digitalRead(AUX3_PIN);
  channel_data.aux4 = !digitalRead(AUX4_PIN);
  channel_data.aux5 = !digitalRead(AUX5_PIN);
  channel_data.aux6 = !digitalRead(AUX6_PIN);
  
  // Read 3-way switches
  channel_data.aux7 = read3WaySwitch(AUX7_PIN1, AUX7_PIN2);
  channel_data.aux8 = read3WaySwitch(AUX8_PIN1, AUX8_PIN2);
  
  // Set receiver ID
  channel_data.receiver_id = system_settings.current_receiver;
  channel_data.timestamp = millis();
}

int16_t readAndMapAnalog(uint8_t pin, int16_t min_val, int16_t max_val, int16_t mid_val) {
  int raw = analogRead(pin);
  
  if (system_settings.throttle_bidirectional && pin == THROTTLE_PIN) {
    // Map from 0-4095 to -511 to 512
    return map(raw, 0, 4095, -511, 512);
  } else {
    // Map from 0-4095 to custom range
    if (raw < mid_val) {
      return map(raw, min_val, mid_val, -511, 0);
    } else {
      return map(raw, mid_val, max_val, 0, 512);
    }
  }
}

int8_t read3WaySwitch(uint8_t pin1, uint8_t pin2) {
  bool state1 = !digitalRead(pin1);
  bool state2 = !digitalRead(pin2);
  
  if (state1 && !state2) return -1;
  if (!state1 && state2) return 1;
  return 0;
}

void transmitData() {
  bool success = radio.write(&channel_data, sizeof(channel_data));
  
  if (!success) {
    Serial.println("Transmission failed!");
  }
}

void loadSettings() {
  EEPROM.get(0, system_settings);
  
  // If EEPROM is empty, initialize with default values
  if (system_settings.current_receiver >= MAX_RECEIVERS) {
    initializeDefaultSettings();
  }
}

void saveSettings() {
  EEPROM.put(0, system_settings);
  EEPROM.commit();
}

void initializeDefaultSettings() {
  system_settings.current_receiver = 0;
  system_settings.throttle_bidirectional = false;
  
  // Initialize trim
  system_settings.trim.pitch_trim = 0;
  system_settings.trim.roll_trim = 0;
  system_settings.trim.yaw_trim = 0;
  
  // Initialize calibration with typical ESP32 ADC ranges
  system_settings.calibration.throttle_min = 0;
  system_settings.calibration.throttle_max = 4095;
  system_settings.calibration.throttle_mid = 2048;
  
  system_settings.calibration.pitch_min = 0;
  system_settings.calibration.pitch_max = 4095;
  system_settings.calibration.pitch_mid = 2048;
  
  system_settings.calibration.roll_min = 0;
  system_settings.calibration.roll_max = 4095;
  system_settings.calibration.roll_mid = 2048;
  
  system_settings.calibration.yaw_min = 0;
  system_settings.calibration.yaw_max = 4095;
  system_settings.calibration.yaw_mid = 2048;
  
  system_settings.calibration.aux1_min = 0;
  system_settings.calibration.aux1_max = 4095;
  system_settings.calibration.aux1_mid = 2048;
  
  system_settings.calibration.aux2_min = 0;
  system_settings.calibration.aux2_max = 4095;
  system_settings.calibration.aux2_mid = 2048;
  
  saveSettings();
}