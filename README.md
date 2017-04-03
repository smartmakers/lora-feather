# Stadtwerke Arduino

**[InputsMonitor.ino](ino/InputsMonitor/InputsMonitor.ino)** - This sketch monitor the Arduino inputs (digital pins 6, 7 and 8, and analog pins 0 and 1). As long as the inputs don't change, the device transmits periodically  a confirmed packet to the server, which allows to check that the device is still functional from the server side. As soon as there is a change in the inputs, the device transmits a confirmed packet to the server (meaning that as long as the device doesn't receive an acknowledge from the server, it will transmit it again).

Note that the device has a built-in limitation to respect the local regulations in terms of maximum radio transmission time. Because of that, if some inputs of the device change multiple times in a short period of time, there might be a delay (in the order of a few minutes) before the corresponding data packet is actually transmitted.

## How to Use

> #### Device Installation Checklist
> * Edit the LoRaWan parameters.
> * Edit the behavior parameters.
> * Build and upload the software.
> * Ground the non-used inputs.

### Setup your Environment

#### Using the Official Arduino IDE

You first need to install the libraries from the [lib](lib) folder manually (see [here](https://www.arduino.cc/en/Guide/Libraries#toc4) if you don't know how to do that). Then, just open the sketch [InputsMonitor.ino](ino/InputsMonitor/InputsMonitor.ino) from the Arduino IDE.

#### Using Platformio and CLion
From the `arduino` root directory, run:
 
 ```
 $ platformio init --ide clion --board nanoatmega328
 ```
 
Then open the directory as a standard CLion project.

#### Edit the LoRaWan Parameters

Each device needs three parameters in order to be able to connect to the network. In order to better keep track of each device, it is recommended to write down those parameters on a label placed on the device itself.


The **application identifier** (AppEUI) identifies a group of devices which have the same purpose. In our case all the devices installed into the transformer stations belongs to the same application, and should then have the same APPEUI.
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

#### Edit the Behavior Parameters (optional)

The device behavior (described above) can be tweaked via its parameters. 

The **input check period** specify how often the device the input values for a change (every second by default).

```
// How often we check the input values (seconds).
#define INPUT_CHECK_PERIOD 1
```
The **maximum transmission period** specify how often the device transmits data if there is no chnage in the inputs (every ten minutes by default). Note that the actual transmission period might be higher if the device tries and transmit more frequently than what is allowed by the local radio transmission regulations.
```
// Period between two data transmissions (seconds) if there is no change in the inputs.
#define MAX_TRANSMIT_PERIOD 600
```
The **analog trigger threshold** is the minimum change on the analog inputs to trigger a transmission. This value should be in the range 0 to 255 which maps linearly to an input from 0 to 5 Volts. Hence the default value of 10 represents an actual threshold of 0.19 Volts.

```
// Minimum absolute difference in analog input for a transmission to be triggered (range 0-255).
#define ANALOG_TRIGGER_THRESOLD 10
```
#### Connect the device

For binary signals, please use the **digital inputs (D6, D7, D8) in priority** over the analog ones (A0,A1), as in the current version of the server software it might be a bit tricky to handle the values from the analog inputs on the server side.

If an analog input is not used it **should be connected to the ground** (otherwise the device will trigger false positive transmissions all the time, as the voltage on the floating inputs goes up and down).

In order to register the device on the LoRaWan server, refer to: https://smartmakers.atlassian.net/wiki/x/BAAoAQ