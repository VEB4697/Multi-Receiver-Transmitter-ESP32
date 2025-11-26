#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <RF24.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "PCF8575.h"
#include <Preferences.h>

// ======================== CONFIGURATION ========================
// Pin Definitions
#define NRF_CE_PIN    5
#define NRF_CSN_PIN   4
#define JOY1_X_PIN    32  // Throttle
#define JOY1_Y_PIN    33  // Yaw
#define JOY2_X_PIN    34  // Roll
#define JOY2_Y_PIN    35  // Pitch
#define POT1_PIN      36  // AUX1
#define POT2_PIN      39  // AUX2
#define BTN_UP_PIN    25
#define BTN_DOWN_PIN  26
#define BTN_SELECT_PIN 27

// I2C Addresses
#define OLED_ADDRESS  0x3C
#define MPU6050_ADDRESS 0x68
#define PCF8575_ADDRESS 0x20

// Display Settings
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

// PCF8575 Pin Mapping
#define PCF_AUX3      0
#define PCF_AUX4      1
#define PCF_AUX5      2
#define PCF_AUX6      3
#define PCF_AUX7_A    4
#define PCF_AUX7_B    5
#define PCF_AUX8_A    6
#define PCF_AUX8_B    7
#define PCF_TRIM_PITCH_UP   8
#define PCF_TRIM_PITCH_DN   9
#define PCF_TRIM_ROLL_UP    10
#define PCF_TRIM_ROLL_DN    11
#define PCF_TRIM_YAW_UP     12
#define PCF_TRIM_YAW_DN     13

// Constants
#define ANALOG_CENTER     2048
#define ANALOG_MAX        4095
#define DEADBAND          50
#define TRIM_STEP         4
#define TRIM_MAX          100
#define UPDATE_RATE_MS    20
#define BUTTON_DEBOUNCE   50
#define MAX_RECEIVERS     5

// ======================== DATA STRUCTURES ========================
struct TelemetryPacket {
    int16_t throttle;
    int16_t pitch;
    int16_t roll;
    int16_t yaw;
    int16_t aux1;
    int16_t aux2;
    uint8_t aux3;
    uint8_t aux4;
    uint8_t aux5;
    uint8_t aux6;
    int8_t aux7;
    int8_t aux8;
    int16_t gyroX;
    int16_t gyroY;
    int16_t gyroZ;
    int16_t accelX;
    int16_t accelY;
    int16_t accelZ;
    int16_t pitchTrim;
    int16_t rollTrim;
    int16_t yawTrim;
    uint8_t controlMode;
    uint8_t checksum;
} __attribute__((packed));

struct CalibrationData {
    int16_t joy1XMin, joy1XMax, joy1XCenter;
    int16_t joy1YMin, joy1YMax, joy1YCenter;
    int16_t joy2XMin, joy2XMax, joy2XCenter;
    int16_t joy2YMin, joy2YMax, joy2YCenter;
    int16_t pot1Min, pot1Max;
    int16_t pot2Min, pot2Max;
};

struct ReceiverAddress {
    uint8_t address[5];
    char name[16];
    bool active;
};

struct Settings {
    uint8_t currentReceiver;
    bool throttleBidirectional;
    uint8_t controlMode;  // 0: Manual, 1: Gyro-assist
    ReceiverAddress receivers[MAX_RECEIVERS];
    CalibrationData calibration;
};

// ======================== GLOBAL OBJECTS ========================
RF24 radio(NRF_CE_PIN, NRF_CSN_PIN);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_MPU6050 mpu;
PCF8575 pcf8575(PCF8575_ADDRESS);
Preferences preferences;

// ======================== GLOBAL VARIABLES ========================
TelemetryPacket txPacket;
Settings settings;
CalibrationData calibration;

// Menu system
enum MenuState {
    MENU_MAIN,
    MENU_RECEIVER,
    MENU_SETTINGS,
    MENU_CALIBRATION,
    MENU_INFO
};

MenuState currentMenu = MENU_MAIN;
int menuIndex = 0;
bool inSubMenu = false;

// Button states
unsigned long lastButtonPress = 0;
bool btnUpPressed = false;
bool btnDownPressed = false;
bool btnSelectPressed = false;

// Trim values
int16_t pitchTrim = 0;
int16_t rollTrim = 0;
int16_t yawTrim = 0;

// Timing
unsigned long lastUpdate = 0;
unsigned long lastDisplayUpdate = 0;

