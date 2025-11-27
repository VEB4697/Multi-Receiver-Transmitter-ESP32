#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// NRF24L01 Configuration
const int CHANNEL_COUNT = 12;
const uint8_t MAX_RECEIVERS = 8;
const uint64_t BASE_PIPES[MAX_RECEIVERS] = {
  0xF0F0F0F0E1LL, 0xF0F0F0F0E2LL, 0xF0F0F0F0E3LL, 0xF0F0F0F0E4LL,
  0xF0F0F0F0E5LL, 0xF0F0F0F0E6LL, 0xF0F0F0F0E7LL, 0xF0F0F0F0E8LL
};

// Receiver Names
const String RECEIVER_NAMES[MAX_RECEIVERS] = {
  "Hexapod-1", "Hexapod-2", "RC Car-1", "RC Car-2", 
  "Drone-1", "Boat-1", "Tank-1", "Custom-1"
};

// Channel Configuration
struct ChannelData {
  int16_t throttle;
  int16_t pitch;
  int16_t roll;
  int16_t yaw;
  int16_t aux1;
  int16_t aux2;
  uint8_t aux3 : 1;
  uint8_t aux4 : 1;
  uint8_t aux5 : 1;
  uint8_t aux6 : 1;
  int8_t aux7;
  int8_t aux8;
  uint8_t receiver_id;
  uint32_t timestamp;
};

// Trim Settings
struct TrimSettings {
  int16_t pitch_trim;
  int16_t roll_trim;
  int16_t yaw_trim;
};

// Calibration Data
struct CalibrationData {
  int16_t throttle_min, throttle_max, throttle_mid;
  int16_t pitch_min, pitch_max, pitch_mid;
  int16_t roll_min, roll_max, roll_mid;
  int16_t yaw_min, yaw_max, yaw_mid;
  int16_t aux1_min, aux1_max, aux1_mid;
  int16_t aux2_min, aux2_max, aux2_mid;
};

// System Settings
struct SystemSettings {
  uint8_t current_receiver;
  bool throttle_bidirectional;
  TrimSettings trim;
  CalibrationData calibration;
  bool save_settings;
};

#endif