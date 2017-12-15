## Periodic-Unconfirmed

#### Init the project
```
platformio init
```
If you want to generate the *CMakeLists* for CLion:
```
platformio init --ide clion
```
#### Update the libraries
Please do not forget this step before building:
```
platformio lib update
```
#### Build and upload the project
On a _Feather M0_:
```
platformio run -t upload
```
On a _Feather 32u4_:
```
platformio run -e feather_32u4 -t upload
```
#### Monitor the device output
```
platformio device monitor
```