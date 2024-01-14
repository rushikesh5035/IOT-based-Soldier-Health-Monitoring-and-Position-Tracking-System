#include<LiquidCrystal.h>
LiquidCrystal lcd(27, 14, 32, 33, 25, 26);  // (Rs, E, D4, D5, D6, D7)
#include <WiFi.h>
#include <ThingSpeak.h>;           //Thingspeak hidder files
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <TinyGPSPlus.h>
TinyGPSPlus gps;
#define SEND 23
#define buzzer 19
#define measure 18

const char* ssid = "”;  // your network SSID (name) 
const char* password = "”;  // your network password

unsigned long myChannelNumber = ; //your channel number
const char * myWriteAPIKey = ""; // your chennel API key

//temperature sensor
int analogtemp = A0;                     //temp analog Value
float temp= 0;             
float avgtemp=0;

//pulse oximeter
// Connections : SCL PIN - D1 , SDA PIN - D2 , INT PIN - D0
#define REPORTING_PERIOD_MS     1000
   PulseOximeter pox;
   float BPM, SpO2;
  uint32_t tsLastReport = 0;

// Timer variables
   unsigned long lastTime = 0;
   unsigned long timerDelay = 30000;

WiFiClient  client;

char mob_1[]="+919767141668";
long wait;

float long1;
float lati1;

void onBeatDetected()
{
    lcd.print("Beat Detected!");
}
void displayInfo()
{
  //Serial.print(F("Location: "));
  if (gps.location.isValid())
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Lat: ");
    lcd.print(gps.location.lat(), 6);
    //Serial.print(F(","));
    lcd.setCursor(0,1);
    lcd.print("Lng: ");
    lcd.print(gps.location.lng(), 6);
    lcd.println();
  }  

  else
  {
    //Serial.print(F("INVALID"));
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("INVALID LOCATION");
  }
  delay(200);
}

void updateSerial()
{
  delay(500);
  while (Serial.available())
  {
    Serial2.write(Serial.read());       //Forward what Serial received to Software Serial Port
  }
  while (Serial2.available())
  {
    Serial.write(Serial2.read());           //Forward what Software Serial received to Serial Port
  }
}
void beep()
{
  digitalWrite(buzzer, HIGH);
  delay(500);
  digitalWrite(buzzer, LOW);
  delay(500);
}
void(* resetFunc) (void) = 0; //declare reset function @ address 0
void setup() 
{
  Serial.begin(9600);  //Initialize serial
  Serial2.begin(9600);
  lcd.begin(16, 2);               // LCD's number of rows and colomns:
  pinMode(23, INPUT_PULLUP); pinMode(measure, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);

    //Blynk.begin(auth, ssid, pass);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Initializing....");
    lcd.setCursor(0,1);
    lcd.print("Pulse Oximeter-.");
    delay(500);

    // Connect to WiFi network
  WiFi.begin(ssid, password);
  //Serial.println("\nConnected.");
  ThingSpeak.begin(client);

 beep(); beep();
 
    if (!pox.begin()) 
    {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("     Failed     ");
        for(;;);
    } 
    else 
    {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("   Success...   ");
    }
 
     // Configure sensor to use 7.6mA for LED drive
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

  // Register a callback routine for every 1 sec
  pox.setOnBeatDetectedCallback(onBeatDetected);

   lastTime = millis();        
}

