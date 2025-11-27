#ifndef UI_CONTROLLER_H
#define UI_CONTROLLER_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

class UIController {
private:
  Adafruit_SSD1306 display;
  SystemSettings* settings;
  
  // Menu state
  uint8_t current_menu;
  uint8_t menu_item;
  bool in_submenu;
  uint8_t submenu_level;
  
  // UI animation
  unsigned long last_animation;
  uint8_t animation_frame;
  
  // Button states
  bool btn_up_prev, btn_down_prev, btn_select_prev;
  unsigned long last_button_press;
  
public:
  UIController(SystemSettings* settings_ptr) : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET), settings(settings_ptr) {
    current_menu = 0;
    menu_item = 0;
    in_submenu = false;
    submenu_level = 0;
    animation_frame = 0;
    last_animation = 0;
    last_button_press = 0;
  }
  
  bool begin() {
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
      return false;
    }
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    return true;
  }
  
  void update(bool btn_up, bool btn_down, bool btn_select) {
    handleInput(btn_up, btn_down, btn_select);
    render();
  }
  
private:
  void handleInput(bool btn_up, bool btn_down, bool btn_select) {
    unsigned long current_time = millis();
    
    // Debounce
    if (current_time - last_button_press < 200) return;
    
    if (btn_up && !btn_up_prev) {
      last_button_press = current_time;
      if (in_submenu) {
        handleSubmenuUp();
      } else {
        menu_item = (menu_item > 0) ? menu_item - 1 : 6;
      }
    }
    
    if (btn_down && !btn_down_prev) {
      last_button_press = current_time;
      if (in_submenu) {
        handleSubmenuDown();
      } else {
        menu_item = (menu_item < 6) ? menu_item + 1 : 0;
      }
    }
    
    if (btn_select && !btn_select_prev) {
      last_button_press = current_time;
      if (in_submenu) {
        handleSubmenuSelect();
      } else {
        enterSubmenu();
      }
    }
    
    btn_up_prev = btn_up;
    btn_down_prev = btn_down;
    btn_select_prev = btn_select;
  }
  
  void handleSubmenuUp() {
    switch(current_menu) {
      case 0: // Receiver Selection
        settings->current_receiver = (settings->current_receiver > 0) ? settings->current_receiver - 1 : MAX_RECEIVERS - 1;
        break;
      case 1: // Throttle Mode
        settings->throttle_bidirectional = !settings->throttle_bidirectional;
        break;
      case 2: // Trim Settings
        adjustTrim();
        break;
      case 3: // Calibration
        handleCalibrationUp();
        break;
      case 4: // Input Monitor
        // No action needed
        break;
      case 5: // System Info
        // No action needed
        break;
    }
  }
  
  void handleSubmenuDown() {
    switch(current_menu) {
      case 0: // Receiver Selection
        settings->current_receiver = (settings->current_receiver < MAX_RECEIVERS - 1) ? settings->current_receiver + 1 : 0;
        break;
      case 1: // Throttle Mode
        settings->throttle_bidirectional = !settings->throttle_bidirectional;
        break;
      case 2: // Trim Settings
        adjustTrim();
        break;
      case 3: // Calibration
        handleCalibrationDown();
        break;
      case 4: // Input Monitor
        // No action needed
        break;
      case 5: // System Info
        // No action needed
        break;
    }
  }
  
  void handleSubmenuSelect() {
    if (current_menu == 3 && submenu_level > 0) {
      // Exit calibration submenu
      submenu_level--;
      if (submenu_level == 0) {
        in_submenu = false;
      }
    } else {
      in_submenu = false;
    }
  }
  
  void enterSubmenu() {
    current_menu = menu_item;
    in_submenu = true;
    submenu_level = 0;
    
    if (current_menu == 3) {
      submenu_level = 1; // Enter calibration menu
    }
  }
  
  void adjustTrim() {
    switch(menu_item) {
      case 0: settings->trim.pitch_trim += 4; break;
      case 1: settings->trim.pitch_trim -= 4; break;
      case 2: settings->trim.roll_trim += 4; break;
      case 3: settings->trim.roll_trim -= 4; break;
      case 4: settings->trim.yaw_trim += 4; break;
      case 5: settings->trim.yaw_trim -= 4; break;
    }
    
    // Limit trim values
    settings->trim.pitch_trim = constrain(settings->trim.pitch_trim, -100, 100);
    settings->trim.roll_trim = constrain(settings->trim.roll_trim, -100, 100);
    settings->trim.yaw_trim = constrain(settings->trim.yaw_trim, -100, 100);
  }
  
  void handleCalibrationUp() {
    // Implementation for calibration up
  }
  
  void handleCalibrationDown() {
    // Implementation for calibration down
  }
  
  void render() {
    display.clearDisplay();
    
    if (in_submenu) {
      renderSubmenu();
    } else {
      renderMainMenu();
    }
    
    display.display();
  }
  
  void renderMainMenu() {
    display.setCursor(0, 0);
    display.println("MAIN MENU");
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    
    const char* menu_items[] = {
      "Receiver Select",
      "Throttle Mode",
      "Trim Settings",
      "Calibration",
      "Input Monitor",
      "System Info",
      "Save & Exit"
    };
    
    for (int i = 0; i < 7; i++) {
      if (i == menu_item) {
        display.print("> ");
      } else {
        display.print("  ");
      }
      display.println(menu_items[i]);
    }
  }
  
  void renderSubmenu() {
    switch(current_menu) {
      case 0: renderReceiverMenu(); break;
      case 1: renderThrottleMenu(); break;
      case 2: renderTrimMenu(); break;
      case 3: renderCalibrationMenu(); break;
      case 4: renderInputMonitor(); break;
      case 5: renderSystemInfo(); break;
    }
  }
  
  void renderReceiverMenu() {
    display.setCursor(0, 0);
    display.println("SELECT RECEIVER");
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    
    for (int i = 0; i < MAX_RECEIVERS; i++) {
      if (i == settings->current_receiver) {
        display.print("> ");
      } else {
        display.print("  ");
      }
      display.print(RECEIVER_NAMES[i]);
      display.print(" (");
      display.print(i + 1);
      display.println(")");
    }
  }
  
  void renderThrottleMenu() {
    display.setCursor(0, 0);
    display.println("THROTTLE MODE");
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    
    display.setCursor(0, 20);
    display.print("Current: ");
    display.println(settings->throttle_bidirectional ? "Bidirectional" : "Unidirectional");
    
    display.setCursor(0, 35);
    display.println("Press UP/DOWN to toggle");
    display.println("Press SELECT to confirm");
  }
  
  void renderTrimMenu() {
    display.setCursor(0, 0);
    display.println("TRIM SETTINGS");
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    
    display.setCursor(0, 15);
    display.print("Pitch: ");
    display.println(settings->trim.pitch_trim);
    
    display.setCursor(0, 25);
    display.print("Roll:  ");
    display.println(settings->trim.roll_trim);
    
    display.setCursor(0, 35);
    display.print("Yaw:   ");
    display.println(settings->trim.yaw_trim);
    
    display.setCursor(0, 50);
    display.println("Use buttons to adjust");
  }
  
  void renderCalibrationMenu() {
    display.setCursor(0, 0);
    display.println("CALIBRATION");
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    
    // Implementation for calibration menu
    display.setCursor(0, 20);
    display.println("Calibration menu");
    display.println("in development");
  }
  
  void renderInputMonitor() {
    display.setCursor(0, 0);
    display.println("INPUT MONITOR");
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    
    // Implementation for input monitoring
    display.setCursor(0, 20);
    display.println("Monitor all inputs");
    display.println("in real-time");
  }
  
  void renderSystemInfo() {
    display.setCursor(0, 0);
    display.println("SYSTEM INFO");
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    
    display.setCursor(0, 15);
    display.print("Battery: ");
    // Add battery reading logic
    display.println("4.2V");
    
    display.setCursor(0, 25);
    display.print("Signal: ");
    // Add signal strength logic
    display.println("Strong");
    
    display.setCursor(0, 35);
    display.print("Receivers: ");
    display.println(MAX_RECEIVERS);
  }
};

#endif