// ======================== FUNCTION PROTOTYPES ========================
void initHardware();
void initNRF24();
void initDisplay();
void initMPU6050();
void initPCF8575();
void loadSettings();
void saveSettings();
void readInputs();
void readAnalogInputs();
void readDigitalInputs();
void readIMU();
void applyCalibration();
void applyTrim();
void calculateChecksum();
void sendData();
void updateDisplay();
void handleButtons();
void handleMenu();
void drawMainScreen();
void drawMenu();
int16_t mapAnalog(int16_t value, int16_t inMin, int16_t inMax);
int16_t applyDeadband(int16_t value, int16_t deadband);

// ======================== SETUP ========================
void setup() {
    Serial.begin(115200);
    Serial.println("RC Transmitter Starting...");
    
    // Initialize I2C
    Wire.begin(21, 22);
    Wire.setClock(400000);
    
    // Initialize SPI
    SPI.begin(18, 19, 23, 4);
    
    // Initialize components
    initHardware();
    initDisplay();
    initNRF24();
    initMPU6050();
    initPCF8575();
    
    // Load settings from NVS
    loadSettings();
    
    // Initialize packet
    memset(&txPacket, 0, sizeof(txPacket));
    
    Serial.println("Initialization complete!");
    
    // Show splash screen
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 20);
    display.println("RC TX v1.0");
    display.setTextSize(1);
    display.setCursor(20, 45);
    display.println("Initializing...");
    display.display();
    delay(2000);
}

// ======================== MAIN LOOP ========================
void loop() {
    unsigned long currentMillis = millis();
    
    // Handle button inputs
    handleButtons();
    handleMenu();
    
    // Main update loop at specified rate
    if (currentMillis - lastUpdate >= UPDATE_RATE_MS) {
        lastUpdate = currentMillis;
        
        // Read all inputs
        readAnalogInputs();
        readDigitalInputs();
        
        if (settings.controlMode == 1) {
            readIMU();
        }
        
        // Apply calibration and trim
        applyCalibration();
        applyTrim();
        
        // Calculate checksum
        calculateChecksum();
        
        // Send data
        sendData();
    }
    
    // Update display at lower rate
    if (currentMillis - lastDisplayUpdate >= 100) {
        lastDisplayUpdate = currentMillis;
        updateDisplay();
    }
}

// ======================== HARDWARE INITIALIZATION ========================
void initHardware() {
    // Configure analog pins
    pinMode(JOY1_X_PIN, INPUT);
    pinMode(JOY1_Y_PIN, INPUT);
    pinMode(JOY2_X_PIN, INPUT);
    pinMode(JOY2_Y_PIN, INPUT);
    pinMode(POT1_PIN, INPUT);
    pinMode(POT2_PIN, INPUT);
    
    // Configure button pins with pull-up
    pinMode(BTN_UP_PIN, INPUT_PULLUP);
    pinMode(BTN_DOWN_PIN, INPUT_PULLUP);
    pinMode(BTN_SELECT_PIN, INPUT_PULLUP);
    
    // Set ADC resolution
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
}

void initNRF24() {
    if (!radio.begin()) {
        Serial.println("NRF24L01 initialization failed!");
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("NRF24 ERROR!");
        display.display();
        while (1) delay(1000);
    }
    
    radio.setPALevel(RF24_PA_MAX);
    radio.setDataRate(RF24_250KBPS);
    radio.setChannel(108);
    radio.setRetries(5, 15);
    radio.enableDynamicPayloads();
    radio.setAutoAck(true);
    
    // Open writing pipe with current receiver address
    if (settings.receivers[settings.currentReceiver].active) {
        radio.openWritingPipe(settings.receivers[settings.currentReceiver].address);
    }
    
    radio.stopListening();
    
    Serial.println("NRF24L01 initialized successfully");
}

void initDisplay() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println("SSD1306 allocation failed");
        while (1) delay(1000);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.display();
    Serial.println("Display initialized");
}

void initMPU6050() {
    if (!mpu.begin(MPU6050_ADDRESS)) {
        Serial.println("Failed to find MPU6050 chip");
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("MPU6050 ERROR!");
        display.display();
        delay(2000);
        return;
    }
    
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    
    Serial.println("MPU6050 initialized");
}

void initPCF8575() {
    pcf8575.begin();
    
    // Set pins 0-13 as inputs
    for (int i = 0; i < 14; i++) {
        pcf8575.pinMode(i, INPUT_PULLUP);
    }
    
    Serial.println("PCF8575 initialized");
}

