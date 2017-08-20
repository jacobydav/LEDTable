#include "src\Adafruit_TLC5947\Adafruit_TLC5947.h"
#include <avr/interrupt.h>

const int totalPatterns=6;
const int totalLeds=45;

//This is a representation of the layout of the LEDs viewed from above with the control panel above Row 1.
//There are 7 rows, Rows 1,3,5,7 have 6 LEDs
//Rows 2,4,6 have 7 LEDs
byte ledIndArray[]={  32,31,27,28,30,29, //the index for each led
                      37,38,36,42,24,26,25,
                      3,4,5,41,40,39,
                      11,45,2,7,6,1,8,
                      47,10,46,9,44,43,
                      19,18,21,17,14,23,20,
                      15,13,12,0,22,16};
byte ledRowCnt[] = {6,7,6,7,6,7,6};  //The number of LEDs in each row

volatile int pChange;          // interrupt for pattern change
int currentPattern;
int k;
//MSG pin setup
int analogPin = 0; // read from multiplexer using analog input 0
int strobePin = 7; // strobe is attached to digital pin 2
int resetPin = 5; // reset is attached to digital pin 3
//Overall intensity begin
//This will give control over the brightness. For example if the table is too
//bright at night the potentiometer on the control panel can be used to dim it.
//Every intensity value applied to an LED in any of the patterns will first
//be multiplied by a scale factor determined by the potentiometer position.
//The value read from the analog pin has a range from 0-1024.
//These values will be scaled to between 0.25-1. Where 0.25 is 1/4th normal
//brightness and 1 is normal brightness. I choose this scale because I don't
//there is any need to dim the LEDs below 1/4th brightness.
int ovIntenPin = 1; // analog pin 1
int ovIntenMaxVal = 1024; 
int ovIntenMinVal = 0;
float ovIntenScaleMax = 1.0;
float ovIntenScaleMin = 0.0;
volatile float ovIntScaleVal = 0.5;
//Overall intensity end
//Ambient light intensity begin
byte ambLightPin = 2;  //analog pin 2
int ambLightMaxVal = 500;   //bright
int ambLightMinVal = 60;    //dark
float ambLightScaleMax = 1.0;
float ambLightScaleMin = 0.2;
volatile float ambLightScaleVal = 1.0;
//Ambient light intensity end
//Motion activation setup begin
unsigned long motion_on_time_ms = 1800000;   //length of time to remain on without seeing any motion, unit is milliseconds
volatile unsigned long last_motion_time_ms = 0;   //Last time motion was detected. If curr_time - last_motion_time_ms = motion_on_time_ms, then the table will go to sleep.
//Motion activation setup end

//TLC5947 begin
// How many boards do you have chained?
#define NUM_TLC5974 2

#define data_pin   11
#define clock_pin  10
#define latch_pin   9
#define oe  -1  // set to -1 to not use the enable pin (its optional)

Adafruit_TLC5947 tlc = Adafruit_TLC5947(NUM_TLC5974, clock_pin, data_pin, latch_pin);
//TLC5947 end

