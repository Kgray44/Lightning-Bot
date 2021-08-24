#define BLYNK_PRINT Serial
#define ICACHE_RAM_ATTR

#include <DFRobot_AS3935_I2C.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WidgetRTC.h>
#include <WidgetTimeInput.h>

char auth[] = "oYjZnE2WCVLi2j9a9LmphpPNjJ2sC40I";
char ssid[] = "Apple Network 85064d";
char pass[] = "12344321";

#if defined(ESP32) || defined(ESP8266)
#define IRQ_PIN       0 
#else
#define IRQ_PIN       2
#endif

// Antenna tuning capcitance (must be integer multiple of 8, 8 - 120 pf)
#define AS3935_CAPACITANCE   96

// I2C address
#define AS3935_I2C_ADDR       AS3935_ADD3

void AS3935_ISR();

WidgetTerminal terminal(V5);
WidgetRTC rtc;

DFRobot_AS3935_I2C  lightning0((uint8_t)IRQ_PIN);

volatile int8_t AS3935IsrTrig = 0;

uint8_t lightningDistKm;
uint32_t lightningEnergyVal;

String distanceMeasurement = "miles";

boolean locationSet = false;
boolean disturberSet = false;
boolean noiseSet = false;
boolean watchdogSet = false;
boolean spikeSet = false;
boolean notificationsSet = false;
boolean restartSet = false;

boolean notifications = true;

int noiseVal = 2;
int watchdogVal = 2;
int spikeVal = 2;

BLYNK_CONNECTED() {
  rtc.begin();
}

BLYNK_WRITE(V5){  
  //location
  if (String("location") == param.asStr()) {
    if (locationSet == false){
      terminal.println("You said: 'location'") ;
      terminal.println("Please type 'indoor' or 'outdoor':") ;
      locationSet = true;
    }
  }
  if (locationSet == true){
    if (String("indoor") == param.asStr()){
      terminal.println("Success!");
      terminal.println("Location setting set to 'indoor'.");
      terminal.println("");
      lightning0.setIndoors();
      locationSet = false;
    }
    else if (String("outdoor") == param.asStr()){
      terminal.println("Success!");
      terminal.println("Location setting set to 'outdoor'.");
      terminal.println("");
      lightning0.setOutdoors();
      locationSet = false;
    }
  }

  //disturber
  if (String("disturber") == param.asStr()) {
    if (disturberSet == false){
      terminal.println("You said: 'disturber'") ;
      terminal.println("Please type 'enable' or 'disable':") ;
      disturberSet = true;
    }
  }
  if (disturberSet == true){
    if (String("enable") == param.asStr()){
      terminal.println("Success!");
      terminal.println("Disturber setting set to 'enable'.");
      terminal.println("");
      lightning0.disturberEn();
      disturberSet = false;
    }
    else if (String("disable") == param.asStr()){
      terminal.println("Success!");
      terminal.println("Disturber setting set to 'disable'.");
      terminal.println("");
      lightning0.disturberDis();
      disturberSet = false;
    }
  }

  //noise
  if (String("noise") == param.asStr()) {
    if (noiseSet == false){
      terminal.println("You said: 'noise'") ;
      terminal.print("Current value is '");
      terminal.print(noiseVal);
      terminal.println("'.");
      terminal.println("Please type '1' - '7':") ;
      noiseSet = true;
    }
  }
  if (noiseSet == true){
    //if (String("") != param.asStr()){
    noiseVal = param.asInt();
    if (noiseVal >= 1 && noiseVal <= 7){
      lightning0.setNoiseFloorLvl(noiseVal);
      terminal.println("Success!");
      terminal.print("Noise setting set to '");
      terminal.print(noiseVal);
      terminal.println("'.");
      terminal.println("");
      noiseSet = false;
    }
  }

  //watchdog
  if (String("watchdog") == param.asStr()) {
    if (watchdogSet == false){
      terminal.println("You said: 'watchdog'") ;
      terminal.print("Current value is '");
      terminal.print(watchdogVal);
      terminal.println("'.");
      terminal.println("Please type '1' - '7':");
      watchdogSet = true;
    }
  }
  if (watchdogSet == true){
    watchdogVal = param.asInt();
    if (watchdogVal >= 1 && watchdogVal <= 7){
      lightning0.setWatchdogThreshold(watchdogVal);
      terminal.println("Success!");
      terminal.print("Watchdog setting set to '");
      terminal.print(watchdogVal);
      terminal.println("'.");
      terminal.println("");
      watchdogSet = false;
    }
  }

  //spike
  if (String("spike") == param.asStr()) {
    if (watchdogSet == false){
      terminal.println("You said: 'spike'") ;
      terminal.print("Current value is '");
      terminal.print(spikeVal);
      terminal.println("'.");
      terminal.println("Please type '1' - '7':");
      spikeSet = true;
    }
  }
  if (spikeSet == true){
    spikeVal = param.asInt();
    if (spikeVal >= 1 && spikeVal <= 7){
      lightning0.setSpikeRejection(spikeVal);
      terminal.println("Success!");
      terminal.print("Spike setting set to '");
      terminal.print(spikeVal);
      terminal.println("'.");
      terminal.println("");
      spikeSet = false;
    }
  }

  //notifications
  if (String("notifications") == param.asStr()) {
    if (notificationsSet == false){
      terminal.println("You said: 'notifications'") ;
      terminal.println("Please type 'enable' or 'disable':") ;
      notificationsSet = true;
    }
  }
  if (notificationsSet == true){
    if (String("enable") == param.asStr()){
      terminal.println("Success!");
      terminal.println("Notifications setting set to 'enable'.");
      terminal.println("");
      notifications = true;
      notificationsSet = false;
    }
    else if (String("disable") == param.asStr()){
      terminal.println("Success!");
      terminal.println("Notifications setting set to 'disable'.");
      terminal.println("");
      notifications = false;
      notificationsSet = false;
    }
  }

  if (String("info") == param.asStr()) {
    terminal.println("");
    terminal.println("-------------");
    terminal.println("Lightning Bot");
    terminal.println("");
    terminal.println("Lightning Bot is an interactive lightning sensor.");
    terminal.print("It can sense lightning up to 40km (25 miles) away,");
    terminal.print(" in 2.7km (1.7 miles) increments!  It can also sense");
    terminal.print(" relative lightning strength (arbitrary number, not");
    terminal.println(" legit measurement.)");
    terminal.println("");
    terminal.println("-------------");
    terminal.println("");
    terminal.println("");
  }
  
  //menu
  if (String("menu") == param.asStr()) {
    terminal.println(F(""));
    terminal.println(F("Lightning Settings:"));
    terminal.println(F("(1) 'location'"));
    terminal.println(F("(2) 'disturber"));
    terminal.println(F("(3) 'noise'"));
    terminal.println(F("(4) 'watchdog'"));
    terminal.println(F("(5) 'spike'"));
    terminal.println(F("Other Settings:"));
    terminal.println(F("(6) 'notifications'"));
    terminal.println(F("(7) 'info'"));
    terminal.println(F("(8) 'menu'"));
    terminal.println(F("(9) 'clear'"));
    terminal.println(F("(10) 'restart'"));
    terminal.println(F(""));
  }

  //clear
  if (String("clear") == param.asStr()) {
    terminal.clear();
  }

  //restart
  if (String("restart") == param.asStr()) {
    terminal.println("You said: 'restart'") ;
    terminal.println("Are you sure you want to restart?");
    terminal.println("WiFi connection will be lost for ~20 seconds.");
    terminal.println("Please type 'yes' or 'no':") ;
    restartSet = true;
  }
  if (restartSet == true){
    if (String("yes") == param.asStr()){
      terminal.println("Restarting...");
      terminal.flush();
      Blynk.run();
      ESP.restart();
    }
    else if (String("no") == param.asStr()){
      terminal.println("Okay.");
      terminal.println("");
      restartSet = false;
    }
  }

  // Ensure everything is sent
  terminal.flush();
}