// ======================== SETTINGS MANAGEMENT ========================
void loadSettings() {
    preferences.begin("rc-tx", false);
    
    // Load or initialize settings
    settings.currentReceiver = preferences.getUChar("currRx", 0);
    settings.throttleBidirectional = preferences.getBool("throttleBi", false);
    settings.controlMode = preferences.getUChar("ctrlMode", 0);
    
    // Load receiver addresses
    for (int i = 0; i < MAX_RECEIVERS; i++) {
        String key = "rx" + String(i);
        size_t len = preferences.getBytes(key.c_str(), 
                                          settings.receivers[i].address, 5);
        
        if (len == 0) {
            // Default addresses
            settings.receivers[i].address[0] = 0xE7;
            settings.receivers[i].address[1] = 0xE7;
            settings.receivers[i].address[2] = 0xE7;
            settings.receivers[i].address[3] = 0xE7;
            settings.receivers[i].address[4] = 0xE0 + i;
            settings.receivers[i].active = (i == 0);
            snprintf(settings.receivers[i].name, 16, "RX%d", i + 1);
        } else {
            settings.receivers[i].active = true;
            preferences.getString((key + "n").c_str(), 
                                  settings.receivers[i].name, 16);
        }
    }
    
    // Load calibration
    settings.calibration.joy1XCenter = preferences.getShort("j1xc", ANALOG_CENTER);
    settings.calibration.joy1YCenter = preferences.getShort("j1yc", ANALOG_CENTER);
    settings.calibration.joy2XCenter = preferences.getShort("j2xc", ANALOG_CENTER);
    settings.calibration.joy2YCenter = preferences.getShort("j2yc", ANALOG_CENTER);
    
    settings.calibration.joy1XMin = preferences.getShort("j1xmin", 0);
    settings.calibration.joy1XMax = preferences.getShort("j1xmax", ANALOG_MAX);
    settings.calibration.joy1YMin = preferences.getShort("j1ymin", 0);
    settings.calibration.joy1YMax = preferences.getShort("j1ymax", ANALOG_MAX);
    
    settings.calibration.joy2XMin = preferences.getShort("j2xmin", 0);
    settings.calibration.joy2XMax = preferences.getShort("j2xmax", ANALOG_MAX);
    settings.calibration.joy2YMin = preferences.getShort("j2ymin", 0);
    settings.calibration.joy2YMax = preferences.getShort("j2ymax", ANALOG_MAX);
    
    settings.calibration.pot1Min = preferences.getShort("p1min", 0);
    settings.calibration.pot1Max = preferences.getShort("p1max", ANALOG_MAX);
    settings.calibration.pot2Min = preferences.getShort("p2min", 0);
    settings.calibration.pot2Max = preferences.getShort("p2max", ANALOG_MAX);
    
    preferences.end();
    
    Serial.println("Settings loaded");
}

void saveSettings() {
    preferences.begin("rc-tx", false);
    
    preferences.putUChar("currRx", settings.currentReceiver);
    preferences.putBool("throttleBi", settings.throttleBidirectional);
    preferences.putUChar("ctrlMode", settings.controlMode);
    
    // Save calibration
    preferences.putShort("j1xc", settings.calibration.joy1XCenter);
    preferences.putShort("j1yc", settings.calibration.joy1YCenter);
    preferences.putShort("j2xc", settings.calibration.joy2XCenter);
    preferences.putShort("j2yc", settings.calibration.joy2YCenter);
    
    preferences.end();
    
    Serial.println("Settings saved");
}

