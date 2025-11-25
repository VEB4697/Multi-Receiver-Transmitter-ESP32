# Multi-Receiver RC Transmitter with ESP32

<div align="center">

![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Platform](https://img.shields.io/badge/platform-ESP32-red.svg)
![Status](https://img.shields.io/badge/status-stable-brightgreen.svg)

A professional-grade RC transmitter system with multi-receiver support, IMU integration, OLED display, and comprehensive control features.

[Features](#features) • [Hardware](#hardware-requirements) • [Installation](#installation) • [Documentation](#documentation) • [Contributing](#contributing)

</div>

---

## 🎯 Features

### Core Features
- ✅ **Multi-Receiver Support** - Pair and switch between up to 5 receivers runtime
- ✅ **Dual Joystick Control** - 4-channel primary control (Throttle, Pitch, Roll, Yaw)
- ✅ **8+ Auxiliary Channels** - 2 potentiometers, 4 toggle switches, 1 3-way switch
- ✅ **IMU Integration** - MPU6050 for gyro-stabilized flight modes
- ✅ **OLED Display** - Real-time status, menu system, telemetry
- ✅ **Trim Controls** - 6 trim buttons with 4-point adjustment
- ✅ **I/O Expansion** - PCF8575 for additional inputs
- ✅ **Calibration System** - Full joystick and sensor calibration with NVS storage
- ✅ **Failsafe** - Automatic failsafe on signal loss
- ✅ **Low Latency** - 20ms update rate, <50ms total latency

### Advanced Features
- 🔧 Bidirectional/Unidirectional throttle modes
- 🔧 Manual and Gyro-Assist control modes
- 🔧 Persistent settings storage (ESP32 NVS)
- 🔧 Runtime receiver address switching
- 🔧 Checksum validation for data integrity
- 🔧 Battery monitoring (future enhancement)

---

## 📦 Hardware Requirements

### Main Components

| Component | Quantity | Notes |
|-----------|----------|-------|
| ESP32 Development Board | 1 | 30-pin recommended |
| NRF24L01 Module | 1 | PA+LNA version recommended for range |
| MPU6050 IMU | 1 | 6-axis gyroscope + accelerometer |
| 0.96" OLED Display | 1 | 128x64, I2C, SSD1306 |
| PCF8575 I/O Expander | 1 | 16-bit I2C |
| Dual-Axis Joysticks | 2 | Analog 10kΩ |
| Potentiometers | 2 | 10kΩ linear |
| 2-Way Toggle Switches | 4 | SPDT |
| 3-Way Toggle Switches | 2 | ON-OFF-ON |
| Push Buttons | 9 | 6 for trim, 3 for menu |
| Capacitors | 1 | 10µF electrolytic for NRF24L01 |

### Optional Components
- LiPo Battery (7.4V 2S, 1000-2000mAh)
- Voltage regulator (if using battery)
- Enclosure (3D printed or custom)
- Longer-range antenna for NRF24L01

---

## 🔌 Pin Connections

<details>
<summary><b>Click to expand pin diagram</b></summary>

### ESP32 Pinout

```
I2C Bus:
├─ GPIO 21 (SDA) → OLED, MPU6050, PCF8575
└─ GPIO 22 (SCL) → OLED, MPU6050, PCF8575

NRF24L01 (SPI):
├─ GPIO 23 (MOSI)
├─ GPIO 19 (MISO)
├─ GPIO 18 (SCK)
├─ GPIO 5  (CE)
└─ GPIO 4  (CSN)

Analog Inputs:
├─ GPIO 32 → Joystick 1 X (Throttle)
├─ GPIO 33 → Joystick 1 Y (Yaw)
├─ GPIO 34 → Joystick 2 X (Roll)
├─ GPIO 35 → Joystick 2 Y (Pitch)
├─ GPIO 36 → Potentiometer 1 (AUX1)
└─ GPIO 39 → Potentiometer 2 (AUX2)

Menu Buttons:
├─ GPIO 25 → UP
├─ GPIO 26 → DOWN
└─ GPIO 27 → SELECT

Power:
├─ 3.3V → All I2C devices, NRF24L01
└─ GND  → Common ground
```

</details>

---

## 🚀 Installation

### Quick Start

1. **Install PlatformIO or Arduino IDE**
   ```bash
   # For PlatformIO
   pip install platformio
   
   # Or download Arduino IDE 2.x
   # https://www.arduino.cc/en/software
   ```

2. **Clone Repository**
   ```bash
   git clone https://github.com/yourusername/rc-transmitter-esp32.git
   cd rc-transmitter-esp32
   ```

3. **Install Dependencies**
   ```bash
   # PlatformIO (automatic)
   pio lib install
   
   # Arduino IDE (manual)
   # See docs/library-installation.md
   ```

4. **Upload Transmitter Code**
   ```bash
   # PlatformIO
   pio run -e transmitter --target upload
   
   # Arduino IDE
   # Open src/main.cpp and upload
   ```

5. **Upload Receiver Code**
   ```bash
   # PlatformIO
   pio run -e receiver --target upload
   
   # Arduino IDE
   # Open receiver/receiver.ino and upload
   ```

### Detailed Instructions

See our comprehensive guides:
- 📘 [Complete Setup Guide](docs/setup-guide.md)
- 📗 [Hardware Assembly](docs/hardware-assembly.md)
- 📙 [Software Installation](docs/software-installation.md)
- 📕 [Calibration Guide](docs/calibration-guide.md)

---

## 📚 Documentation

### User Guides
- [Getting Started](docs/getting-started.md)
- [Hardware Assembly](docs/hardware-assembly.md)
- [Pin Configuration](docs/pin-configuration.md)
- [Calibration Procedure](docs/calibration-procedure.md)
- [Menu System Guide](docs/menu-system.md)
- [Troubleshooting](docs/troubleshooting.md)

### Developer Documentation
- [Architecture Overview](docs/architecture.md)
- [Data Protocol](docs/data-protocol.md)
- [API Reference](docs/api-reference.md)
- [Contributing Guide](CONTRIBUTING.md)

---

## 🎮 Usage

### Basic Operation

1. **Power On**
   - Turn on transmitter
   - Wait for OLED to show main screen
   - Check all inputs are responding

2. **Select Receiver**
   - Press SELECT button
   - Navigate to "Receiver"
   - Choose receiver (RX1-RX5)
   - Press SELECT to confirm

3. **Fly/Drive**
   - Throttle: Left joystick vertical
   - Yaw: Left joystick horizontal
   - Pitch: Right joystick vertical
   - Roll: Right joystick horizontal

### Menu Navigation

```
Main Menu
├── Receiver
│   ├── Select Receiver (RX1-RX5)
│   └── Edit Address
├── Settings
│   ├── Throttle Mode (Uni/Bi-directional)
│   ├── Control Mode (Manual/Gyro)
│   └── Display Settings
├── Calibration
│   ├── Joysticks
│   ├── Potentiometers
│   └── IMU
└── Info
    ├── Version
    ├── Battery
    └── Statistics
```

---

## 🔧 Configuration

### Receiver Addresses

Default addresses:
```cpp
RX1: 0xE7E7E7E7E0
RX2: 0xE7E7E7E7E1
RX3: 0xE7E7E7E7E2
RX4: 0xE7E7E7E7E3
RX5: 0xE7E7E7E7E4
```

Change via menu or in code (`config.h`).

### Throttle Modes

**Unidirectional** (Default for aircraft):
- Range: 0 to 1023
- Suitable for: Planes, helis, standard cars

**Bidirectional** (For reversible ESCs):
- Range: -511 to 512
- Suitable for: Boats, 3D planes, robots

### Control Modes

**Manual Mode**:
- Direct joystick control
- No stabilization
- Lowest latency

**Gyro-Assist Mode**:
- IMU-based stabilization
- Self-leveling
- Beginner-friendly

---

## 📊 Performance

| Metric | Value |
|--------|-------|
| Update Rate | 50Hz (20ms) |
| Latency | <20ms (typical) |
| Range (Standard) | 30-50m indoor, 100m+ outdoor |
| Range (PA+LNA) | 200m+ outdoor |
| Battery Life | 8-10 hours (2000mAh) |
| Channels | 12 (8 proportional + 4 digital) |
| Resolution | 12-bit (4096 steps) |

---

## 🛠️ Troubleshooting

### Common Issues

**NRF24L01 not working:**
- Add 10µF capacitor across VCC/GND
- Check wiring (must be 3.3V)
- Reduce wire length (<10cm)
- Try different module

**Display not showing:**
- Verify I2C address (0x3C or 0x3D)
- Check SDA/SCL connections
- Add 4.7kΩ pull-up resistors

**Joystick drift:**
- Recalibrate in menu
- Check power supply stability
- Ensure joysticks are quality components

See [full troubleshooting guide](docs/troubleshooting.md) for more.

---

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guide](CONTRIBUTING.md).

### Ways to Contribute
- 🐛 Report bugs
- 💡 Suggest features
- 📝 Improve documentation
- 🔧 Submit pull requests
- ⭐ Star the repository

### Development Setup

```bash
# Fork and clone
git clone https://github.com/yourusername/rc-transmitter-esp32.git
cd rc-transmitter-esp32

# Create branch
git checkout -b feature/your-feature

# Make changes and test
pio test

# Commit and push
git commit -am "Add feature"
git push origin feature/your-feature

# Create pull request on GitHub
```

---

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## 🙏 Acknowledgments

- **RF24 Library** by TMRh20
- **Adafruit Libraries** for hardware support
- **ESP32 Community** for excellent documentation
- **Contributors** who helped improve this project

---

## 📧 Contact

- **Issues**: [GitHub Issues](https://github.com/yourusername/rc-transmitter-esp32/issues)
- **Discussions**: [GitHub Discussions](https://github.com/yourusername/rc-transmitter-esp32/discussions)
- **Email**: your.email@example.com

---

## 🗺️ Roadmap

### Version 1.1 (Planned)
- [ ] Battery voltage monitoring
- [ ] Telemetry downlink
- [ ] Model memory (save per receiver)
- [ ] Expo curves

### Version 1.2 (Future)
- [ ] Mixing functions (Elevon, V-tail)
- [ ] Wireless firmware update (OTA)
- [ ] Smartphone app (Bluetooth)
- [ ] SD card logging

### Version 2.0 (Vision)
- [ ] Custom PCB design
- [ ] 3D-printed enclosure
- [ ] Touch screen interface
- [ ] Voice alerts

---

## ⚠️ Safety Warning

**IMPORTANT:** This is a remote control system intended for hobby use.

- ⚠️ Always test thoroughly before using with any vehicle
- ⚠️ Test failsafe functionality
- ⚠️ Follow local RC regulations
- ⚠️ Never fly near people until fully tested
- ⚠️ Use proper LiPo battery safety
- ⚠️ Start with low throttle during testing

**The authors are not responsible for any damage or injury resulting from use of this system.**

---

## 📸 Gallery

<details>
<summary><b>Click to view project images</b></summary>

*Coming soon - Add your build photos!*

</details>

---

## ⭐ Star History

[![Star History Chart](https://api.star-history.com/svg?repos=yourusername/rc-transmitter-esp32&type=Date)](https://star-history.com/#yourusername/rc-transmitter-esp32&Date)

---

<div align="center">

**Made with ❤️ by the RC Community**

[⬆ Back to Top](#multi-receiver-rc-transmitter-with-esp32)

</div>