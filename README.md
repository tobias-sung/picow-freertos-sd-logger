This README file was generated using Claude, with a few adjustments.

# Timestamped Debug Logger with SD Card Storage for the Raspberry Pi Pico (FreeRTOS) 

This project demonstrates debug message logging on a Raspberry Pi Pico W. Messages are outputted via a software UART interface (using PIO, which frees up the hardware UART for other purposes) and saves debug messages to a text file on an SD card via SPI SD card reader. 

## Features

- **Custom PIO UART**: Debug messages are transmitted via a PIO (Programmable I/O) UART TX interface instead of the standard hardware UART. 
- **SD Card Logging**: All debug messages are automatically saved to a text file on an SD card.
- **FreeRTOS Integration**: Built on FreeRTOS for multitasking and real-time capabilities.
- **SD Card Support**: Uses the [FreeRTOS-FAT-CLI-for-RPi-Pico](https://github.com/carlk3/FreeRTOS-FAT-CLI-for-RPi-Pico) library for SD card interfacing.

## Hardware Requirements

- Raspberry Pi Pico (I used a Pico 2W for development)
- SD card reader (SPI interface)
- USB cable for programming and debug output

## Building the Project

### Prerequisites

Ensure you have the following installed:
- CMake (version 3.13 or higher)
- ARM GCC toolchain
- Pico SDK
- FreeRTOS Kernel

### Environment Setup

Set the following environment variables:
```bash
export PICO_SDK_PATH=/path/to/pico-sdk
export FREERTOS_KERNEL_PATH=/path/to/freertos-kernel
```

### Build Steps

1. Navigate to the project directory:
```bash
cd /project_directory
```

2. Create a build directory and configure:
```bash
mkdir -p build
cd build
cmake ..
```

3. Build the project:
```bash
make
```

4. The compiled binary will be generated as `blink.uf2` in the build directory.

## Loading onto Pico W

1. Connect your Pico W to your computer via USB while holding the **BOOTSEL** button.
2. The Pico will appear as a mass storage device.
3. Copy the `blink.uf2` file to the Pico:
```bash
cp build/blink.uf2 /media/YOUR_USERNAME/RPI-RP2/
```

4. The Pico will automatically reboot and start running the program.

## Usage

- Debug messages are printed via the custom PIO UART TX.
- All messages are simultaneously logged to a text file on the SD card.
- Monitor the debug output via a serial terminal (connected to the PIO UART TX pin).

## Project Structure

- `main.c` - Main program entry point
- `debug_uart.c / debug_uart.h` - Custom PIO UART implementation
- `debug_uart.pio` - PIO assembly for UART TX
- `rtc.c / rtc.h` - Real-time clock functionality
- `hw_config.c / hw_config.h` - Hardware configuration
- `libraries/` - External dependencies (FreeRTOS-FAT, DS3231)

## License

See LICENSE file for details.