void setup() 
{
  //**********interrupt setup begin**************
  noInterrupts();           // disable all interrupts
  //Timed interrupt
  // initialize timer1  
  TCCR1A = 0;
  TCCR1B = 0;
  // Set timer1_counter to the correct value for our interrupt interval
  TCNT1 = 60000;   // preload timer. We want it to run soon after startup because the intensity adjustments are calculated here. After the first run, there will be no pre-loading.
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  //Mode button interrupt - this will change the current LED pattern
  pinMode(2, INPUT);      // Make digital 2 an input
  // attach our interrupt pin to it's ISR
  attachInterrupt(0, patternChange, LOW);
  //Motion detector interrupt - this will happen when motion is detected
  pinMode(3, INPUT);      // Make digital 2 an input
  // attach our interrupt pin to it's ISR
  attachInterrupt(1, motion_ISR, FALLING);
  // we need to call this to enable interrupts
  interrupts();
  pChange=0;
  currentPattern=1;  //Set to 1 for standard operation (randomly select patterns). Set this value to a specific pattern number if you want to test only that pattern.
  //**********interrupt setup end**************
  Serial.begin(9600); //used in test mode only
  //*********msg setup begin********************
  pinMode(analogPin, INPUT);
  pinMode(strobePin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  analogReference(DEFAULT);

  digitalWrite(resetPin, LOW);
  digitalWrite(strobePin, HIGH);
  //*********msg setup end********************
  //*********overall intensity setup begin********************
  pinMode(ovIntenPin, INPUT);
  //*********overall intensity setup end********************
  randomSeed(analogRead(2));  //seed the random number with an unconnected pin(2) read value
  
  k=0;
  //TLC5947 begin
  tlc.begin();
  if (oe >= 0) {
    pinMode(oe, OUTPUT);
    digitalWrite(oe, LOW);
  }
  //Set the LEDs to zero
  for(int j=0; j < 48; j++)
  {
      tlc.setPWM(j,0);
  }
  tlc.write();
  //TLC5947 end
}

// The interrupt hardware calls this when the mode button is pressed
void patternChange()
{
  pChange=1;
}

// The interrupt hardware calls this when motion is detected
void motion_ISR()
{
  last_motion_time_ms=millis();
  //Check if we are currently sleeping
  if(pChange==2)
    pChange=0;
}

//Timed interrupt handler
//This interrupt routine will be called every few seconds.
//This is where we will check if it has been too long since motion was detected.
ISR(TIMER1_OVF_vect)        // interrupt service routine 
{
  TCNT1 = 0;   // preload timer
  unsigned long curr_time = millis();
  if (curr_time - last_motion_time_ms > motion_on_time_ms)
  {
    pChange=2;
  }
  //Update the overall intensity begin
  //Read the potentiometer value
  int ovIntVal=analogRead(ovIntenPin);
   //Scale the value
  ovIntScaleVal = ((ovIntenScaleMax-ovIntenScaleMin)/(ovIntenMaxVal-ovIntenMinVal))*(ovIntVal-ovIntenMinVal) + ovIntenScaleMin;
  //Update the overall intensity end
  //Update the intensity based on ambient light begin
  //Read the potentiometer value
  int ambLightVal=analogRead(ambLightPin);
  if(ambLightVal<ambLightMinVal)
    ambLightVal = ambLightMinVal;
  if(ambLightVal>ambLightMaxVal)
    ambLightVal = ambLightMaxVal;
  //Scale the analog value to between 0.5 and 1 where 0.5 means the room is dark and 1 means the room is bright.
  ambLightScaleVal = ((ambLightScaleMax-ambLightScaleMin)/(ambLightMaxVal-ambLightMinVal))*(ambLightVal-ambLightMinVal) + ambLightScaleMin;
   //Scale the value using the overall intensity potentiometer
  ovIntScaleVal = ambLightScaleVal*ovIntScaleVal;
  //Update the intensity based on ambient light end
}

//randomly select a pattern and a timelength to run it
void runPattern1()
{
  randomSeed(analogRead(5));  //seed the random number with an unconnected pin(5) read value
  long pattTime;
  long minPattTime=30000;  //30 seconds
  long maxPattTime=300000; //300 seconds or 5 minutes
  int currRandPatt;
  //begin loop              
  while(currentPattern==1 && pChange==0)
  {
       //reset the led levels
      for(int i=0;i<48;i++)
      {
         tlc.setPWM(i, 0);
      }
     tlc.write();
     
      //pick the next pattern at random
      //the total patterns minus the sound activated and the current pattern
      currRandPatt=random(0,totalPatterns-2);      
      //get the run time for the pattern
      pattTime=random(minPattTime,maxPattTime);
      switch (currRandPatt)
      {
         case 0:
            currentPattern=3;
            runPattern3(pattTime);
            currentPattern=1;
           break;
         case 1:
             currentPattern=4;
             runPattern4(pattTime);
             currentPattern=1;
           break;
         case 2:
             currentPattern=5;
             runPattern5(pattTime);
             currentPattern=1;
           break;
         case 3:
             currentPattern=6;
             runPattern6(pattTime);
             currentPattern=1;
           break;
      }
  }

}

// 2. SOUND ACTIVATED
//*******************************************************************************************************************                
void runPattern2()
{
  int i,j;
  int fadeStep=150;
  int ledOffset=0;
  int maxLedsOn=int(double(totalLeds)*.88);
  int chanHeight=0;  //holds the number of LEDs to activate
  int chanHeightTotal=0; //holds the total number of LEDs to activate
                            //this is to protect against all the lights coming on
                            //because of static or equipment pop   
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

 int msgLedOrd[]={ 7, 2, 6,42,17,
                  41,24,30,26,25,29,28,
                   5,36,31,38,37,32,27,
                  45,11, 4,10, 3,47,
                  46,21,13,18,19,15,12,
                   9,14,22,23,20,16, 0,
                   1, 8,44,40,43,39};

  int bandMinGap[]={100,100,100,100,100,100,100};  //Min gap between recentMin and recentMax

//  for (int i = 0; i < 7; i++)
//  {    
//      Serial.print(bandMinGap[i]);
//      Serial.print("   ");
//  }
//  Serial.println();
//  Serial.println();
  
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
         
     delay(5);
  }
}

