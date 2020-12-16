# Attiny13RFDecoder
It is mostly taken from https://sites.google.com/site/sergiosprojects/tiny-rf-decoder
i just made some changes for my application
## To upload code you need to follow these steps:
1. Upload ArduinoISP sketch to an arduino uno 
2. Connent MISO,MOSI,SCK and Reset pins to Attiny13
3. Add 
  "https://raw.githubusercontent.com/sleemanj/optiboot/master/dists/package_gogo_diy_attiny_index.json"
   to Additional Boards Manager URLs and then go to Boards Manager and type attiny then install DIY ATtiny by James Sleeman 
4. Now choose Attiny13 from Board Selector
5. Make sure to Select "No Millis,No Tone" from "Millis, Tone Support" option 
6. Choose Arduino as ISP programer and upload code
7. You need to connect RF Reciver module to PB0 ,a Buzzer with a 100ohm Resistor Series to PB4 and a Push button with a 10k Pull-Up resistor to PB3 pins of Attiny13 
8. For the first time use you need to add a RF remote controller to EEPROM of the Attiny13(you can add up to 16 Remotes)
   to do this you need to press and hold push button for 3 sec then the buzzer start to beep once , now press the remote button you want to add 
9. If everything works fine you can hear the buzzer beeps 3 times , now you can use the corresponding button on your RF remote controller to toggle The Output Switch pin (PB1) for about 200ms at 9.6MHz frequency 
