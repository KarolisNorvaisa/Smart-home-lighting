#include <Arduino.h>
#include <analogWrite.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <RCSwitch.h>
#include <OneWire.h>
#include <DallasTemperature.h>
Ticker ticker;



int temp;
int LED = 2;
int LEDB = 15;

#define ONE_WIRE_BUS 27
#define M1  12
#define M2  14
#define M3  13
#define LDRpin  34
#define PIRpin 25
#define Switchpin1  32
#define Switchpin2  33

int RFPAYLOAD = 0;

bool USE_PIR = true;
bool PIR_ACTIVE = false;
bool Dimmer1_on_switch = false;
bool Dimmer2_on_switch = false;
////////////INPUT//////////////////////////
bool NightLight_inside = false;
bool NightLight_outside = false;
bool stateS1 = false;
bool stateS2 = false;

bool needtochange = false;
bool ReadyToRead = true;
bool ReadyToRead2 = true;
bool PIR_Timerioflagas1=true;
bool TurnOFF_inside     = false;
bool TrunOFF_outside   = false;
bool TurnON_inside     = false;
bool published= false;
bool TurnON_outside = false;
bool switch1pre = false;
bool switch2pre = false;
bool switch1now = true;
bool switch2now = true;
bool Dimmer1_on_pir = false;
bool PIR_ACTIVE_again=false;
bool Flag1=true;
///////////OUTPUT//////////////////////////
bool PirInside = false;
byte D1 = 100;
byte D2 = 100;
byte D3 = 100;
///////////////////////////////////////////
long passed = 0;
byte Reconnect_MQTT_Attempts=0, Reconnect_WIFI_Attempts=0;
long passed2 = 0;
unsigned long lastReconnectAttempt = 0, lastReconnectAttempt_MQTT=0,lastReconnectAttempt_WIFI=0,timeSinceLastRead=0, SinceLastHeartbeat=0, LightON_Time=0;
int datat = 0;

long LDR_time , Dimmer1_on_time, Timer2;
int LDR_Value;



const char *mqtt_broker = "broker.hivemq.com";
const char *topic = "pastrevys/namai";
const char *mqtt_username = "karolis";
const char *mqtt_password = "223";
const int   mqtt_port = 1883;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
RCSwitch mySwitch = RCSwitch();
RCSwitch mySwitchT = RCSwitch();
WiFiClient espClient;
PubSubClient client(espClient);
  WiFiManager wm;

void IRAM_ATTR PIR_ACTIVADED(){
      PIR_ACTIVE = true;

}
void setup() {
  Serial.begin(9600);
  mySwitch.enableReceive(22);
  mySwitchT.enableTransmit(23);
  mySwitchT.setRepeatTransmit(3);
  analogWriteFrequency(1000); 
  sensors.begin();

  pinMode(LEDB, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(M1, OUTPUT); 
  pinMode(M2, OUTPUT);
  pinMode(M3, OUTPUT);
  pinMode(Switchpin1, INPUT);
  pinMode(Switchpin2, INPUT);
  pinMode(LDRpin, INPUT);
  pinMode(PIRpin, INPUT_PULLDOWN);

  attachInterrupt(digitalPinToInterrupt(PIRpin), PIR_ACTIVADED, RISING);

  
  ticker.attach(0.6, tick);
  // wm.resetSettings();
  wm.setConfigPortalTimeout(60); 
  wm.setAPCallback(configModeCallback);
  if (!wm.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    ESP.restart();
    delay(1000);
  }

  Serial.println("connected...yeey :)");
  ticker.detach();
 
  Serial.println("Connected to the WiFi network");
  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("mqtt broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  // publish and subscribe
  client.subscribe(topic);
  client.subscribe("pastrevys/namai2");
  LDR_time = millis();
}



boolean reconnect() {
  String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
  if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
    // Once connected, publish an announcement...
  client.subscribe("pastrevys/namai2");
    client.subscribe(topic);
  }
  return client.connected();
}

void callback(char *topic, byte *payload, unsigned int length) {
  tick();
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char) payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");

  datat += ((char)payload[0] - '0') * 100;
  datat += ((char)payload[1] - '0') * 10;
  datat += ((char)payload[2] - '0');
  proc (datat);
  datat = 0;
tick();

}

void tick() {
  //toggle state
  digitalWrite(LED, !digitalRead(LED));     
}
void tick1() {
  //toggle state
  digitalWrite(LEDB, !digitalRead(LEDB));     // set pin to the opposite state
}
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
  ticker.attach(0.2, tick);
}

int TempDS18B20(){
    sensors.requestTemperatures();
    return (sensors.getTempCByIndex(0));
}




int proc (int payload) {

  if (payload >= 100 and payload <= 199) {
    Dimmer1 (payload);
  }
  if (payload >= 200 and payload <= 299) {
    Dimmer2 (payload);
  }
  if (payload >= 300 and payload <= 399) {
    Dimmer3 (payload);
  }
  if (payload >= 400 and payload <= 499) {
    Tasker (payload);
  }

}