//flowers pattern
//************************************************************************************************
//lit up a central led, then have the surrounding leds
//move in a circle around it
void runPattern3(long runLength)
{
  int i,j;
  int dir=1;
  int delayAmount=100;
  int fadeStep=150;
  byte pedalChangeCntThresh=2;
  int maxTime=int(20000.0/delayAmount); //max=20 seconds
  int minTime=int(12000.0/delayAmount);  //min=8 seconds
  //Serial.print("maxTime=");
  //Serial.print(maxTime);
  //Serial.print("minTime=");
  //Serial.println(minTime);
  //(This part is confusing) indOffset is the offset from the
  //center of the flower for each petal. There are six petals for each flower.
  //The offsets are the changes in the index value.
  //Example: If you consider the physical layout of the leds you have a row of 6
  //a row of 7, a row of 6, and continuing on
  //  0, 1, 2, 3, 4, 5
  //6, 7, 8, 9,10,11,12
  // 13,14,15,16,17,18
  //If led 7 is the flower center, then the petals are 6,0,1,8,14,13.
  //These are the values you get if you add the values in indOffset to 7.
  //It is more confusing though because the channel on the TLC is not in
   //the order of the layout of the LEDs so we have to index into ledIndArray.
  int indOffset[] = {-1,-7,-6,1,7,6};
  byte numPetals = 6;
  //The index into ledIndArray of the leds that are able to be flower centers.
  //Basically, this is all the LEDs that are not on one of the edges. The reason
   //I am using the index in ledIndArray is because the calculation to find the
  //surrounding petals is based on the row and column of the led array. 
  int flowerInds[] = { 7,8,9,10,11,
                        14,15,16,17,
                        20,21,22,23,24,
                        27,28,29,30,
                        33,34,35,36,37};
                        
  byte numFlowers = 23;  //the number of flowers
  
  long f1Ind=random(0,numFlowers-1);  //initial flower 1 center
  long f1TimeOut=random(minTime,maxTime);  //flower 1 timeout
  long f1Time=0;      //flower 1 current time
  //byte pedalChangeCntThresh = 1;  //After this many iterations, change the petal
  byte f1CurrPedalChangeCnt = 1;//random(0,pedalChangeCntThresh);
  byte f2CurrPedalChangeCnt = 1;//random(0,pedalChangeCntThresh);
  //bool f1ChangePetal = false;  //change the petal every other iterations
  //bool f2ChangePetal = true;  //change the petal every other iterations
  int f1CurrPetal=random(0,numPetals);
  int f1CurrDir=random(0,2);  //Determines the direction of travel for the petals. 0 = CCW, 1=CW.
  long f2Ind=random(0,numFlowers-1);
  //make sure that flower 2 is different than flower 1
  while(f1Ind==f2Ind)
      f2Ind=random(0,numFlowers-1);
      
  long f2TimeOut=random(minTime,maxTime);  //flower 2 timeout
  long f2Time=0;      //flower 2 current time
  int f2CurrPetal=random(0,numPetals);
  int f2CurrDir=random(0,2);  //Determines the direction of travel for the petals. 0 = CCW, 1=CW.
  //int fadeStep=(int)(250*ovIntScaleVal);  //Adjust the fadestep for the current intensity scaling factor
  
  int ledLevel[]={0,0,0,0,0,0,0,0,0,0, //stores the level for each led
                  0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0};
        
  //offsets are where to start finding the values for each flower
  //int ledOffsets[]={0,8,16,23,32,39,47,54,60,67};
  long totRunTime=-2;   //the total time the pattern has run
  
  while(currentPattern==3 && pChange==0 && totRunTime<runLength)
  {
    //Set the LEDs to zero
    for(int j=0; j < 48; j++)
    {
        tlc.setPWM(j,0);
    }
      
    //flower 1
    if(f1Time<f1TimeOut)
    {
      //keep the central led on
      ledLevel[ledIndArray[flowerInds[f1Ind]]]=4000;      
      //set the current petal to a bright value
      ledLevel[ledIndArray[flowerInds[f1Ind]+indOffset[f1CurrPetal]]]=3000;      
      //After a certain number of iterations, change the petal
      if(f1CurrPedalChangeCnt>=pedalChangeCntThresh)
      {
        if(f1CurrDir==0)
        f1CurrPetal++;
        else
        f1CurrPetal--;
        
        f1CurrPedalChangeCnt=0;
      }
      else
        f1CurrPedalChangeCnt++;    
      //check to make sure we have not reached the last pedal
      if(f1CurrPetal>=numPetals)
        f1CurrPetal=0;
      if(f1CurrPetal<0)
        f1CurrPetal=numPetals-1;
      //increase the time
      f1Time++;
    }
    else
    {
      //get the next flower values
      f1Ind=random(0,numFlowers-1);  //initial flower 1 center
      while(f1Ind==f2Ind)
      f1Ind=random(0,numFlowers-1);
      
      f1TimeOut=random(minTime,maxTime);  //flower 1 timeout
      f1Time=0;      //flower 1 current time
      f1CurrPetal=random(0,numPetals);
      f1CurrDir=random(0,2);  //Determines the direction of travel for the petals. 0 = CCW, 1=CW.
    }
          
    //flower 2
    if(f2Time<f2TimeOut)
    {
      //keep the central led on
      ledLevel[ledIndArray[flowerInds[f2Ind]]]=4000;      
      //set the current pedal to a bright value
      ledLevel[ledIndArray[flowerInds[f2Ind]+indOffset[f2CurrPetal]]]=3000;       
      //After a certain number of iterations, change the petal
      if(f2CurrPedalChangeCnt>=pedalChangeCntThresh)
      {
        if(f1CurrDir==0)
        f2CurrPetal++;
        else
        f2CurrPetal--;
        
        f2CurrPedalChangeCnt=0;
      }
      else
        f2CurrPedalChangeCnt++;
     
      //check to make sure we have not reached the last pedal
      if(f2CurrPetal>=numPetals)
        f2CurrPetal=0;
      if(f2CurrPetal<0)
        f2CurrPetal=numPetals-1;
      //increase the time
      f2Time++;     
    }
    else
    {       
      //get the next flower values
      f2Ind=random(0,numFlowers-1);  //initial flower 2 center
      while(f1Ind==f2Ind)
        f2Ind=random(0,numFlowers-1);
      
      f2TimeOut=random(minTime,maxTime);  //flower 2 timeout
      f2Time=0;      //flower 2 current time
      f2CurrPetal=random(0,numPetals);
      f2CurrDir=random(0,2);  //Determines the direction of travel for the petals. 0 = CCW, 1=CW.
    }
          
     //Fade all the leds except the flower center
     for(i=0;i<48;i++)
     {
       if(i!=ledIndArray[flowerInds[f1Ind]] && i!=ledIndArray[flowerInds[f2Ind]])
       {
         ledLevel[i] = ledLevel[i]-fadeStep;
        //check to make sure the value is greater than 0
        if(ledLevel[i]<0)
          ledLevel[i]=0; 
       }
     }  
      //set the TLC
      for(i=0;i<48;i++)
      {
        int adjValue = (int)(ledLevel[i]*ovIntScaleVal);
        tlc.setPWM(i, adjValue);
      }    
      /* Tlc.update() sends the data to the TLCs.  This is when the LEDs will
         actually change. */
      tlc.write();
      
      delay(delayAmount);
      
      //check to make sure the pattern was called from the random pattern selector
      if(runLength!=-1)
          totRunTime=totRunTime+delayAmount;          
  }
}


