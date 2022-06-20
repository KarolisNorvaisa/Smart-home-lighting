# Smart-home-lighting
 Simple ESP32 smart home lightning solution with motion sensor and acting as a hub for RF-WIFI communication.

![IMG_20211119_152531](https://user-images.githubusercontent.com/107571973/174671767-01f5a0c2-d79c-463b-82e8-8ec133a4a264.jpg)

On PCB it has RF 433 Mhz transmitter and receiver modules. They are used to catch RF signals from smoke detectors, remotes and security sensors. Later those signals can be retransmitted via MQTT to other devices or using same RF just extending its range. This project main job is to record temperature from DS18B20, wait for interrupt from PIR sensor, control LED light's and manage signals. 

Main project advantages:

-Everyting works almost in real time using "super loop" principle, except soft stepless dimming where there is a delay. 

-If motion is sensed light is turned on for 20 sec. at 30% max brightness. Light wont activate if LDR value is above treshold.

-If someone is still in the room turn off timer is reactivated everytime someone moves.

-Lights can be controlled with a mobile phone or PC using MQTT protocol.

-2 simple SPDT buttons are used to control lights manually that works in combination with mqtt control, with coded debounce.

-Project can be used as a main comunnication hub to receive/retransmit RF-MQTT and vice versa signals. 

-If WIFI or MQTT connection is lost system tries to reconnect again waiting each time 50s in between, if after 4 reconnection cycles it still fails system is restarted.

![288360893_325548933117638_2019648095136514266_n](https://user-images.githubusercontent.com/107571973/174674358-551179a1-3738-48d3-8639-9014c2cbc0f8.jpg)
![288610901_2804633179672157_162581995424196044_n](https://user-images.githubusercontent.com/107571973/174674372-e74435c7-b823-4575-b520-aab4cc0877a7.jpg)
