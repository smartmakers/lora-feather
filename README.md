# LoRa Feather

This repository contains [PlatformIO](http://platformio.org/) projects for both versions of the Adafruit LoRa Feather ([LoRa Feather 32u4](https://www.adafruit.com/product/3078)  and [LoRa Feather M0](https://www.adafruit.com/product/3178)). 

**[periodic-confirmed](periodic-confirmed)**, **[periodic-unconfirmed](periodic-unconfirmed)** sends periodically a confirmed or unconfirmed payload (with the battery level and, in the confirmed case, the rssi value of the previous acknowledge).  

## Usage with PlatformIO and CLion
Initialize the PlatformIO project using the following command;
 ```
platformio init --board feather32u4 --ide clion --project-option="lib_deps=git@gitlab.com:smartmakers/lmic-arduino.git"
 ```
Then open the directory as a standard CLion project.

## Edit the LoRaWan Parameters

Each device needs three parameters in order to be able to connect to the network. In order to better keep track of each device, it is recommended to write down those parameters on a label placed on the device itself.


The **application identifier** (AppEUI) identifies a group of devices which have the same purpose. Typically, this parameter can not be changed for of-the-shelf LoRaWan devices, and it is typically unique to the manufacturer, or to a specific device model.
``` 
// APPEUI: Application ID (LSBF)
static const u1_t APPEUI[8] PROGMEM = { 0x45, 0x18, 0x00, 0xd0, 0x7e, 0xd5, 0xb3, 0x70 };
```
The **device Identifier** (DevEUI) identifies the device (same purpose as the MAC address of a computer).
It must be unique for each device.
```
// DEVEUI: Unique device ID (LSBF)
static const u1_t DEVEUI[8] PROGMEM = { 0x37, 0x41, 0x43, 0x41, 0x35, 0x30, 0x31, 0x41 };
```
The **application key** is used to validate the identity of the device when it tries to connect to the network (basically a password). It should be unique for each device and random (not all zeros, or easily guessable key).
```
// APPKEY: Device-specific AES key.
static const u1_t APPKEY[16] PROGMEM = { 0x4C, 0xB0, 0xB2, 0xBB, 0xD6, 0xBB, 0xE7, 0x62, 0x2A, 0xF7, 0xFA, 0x40, 0xC6, 0xEB, 0xB0, 0x67 };
```