void setup(){

  Serial.begin(9600);
  Serial.println("DFRobot AS3935 lightning sensor begin!");

  lightning0.setI2CAddress(AS3935_ADD3);

  while (lightning0.begin() != 0){
    Serial.print(".");
  }

  if(lightning0.defInit() != 0){
    Serial.println("I2C init fail");
    while(1){}  
  }
  
  lightning0.powerUp();
  
  //set indoors or outdoors models
  lightning0.setIndoors();
  //lightning0.setOutdoors();

  //disturber detection
  lightning0.disturberEn();
  //lightning0.disturberDis();

  lightning0.setIRQOutputSource(0);

  #if defined(ESP32) || defined(ESP8266)
  attachInterrupt(digitalPinToInterrupt(IRQ_PIN),AS3935_ISR,RISING);
  #else
  attachInterrupt(/*Interrupt No*/0,AS3935_ISR,RISING);
  #endif
  
  delay(500);
  
  //set capacitance
  lightning0.setTuningCaps(AS3935_CAPACITANCE);
  
  Serial.println("AS3935 manual cal complete");
  
  // Enable interrupt (connect IRQ pin IRQ_PIN: 2, default)
//  Connect the IRQ and GND pin to the oscilloscope.
//  uncomment the following sentences to fine tune the antenna for better performance.
//  This will dispaly the antenna's resonance frequency/16 on IRQ pin (The resonance frequency will be divided by 16 on this pin)
//  Tuning AS3935_CAPACITANCE to make the frequency within 500/16 kHz ± 3.5%
//  lightning0.setLcoFdiv(0);
//  lightning0.setIRQOutputSource(3);

// Set the noise level,more than 7 will use the default value:2
  lightning0.setNoiseFloorLvl(2);
  //uint8_t noiseLv = lightning0.getNoiseFloorLvl();

//used to modify WDTH,alues should only be between 0x00 and 0x0F (0 and 7)
  lightning0.setWatchdogThreshold(2);
  //uint8_t wtdgThreshold = lightning0.getWatchdogThreshold();

//used to modify SREJ (spike rejection),values should only be between 0x00 and 0x0F (0 and 7)
  lightning0.setSpikeRejection(2);
  //uint8_t spikeRejection = lightning0.getSpikeRejection();

  Blynk.begin(auth, ssid, pass);
  
  //info
  terminal.clear();
  terminal.println("-------------");
  terminal.println("Lightning Bot");
  terminal.println("");
  terminal.println("Lightning Bot is an interactive lightning sensor.");
  terminal.print("It can sense lightning up to 40km (25 miles) away,");
  terminal.print(" in 2.7km (1.7 miles) increments!  It can also sense");
  terminal.print(" relative lightning strength (arbitrary number, not");
  terminal.println(" legit measurement.)");
  terminal.println("");
  terminal.println("-------------");
  terminal.println("");
  terminal.println("");

  //menu
  terminal.println(F("Lightning Settings:"));
  terminal.println(F("(1) 'location'"));
  terminal.println(F("(2) 'disturber"));
  terminal.println(F("(3) 'noise'"));
  terminal.println(F("(4) 'watchdog'"));
  terminal.println(F("(5) 'spike'"));
  terminal.println(F("Other Settings:"));
  terminal.println(F("(6) 'notifications'"));
  terminal.println(F("(7) 'info'"));
  terminal.println(F("(8) 'menu'"));
  terminal.println(F("(9) 'clear'"));
  terminal.println(F("(10) 'restart'"));
  terminal.println(F(""));
  
  terminal.flush();

  Blynk.virtualWrite(V1, 0);
  Blynk.virtualWrite(V2, 0);
  Blynk.virtualWrite(V3, 0);
}

