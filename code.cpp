#include <ESP8266WiFi.h>
#include <Servo.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
const char *ssid =  "AL2020GHM";     // Enter your WiFi Name
const char *pass =  "87654321"; // Enter your WiFi Password

#define MQTT_SERV "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_NAME "BackBitr"
#define MQTT_PASS "aio_MouH52yOwTXGd770KMomkuErBPAN"
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800,60000);
Servo myservo;                          //servo as gate
Servo myservos;                               //servo as gate

int carEnter = D7;                      // entry sensor 
int carExited = D6;                   //exi sensor
int slot3 = D5;                 
int slot2 = D2;
int slot1  = D1;
int count =0; 
int CLOSE_ANGLE = 145;  // The closing angle of the servo motor arm
int OPEN_ANGLE = 0;  // The opening angle of the servo motor arm              
int  hh, mm, ss;
int pos;
int pos1;

int entry1, entry2, entry3, exit1, exit2, exit3;

String h, m,EntryTimeSlot1,ExitTimeSlot1, EntryTimeSlot2,ExitTimeSlot2, EntryTimeSlot3,ExitTimeSlot3;
boolean entrysensor, exitsensor,s1,s2,s3;
float price = 2.0; //price per min

boolean s1_occupied = false;
boolean s2_occupied = false;
boolean s3_occupied = false;

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERV, MQTT_PORT, MQTT_NAME, MQTT_PASS);

//Set up the feed you're subscribing to
Adafruit_MQTT_Subscribe EntryGate = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/EntryGate");
Adafruit_MQTT_Subscribe ExitGate = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/ExitGate");

//Set up the feed you're publishing to
Adafruit_MQTT_Publish CarsParked = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/CarsParked");
Adafruit_MQTT_Publish EntrySlot1 = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/EntrySlot1");
Adafruit_MQTT_Publish ExitSlot1 = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/ExitSlot1");
Adafruit_MQTT_Publish EntrySlot2 = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/EntrySlot2");
Adafruit_MQTT_Publish ExitSlot2 = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/ExitSlot2");
Adafruit_MQTT_Publish EntrySlot3 = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/EntrySlot3");
Adafruit_MQTT_Publish ExitSlot3 = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/ExitSlot3");
Adafruit_MQTT_Publish FeesSlot1 = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/FeesSlot1");
Adafruit_MQTT_Publish FeesSlot2 = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/FeesSlot2");
Adafruit_MQTT_Publish FeesSlot3 = Adafruit_MQTT_Publish(&mqtt,MQTT_NAME "/f/FeesSlot3");

void setup() {
  delay(1000);
  Serial.begin (9600); 
  mqtt.subscribe(&EntryGate);
  mqtt.subscribe(&ExitGate);
  timeClient.begin(); 
  myservo.attach(D4);
  myservo.write(CLOSE_ANGLE);      // servo pin to D6
  // myservos.attach(D5);       // servo pin to D5
  pinMode(carExited, INPUT);    // ir as input
  pinMode(carEnter, INPUT);     // ir as input
  pinMode(slot1, INPUT);
  pinMode(slot2, INPUT);
  pinMode(slot3, INPUT);
  WiFi.begin(ssid, pass);                                     //try to connect with wifi
  Serial.print("Connecting to ");
  Serial.print(ssid);                          // display ssid
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");                          // if not connected print this
    delay(500);
  }
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP Address is : ");
  Serial.println(WiFi.localIP());                                            //print local IP address
}

