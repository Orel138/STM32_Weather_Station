<a name="readme-top"></a>

<h1 align="center">
  <br> STM32_Weather_Station <br>
</h1>

<div align="center">

[![Orel138 - STM32_Weather_Station](https://img.shields.io/static/v1?label=Orel138&message=STM32_Weather_Station&color=blue&logo=github)](https://github.com/Orel138/STM32_Weather_Station "Go to GitHub repo")
[![stars - STM32_Weather_Station](https://img.shields.io/github/stars/Orel138/STM32_Weather_Station?style=social)](https://github.com/Orel138/STM32_Weather_Station)
[![forks - STM32_Weather_Station](https://img.shields.io/github/forks/Orel138/STM32_Weather_Station?style=social)](https://github.com/Orel138/STM32_Weather_Station)

[![license](https://custom-icon-badges.demolab.com/github/license/Orel138/STM32_Weather_Station?logo=law&logoColor=white)](https://github.com/Orel138/STM32_Weather_Station/blob/main/LICENSE "license MIT")
[![issues](https://custom-icon-badges.demolab.com/github/issues-raw/Orel138/STM32_Weather_Station?logo=issue)](https://github.com/Orel138/STM32_Weather_Station/issues "issues")

[![STM32](https://img.shields.io/badge/STM32-message?style=flat&logo=stmicroelectronics&color=%2303234B)](https://st.com "STM32")

</div>

<div align="center">
  <h4>
    <a href="#about">About</a> |
    <a href="#requirements">Requirements</a> |
    <a href="#installation">Installation</a> |
    <a href="#usage">Usage</a> |
    <a href="#references">References</a> |
    <a href="#contributing">Contributing</a> |
    <a href="#license">License</a>
  </h4>
</div>

<div align="center">
  <sub>Built by
  <a href="https://orel138.github.io">Orel138</a> and
  <a href="https://github.com/orel138/STM32_Weather_Station/graphs/contributors">contributors </a>
</div>
<br>

### Weather station using STM32F769I-DISCOVERY, I-NUCLEO-LRWAN1 and FreeRTOS
This project provides an implementation of a weather station on STM32F7 microcontroller using FreeRTOS, MbedTLS, WM-SG-SM-42 LoRa module from USI, and ESP8266 Wi-Fi module.

## Table of Contents

- [About](#about)
- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
- [References](#references)
- [Contributing](#contributing)
- [License](#license)

## About

(WIP)

### Features
- (WIP)

### Project Structure
The project is structured to follow best practices for STM32 development using FreeRTOS, with a focus on modularity and reusability.

### Alignment with STMicroelectronics' STM32Cube Topology
_STM32_Weather_Station_ follows the topology of the official packages provided by STMicroelectronics, known as ["STM32Cube."](https://github.com/STMicroelectronics/STM32Cube_MCU_Overall_Offer)

This means that the project is compatible with various STM32Cube Firmware Packages such as **STM32Cube Firmware Packages** (e.g., [STM32CubeF7](https://github.com/STMicroelectronics/STM32CubeF7), [STM32CubeWB](https://github.com/STMicroelectronics/STM32CubeWB)) or **STM32Cube Extension Softwares** (e.g., [X-CUBE-AZURE](https://github.com/STMicroelectronics/x-cube-azure-telematics), [X-CUBE-FREERTOS](https://github.com/STMicroelectronics/x-cube-freertos)), enabling further expansion and customization for specific applications.

<p align="right"><a href="#readme-top">~~~~~ back to top ~~~~~</a></p>

## Architecture Overview

### Software Bill Of Materials
This Software Bill Of Materials (SBOM) itemizes the software components included in this package, detailing the copyright holder and licensing terms for each.

|   Component   |   Version |   Copyright |   License |
|  ---          |    :-:    |     :-:     |       --: |
|   CMSIS Core  |   x   |   ARM Limited |   [Apache License 2.0](https://opensource.org/license/apache-2-0) |
|   CMSIS Device STM32F7   |   x   |   ARM Limited |   [Apache License 2.0](https://opensource.org/license/apache-2-0) |
|   Drivers HAL/LL STM32F7   |   x   |   STMicroelectronics |   [BSD-3-Clause](https://opensource.org/license/BSD-3-Clause) |
|   BSP Components   |   x   |   STMicroelectronics |   [BSD-3-Clause](https://opensource.org/license/BSD-3-Clause) |
|   FreeRTOS-Kernel   |   x   |   Amazon.com |   [MIT](https://opensource.org/license/MIT) |
|   Applications projects   |   x   |   Amazon.com, STMicroelectronics, Orel138 |   [MIT](https://opensource.org/license/MIT) |


<p align="right"><a href="#readme-top">~~~~~ back to top ~~~~~</a></p>

## Requirements

### Hardware required
- [32F769IDISCOVERY Discovery kit](https://www.st.com/en/evaluation-tools/32f769idiscovery.html) - Discovery kit with STM32F769NI MCU
  - STM32F769NIH6 Arm® Cortex®‑M7 core-based microcontroller with 2 Mbytes of flash memory and 532 Kbytes of RAM.
  - 4-inch 800 x 472‑pixel capacitive touch TFT color LCD.
  - Wi‑Fi® daughterboard (for ESP8266 module).
- [I-NUCLEO-LRWAN1](https://www.st.com/en/evaluation-tools/i-nucleo-lrwan1.html) - USI® STM32™ Nucleo expansion board for LoRa™
  - USI® WM-SG-SM-42 LPWAN module supporting LoRa® technology.
  - ST relative humidity and temperature sensor (HTS221).
  - ST pressure sensor (LPS22HB).
- ESP8266

### Development toolchains and compilers
Before you begin, ensure you have the following software installed:

- STM32CubeIDE v1.14.1 or later
- STM32CubeMX v6.11.0 or later
- STM32CubeProgrammer v2.16.0 or later

<p align="right"><a href="#readme-top">~~~~~ back to top ~~~~~</a></p>

### Installation

To install STM32_Weather_Station, follow these steps:

1. Clone the repository
   ```bash
   git clone https://github.com/Orel138/STM32_Weather_Station.git
   ```
2. Navigate to the project directory
   ```bash
   cd STM32_Weather_Station
   ```
3. Initialize the submodules
   ```bash
   git submodule update --init
   ```
4. Open the project with STM32CubeIDE (open the .project or .cproject file).
Projects for STM32CubeIDE are located in Projects/{Board}/Applications/{FreeRTOS-TOTP}/STM32CubeIDE/.
5. Build the project in STM32CubeIDE.
6. Debug in the IDE or use STM32CubeProgrammer to flash the executable.
7. The executable (.elf file) will be in the Projects/{Board}/Applications/{FreeRTOS-TOTP}/STM32CubeIDE/Debug folder.
  - You can now drag and drop the .elf file to your board and see the user LED blink.
  - Or you can modify the project for your STM32 target with STM32CubeIDE and STM32CubeMX.

<p align="right"><a href="#readme-top">~~~~~ back to top ~~~~~</a></p>

## Usage

Work in Progress (WIP)

<p align="right"><a href="#readme-top">~~~~~ back to top ~~~~~</a></p>

## References

Work in Progress (WIP)

<p align="right"><a href="#readme-top">~~~~~ back to top ~~~~~</a></p>

## Contributing
We welcome your contributions to _STM32_Weather_Station_.

To contribute:
1. Fork the repository.
2. Create a new branch: git checkout -b [branch-name].
3. Make your changes and commit them: git commit -m '[commit-message]'.
4. Push to the original branch: git push origin [project-name]/[location].
5. Create the pull request.

<p align="right"><a href="#readme-top">~~~~~ back to top ~~~~~</a></p>

### License

STM32_Weather_Station is released under the [MIT license]() © [Orel138](https://github.com/Orel138).

<p align="right"><a href="#readme-top">~~~~~ back to top ~~~~~</a></p>

> [!TIP]
> I trust you'll find this project enjoyable. Should you appreciate the project, bestowing a small ⭐ on it is a meaningful gesture, signifying: **My efforts are recognized.** Your support would be greatly valued. _Many thanks!_