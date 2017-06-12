# Feather Platformio Projects

This directory contains all the [PlatformIO](http://platformio.org/) projects for both versions of the Adafruit LoRa Feather ([LoRa Feather 32u4](https://www.adafruit.com/product/3078)  and [LoRa Feather M0](https://www.adafruit.com/product/3178)). 

* **[loralab](loralab)** iterates trough sending different types of payloads and resets the device from time to time. For continuous functionnal testing of the developer instances. 

* **[periodic-confirmed](periodic-confirmed)**, **[periodic-unconfirmed](periodic-unconfirmed)** sends periodically a confirmed or unconfirmed payload (with the battery level and, in the confirmed case, the rssi value of the previous acknowledge).  

* **[push-button](push-button)** turns the led on the push of a button.
