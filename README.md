# Azure AZ3166 MXChip IoT DevKit SDK Update

This project is a fork of the original [Azure IoT SDK](https://github.com/microsoft/devkit-sdk) for the MXChip AZ3166 IoT DevKit for PlatformIO. The original SDK is no longer maintained, and this fork aims to provide an updated version of the SDK with the latest features.

## Features

- Removal of the board telemetry collector code as it is now defunct.

## Usage

1. Make sure you have the latest firmware on your AZ3166 board. You can find the latest firmware [here](/firmware/devkit-firmware-2.0.0.bin).  Just copy the firmware file to the root of the AZ3166 board and it will automatically flash the firmware.  
    - This is especially the case if you have tried running the Eclipse RTOS sample as it appears to change the bootloader.
1. Install VSCode and the PlatformIO extension.
1. Create a new PlatformIO project using the AZ3166 board.
1. Update the `platformio.ini` file to point to this repository.
    ```
    platform_packages =
        framework-arduinostm32mxchip@https://github.com/howardginsburg/framework-arduinostm32mxchip.git
    ```