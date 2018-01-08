## Periodic-Unconfirmed

This implementation transmits periodically an unconfirmed LoRaWAN message: either at a preconfigured period (30 seconds by default), or as often as is allowed by the radio duty-cycle regulations (around every 2 minutes at SF12 if you don't cheat). 

#### Initialize the Platformio project

In the directory containing the `platformio.ini` configuration file, run the command: 

```
platformio init
```

If you want to work on this project using the [CLion](https://www.jetbrains.com/clion/) IDE, you can also directly generate the required `CMakeLists.txt` by running the following command instead :

```
platformio init --ide clion
```

#### Update the dependencies

You can update the external dependencies with the following command: 

```
platformio lib update
```

#### Build and upload the project to the Feather

With the Feather connected, you can now build and upload the software:

For a _Feather M0_:

```
platformio run -t upload
```

For a _Feather 32u4_:

```
platformio run -e feather_32u4 -t upload
```

#### Monitor the device output

The device has a debugging output which you can read using:

```
platformio device monitor
```