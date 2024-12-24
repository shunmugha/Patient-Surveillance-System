#include <LiquidCrystal.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include "SoftwareSerial.h"
 
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

SoftwareSerial gsm(9,10); //Rx, Tx

const int switchPin = 8;

int tempePin = A0;

int tempeValue = 0;

unsigned char switchvalue = 0;
unsigned char TempeHighBit = 0;
unsigned char TempeLowBit = 0;
unsigned char Message1SentBit = 0;
unsigned char Message2SentBit = 0;
unsigned char ReadHB_Bit = 0;
unsigned char Count = 0;
 
#define REPORTING_PERIOD_MS     1000
 
PulseOximeter pox;
uint32_t tsLastReport = 0;
 
void onBeatDetected()
{      
  //Serial.println("Beat!");        
}
 
void setup()
{
  Serial.begin(9600);
  gsm.begin(9600);

  pinMode(switchPin, INPUT);
  
  lcd.begin(16,2);
  lcd.print("Initializing....");
  lcd.setCursor(0,1);
  lcd.print("  IoT  &  GSM   ");
  delay(5000);  
  delay(5000);

  lcd.clear();  

  // Initialize the PulseOximeter instance
  // Failures are generally due to an improper I2C wiring, missing power supply
  // or wrong target chip
  if (!pox.begin()) 
  {
    //Serial.println("FAILED");        
    for(;;);
  } 
  else 
  {
    //Serial.println("SUCCESS");             
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

  // Register a callback for the beat detection
  pox.setOnBeatDetectedCallback(onBeatDetected);  
}
 
void loop()
{
  Start:
  ReadHB_Bit = 1; 
  delay(100);
    
  if (!pox.begin()) 
  {
    //Serial.println("FAILED");        
    for(;;);
  } 
  else 
  {
   // Serial.println("SUCCESS");             
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);

  // Register a callback for the beat detection
  pox.setOnBeatDetectedCallback(onBeatDetected); 
  
  ReadHB:     
  // Make sure to call update as fast as possible
  pox.update();
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) 
  {
    Count++;

    if(Count >= 20)  
    {
      Count = 0;
      Serial.print("Heart rate:");
      Serial.print(pox.getHeartRate());
      Serial.print("bpm / SpO2:");
      Serial.print(pox.getSpO2());
      Serial.println("%");
    }     
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("BPM : ");
    lcd.print(pox.getHeartRate());
    
    lcd.setCursor(0,1);
    lcd.print("SpO2: ");
    lcd.print(pox.getSpO2());
    lcd.print("%");        
    
    tsLastReport = millis();       
  }
    
 Cont: 
  switchvalue = digitalRead(switchPin);
  if((switchvalue == 0) && (ReadHB_Bit == 0))
  {               
    goto Start;
  }

  if((switchvalue == 0) && (ReadHB_Bit == 1))
  {               
    goto ReadHB;
  }
  
  if(switchvalue == 1)
  {
    ReadHB_Bit = 0;    
    lcd.clear();
    
    tempeValue = analogRead(tempePin);
    tempeValue = tempeValue / 2 ;
    tempeValue = (tempeValue * (9/5)) + 32 ;
    delay(100);
    
    lcd.setCursor(0,0); 
    lcd.print("BodyTemp:");
    lcd.print(tempeValue);
    lcd.print((char)223);
    lcd.print("F");
    delay(2000);    
    
    Serial.print("BodyTemp.: ");
    Serial.print(tempeValue);
    Serial.println("Deg.F;");  
    delay(5000); 

    if(tempeValue > 100)
    {
      TempeHighBit = 1;
    }

    if(tempeValue < 95)
    {
      TempeLowBit = 1;
    }

    if((tempeValue < 100) && (tempeValue > 95))
    {
      TempeHighBit = 0;
      TempeLowBit = 0;
      Message1SentBit = 0;
      Message2SentBit = 0;    
    }

    if((TempeHighBit) && (Message1SentBit == 0))
    {
      Message1SentBit = 1;
    
      if (gsm.available()>0)
      Serial.write(gsm.read());
      
      lcd.setCursor(0,1);
      lcd.print("Message Sending1");
      delay(1000);   

      gsm.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
      delay(1000);  // Delay of 1000 milli seconds or 1 second
      gsm.println("AT+CMGS=\"+916369445959\"\r"); // Replace x with mobile number
      delay(1000);
      gsm.println("Body Temperature High; Please take necessary action immediately ; ");// The SMS text you want to send
      delay(100);
      gsm.println((char)26);// ASCII code of CTRL+Z 
      delay(5000); 

      lcd.setCursor(0,1);
      lcd.print("Message Sent....");
      delay(1000);            
    }

    if((TempeLowBit) && (Message2SentBit == 0))
    {
      Message2SentBit = 1;
    
      if (gsm.available()>0)
      Serial.write(gsm.read());
      
      lcd.setCursor(0,1);
      lcd.print("Message Sending1");
      delay(1000);   

      gsm.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
      delay(1000);  // Delay of 1000 milli seconds or 1 second
      gsm.println("AT+CMGS=\"+916369445959\"\r"); // Replace x with mobile number
      delay(1000);
      gsm.println("Body Temperature Low; Please take necessary action immediately ; ");// The SMS text you want to send
      delay(100);
      gsm.println((char)26);// ASCII code of CTRL+Z 
      delay(5000); 

      lcd.setCursor(0,1);
      lcd.print("Message Sent....");
      delay(1000);            
    }    
  }
 goto Cont;
}