void loop() {

 MQTT_connect();
 timeClient.update();
 hh = timeClient.getHours();
 mm = timeClient.getMinutes();
 ss = timeClient.getSeconds();
 h= String(hh); 
 m= String(mm);
 h +" :" + m;
 
 entrysensor= !digitalRead(carEnter);
 exitsensor = !digitalRead(carExited);
 s1 = digitalRead(slot1);
 s2 = digitalRead(slot2);
 s3 = digitalRead(slot3);
 
   if (entrysensor == 1 && count < 3) {                     // if high then count and send data
   count=  count+1;                             //increment count
   myservo.write(OPEN_ANGLE);
   delay(3000);
   myservo.write(CLOSE_ANGLE);
   }
   
   if (exitsensor == 1 && count > 0) {                            //if high then count and send
   count= count-1;                                    //decrement count
   myservo.write(OPEN_ANGLE);
   delay(3000);
   myservo.write(CLOSE_ANGLE);
  }
  if (! CarsParked.publish(count)) {}

  if (s1 == 0 && s1_occupied == false) {                     
        Serial.print("Occupied1 ");
        EntryTimeSlot1 =  h +" :" + m;
        entry1 = hh*60 + mm;
        // Serial.print("EntryTimeSlot1");
        Serial.println(EntryTimeSlot1);
        Serial.println(entry1);
        s1_occupied = true;
        if (! EntrySlot1.publish((char*) EntryTimeSlot1.c_str())){}
    }
        
  if(s1 == 1 && s1_occupied == true) {
       Serial.print("Available1 ");
       exit1 = hh*60 + mm;
       ExitTimeSlot1 =  h +" :" + m;
      //  Serial.print("ExitTimeSlot1");
       Serial.println(ExitTimeSlot1);
        float fees = parking_fees(entry1, exit1, price);
        String feeStr = String(fees);
        Serial.print("Parking Fees: "); 
        Serial.println(fees);
       s1_occupied = false;
        if (! ExitSlot1.publish((char*) ExitTimeSlot1.c_str())){} 
        if (! FeesSlot1.publish((char*) feeStr.c_str())){} 
    }
  if (s2 == 0 && s2_occupied == false) {                     
      Serial.print("Occupied2 ");
      EntryTimeSlot2 =  h +" :" + m;
      entry2 = hh*60 + mm;
      // Serial.print("EntryTimeSlot2");
      Serial.println(EntryTimeSlot2);
     
      s2_occupied = true;
      if (! EntrySlot2.publish((char*) EntryTimeSlot2.c_str())){}
    }
  if(s2 == 1 && s2_occupied == true) {
       Serial.print("Available2 ");
       ExitTimeSlot2 =  h +" :" + m;
       exit2 = hh*60 + mm;
       //Serial.print("ExitTimeSlot2");
       Serial.println(ExitTimeSlot2);
       float fees = parking_fees(entry2, exit2, price);
        Serial.println(fees);
        String feeStr = String(fees);
       s2_occupied = false;
        if (! ExitSlot2.publish((char*) ExitTimeSlot2.c_str())){}
        if (! FeesSlot2.publish((char*) feeStr.c_str())){} 
  }
  if (s3 == 0 && s3_occupied == false) {                     
      Serial.print("Occupied3 ");
      EntryTimeSlot3 =  h +" :" + m;
      entry3 = hh*60 + mm;
     //Serial.print("EntryTimeSlot3: ");
      Serial.println(EntryTimeSlot3);
      s3_occupied = true;
       if (! EntrySlot3.publish((char*) EntryTimeSlot3.c_str())){}
    }
  if(s3 == 1 && s3_occupied == true) {
       Serial.print("Available3 ");
       ExitTimeSlot3 =  h +" :" + m;
       exit3 = hh*60 + mm;
       //Serial.print("ExitTimeSlot3: ");
       Serial.println(ExitTimeSlot3);
       float fees = parking_fees(entry3, exit3, price);
       String feeStr = String(fees);
        Serial.println(fees);
       s3_occupied = false;
        if (! ExitSlot3.publish((char*) ExitTimeSlot3.c_str())){ }
        if (! FeesSlot3.publish((char*) feeStr.c_str())){} 
  } 

  Adafruit_MQTT_Subscribe * subscription;
  while ((subscription = mqtt.readSubscription(5000)))
  {
    
   if (subscription == &EntryGate)
     {
      //Print the new value to the serial monitor
      Serial.println((char*) EntryGate.lastread);
     
   if (!strcmp((char*) EntryGate.lastread, "ON"))
      {
       myservos.write(OPEN_ANGLE);
       delay(3000);
       myservos.write(CLOSE_ANGLE);
      }
    }
    if (subscription == &ExitGate)
      {
        //Print the new value to the serial monitor
        Serial.println((char*) EntryGate.lastread);
      
        if (!strcmp((char*) ExitGate.lastread, "ON"))
            {
            myservo.write(OPEN_ANGLE);
            delay(3000);
            myservo.write(CLOSE_ANGLE);
            }
        }
  }  
}
void MQTT_connect() 
{
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) 
  {
    return;
  }

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) // connect will return 0 for connected
  { 
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) 
       {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
}

float parking_fees(int entry, int exit, float price) {

  // int entryHour = EntryTimeSlot.substring(0, 2).toInt();
  // int entryMinute = EntryTimeSlot.substring(4).toInt();
  // int exitHour = ExitTimeSlot.substring(0, 2).toInt();
  // int exitMinute = ExitTimeSlot.substring(4).toInt();

  // int entryTimeMinutes = entryHour * 60 + entryMinute;
  // int exitTimeMinutes = exitHour * 60 + exitMinute;

  // Calculate time difference in minutes
  int timeDifferenceMinutes = exit - entry;
  
  float fees = timeDifferenceMinutes * price;
  Serial.print("from parking fees");
  // Serial.println(entryTimeMinutes);
  // Serial.println(exitTimeMinutes);
  Serial.println(timeDifferenceMinutes);
  Serial.println(fees);
  return fees;
}