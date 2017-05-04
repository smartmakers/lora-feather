## Adeunis

This repository contains tools to work with the handheld LoRaWan device from Adeunis:
* **[adeunis](adeunis)**: Go package for the decoding of the adeunis handeld device payload.
* **[cmd/import](cmd/import)**: CLI tool to import the adeunis data from a SmartMakers server and export it in a CSV format compatible with [Google MyMaps](https://www.google.de/maps/d/). 
* **[database](database)**: Go package for the import of raw data from the SmartMakers LoRaWan server.
* **[scripts](scripts)**: Scripts for the configuration of the handheld device over serial link.

#### adeunis-import
Download the release for your system:

* Windows (AMD64): [adeunis-import.exe](/build/adeunis-import.exe)
* Linux (AMD64): [adeunis-import](/build/adeunis-import)

For usage, see:
```bash
adeunis-import --help
```