void loop() 
{
   pox.update();

    BPM = pox.getHeartRate();
    SpO2 = pox.getSpO2();

     avgtemp=0;
        for (int i = 0; i < 10; i++) 
        {
          temp= analogRead(analogtemp);   // Read ADC value
          temp = (temp*500) / 4096;       //5000 (5V), 10mV/* change, 12 bit ADC
          avgtemp=avgtemp+temp;
          delay(5);
        }
        avgtemp = avgtemp/10.0;
    
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) 
    {
        lcd.clear();
        lcd.setCursor(0,0);          
        lcd.print(" O2="); lcd.print(pox.getSpO2());  lcd.print("%");   lcd.print(" T :"); lcd.print(avgtemp);  lcd.print(" C");
        lcd.setCursor(0,1);          
        lcd.print("HeartBeat ="); lcd.print(pox.getHeartRate());
        //oxilevel=pox.getSpO2();
        BPM = pox.getHeartRate();
        SpO2 = pox.getSpO2();
        
        //tsLastReport = millis(); 
        lastTime = millis();
    } 
    if(!digitalRead(measure))
    {
    if(!digitalRead(SEND) || avgtemp >40 || avgtemp<=20 || BPM<=30 || BPM>=90 || SpO2<=85)
    {
      beep();
           // updateSerial();
      while (Serial2.available() > 0)
        if (gps.encode(Serial2.read()))
        {
          //displayInfo();
        }

        if (millis() > 5000 && gps.charsProcessed() < 10)
        {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.println(F("No GPS detected:"));
          while (true);
        }

        delay(500);
  
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Temp = ");
        //Serial.print(avgtemp);           // Print Temperature on the serial window
        //Serial.print("°C\n");
        delay(100);
 
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("  Sending Msg  ");    // lcd print
        
        Serial2.print("AT+CMGS=\""); Serial2.print(mob_1); Serial2.println("\"");
        delay(500);
        Serial2.println("Soldier Report:"); Serial2.print("\r\n");
        delay(200);
               
        Serial2.print("   Temp: "); Serial2.println(avgtemp); Serial2.print("\r\n");
        delay(500);
        Serial2.print("        HB: "); Serial2.println(BPM); Serial2.print("\r\n");
        delay(500);
        Serial2.print("    SPO2: "); Serial2.println(SpO2);  Serial2.print("\r\n");
        delay(500);
        
        Serial2.print("www.google.com/maps/@");
        Serial2.print(gps.location.lat(), 6);
        Serial2.print(",");
        Serial2.print(gps.location.lng(), 6);
        Serial2.print(",299m/data=!3m1!1e3");
        Serial2.println("\r\n");
        delay(500);
        
        Serial2.write(26);

        beep();

 delay(2000);
        pox.begin();
        if (!pox.begin()) 
  {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("     Failed     ");
        for(;;);
    } 

    lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Make  Switch Off");    // lcd print
        lcd.setCursor(0,1);
        lcd.print("   to  enable   ");    // lcd print
        delay(5000);

  resetFunc();  //call reset
    }
 //delay(10);
    }
 
wait++;
 
  if(wait==1000)
  {
    // updateSerial();
      while (Serial2.available() > 0)

        if (gps.encode(Serial2.read()))
        {
          //displayInfo();
        }
        
        if (gps.location.isValid())
        {
          lati1=gps.location.lat(); delay(200);
          long1=gps.location.lng(); delay(200);
      
          lati1=gps.location.lat(); delay(200);
          long1=gps.location.lng(); delay(200);
        }  
   
    beep();

  // set the fields with the values
      ThingSpeak.setField(1, avgtemp);
      ThingSpeak.setField(2, BPM);
      ThingSpeak.setField(3, SpO2);
       ThingSpeak.setField(4, long1);
      ThingSpeak.setField(5, lati1);

  // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
    // pieces of information in a channel.  Here, we write to field 1.
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
      delay(1100);
    //uncomment if you want to get temperature in Fahrenheit
    //int x = ThingSpeak.writeField(myChannelNumber, 1, temperatureF, myWriteAPIKey);

    if(x == 200){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Sending-> Server");    // lcd print
      delay(1000);
    }
    else{
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("    Error...    ");    // lcd print
      delay(1000);
    }
    wait=0;
    beep();
    
    pox.begin();
        if (!pox.begin()) 
    {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("     Failed     ");
        for(;;);
    } 
    
    lastTime = millis();
  }
}
