## Periodic-Confirmed

#### Init the project
```
platformio init
```
If you want to generate the *CMakeList* for CLion:
```
platformio init
```
#### Build and upload the project
On a _Feather M0_:
```
platformio run -e feather_32u4 -t upload
```
On a _Feather 32u4_:
```
platformio run -e feather_m0 -t upload
```
#### Monitor the device output
```
platformio device monitor
```