//by color
//******************************************************************************************************
void runPattern4(long runLength)
{
  int i;
  randomSeed(analogRead(5));  //seed the random number with an unconnected pin(5) read value
  int fadeOutStepMin=50;
  int fadeOutStepMax=150;
  int fadeInStepMin=30;
  int fadeInStepMax=100;
  int fadeOutStep=random(fadeOutStepMin,fadeOutStepMax);
  int fadeInStep=random(fadeInStepMin,fadeInStepMax);
  int ledOffset=0;
  int delayAmount=200;
  int dir=1;  //direction; fading in=1 fading out=-1
  //byte ledInd;
  
  int ledLevel[]={0,0,0,0,0,0,0,0,0,0, //stores the level for each led
                  0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0};
               
  //by color fade variables
  int bcNum=8;
  int bcLedNum[]={6,6,6,6,6,6,3,6}; //orange, aqua, red, yellow, green, violet, white, blue, 
  //Note: Use the LED index from the physical layout. The physical layout is numbered 0-47
  //top to bottom, left to right. Viewed from the top of the table with the control panel at the top.
  int bcLedOrd[]={4,12,19,27,34,42,
                  5,13,20,28,35,43,
                  1,9,16,24,31,39,
                  3,11,18,26,33,41,
                  2,10,17,25,32,40,
                  6,14,21,29,36,44,
                  7,22,37,
                  0,8,15,23,30,38};
  long totRunTime=-2;   //the total time the pattern has run
  int currCol=random(0,bcNum);
  //calculate the offset
  for(i=0;i<currCol;i++)
     ledOffset=ledOffset+bcLedNum[i];

  //Set the LEDs to zero
  for(int j=0; j < 48; j++)
  {
      tlc.setPWM(j,0);
  }
  
  while(currentPattern==4 && pChange==0 && totRunTime<runLength)
  {  
      //fade currently lit leds
      if(dir==1) //fading in
      {
        for(i=ledOffset;i<(ledOffset+bcLedNum[currCol]);i++)
        {
            ledLevel[ledIndArray[bcLedOrd[i]]]=ledLevel[ledIndArray[bcLedOrd[i]]]+fadeInStep;
            
            if(ledLevel[ledIndArray[bcLedOrd[i]]]>4000)
            {
              ledLevel[ledIndArray[bcLedOrd[i]]]=4000;
              dir=-1;
            }
        }
      }
      else   //fading out
      {
        for(i=ledOffset;i<(ledOffset+bcLedNum[currCol]);i++)
        {
            ledLevel[ledIndArray[bcLedOrd[i]]]=ledLevel[ledIndArray[bcLedOrd[i]]]-fadeOutStep;
            
            if(ledLevel[ledIndArray[bcLedOrd[i]]]<0) //change direction
            {
              ledLevel[ledIndArray[bcLedOrd[i]]]=0;
              dir=1;
            }
        }
        if(dir==1)  //change to next color
        {
            ledOffset=ledOffset+bcLedNum[currCol];
              if(currCol==(bcNum-1)) //advance the current color number
              {
                currCol=0;
                ledOffset=0;
              }
              else
              {
                currCol++;
              }
              //randomly select the fade amounts
              fadeOutStep=random(fadeOutStepMin,fadeOutStepMax);
              fadeInStep=random(fadeInStepMin,fadeInStepMax);
        }
      }
      //set the TLC
      for(i=0;i<48;i++)
      {
        //Adjust for the overall intensity scale factor
        int adjLedLevel = (int)(ledLevel[i]*ovIntScaleVal);
        tlc.setPWM(i, adjLedLevel);
      }    

     tlc.write();
      
      delay(delayAmount);
      //check to make sure the pattern was called from the random pattern selector
      if(runLength!=-1)
          totRunTime=totRunTime+delayAmount;
  }
}

