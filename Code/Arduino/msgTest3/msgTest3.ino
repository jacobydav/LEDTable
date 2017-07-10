/*This code will read the values from the MSGEQ7 and turn the LEDs on
with the music.*/
#include "src\Adafruit_TLC5947\Adafruit_TLC5947.h"

int analogPin = 0; // read from multiplexer using analog input 0
int strobePin = 7; // strobe is attached to digital pin 7
int resetPin = 5; // reset is attached to digital pin 5
int spectrumValue[7]; // to hold a2d values

int recentMax[]={15,15,15,15,15,15,15}; //hold the recent max value returned by MSG
int recentMin[]={600,600,600,600,600,600,600}; //hold the recent min value returned by MSG

int bandLedNum[]={10,10,10,10,10,10,10};

int chanHeight=0;  //holds the number of LEDs to activate
     int ledOffset=0;   //holds the current offset for the led index
     int chanHeightTotal=0; //holds the total number of LEDs to activate
                            //this is to protect against all the lights coming on
                            //because of static or equipment pop   

const int totalLeds=45;
volatile int pChange;          // interrupt for pattern change
int currentPattern;
float ovIntScaleVal = 1.0;

//TLC declarations begin
// How many boards do you have chained?
#define NUM_TLC5974 2

#define data_pin   11
#define clock_pin  10
#define latch_pin   9
#define oe  -1  // set to -1 to not use the enable pin (its optional)

Adafruit_TLC5947 tlc = Adafruit_TLC5947(NUM_TLC5974, clock_pin, data_pin, latch_pin);
//TLC declarations end

