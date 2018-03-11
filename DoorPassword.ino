/*********************************************************
This is a library for the MPR121 12-channel Capacitive touch sensor

Designed specifically to work with the MPR121 Breakout in the Adafruit shop 
  ----> https://www.adafruit.com/products/

These sensors use I2C communicate, at least 2 pins are required 
to interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada for Adafruit Industries.  
BSD license, all text above must be included in any redistribution
**********************************************************/

#include <Wire.h>
#include "Adafruit_MPR121.h"

#define LED_PIN             13
#define GREEN_LED_PIN       LED_PIN
#define RED_LED_PIN         12
#define BEEPER_GND_PIN      7
#define BEEPER_DRIVER_PIN   9
#define BEEPER_FREQ         1200
#define BEEPER_DURATION     100
#define KEYPAD_INT_PIN      2
#define LOCK_STATUS_LOCKED  false
#define LOCK_STATUS_UNLOCKED  true

// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

const char password[] = "1234";
char pin[5];
uint8_t idx = 0;
bool lockStatus;
char button;

void setup() 
{
  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);             // Set LED pin as output
  pinMode(GREEN_LED_PIN, OUTPUT);       // Set LED pin as output
  pinMode(RED_LED_PIN, OUTPUT);         // Set LED pin as output
  digitalWrite(LED_PIN, LOW);           // Turn off LED
  pinMode(BEEPER_DRIVER_PIN, OUTPUT);   // Set beeper pin as output
  pinMode(BEEPER_GND_PIN, OUTPUT);      // Set beeper pin as output
  digitalWrite(BEEPER_DRIVER_PIN, LOW); // Set beeper driver pin low
  digitalWrite(BEEPER_GND_PIN, LOW);    // Set beeper ground pin low
  
  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) 
  {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }

  //attachInterrupt(digitalPinToInterrupt(KEYPAD_INT_PIN), buttonHandler, FALLING);
  //interrupts();

  actuateLock();
  redOn();
  greenOff();
  lockStatus = LOCK_STATUS_LOCKED;
}

char buttonRemap[] = {'1', '4', '7', '*', '2', '5', '8', '0', '3', '6', '9', '#'};

void loop() 
{
  button = buttonHandler();

  if(lockStatus == LOCK_STATUS_UNLOCKED)
  {
    if(button == '#')
    {
      greenOff();
      redOn();
      lockBeep();
      actuateLock();
      lockStatus = LOCK_STATUS_LOCKED;
      Serial.println("Locked");
    }
  }

  else
  {
    if(button == '#')
    {
      pin[idx] = 0;
      idx = 0;
      Serial.println();
      if(comparePin(pin, password))
      {
        greenOn();
        redOff();
        unlockBeep();
        actuateUnlock();
        lockStatus = LOCK_STATUS_UNLOCKED;
        Serial.println("Unlocked");
      }
      else
      {
        wrongPinBeep();
        Serial.println("Wrong pin");
      }
    }
    else if(button >= '0' && button <= '9')
    {
      Serial.print(button);
      pin[idx] = button;
      if(idx < 4)
      {
        idx++;
      }
    }
  }
    
  delay(80);
}

void debugRoutine (void)
{
  // debugging info, what
  Serial.print("\t\t\t\t\t\t\t\t\t\t\t\t\t 0x"); Serial.println(cap.touched(), HEX);
  Serial.print("Filt: ");
  for (uint8_t i=0; i<12; i++) {
    Serial.print(cap.filteredData(i)); Serial.print("\t");
  }
  Serial.println();
  Serial.print("Base: ");
  for (uint8_t i=0; i<12; i++) {
    Serial.print(cap.baselineData(i)); Serial.print("\t");
  }
  Serial.println();
}

char buttonHandler (void)
{
  char retVal = 0;
  
  // Get the currently touched pads
  currtouched = cap.touched();

  // If any button is touched
  if(currtouched)
  {
    // Loop through to find out which button is touched
    for (uint8_t i=0; i<12; i++) 
    {
      // Look for bit set, which corresponds to the MPR121 cap touch pin
      if (currtouched & 0x01) 
      {
        //Serial.println(currtouched, HEX);
        //Serial.print(buttonRemap[i]);         // Remap MPR121 pin to the keypad key number
        retVal = buttonRemap[i];
        tone(BEEPER_DRIVER_PIN, BEEPER_FREQ, BEEPER_DURATION);   // Make a short beep for audible feedback
  
        // Wait for button to be released
        while(currtouched)
        {
          currtouched = cap.touched();
          delay(10);
        }
        
        //Serial.println("released");
        
        break;
      }

      // Shift result to test next bit position
      else
      {
        currtouched = currtouched >> 1;
      }      
    }
  }

  return retVal;
}


bool comparePin (char pin1[], char pin2[])
{
  return !strcmp(pin1, pin2);
}


void greenOn (void)
{
  digitalWrite(GREEN_LED_PIN, HIGH);
}


void redOn (void)
{
  digitalWrite(RED_LED_PIN, HIGH);
}

void greenOff (void)
{
  digitalWrite(GREEN_LED_PIN, LOW);
}


void redOff (void)
{
  digitalWrite(RED_LED_PIN, LOW);
}


void actuateLock (void)
{
  
}


void actuateUnlock (void)
{
  
}

void lockBeep (void)
{
  tone(BEEPER_DRIVER_PIN, BEEPER_FREQ);
  delay(1000);
  noTone(BEEPER_DRIVER_PIN);  
}

void unlockBeep (void)
{
  tone(BEEPER_DRIVER_PIN, BEEPER_FREQ);
  delay(100);
  noTone(BEEPER_DRIVER_PIN);  
  delay(100);
  tone(BEEPER_DRIVER_PIN, BEEPER_FREQ);
  delay(100);
  noTone(BEEPER_DRIVER_PIN);  
  delay(100);
  tone(BEEPER_DRIVER_PIN, BEEPER_FREQ+1000);
  delay(600);
  noTone(BEEPER_DRIVER_PIN);  
}

void wrongPinBeep (void)
{
  tone(BEEPER_DRIVER_PIN, BEEPER_FREQ+1000);
  delay(100);
  noTone(BEEPER_DRIVER_PIN);  
  delay(100);
  tone(BEEPER_DRIVER_PIN, BEEPER_FREQ+1000);
  delay(100);
  noTone(BEEPER_DRIVER_PIN);  
  delay(100);
  tone(BEEPER_DRIVER_PIN, BEEPER_FREQ);
  delay(600);
  noTone(BEEPER_DRIVER_PIN);  
}

