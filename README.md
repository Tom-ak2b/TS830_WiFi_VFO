# TS830_WiFi_VFO

![Radio image](Images/http192.168.1.180.png) 



This VFO was built and tested with the Kenwood TS-830 and no other. I cannot verify its use with any other radio.
The cable and connector were bought from https://www.k4eaa.com/ca-24.htm  All DIN connectors are not the same! If in doubt, buy this one.

![Radio image](Images/VFO_DIN.png) 

The plug wiring is as shown. The EXT VFO jack on the Ts-830 has a push switch that disconnects pin 4 from pin 1 when activated
from the shell of the external DIN plug. Wiring just the connector and "trying it out" without the shell will not work since the radio will not 
know that an external VFO is being used. 

The board I used for this project is ESP-WROOM-32 ESP32 ESP-32S Development Board 2.4GHz Dual-Mode WiFi + Bluetooth Dual Cores Microcontroller Processor Integrated with Antenna RF AMP Filter AP STA Compatible with Arduino IDE (1 PCS) https://a.co/d/02lSffuX