void setup() 
{
  Serial.begin(9600);

  //TLC init begin
  tlc.begin();
  if (oe >= 0) {
    pinMode(oe, OUTPUT);
    digitalWrite(oe, LOW);
  }
  //Set the LEDs to zero
  for(int j=0; j < 48; j++)
      tlc.setPWM(j,0);

  tlc.write();
  //TLC init end
  
  pinMode(analogPin, INPUT);
  pinMode(strobePin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  analogReference(DEFAULT);

  digitalWrite(resetPin, LOW);
  digitalWrite(strobePin, HIGH);

  currentPattern = 2;
}

void loop() 
{
  runPattern2();
}

void runPattern2()
{
  int i,j;
  int fadeStep=150;
  int maxLedsOn=int(double(totalLeds)*.88);
  int ledLevel[]={0,0,0,0,0,0,0,0,0,0, //stores the level for each led
                  0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0};
  //msg variables
  int spectrumValue[7]; // to hold a2d values
  int recentMax[]={15,15,15,15,15,15,15}; //hold the recent max value returned by MSG
  int recentMin[]={600,600,600,600,600,600,600}; //hold the recent min value returned by MSG
  int msgVal;
  int bandLedNum[]={5,7,7,6,7,7,6};
  //store the order of leds to use in the msg equalizer
  //there are 7 bands.  Each bands leds are offset by its value in bandLedNum
//  int msgLedOrd[]={31,38,44,30,35,37,46,
//                   32,39,47,35,34,43,41,
//                   26,24,28,33,22,0,
//                   16,4,12,23,14,11,7,
//                   20,13,5,17,14,10,6,
//                   25,21,18,29,19,0,
//                   27,37,11,0,0};

 int msgLedOrd[]={ 7, 2, 6,42,17,
                  41,24,30,26,25,29,28,
                   5,36,31,38,37,32,27,
                  45,11, 4,10, 3,47,
                  46,21,13,18,19,15,12,
                   9,14,22,23,20,16, 0,
                   1, 8,44,40,43,39};

  int bandMinGap[]={100,100,100,100,100,100,100};  //Min gap between recentMin and recentMax

  for (int i = 0; i < 7; i++)
  {    
      Serial.print(bandMinGap[i]);
      Serial.print("   ");
  }
  Serial.println();
  Serial.println();
  
  delay(100);
    
  while(currentPattern==2 && pChange==0)
  {
    ledOffset=0;
    
    digitalWrite(resetPin, HIGH);
    digitalWrite(resetPin, LOW);

    //get the response from the MSG
    for (int i = 0; i < 7; i++)
    {
      digitalWrite(strobePin, LOW);
      delayMicroseconds(30); // to allow the output to settle
      spectrumValue[i] = analogRead(analogPin);
      
      digitalWrite(strobePin, HIGH);
    }
    
    //for each frequency band
    for (int i = 0; i < 7; i++)
    {
      // comment out/remove the serial stuff to go faster
      // - its just here for show
//      if (spectrumValue[i] < 10)
//      {
//        Serial.print("    ");
//        Serial.print(spectrumValue[i]);
//      }
//      else if (spectrumValue[i] < 100 )
//      {
//        Serial.print("   ");
//        Serial.print(spectrumValue[i]);
//      }
//      else
//      {
//        Serial.print("  ");
//        Serial.print(spectrumValue[i]);
//      }
      //keep track of the max for scaling
        if(spectrumValue[i]>recentMax[i])
        {
          recentMax[i]=spectrumValue[i];
        }
        //keep track of the min for scaling
        if(spectrumValue[i]<recentMin[i])
        {
          recentMin[i]=spectrumValue[i];
        }
        
    }

    //  //Show recent min
//    Serial.print("  ");
//      for (int i = 0; i < 7; i++)
//      {
//        Serial.print("  ");
//        Serial.print(recentMin[i]);
//      }
//      Serial.print("  ");
//      //  //Show recent max
//      for (int i = 0; i < 7; i++)
//      {
//        Serial.print("  ");
//        Serial.print(recentMax[i]);
//      }
//      Serial.print("  ");
    //fade the current led levels
    for(i=0;i<totalLeds;i++)
    {
      if(ledLevel[i]>0)  //if the led was lit from last iteration
      {
        ledLevel[i]=ledLevel[i]-fadeStep;  //fade 
        if(ledLevel[i]<0)            //check to see if it is faded out
          ledLevel[i]=0;
      }
    } 

    //Calculate the number of leds that should be on for each channel
    for (int i = 0; i < 7; i++)
    {    
        //calculate the percentage of the max
        //and multiply it times the number of leds that represent the current band
        //Check for a minimum difference between the recentMin and recentMax. This is because
        //when the audio is quiet, the recentMax and recentMin will be very close. This will
        //cause alot of fluctuations.        
        if((recentMax[i]-recentMin[i])>bandMinGap[i])
        {
         chanHeight=(int)((float((spectrumValue[i]-recentMin[i]))/((float)recentMax[i]-recentMin[i]))*bandLedNum[i]);         
        }
        else
        {
          chanHeight=(int)((float((spectrumValue[i]-recentMin[i]))/((float)bandMinGap[i]))*bandLedNum[i]);
        }
        
       chanHeightTotal=chanHeightTotal+chanHeight;

//       Serial.print("  ");
//       Serial.print(chanHeight);
  
       //turn on the lights
       for(j=0; j < chanHeight; j++)
       {
           ledLevel[j+ledOffset]=4000;
       }
       //update the offset
       ledOffset=ledOffset+bandLedNum[i];
                      
       //Shrink the gap between max and min to adjust to changes in overall volume levels
       if(recentMin[i]<1000)
         recentMin[i]++;
       if(recentMax[i]>0 && recentMax[i]>recentMin[i])
         recentMax[i]--;
    }
          
    //set the TLC
    if(chanHeightTotal<maxLedsOn) //only if the total number of lit LEDs is less than 88%
    {                                 //update the lights
      for(i=0;i<totalLeds;i++)
      {
        //Adjust for the overall intensity scale factor
        int adjLedLevel = (int)(ledLevel[i]*ovIntScaleVal);
        //Serial.print(" H ");
        //Serial.print(adjLedLevel);
        //Serial.print("   ");
        //Serial.print(msgLedOrd[i]);
        //Serial.print(" H ");
        tlc.setPWM(msgLedOrd[i], adjLedLevel);
      }
    }
    else   //only turn on the leds up to maxLedsOn
    {
      for(i=0;i<maxLedsOn;i++)
      {
        //Adjust for the overall intensity scale factor
        int adjLedLevel = (int)(ledLevel[i]*ovIntScaleVal);
        tlc.setPWM(msgLedOrd[i], adjLedLevel);
      }
    }
      
     tlc.write();
        
//     Serial.println();
//
//     for (int i = 0; i < 7; i++)
//    {    
//        Serial.print(bandMinGap[i]);
//        Serial.print("   ");
//    }
//    Serial.println();
//    Serial.println();
//
//    Serial.println();
//
//     for (int i = 0; i < 7; i++)
//    {    
//        Serial.print(bandLedNum[i]);
//        Serial.print("   ");
//    }
//    Serial.println();
//    Serial.println();
          
     delay(5);
  }
}