void Dimmer1(int pay) {
  byte dim = map(pay, 100, 199, 0, 255);
  
    for (int i=D1; i<=dim; i++){
    analogWrite(M1, i);
    delayMicroseconds(200);
    Serial.println("dimmeris1++");
    }
  for (int i=D1; i>=dim;i--){
    analogWrite(M1, i);
    delayMicroseconds(200);
    Serial.println("dimmeris1--"); }
  
  Serial.print("Dimmer1 val= "); Serial.println(D1);
  pub (pay,"pastrevys/namai/DIMMER1_STATUS");
    D1 = dim;
    dim = 0;
  

}
void Dimmer2(int pay) {
  byte i = map(pay, 200, 299, 0, 255); 

   for (int i1=D2; i1<=i; i1++){
    analogWrite(M2, i1);
    delay(0.6);
    Serial.println("dimmeris2++");
    }
  for (int i1=D2; i1>=i; i1--){
    analogWrite(M2, i1);
    delay(0.6);
    Serial.println("dimmeris2--"); }
    pub (pay,"pastrevys/namai/DIMMER2_STATUS");

  Serial.print("Dimmer2 val="); Serial.println(D2);
   D2 = i;
   i = 0;
}
void Dimmer3(int pay) {
  byte i = map(pay, 300, 399, 0, 255);  analogWrite(M3, i);
  D3 = i;
  Serial.print("Dimmer3 val="); Serial.println(D3);
}

void Tasker(int pay) {

  Serial.print("Tasker:: ");
  Serial.print(pay);
  Serial.println("");
  switch (pay) {
    case 401:     Dimmer1(199);               break;   //Ijungti 1 dimmeri MAX
    case 402:     Dimmer2(299);               break;   //Ijungti 2 dimmeri MAX
    case 403:     NightLight_inside = true;   break;   //Ijungti 1 dimmeri NIGHTLIGHT
    case 404:     NightLight_inside = false;  break;   //ISjungti 1 dimmeri NIGHTLIGHT
    case 405:     NightLight_outside = true;  break;   //Ijungti 2 dimmeri NIGHTLIGHT
    case 406:     NightLight_outside = false; break;   //ISjungti 2 dimmeri NIGHTLIGHT
    case 407:     Dimmer1(100);               break;   //ISjungti 1 dimmeri
    case 408:     Dimmer2(200);               break;   //ISjungti 2 dimmeri
    case 409:     PIR_ACTIVE = true;          break;   //Testavimui Aktyvuoti PIR dirbtinai
    case 410:     USE_PIR = false;            break;   //Deaktyvuoti  PIR VEIKIMA
    case 411:     ESP.restart();              break;   // Restartuoti sistema
  }
}



void change (byte i) {
  if   (i == 1) {
    if (D1 < 150) {
      Dimmer1(199);
      Dimmer1_on_switch = true;
    }
    else if (D1 > 150) {
      Dimmer1(100);
      Dimmer1_on_switch = false;
    }

  }
   if (i == 2) {
    if (D2 < 150) {
      Dimmer2(299);
      Dimmer2_on_switch = true;
    }
    else if (D2 > 150) {
      USE_PIR == true;
      Dimmer2(200);
      Dimmer2_on_switch = false;
    }
  }
}

void RFdecode(int Data){
  
  if (Data>10000 and Data<10100) pub((Data-10000),"pastrevys/namai/temperatura1");
  if (Data>20000 and Data<20100) pub((Data-20000),"pastrevys/namai/temperatura2");
  if (Data>30000 and Data<30100) pub((Data-30000),"pastrevys/namai/temperatura3");
  if (Data>40000 and Data<40100) pub((Data-40000),"pastrevys/namai/temperatura4");
  if (Data==4007926 and published==false){pub(1, "pastrevys/namai/durys");   published=true;}
  if (Data==2293506 and published==false){pub(605, "pastrevys/namai/virtuve/com");   published=true; Dimmer1(199);}
  if (Data==2293508 and published==false){pub(606, "pastrevys/namai/virtuve/com");   published=true; Dimmer1(100);}

  
  }
  void reconnect_WIFI(){
  
    long now = millis();
    Serial.println("Lost WIFI");
    if (now-lastReconnectAttempt_WIFI>50000){
       Serial.println("Trying to reconnect");
      lastReconnectAttempt_MQTT = now;
      Reconnect_MQTT_Attempts++;
      Serial.println("Reconnect attempts");
      Serial.println(Reconnect_MQTT_Attempts);  
       WiFi.persistent (false);
       Serial.println("Disconnect wifi");
       delay(1000);
       wm.autoConnect("Virtuves_Valdiklis");
       Serial.println("Autoconnect");
       delay(1000);   
    if (WiFi.status() == WL_CONNECTED){lastReconnectAttempt_WIFI=0;
     lastReconnectAttempt_WIFI = 0;
     Reconnect_WIFI_Attempts=0;
     reconnect();
     Serial.println("WIFI reconnect success");

    }}
   if (Reconnect_WIFI_Attempts>3){
    Serial.println("Reconnect unsuccessful, restarting ESP");
    ESP.restart();
          

  }  }