// ======================== INPUT READING ========================
void readAnalogInputs() {
    // Read raw analog values
    int16_t rawThrottle = analogRead(JOY1_X_PIN);
    int16_t rawYaw = analogRead(JOY1_Y_PIN);
    int16_t rawRoll = analogRead(JOY2_X_PIN);
    int16_t rawPitch = analogRead(JOY2_Y_PIN);
    int16_t rawAux1 = analogRead(POT1_PIN);
    int16_t rawAux2 = analogRead(POT2_PIN);
    
    // Map to -511 to 512 range
    txPacket.throttle = mapAnalog(rawThrottle, 
                                   settings.calibration.joy1XMin,
                                   settings.calibration.joy1XMax);
    txPacket.yaw = mapAnalog(rawYaw - settings.calibration.joy1YCenter,
                             -ANALOG_CENTER, ANALOG_CENTER);
    txPacket.roll = mapAnalog(rawRoll - settings.calibration.joy2XCenter,
                              -ANALOG_CENTER, ANALOG_CENTER);
    txPacket.pitch = mapAnalog(rawPitch - settings.calibration.joy2YCenter,
                               -ANALOG_CENTER, ANALOG_CENTER);
    txPacket.aux1 = mapAnalog(rawAux1,
                              settings.calibration.pot1Min,
                              settings.calibration.pot1Max);
    txPacket.aux2 = mapAnalog(rawAux2,
                              settings.calibration.pot2Min,
                              settings.calibration.pot2Max);
    
    // Apply deadband to centered inputs
    txPacket.yaw = applyDeadband(txPacket.yaw, DEADBAND);
    txPacket.roll = applyDeadband(txPacket.roll, DEADBAND);
    txPacket.pitch = applyDeadband(txPacket.pitch, DEADBAND);
    
    // Handle throttle mode
    if (!settings.throttleBidirectional) {
        // Unidirectional: 0 to 1023
        txPacket.throttle = map(txPacket.throttle, -511, 512, 0, 1023);
    }
}

void readDigitalInputs() {
    // Read PCF8575 inputs
    uint16_t pcfInputs = pcf8575.digitalReadAll();
    
    // Toggle switches (AUX3-AUX6)
    txPacket.aux3 = !(pcfInputs & (1 << PCF_AUX3)) ? 1 : 0;
    txPacket.aux4 = !(pcfInputs & (1 << PCF_AUX4)) ? 1 : 0;
    txPacket.aux5 = !(pcfInputs & (1 << PCF_AUX5)) ? 1 : 0;
    txPacket.aux6 = !(pcfInputs & (1 << PCF_AUX6)) ? 1 : 0;
    
    // 3-way switch (AUX7)
    bool aux7A = !(pcfInputs & (1 << PCF_AUX7_A));
    bool aux7B = !(pcfInputs & (1 << PCF_AUX7_B));

    // 3-way switch (AUX8)
    bool aux8A = !(pcfInputs & (1 << PCF_AUX8_A));
    bool aux8B = !(pcfInputs & (1 << PCF_AUX8_B));
    
    if (aux7A && !aux7B) txPacket.aux7 = 1;
    else if (!aux7A && aux7B) txPacket.aux7 = -1;
    else txPacket.aux7 = 0;
    
    if (aux8A && !aux8B) txPacket.aux8 = 1;
    else if (!aux8A && aux8B) txPacket.aux8 = -1;
    else txPacket.aux8 = 0;
    
    // Trim buttons
    static unsigned long lastTrimUpdate = 0;
    if (millis() - lastTrimUpdate > 200) {
        if (!(pcfInputs & (1 << PCF_TRIM_PITCH_UP)) && pitchTrim < TRIM_MAX) {
            pitchTrim += TRIM_STEP;
            lastTrimUpdate = millis();
        }
        if (!(pcfInputs & (1 << PCF_TRIM_PITCH_DN)) && pitchTrim > -TRIM_MAX) {
            pitchTrim -= TRIM_STEP;
            lastTrimUpdate = millis();
        }
        if (!(pcfInputs & (1 << PCF_TRIM_ROLL_UP)) && rollTrim < TRIM_MAX) {
            rollTrim += TRIM_STEP;
            lastTrimUpdate = millis();
        }
        if (!(pcfInputs & (1 << PCF_TRIM_ROLL_DN)) && rollTrim > -TRIM_MAX) {
            rollTrim -= TRIM_STEP;
            lastTrimUpdate = millis();
        }
        if (!(pcfInputs & (1 << PCF_TRIM_YAW_UP)) && yawTrim < TRIM_MAX) {
            yawTrim += TRIM_STEP;
            lastTrimUpdate = millis();
        }
        if (!(pcfInputs & (1 << PCF_TRIM_YAW_DN)) && yawTrim > -TRIM_MAX) {
            yawTrim -= TRIM_STEP;
            lastTrimUpdate = millis();
        }
    }
}

void readIMU() {
    sensors_event_t accel, gyro, temp;
    mpu.getEvent(&accel, &gyro, &temp);
    
    // Convert to int16_t (multiply by 100 for precision)
    txPacket.gyroX = (int16_t)(gyro.gyro.x * 100);
    txPacket.gyroY = (int16_t)(gyro.gyro.y * 100);
    txPacket.gyroZ = (int16_t)(gyro.gyro.z * 100);
    txPacket.accelX = (int16_t)(accel.acceleration.x * 100);
    txPacket.accelY = (int16_t)(accel.acceleration.y * 100);
    txPacket.accelZ = (int16_t)(accel.acceleration.z * 100);
}

