# SDAnalyst - Professional SD Card Analysis Tool

[![Version](https://img.shields.io/badge/version-1.3.2-blue.svg)](https://github.com/yourusername/SDAnalyst)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Raspberry%20Pi%20Pico-red.svg)](https://www.raspberrypi.org/products/raspberry-pi-pico/)

A professional-grade SD card analysis tool for the Raspberry Pi Pico that provides comprehensive filesystem analysis, partition table parsing, and Unix-style directory listings.

## ✨ Features

### 🔍 **Comprehensive Analysis**
- **MBR & GPT Partition Tables** - Full support for both legacy MBR and modern GPT partitioning
- **Multiple Filesystems** - FAT12/16/32 analysis with exFAT and ext2/3/4 detection
- **Long Filename Support** - Complete LFN parsing with 8.3 fallback display
- **Professional Output** - Unix-style `ls -l` formatted directory listings

### 📊 **Advanced Features**
- **Real-time Analysis** - Live SD card analysis via USB serial
- **Hex Dump Viewer** - Raw sector data display for forensic analysis
- **Date/Time Parsing** - FAT timestamp conversion to human-readable format
- **File Attributes** - Complete permission and attribute analysis
- **Size Calculations** - Automatic size conversion (bytes/KB/MB/GB)

### 🛠️ **Hardware Integration**
- **SPI Interface** - High-speed SD card communication
- **Configurable Pins** - Customizable GPIO pin assignment
- **USB Serial Output** - Real-time results via USB connection
- **Error Handling** - Comprehensive error detection and reporting

## 🚀 Quick Start

### Hardware Requirements
- Raspberry Pi Pico
- SD card slot/breakout board
- Jumper wires

### Wiring Diagram
```
Pico Pin | SD Card Pin | Function
---------|-------------|----------
GPIO 2   | CLK         | SPI Clock
GPIO 3   | CMD         | SPI MOSI
GPIO 4   | DAT0        | SPI MISO
GPIO 5   | DAT3        | SPI CS
3.3V     | VDD         | Power
GND      | VSS/GND     | Ground
```

### Building the Firmware

1. **Prerequisites**
   ```bash
   # Install Pico SDK
   sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential
   
   # Set up Pico SDK environment
   export PICO_SDK_PATH=/path/to/pico-sdk
   ```

2. **Build Process**
   ```bash
   mkdir build
   cd build
   cmake ..
   make -j4
   ```

3. **Flash to Pico**
   ```bash
   ./flash_pico.sh
   ```

## 📋 Usage

1. **Connect Hardware** - Wire SD card to Pico according to diagram
2. **Flash Firmware** - Upload the compiled `.uf2` file
3. **Insert SD Card** - Place card in slot
4. **Connect Serial** - Use terminal program (115200 baud, 8N1)
5. **View Results** - Analysis starts automatically on power-up

### Sample Output
```
********************************************************************************
Raspberry Pi Pico SD Card Reader
Version: 1.3.2
Built: Aug 11 2025 19:25:50
================================

SD Card Information:
Type: SDHC
Capacity: 512.00 MB (1048576 blocks)

=== Directory listing for / ===
drwxrwxrwx  1      <DIR> Jul 12 19:44 System Volume Information [SYSTEM~1]
-rw-rw-rw-  1       1024 Jul 28 16:06 CPMIDE.ID
-rw-rw-rw-  1  515579904 Jul 12 14:35 RomWBW-3.6.1-dev10.img [ROMWBW~1.IMG]
total 553677
3 files and directories
```

## 🔧 Configuration

### Pin Configuration
Edit `src/main.c` to change GPIO pins:
```c
#define SD_PIN_SCK  2  // SPI Clock
#define SD_PIN_MOSI 3  // SPI MOSI  
#define SD_PIN_MISO 4  // SPI MISO
#define SD_PIN_CS   5  // SPI Chip Select
```

### Serial Settings
- **Baud Rate**: 115200
- **Data Bits**: 8
- **Parity**: None
- **Stop Bits**: 1
- **Flow Control**: None

## 📁 Project Structure

```
SDAnalyst/
├── src/
│   ├── main.c           # Main application logic
│   ├── sd_card.c        # SD card SPI communication
│   ├── sd_card.h        # SD card interface header
│   └── fatfs_disk.c     # FAT filesystem support
├── CMakeLists.txt       # CMake build configuration
├── flash_pico.sh        # Flashing utility script
├── connect_pico.ps1     # Windows connection script
└── README.md           # This file
```

## 🛡️ Supported Filesystems

| Filesystem | Status | Features |
|------------|--------|----------|
| FAT12      | ✅ Full | Directory listing, LFN, dates |
| FAT16      | ✅ Full | Directory listing, LFN, dates |
| FAT32      | ✅ Full | Directory listing, LFN, dates |
| exFAT      | 🔍 Detection | Identification only |
| ext2/3/4   | 🔍 Detection | Identification only |

## 🧪 Partition Table Support

- **MBR (Master Boot Record)** - Complete parsing and analysis
- **GPT (GUID Partition Table)** - Full header and entry parsing
- **Protective MBR** - Automatic GPT detection
- **Multiple Partitions** - Analysis of all partition entries

## 🔍 Troubleshooting

### Common Issues

**SD Card Not Detected**
- Check wiring connections
- Verify 3.3V power supply
- Ensure card is properly inserted
- Try different SD card

**Compilation Errors**
- Verify Pico SDK installation
- Check PICO_SDK_PATH environment variable
- Ensure cross-compiler is installed

**Serial Connection Issues**
- Check USB cable connection
- Verify COM port selection
- Confirm baud rate (115200)

## 📊 Technical Specifications

- **MCU**: RP2040 (Raspberry Pi Pico)
- **Interface**: SPI (100kHz initialization, supports higher speeds)
- **Supported Cards**: SD, SDHC, SDXC
- **Output**: USB Serial (CDC)
- **Memory Usage**: ~32KB Flash, ~8KB RAM
- **Analysis Speed**: ~1-2 seconds for typical cards

## 🤝 Contributing

Contributions are welcome! Please feel free to submit pull requests, report bugs, or suggest features.

### Development Setup
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **Raspberry Pi Foundation** - For the excellent Pico platform
- **SD Association** - For comprehensive SD card specifications
- **FAT Specification** - Microsoft FAT filesystem documentation
- **GPT Specification** - UEFI Forum GPT documentation

## 📞 Support

For support, questions, or suggestions:
- Open an issue on GitHub
- Check the troubleshooting section
- Review the wiring diagram

---

**SDAnalyst** - Professional SD card analysis made simple! 🔍✨