void loop()
{
  
  // It does nothing until an interrupt is detected on the IRQ pin.
 // while (AS3935IsrTrig == 0) {delay(1);}
  //delay(5);

  if (AS3935IsrTrig != 0){
    delay(5);
    
    // Reset interrupt flag
    AS3935IsrTrig = 0;

    // Get interrupt source
    uint8_t intSrc = lightning0.getInterruptSrc();
    if (intSrc == 1)
    {
      // Get rid of non-distance data
      lightningDistKm = lightning0.getLightningDistKm();
      String currentTime = String(hour()) + "." + String(minute()) + "." + String(second());
      Serial.println("Lightning occurred!");
      Serial.print("Distance: ");
      Serial.print(lightningDistKm);
      Serial.println(" km");
    
      Blynk.virtualWrite(V1, currentTime.toInt());
      if (distanceMeasurement == "km"){
        Blynk.virtualWrite(V3, lightningDistKm);
      }
      else if (distanceMeasurement == "miles"){
        Blynk.virtualWrite(V3, lightningDistKm/1.609);
      }

      // Get lightning energy intensity
      lightningEnergyVal = lightning0.getStrikeEnergyRaw();
      Serial.print("Intensity: ");
      Serial.print(lightningEnergyVal);
      Serial.println("");
    
      Blynk.virtualWrite(V2, lightningEnergyVal);
    
      terminal.println("");
      terminal.print(currentTime);
      terminal.print(": Lightning.  ");
      if (distanceMeasurement == "km"){
        terminal.print(lightningDistKm);
        terminal.print(" km. ");
      }
      else if (distanceMeasurement == "miles"){
        terminal.print(lightningDistKm/1.609);
        terminal.print(" miles. ");
      }
      terminal.print(lightningEnergyVal);
      terminal.println(" relative strength.");
    
      if (notifications == true){
        if (distanceMeasurement == "km"){
          Blynk.notify("Lightning! Distance is " + String(lightningDistKm) + " km.  Strength is " + String(lightningEnergyVal) + ".  If you are not already indoors, please do so.");
        }
        else if (distanceMeasurement == "miles"){
          Blynk.notify("Lightning! Distance is " + String(lightningDistKm/1.609) + " miles.  Strength is " + String(lightningEnergyVal) + ".  If you are not already indoors, please do so.");
        }
      }
    }
    else if (intSrc == 2)
    {
      String currentTime = String(hour()) + ":" + String(minute()) + ":" + String(second());
      
      Serial.println("Disturber discovered!");
      
      terminal.println("");
      terminal.print(currentTime);
      terminal.println(" Disturber");
    }
    else if (intSrc == 3)
    {
      Serial.println("Noise level too high!");
    }
  }
  
  //View register data
  //lightning0.printAllRegs();

  Blynk.run();
}

//IRQ handler for AS3935 interrupts
ICACHE_RAM_ATTR void AS3935_ISR()
{
  AS3935IsrTrig = 1;
}