//cross swirl
//*************************************************************************************************************************
void runPattern5(long runLength)
{
  int i,j;
  int delayAmount=80;   //Parameter A
  int fadeStep=200;      //Parameter B 
  //Each row is the indeces of the LEDs for a position in the swirl.
  //Note: These are indeces based on the physical layout. So they should be referenced to ledIndArray.
 //Do not include the center LED 
  byte swirlInds[] = { 0,1,8,15,29,36,43,44,
                       2,9,16,28,35,42,
                       3,10,16,28,34,41,
                       4,11,17,16,28,27,33,40,
                       5,4,10,16,28,34,40,39,
                       5,12,18,17,23,21,27,26,32,39,
                       18,17,23,21,27,26,
                       25,24,23,21,20,19,
                       31,30,23,21,14,13,
                       44,38,31,30,23,21,14,13,6,0,
                       43,37,30,29,15,14,7,1};
                       
  byte swirlCnt[] = {8,6,6,8,8,10,6,6,6,10,8};
                        
  byte numPos = 11;  //the number of swirl positions
  
  int ledLevel[]={0,0,0,0,0,0,0,0,0,0, //stores the level for each led
                  0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0};
        
  int currSwirlPos=random(0,numPos);  //initial swirl position
  //The center of all the swirls. This is the LED in the center of the table.
  //This LED will have its own pattern. Use the index from the physical layout.
  byte centralInd = 22;
  ledLevel[ledIndArray[centralInd]] = 4000; //Start with the cenral led on.
  int centLEDFadeStep = -100;
  //Change swirl position after x iterations of the loop.
  byte posChangeThresh = 10;
  byte currposChangeCnt = 0;
  long totRunTime=-2;   //the total time the pattern has run

  //Set the LEDs to zero
  for(int j=0; j < 48; j++)
      tlc.setPWM(j,0);
      
  while(currentPattern==5 && pChange==0 && totRunTime<runLength)
  {
     //Get the starting index for the current swirl position
     byte startInd = 0;
     for(i=0;i<currSwirlPos;i++)
          startInd=startInd+swirlCnt[i];
     //Stop index is the start index plus the number of leds in the current swirl position
     byte stopInd = startInd+swirlCnt[currSwirlPos];
                    
     //Turn on the leds for the current swirl position
     for(i=startInd;i<stopInd;i++)
     {
       ledLevel[ledIndArray[swirlInds[i]]] = ledLevel[ledIndArray[swirlInds[i]]] +500;
       if(ledLevel[ledIndArray[swirlInds[i]]]>4000)
         ledLevel[ledIndArray[swirlInds[i]]] = 4000;      
     }
    
     //Fade all the leds except the center
     //and the current swirl positions
     for(i=0;i<48;i++)
     {
       bool bFade = true;
       for(j=startInd;j<stopInd;j++)
       {
         if(ledIndArray[swirlInds[j]]==i)
           bFade=false;
       }
       if(i!=ledIndArray[centralInd] && bFade==true)
       {
         ledLevel[i] = ledLevel[i]-fadeStep;
         //check to make sure the value is greater than 0
         if(ledLevel[i]<0)
           ledLevel[i]=0; 
       }       
     }       

    //Fade the central LED. It will fade in and out.
    ledLevel[ledIndArray[centralInd]]=ledLevel[ledIndArray[centralInd]]+centLEDFadeStep;
    
    if(ledLevel[ledIndArray[centralInd]]>4000)
    {
      ledLevel[ledIndArray[centralInd]] = 4000;
      centLEDFadeStep=centLEDFadeStep*-1;  //Change fade direction
    }
    if(ledLevel[ledIndArray[centralInd]]<0)
    {
      ledLevel[ledIndArray[centralInd]] = 0;
      centLEDFadeStep=centLEDFadeStep*-1; //Change fade direction
    }
          
    //Move to the next position
    currposChangeCnt++;  //We only change position after a certain number of loops
    if(currposChangeCnt>posChangeThresh)
    {
      currposChangeCnt=0;
      currSwirlPos++;
      if(currSwirlPos>=numPos)
        currSwirlPos=0;    
    }
  
    //set the TLC
    for(i=0;i<48;i++)
    {
      int adjValue = (int)(ledLevel[i]*ovIntScaleVal);
      tlc.setPWM(i, adjValue);
    }    
    
    tlc.write();
    
    delay(delayAmount);
      
    //check to make sure the pattern was called from the random pattern selector
    if(runLength!=-1)
        totRunTime=totRunTime+delayAmount;          
  }  
}

