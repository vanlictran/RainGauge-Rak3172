# RainGauge
The device uses LoRaWAN to uplink temperature, humidity and precipitation sensor data through calculation.

Support sensors:
- [Digital humidity and temperature sensor - SHTC3](https://www.sensirion.com/products/catalog/SHTC3/)

## Getting Started

### Hardware

- USB to UART Converter
- RainGauge board

### Sortware

- Arduino IDE (version v1.8.13 or above is recommended)
- RUI3 lastest firmware for RAK3172: [RAK3172-E_latest_final.hex](https://downloads.rakwireless.com/RUI/RUI3/Image/RAK3172-E_latest_final.hex)
- (STM32CubeProgammer)[https://www.st.com/en/development-tools/stm32cubeprog.html]

### Additional Libraries

- Adafruit_SHTC3.h


## Installation

### Hardware connection:

![image](https://github.com/XuanMinh201/RainGauge/assets/75436464/43590e25-210d-4a73-9fea-75715c76654b)

On the RainGauge board

![image](https://github.com/XuanMinh201/RainGauge/assets/75436464/83794843-a9d2-4d8a-8205-b86d8a1eae8a)

### In STM32CubeProgammer:
  -  Hold the **B_RAK (boot)** button and press **R_RAK (reset)** button and release the **B_RAK (boot)** button to enter bootmode.
  -  Select UART, Baudrate 115200 and press Connect.
  -  Open RAK3172-E_latest_final.hex
  -  Select the address as in following image if needed
  -  Press Download to upload firmwave
  -  After download success, press **R_RAK (reset)** button to exit the bootmode

### In Arduino IDE:
  -  Add this JSON in Additional Boards Manager URLs [\(Show me how?\)](https://support.arduino.cc/hc/en-us/articles/360016466340-Add-third-party-platforms-to-the-Boards-Manager-in-Arduino-IDE):

```  
https://raw.githubusercontent.com/RAKWireless/RAKwireless-Arduino-BSP-Index/main/package_rakwireless.com_rui_index.json
```

  -  Go to **Tool -> Board -> Boards Manager**, search & install **RAKwireless RUI STM32 Boards**
  -  Open ```ATC_Command_RF210.ino``` sketch, seletc **WisDou RAK3172-T Board** from **Tool** menu
  -  Plug in your board and upload


<!-- ![image](https://github.com/XuanMinh201/RF210/assets/75436464/141710ed-1294-46ea-9951-63bea73622ed) -->
<img src="https://github.com/XuanMinh201/RF210/assets/75436464/141710ed-1294-46ea-9951-63bea73622ed" height="450">

  -  Select **Tool -> Board -> RAKwireless RUI STM32 Modules -> WisDuo RAK3172 Evaluation Board**
    
<!-- ![image](https://github.com/XuanMinh201/RF210/assets/75436464/146c570a-ec82-45bc-ada0-89544624b861) -->
<img src="https://github.com/XuanMinh201/RF210/assets/75436464/146c570a-ec82-45bc-ada0-89544624b861" height="450">