void reconnect_MQTT(){
  long now = millis();
  if (now - lastReconnectAttempt_MQTT > 5000) {
    lastReconnectAttempt_MQTT = now;
    Reconnect_MQTT_Attempts=+1;
  if (reconnect()) {
    lastReconnectAttempt_MQTT = 0;
    Reconnect_MQTT_Attempts=0;}  }
  if (Reconnect_MQTT_Attempts>3){
    ESP.restart();
  }
}

void update_values(){
  
  //pub (D1,pastrevys/namai/DIMMER1_STATUS
  
}

void STATELED() {
  if (WiFi.status() == 3) {
    digitalWrite(LED, LOW);
  }
  else  digitalWrite(LED, HIGH);
  if (client.state() == 0) {
    digitalWrite(LEDB, LOW);
  }
  else  digitalWrite(LEDB, HIGH);
}

boolean pub (int DATA, char *topikas){
    tick1();
    String temp_str;
    char temps[50];
    temp_str=(String)DATA;
    temp_str.toCharArray(temps, temp_str.length() + 1);    
    Serial.print("Publishinta I ");
    Serial.print(topikas);
    Serial.print(" reiksme: ");
    Serial.println(temps);
    tick1();
    return client.publish(topikas,temps );
}

void loop() {
  if (mySwitch.available()) {
    RFPAYLOAD = mySwitch.getReceivedValue();
    Serial.print("GAUTA PER RF: "); Serial.println(RFPAYLOAD);
    RFdecode(RFPAYLOAD);
    pub (RFPAYLOAD,"pastrevys/namai/hub/RF");
  } 

////////////////////////////////Mygtukai//////////////////////
  if (ReadyToRead == true ) {
    stateS1 = digitalRead(Switchpin1);
    passed = millis();
    ReadyToRead = false;
    
  }
  if (stateS1 != switch1pre) {
    switch1pre = stateS1;
    change(1);
    Serial.println("MYGTUKAS1");
  }
  if (ReadyToRead2 == true ) {
    stateS2 = digitalRead(Switchpin2);
    passed2 = millis(); 
    ReadyToRead2 = false;
    //
  }
  
  if (stateS2 != switch2pre) {
    switch2pre = stateS2;
    change(2);Serial.println("MYGTUKAS2");
    
      //   
      }
  if (millis()-passed>250){ReadyToRead=true;}
  if (millis()-passed2>250){ReadyToRead2=true;}
////////////////////////////////////////////////////////////


  if (PIR_ACTIVE == HIGH) {
    pub(607, "pastrevys/namai/virtuve/com");
  }


// TIMERIS
  if (millis() - LDR_time >= 5000) { 
    if (Dimmer1_on_pir == false){LDR_Value = analogRead(LDRpin);}
    LDR_time = millis();
    STATELED();
    published=false; // atstatomas publishinimo 1 karta flagas
    }
    //Serial.println (LDR_Value);
  if (millis() - Timer2 >= 30000){
    Serial.println (TempDS18B20());
    pub (TempDS18B20(),"pastrevys/namai/koridorius/temp");
        Timer2 = millis();

      ///////////////////////////////
  //NIGHTLIGHT//////////
  if (NightLight_inside and D1<2 and LDR_Value > 800){Dimmer1(102);}
  if (NightLight_outside and D2<20 and LDR_Value > 500){Dimmer2(202);}
    
  }

  if (PIR_ACTIVE == true and USE_PIR == true) {
    if (LDR_Value > 500 and Dimmer1_on_pir == false and D1 <12 ) { 
      Dimmer1_on_pir = true;
      Dimmer1_on_time = millis();
      Dimmer1(130);
      PIR_ACTIVE=false;
      
    }

  }


  if (Dimmer1_on_pir and PIR_ACTIVE == true){
    Dimmer1_on_time = millis();
    // atnaujiname automatinio isjungimo timeri, jeigu vel suveike pir (reiskia kazkas vis dar patalpoje)
    PIR_ACTIVE=false;
  }
  
  if (Dimmer1_on_pir == true and millis() - Dimmer1_on_time >= 10000 and D1<90) {
    Dimmer1(100);
    Dimmer1_on_pir = false;
    PIR_ACTIVE = false;
    Serial.println("PIROUT");
    pub (1,"pastrevys/namai/PIR_STATUS");
    PIR_Timerioflagas1=true;

  }
  mySwitch.resetAvailable();
  client.loop();


  if ((!client.connected()) and (WiFi.status() == WL_CONNECTED) ) {
  reconnect_MQTT();}
// jei mqtt neveikia o wifi veikia tada pradedam mqtt prisijungimo cikla.

  if (WiFi.status() != WL_CONNECTED){
    reconnect_WIFI();
 // jei wifi atsijunges pradedam prisijungima vel.
  }
  
}

/*
                TOPIKAI
                pastrevys/namai/temperatura1
                pastrevys/namai/temperatura2
                pastrevys/namai/temperatura3
                pastrevys/namai/temperatura4
                pastrevys/namai/temperatura5
                pastrevys/namai/PIR_STATUS
                pastrevys/namai/DIMMER1_STATUS
                pastrevys/namai/DIMMER2_STATUS 
               
                
*/