//snakes
//**********************************************************************************************************
//This will light up a random LED and then randomly "snake" out from there with a fading tail.
//input: runLength - the number of milliseconds to run this pattern
void runPattern6(long runLength)
{
  int delayAmount=100;   //delay between loops
  int fadeStep=150;      //amount to fade the LEDs at each loop
  int snakeChangeCntThresh = 5; //number of loops before moving to the next LED
  //(This part is confusing) indOffset is the offset from the
  //center of the flower for each petal. There are six neighbors for each led.
  //The offsets are the changes in the index value.
  //Example: If you consider the physical layout of the leds you have a row of 6
  //a row of 7, a row of 6, and continuing on
  //  0, 1, 2, 3, 4, 5
  //6, 7, 8, 9,10,11,12
  // 13,14,15,16,17,18
  //If led 7 is the center, then the neighbors are 6,0,1,8,14,13.
  //These are the values you get if you add the values in indOffset to 7.
  //It is more confusing though because the channel on the TLC is not in
   //the order of the layout of the LEDs so we have to index into ledIndArray.
  int indOffset[] = {-1,-7,-6,1,7,6};
  byte numNeighs = 6;

  //Edge leds
  //All the LEDs on the border of the table. When we hit one of these we will change direction.
  byte borderLEDList[] = {0,
                          1,2,3,4,
                          5,
                          6,19,32,
                          13,26,
                          18,31,
                          12,25,38,
                          39,
                          40,41,42,43,
                          44};
  int numBorderLED = 22;
  int potDirList[] =  {/*0*/1,7,6,
                       /*1*/-1,1,6,7,/*2*/-1,1,6,7,/*3*/-1,1,6,7,/*4*/-1,1,6,7,
                       /*5*/-1,6,7,
                       /*6*/-6,1,7,/*19*/-6,1,7,/*32*/-6,1,7,
                       /*13*/-7,-6,1,6,7,/*26*/-7,-6,1,6,7,
                       /*18*/-6,-7,-1,6,7,/*31*/-6,-7,-1,6,7,
                       /*12*/-7,-1,6,/*25*/-7,-1,6,/*38*/-7,-1,6,
                       /*39*/-7,-6,1,
                       /*40*/-1,-7,-6,1,/*41*/-1,-7,-6,1,/*42*/-1,-7,-6,1,/*43*/-1,-7,-6,1,
                       /*44*/-1,-7,-6};
  byte potDirCntList[] = {3,
                          4,4,4,4,
                          3,
                          3,3,3,
                          5,5,
                          5,5,
                          3,3,3,
                          3,
                          4,4,4,4,
                          3};
                              
  int s1Ind=22;  //initial led 1 center
  int s1IndPrev = 0;
  int s1CurrOffset=indOffset[random(0,numNeighs)];    
  byte s1CurrChangeCnt = 1;
    
  int ledLevel[]={0,0,0,0,0,0,0,0,0,0, //stores the level for each led
                  0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0};
        
  int totRunTime=-2;   //the total time the pattern has run
  
  while(currentPattern==6 && pChange==0 && totRunTime<runLength)
  {           
     //turn the central led on
     ledLevel[ledIndArray[s1Ind]]=4000;
     
     //After a certain number of iterations, move to the next LED
    if(s1CurrChangeCnt>=snakeChangeCntThresh)
    {
      //Check to see if we are on a border LED.
      //If we are, then we need to change direction.
      bool isChangeDir = false;
      int borderLEDInd=0;      
      for(int i=0;i<numBorderLED;i++)
      {        
        if(s1Ind == borderLEDList[i])
        {
          isChangeDir=true;
          borderLEDInd=i;
          break;
        }
      }
      //Change direction if necessary
      if(isChangeDir==true)
      {
        byte numPotDirs = potDirCntList[borderLEDInd];
        byte newDirInd = random(0,numPotDirs);
        int newDirBeginInd = 0;
        for(int i=0;i< borderLEDInd;i++)
          newDirBeginInd = newDirBeginInd+potDirCntList[i];
        s1CurrOffset = potDirList[newDirBeginInd+newDirInd];
      }
      
      //Continue in the current direction
      s1Ind = s1Ind + s1CurrOffset;

      s1CurrChangeCnt=0;
    }
  
     //Fade all the leds
     for(int i=0;i<48;i++)
     {
       ledLevel[i] = ledLevel[i]-fadeStep;
      //check to make sure the value is greater than 0
      if(ledLevel[i]<0)
        ledLevel[i]=0;       
     }  
      //set the TLC
      for(int i=0;i<48;i++)
      {
        int adjValue = (int)(ledLevel[i]*ovIntScaleVal);
        tlc.setPWM(i, adjValue);
      }    
     
      tlc.write();
      
      delay(delayAmount);
      
      //check to make sure the pattern was called from the random pattern selector
      if(runLength!=-1)
          totRunTime=totRunTime+delayAmount;

       s1CurrChangeCnt++;
  }  
}