void applyCalibration() {
    // Already applied in readAnalogInputs()
}

void applyTrim() {
    txPacket.pitchTrim = pitchTrim;
    txPacket.rollTrim = rollTrim;
    txPacket.yawTrim = yawTrim;
    
    // Apply trim to control values
    txPacket.pitch = constrain(txPacket.pitch + pitchTrim, -511, 512);
    txPacket.roll = constrain(txPacket.roll + rollTrim, -511, 512);
    txPacket.yaw = constrain(txPacket.yaw + yawTrim, -511, 512);
}

void calculateChecksum() {
    uint8_t sum = 0;
    uint8_t* data = (uint8_t*)&txPacket;
    for (size_t i = 0; i < sizeof(txPacket) - 1; i++) {
        sum ^= data[i];
    }
    txPacket.checksum = sum;
}

void sendData() {
    txPacket.controlMode = settings.controlMode;
    
    bool success = radio.write(&txPacket, sizeof(txPacket));
    
    if (!success) {
        Serial.println("Transmission failed");
    }
}

// ======================== BUTTON HANDLING ========================
void handleButtons() {
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastButtonPress < BUTTON_DEBOUNCE) {
        return;
    }
    
    bool upState = !digitalRead(BTN_UP_PIN);
    bool downState = !digitalRead(BTN_DOWN_PIN);
    bool selectState = !digitalRead(BTN_SELECT_PIN);
    
    if (upState && !btnUpPressed) {
        btnUpPressed = true;
        lastButtonPress = currentMillis;
        menuIndex--;
        if (menuIndex < 0) menuIndex = 0;
    } else if (!upState) {
        btnUpPressed = false;
    }
    
    if (downState && !btnDownPressed) {
        btnDownPressed = true;
        lastButtonPress = currentMillis;
        menuIndex++;
    } else if (!downState) {
        btnDownPressed = false;
    }
    
    if (selectState && !btnSelectPressed) {
        btnSelectPressed = true;
        lastButtonPress = currentMillis;
        inSubMenu = !inSubMenu;
    } else if (!selectState) {
        btnSelectPressed = false;
    }
}

void handleMenu() {
    // Menu logic handled in button handler
}

// ======================== DISPLAY ========================
void updateDisplay() {
    display.clearDisplay();
    
    if (inSubMenu) {
        drawMenu();
    } else {
        drawMainScreen();
    }
    
    display.display();
}

void drawMainScreen() {
    // Title
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("TX: ");
    display.print(settings.receivers[settings.currentReceiver].name);
    
    // Channel values
    display.setCursor(0, 12);
    display.print("T:");
    display.print(txPacket.throttle);
    display.print(" P:");
    display.print(txPacket.pitch);
    
    display.setCursor(0, 22);
    display.print("R:");
    display.print(txPacket.roll);
    display.print(" Y:");
    display.print(txPacket.yaw);
    
    display.setCursor(0, 32);
    display.print("A1:");
    display.print(txPacket.aux1);
    display.print(" A2:");
    display.print(txPacket.aux2);
    
    // Trim indicators
    display.setCursor(0, 42);
    display.print("Trim P:");
    display.print(pitchTrim);
    
    display.setCursor(0, 52);
    display.print("R:");
    display.print(rollTrim);
    display.print(" Y:");
    display.print(yawTrim);
    
    // Mode indicator
    display.setCursor(90, 0);
    display.print(settings.controlMode == 0 ? "MAN" : "GYR");
}

void drawMenu() {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("=== MENU ===");
    
    const char* menuItems[] = {"Receiver", "Settings", "Calibration", "Info", "Exit"};
    int maxItems = 5;
    
    for (int i = 0; i < maxItems; i++) {
        display.setCursor(0, 12 + i * 10);
        if (i == menuIndex % maxItems) {
            display.print("> ");
        } else {
            display.print("  ");
        }
        display.println(menuItems[i]);
    }
}

// ======================== UTILITY FUNCTIONS ========================
int16_t mapAnalog(int16_t value, int16_t inMin, int16_t inMax) {
    return map(value, inMin, inMax, -511, 512);
}

int16_t applyDeadband(int16_t value, int16_t deadband) {
    if (abs(value) < deadband) {
        return 0;
    }
    return value;
}