void loop() 
{      
  delay(1000);
  //*********on interrupt, turn off all LEDs*********************
  //Set the LEDs to zero
  for(int j=0; j < 48; j++)
  {
      tlc.setPWM(j,0);
  }
  tlc.write();
  //****************************************************************
  //move to the next pattern after an interrupt
  if(pChange==1)
  {
    if(currentPattern<totalPatterns)  
      currentPattern=currentPattern+1;
    else
      currentPattern=1;
      //reset pChange
    pChange=0;
  }
  else if(pChange==2)   //No motion was detected
  {
    //Set the LEDs to zero
    for(int j=0; j < 48; j++)
    {
        tlc.setPWM(j,0);
    }
    tlc.write();
    //pChange will be reset by the motion_ISR
    //Wait for a long time. The interrupt will still work so as soon as motion is detected, we will wake up.
    while(pChange==2)
    {
      delay(5000);
    }
  }
    
  //passing -1 to the patterns means that they have no timeout limit
  if(currentPattern==1)
    runPattern1();
  else if(currentPattern==2)
    runPattern2();
  else if(currentPattern==3)
    runPattern3(-1);
  else if(currentPattern==4)
    runPattern4(-1);
  else if(currentPattern==5)
    runPattern5(-1);
  else if(currentPattern==6)
    runPattern6(-1);  
}

