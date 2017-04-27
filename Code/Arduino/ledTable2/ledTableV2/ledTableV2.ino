#include "Tlc5940.h"
#include <avr/interrupt.h>

const int totalPatterns=8;
const int totalLeds=45;

//This is a representation of the layout of the LEDs viewed from above with the control panel above Row 1.
//There are 7 rows, Rows 1,3,5,7 have 6 LEDs
//Rows 2,4,6 have 7 LEDs
byte ledIndArray[]={  42,44,46,43,47,45, //the index for each led
                36,37,38,35,39,41,40,
                 29,30,31,32,34,33,
                18,21,25,27,26,24,28,
                 19,17,20,16,23,22,
                 9,10,13,14, 4,11, 8,
                  3, 5, 6, 7,12,15};
                  
volatile int pChange;          // interrupt?
int currentPattern;
int k;
//MSG pin setup
int analogPin = 0; // read from multiplexer using analog input 0
int strobePin = 7; // strobe is attached to digital pin 2
int resetPin = 5; // reset is attached to digital pin 3
              

void setup() 
{
  //**********interrupt setup **************
  pinMode(2, INPUT);      // Make digital 2 an input
  // attach our interrupt pin to it's ISR
  attachInterrupt(0, patternChange, LOW);
  // we need to call this to enable interrupts
  interrupts();
  pChange=0;
  currentPattern=3;
  
  //Serial.begin(9600);
  //*********msg setup********************
  pinMode(analogPin, INPUT);
  pinMode(strobePin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  analogReference(DEFAULT);

  digitalWrite(resetPin, LOW);
  digitalWrite(strobePin, HIGH);
  
  randomSeed(analogRead(2));  //seed the random number with an unconnected pin(2) read value
  
  k=0;
  /* Call Tlc.init() to setup the tlc.
     You can optionally pass an initial PWM value (0 - 4095) for all channels.*/
  Tlc.init();
  //Serial.println("MSGEQ7 and LED test by Jacob");
}

// The interrupt hardware calls this 
void patternChange()
{
  pChange=1;
}


//randomly select a pattern and a timelength to run it
void runPattern1()
{
  randomSeed(analogRead(2));  //seed the random number with an unconnected pin(2) read value
  int pattTime;
  int minPattTime=30000;  //30 seconds
  int maxPattTime=300000; //300 seconds or 5 minutes
  int currRandPatt;
  //begin loop              
  while(currentPattern==1 && pChange==0)
  {
         /* Tlc.clear() sets all the grayscale values to zero, but does not send
         them to the TLCs.  To actually send the data, call Tlc.update() */
      Tlc.clear();
       //reset the led levels
      for(int i=0;i<totalLeds;i++)
      {
         Tlc.set(i, 0);
      }
     /* Tlc.update() sends the data to the TLCs.  This is when the LEDs will
         actually change. */
      Tlc.update();
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
         case 4:
             currentPattern=7;
             runPattern7(pattTime);
             currentPattern=1;
           break; 
         case 5:
             currentPattern=8;
             runPattern8(pattTime);
             currentPattern=1;
           break;   
      }
  }

}

//SOUND ACTIVATED
                
void runPattern2()
{
  int i,j;
  int fadeStep=500;
  int maxLedsOn=int(double(totalLeds)*.88);
  int ledLevel[]={0,0,0,0,0,0,0,0,0,0, //stores the level for each led
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0};
  //msg variables
  int spectrumValue[7]; // to hold a2d values
  int recentMax[]={15,15,15,15,15,15,15}; //hold the recent max value returned by MSG
  int recentMin[]={60,60,60,60,60,60,60}; //hold the recent min value returned by MSG
  int minMaxStep=5;
  int msgVal;
  int bandLedNum[]={8,7,6,5,6,4,5};
  //store the order of leds to use in the msg equalizer
  //there are 7 bands.  Each bands leds are offset by its value in bandLedNum
  int msgLedOrd[]={13,7,12,8,17,18,9,14,
                  27,26,30,28,22,23,24,
                  20,16,15,21,25,19,
                  6,0,1,2,3,
                  34,29,35,39,38,33,
                  5,4,11,10,
                  37,32,40,31,36};
  
   /* Tlc.clear() sets all the grayscale values to zero, but does not send
         them to the TLCs.  To actually send the data, call Tlc.update() */
    Tlc.clear();
   /* Tlc.update() sends the data to the TLCs.  This is when the LEDs will
         actually change. */
    Tlc.update();
    delay(100);
    
  while(currentPattern==2 && pChange==0)
  {
    digitalWrite(resetPin, HIGH);
    digitalWrite(resetPin, LOW);

    //get the response from the MSG
    for (i = 0; i < 7; i++)
    {
      digitalWrite(strobePin, LOW);
      delayMicroseconds(30); // to allow the output to settle
      msgVal=analogRead(analogPin);
      spectrumValue[i] = msgVal;
      
      //keep track of the max for scaling
      if(msgVal>recentMax[i])
      {
        recentMax[i]=msgVal;
      }
      //keep track of the min for scaling
//      if(msgVal<recentMin[i])
//      {
//        recentMin[i]=msgVal;
//      }
        
      digitalWrite(strobePin, HIGH);
    }
    
    //signal the LED
    /* Tlc.clear() sets all the grayscale values to zero, but does not send
         them to the TLCs.  To actually send the data, call Tlc.update() */
    Tlc.clear();
    
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
    
      /* Tlc.set(channel (0-15), value (0-4095)) sets the grayscale value for
         one channel (15 is OUT15 on the first TLC, if multiple TLCs are daisy-
         chained, then channel = 16 would be OUT0 of the second TLC, etc.).
  
         value goes from off (0) to always on (4095).
  
         Like Tlc.clear(), this function only sets up the data, Tlc.update()
         will send the data. */
     int chanHeight=0;  //holds the number of LEDs to activate
     int ledOffset=0;   //holds the current offset for the led index
     int chanHeightTotal=0; //holds the total number of LEDs to activate
                            //this is to protect against all the lights coming on
                            //because of static or equipment pop   
      //for each frequency band
      for (i = 0; i < 7; i++)
      {
          //calculate the percentage of the max
          //and multiply it times the number of leds that represent the current band
         chanHeight=(int)((float((spectrumValue[i]-recentMin[i]))/((float)recentMax[i]-recentMin[i]))*bandLedNum[i]);
         //chanHeight=(int)((float((spectrumValue[i]-(recentMin[i]/2)))/(recentMax[i]-recentMin[i]))*bandLedNum[i]);
         chanHeightTotal=chanHeightTotal+chanHeight;
         //turn on the lights
         for(j=0; j < chanHeight; j++)
         {
             ledLevel[j+ledOffset]=4000;
         }
         //update the offset
         ledOffset=ledOffset+bandLedNum[i];
      }
      
      //set the TLC
      
      if(chanHeightTotal<maxLedsOn) //only if the total number of lit LEDs is less than 88%
      {                                 //update the lights
        for(i=0;i<totalLeds;i++)
        {
          Tlc.set(msgLedOrd[i], ledLevel[i]);
        }
      }
      else   //only turn on the leds up to maxLedsOn
      {
        for(i=0;i<maxLedsOn;i++)
        {
          Tlc.set(msgLedOrd[i], ledLevel[i]);
        }
      }
      
        /* Tlc.update() sends the data to the TLCs.  This is when the LEDs will
         actually change. */
       Tlc.update();
         
      //adapt the min and max by slowly increasing or decresing
      //this should force it to adjust its range when songs are louder or softer
//      for (i = 0; i < 7; i++)
//      {
//        if(recentMax[i]>550)  //the min for the max is 550 
//          recentMax[i]= recentMax[i]-minMaxStep;
//        
//        if(recentMin[i]<101)  //the max possible value for min is 101
//         recentMin[i]= recentMin[i]+minMaxStep;
//         
//      }
      
      delay(100);
  }
}

//race one led at a time
//with a fading comet tail
//input: runLength - the number of milliseconds to run this pattern
void runPattern3(int runLength)
{
  byte ledInd;
  int dir=1;
  int fadeStep=900;
  int delayAmount=50;
  //stores the level for each led
  int ledLevel[]={0,0,0,0,0,0,
                 0,0,0,0,0,0,0,
                  0,0,0,0,0,0,
                 0,0,0,0,0,0,0,
                  0,0,0,0,0,0,
                 0,0,0,0,0,0,0,
                  0,0,0,0,0,0};
  int totRunTime=-2;   //the total time the pattern has run
  //begin loop              
  while(currentPattern==3 && pChange==0 && totRunTime<runLength)
  {
    /* Tlc.clear() sets all the grayscale values to zero, but does not send
       them to the TLCs.  To actually send the data, call Tlc.update() */
    
    Tlc.clear();
    //set the levels in the array
    for(int j=0; j < totalLeds; j++)
    {
      ledInd = ledIndArray[j];
      if(j==k)  //set the current led to its max value
      {
        ledLevel[ledInd]=4000;  
      }
      else    //fade the led if it is on
      {
        if(ledLevel[ledInd]>0)
        {
          ledLevel[ledInd]=ledLevel[ledInd]-fadeStep;
          if(ledLevel[ledInd]<0)  //check to see if the value is negative
            ledLevel[ledInd]=0;
        }
      }
    } 
    //set the led values in the tlc
    for(int j=0; j < 48; j++)
      {
        ledInd = ledIndArray[j];
        if(j<totalLeds)
          Tlc.set(ledInd,ledLevel[ledInd]);
        else
          Tlc.set(ledInd, 0);
      } 
      k=k+dir;
      //reset k when it reaches the end
      if(k==totalLeds-1)
      {
        dir=-1;
      }
      else if(k==0)
      {
        dir=1;
      }
        
    /* Tlc.update() sends the data to the TLCs.  This is when the LEDs will
       actually change. */
    Tlc.update();

    delay(delayAmount);
    //check to make sure the pattern was called from the random pattern selector
    if(runLength!=-1)
       totRunTime=totRunTime+delayAmount;
  }
}

//flowers pattern
//lit up a central led, then have the surrounding leds
//move in a circle around it
void runPattern4(int runLength)
{
  //left to right fade variables
  int i,j;
  int dir=1;
  int delayAmount=100;
  int maxTime=int(10000.0/delayAmount); //max=11 seconds
  int minTime=int(5000.0/delayAmount);  //min=5 seconds
  int flowerNum=10;  //the number of flowers
  int f1Ind=random(0,flowerNum-1);  //initial flower 1 center
  int f1TimeOut=random(minTime,maxTime);  //flower 1 timeout
  int f1Time=0;      //flower 1 current time
  int f1CurrPedal=0;
  int f2Ind=random(0,flowerNum-1);
  //make sure that flower 2 is different than flower 1
  while(f1Ind==f2Ind)
      f2Ind=random(0,flowerNum-1);
      
  int f2TimeOut=random(7,50);  //flower 2 timeout
  int f2Time=0;      //flower 2 current time
  int f2CurrPedal=0;
  int fadeStep=1650;
  
  int ledLevel[]={0,0,0,0,0,0,0,0,0,0, //stores the level for each led
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0};
               
  
  int ledNum[]={8,8,7,9,7,8,7,6,7,6};  //number of LEDs in each column
  //the first value in each set is the central led, the rest are the pedals
  //some will rotate clockwise, some counterclockwise
  int ledOrd[]={36,33,34,39,38,37,31,32, //8 //the order to turn on the LEDs
                38,36,37,27,28,29,40,39, //8
                24,19,23,27,28,25,20,    //7
                12,11,6,7,10,13,18,17,16,//9
                8,9,3,7,10,14,15,        //7
                6,2,7,10,12,11,5,1,      //8
                3,8,9,4,0,2,7,           //7
                15,14,8,9,21,20,         //6
                17,18,23,22,26,16,12,    //7
                40,30,35,39,38,29};      //6  73 total
  //offsets are where to start finding the values for each flower
  int ledOffsets[]={0,8,16,23,32,39,47,54,60,67};
  int totRunTime=-2;   //the total time the pattern has run
  
  while(currentPattern==4 && pChange==0 && totRunTime<runLength)
  {
      /* Tlc.clear() sets all the grayscale values to zero, but does not send
         them to the TLCs.  To actually send the data, call Tlc.update() */
      Tlc.clear();
      
      //flower 1
     if(f1Time<f1TimeOut)
     {
       //keep the central led on
       ledLevel[ledOrd[ledOffsets[f1Ind]]]=4000;
       //set the current pedal to max value
       ledLevel[ledOrd[ledOffsets[f1Ind]+1+f1CurrPedal]]=4000;
       //fade the pedals
       //start at the first pedal, which is in position 1
       for(i=ledOffsets[f1Ind]+1; i < ledOffsets[f1Ind]+ledNum[f1Ind]; i++)
       {
         //fade the pedals  
         ledLevel[ledOrd[i]]=ledLevel[ledOrd[i]]-fadeStep;
         //check to make sure the value is greater than 0
           if(ledLevel[ledOrd[i]]<0)
              ledLevel[ledOrd[i]]=0;           
       }
       //change the current pedal
       f1CurrPedal++;
       //check to make sure we have not reached the last pedal
       if(f1CurrPedal>=ledNum[f1Ind]-1)
         f1CurrPedal=0;
       //increase the time
       f1Time++;
     }
    else
     {
       //turn off all the lights for the current flower 
       for(i=ledOffsets[f1Ind]; i < ledOffsets[f1Ind]+ledNum[f1Ind]; i++)
       {
         ledLevel[ledOrd[i]]=0;
       }
       //get the next flower values
       f1Ind=random(0,flowerNum-1);  //initial flower 1 center
       while(f1Ind==f2Ind)
         f1Ind=random(0,flowerNum-1);
         
       f1TimeOut=random(minTime,maxTime);  //flower 1 timeout
       f1Time=0;      //flower 1 current time
       f1CurrPedal=0;
     }
    
    //flower 2
   if(f2Time<f2TimeOut)
     {
       //keep the central led on
       ledLevel[ledOrd[ledOffsets[f2Ind]]]=4000;
       //set the current pedal to max value
       ledLevel[ledOrd[ledOffsets[f2Ind]+1+f2CurrPedal]]=4000;
       //fade the pedals
       //start at the first pedal, which is in position 1
       for(i=ledOffsets[f2Ind]+1; i < ledOffsets[f2Ind]+ledNum[f2Ind]; i++)
       {
         //fade the pedals  
         ledLevel[ledOrd[i]]=ledLevel[ledOrd[i]]-fadeStep;
         //check to make sure the value is greater than 0
           if(ledLevel[ledOrd[i]]<0)
              ledLevel[ledOrd[i]]=0;           
       }
       //change the current pedal
       f2CurrPedal++;
       //check to make sure we have not reached the last pedal
       if(f2CurrPedal>=ledNum[f2Ind]-1)
         f2CurrPedal=0;
       //increase the time
       f2Time++;
     }
    else
     {
       //turn off all the lights for the current flower 
       for(i=ledOffsets[f2Ind]; i < ledOffsets[f2Ind]+ledNum[f2Ind]; i++)
       {
         ledLevel[ledOrd[i]]=0;
       }
       //get the next flower values
       f2Ind=random(0,flowerNum-1);  //initial flower 2 center
       while(f2Ind==f1Ind)
         f2Ind=random(0,flowerNum-1);
         
       f2TimeOut=random(minTime,maxTime);  //flower 2 timeout
       f2Time=0;      //flower 1 current time
       f2CurrPedal=0;
     } 
     
      //set the TLC
      for(i=0;i<totalLeds;i++)
      {
          Tlc.set(i, ledLevel[i]);
      }    
      /* Tlc.update() sends the data to the TLCs.  This is when the LEDs will
         actually change. */
      Tlc.update();
      
      delay(delayAmount);
      //check to make sure the pattern was called from the random pattern selector
      if(runLength!=-1)
          totRunTime=totRunTime+delayAmount;
  }
}


//by color
//
void runPattern5(int runLength)
{
  int i;
  randomSeed(analogRead(2));  //seed the random number with an unconnected pin(2) read value
  int fadeOutStepMin=50;
  int fadeOutStepMax=150;
  int fadeInStepMin=30;
  int fadeInStepMax=100;
  int fadeOutStep=random(fadeOutStepMin,fadeOutStepMax);
  int fadeInStep=random(fadeInStepMin,fadeInStepMax);
  int ledOffset=0;
  int delayAmount=200;
  int dir=1;  //direction; fading in=1 fading out=-1
  
  int ledLevel[]={0,0,0,0,0,0,0,0,0,0, //stores the level for each led
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0};
               
  //by color fade variables
  int bcNum=6;
  int bcLedNum[]={6,7,7,7,7,7}; //orange, red, aqua, yellow,  green, blue, 
  int bcLedOrd[]={33,29,18,21,14,1,
                36,40,27,16,19,7,9,
                34,35,28,23,13,5,4,
                37,22,30,20,12,10,3,
                31,38,24,17,11,15,2,
                32,39,26,25,8,6,0};
  int totRunTime=-2;   //the total time the pattern has run
  int currCol=random(0,bcNum);
  //calculate the offset
  for(i=0;i<currCol;i++)
     ledOffset=ledOffset+bcLedNum[i];
  
  while(currentPattern==5 && pChange==0 && totRunTime<runLength)
  {
      /* Tlc.clear() sets all the grayscale values to zero, but does not send
         them to the TLCs.  To actually send the data, call Tlc.update() */
      Tlc.clear();
      
      //fade currently lit leds
      if(dir==1) //fading in
      {
        for(i=ledOffset;i<(ledOffset+bcLedNum[currCol]);i++)
        {
            ledLevel[bcLedOrd[i]]=ledLevel[bcLedOrd[i]]+fadeInStep;
            
            if(ledLevel[bcLedOrd[i]]>4000)
            {
              ledLevel[bcLedOrd[i]]=4000;
              dir=-1;
            }
        }
      }
      else   //fading out
      {
        for(i=ledOffset;i<(ledOffset+bcLedNum[currCol]);i++)
        {
            ledLevel[bcLedOrd[i]]=ledLevel[bcLedOrd[i]]-fadeOutStep;
            
            if(ledLevel[bcLedOrd[i]]<0) //change direction
            {
              ledLevel[bcLedOrd[i]]=0;
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
      for(i=0;i<totalLeds;i++)
      {
        Tlc.set(i, ledLevel[i]);
      }    
      /* Tlc.update() sends the data to the TLCs.  This is when the LEDs will
         actually change. */
      Tlc.update();
      
      delay(delayAmount);
      //check to make sure the pattern was called from the random pattern selector
      if(runLength!=-1)
          totRunTime=totRunTime+delayAmount;
  }
}

//left to right
void runPattern6(int runLength)
{
  //left to right fade variables
  int i,j;
  int delayAmount=250;
  int dir=1;
  int ledOffset;
  int currColTimeOut=10;
  int currColTime=0;
  int fadeOutStep=350;
  int fadeInStep=4000/currColTimeOut;
  int currColVal=fadeInStep;
  int currCol=0;
  int ledLevel[]={0,0,0,0,0,0,0,0,0,0, //stores the level for each led
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0};
               
  
  int ltrNum=7;  //the number of columns
  
  int ltrLedNum[]={5,5,6,6,6,7,6};  //number of LEDs in each column
  int ltrLedOrd[]={32,33,34,39,35,  //the order to turn on the LEDs
                31,36,37,38,40,
                26,22,27,28,29,30,
                16,17,18,23,24,25,
                11,12,13,19,20,21,
                5,6,7,10,8,14,15,
                1,2,3,9,0,4};
  int totRunTime=-2;   //the total time the pattern has run
  
  while(currentPattern==6 && pChange==0 && totRunTime<runLength)
  {
      /* Tlc.clear() sets all the grayscale values to zero, but does not send
         them to the TLCs.  To actually send the data, call Tlc.update() */
      Tlc.clear();
      
      //fade currently lit leds
      for(i=0;i<totalLeds;i++)
      {
          ledLevel[i]=ledLevel[i]-fadeOutStep;
          if(ledLevel[i]<0)
            ledLevel[i]=0;
      }
      //set the current column to current brightness
      ledOffset=0;   //holds the current offset for the led index 
      for(i=0; i < ltrNum; i++)
      {
          if(i==currCol)
          {
            for(j=0;j<ltrLedNum[i];j++)
            {  
              ledLevel[ltrLedOrd[ledOffset+j]]=currColVal;
            }
          }
          ledOffset=ledOffset+ltrLedNum[i];            
      } 
        
      //set the TLC
      for(i=0;i<totalLeds;i++)
      {
        Tlc.set(i, ledLevel[i]);
      }    
      /* Tlc.update() sends the data to the TLCs.  This is when the LEDs will
         actually change. */
      Tlc.update();
      
      delay(delayAmount);
      //check to make sure the pattern was called from the random pattern selector
      if(runLength!=-1)
          totRunTime=totRunTime+delayAmount;
      //check to see if it is time to move to the next column
      currColTime++;
      currColVal=currColVal+fadeInStep;
      if(currColTime>currColTimeOut)
      {
        currColTime=0;
        currColVal=fadeInStep;
        currCol=currCol+dir;
        if(currCol>=ltrNum)  //when the last column is reached, start back at 0
        {
          currCol--;
          dir=-1;
        }
        else if(currCol==-1) //when we return to the first column
        {
            currCol=0;
            dir=1; 
        }
      }
  }
}

//cross swirl
void runPattern7(int runLength)
{
  int i,j;
  int delayAmount=50;
  int dir=1;
  int ledOffset;
  int currDirTimeOut;
  int currDirTime=0;
  int currColTimeOut=5;
  int currColTime=0;
  int fadeOutStep=550;
  int fadeInStep=4000/currColTimeOut;
  int currColVal=fadeInStep;
  int currCol=0;
  int ledLevel[]={0,0,0,0,0,0,0,0,0,0, //stores the level for each led
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0};
               
  
  int ltrNum=9;  //the number of columns
  
  int ltrLedNum[]={5,7,9,11,9,9,9,6,6};  //number of LEDs in each column
  int ltrLedOrd[]={26,22,23,19,15,  //the order to turn on the LEDs
                31,27,23,19,14,8,9,
                32,37,27,24,19,14,8,3,4,
                33,34,36,37,27,23,19,14,8,3,0,
                39,38,28,24,19,13,12,6,2,
                35,40,38,27,23,18,12,6,1,
                30,29,28,24,19,13,12,11,5,
                25,20,19,18,12,11,
                21,20,19,18,17,16
                };
  int totRunTime=-2;   //the total time the pattern has run
  
  //set the time for changing the direction
  if(runLength!=-1)
      currDirTimeOut=random(runLength/10,runLength/2);
  else    //if the time is infinite, use 16 seconds
      currDirTimeOut=16000;
  
  while(currentPattern==7 && pChange==0 && totRunTime<runLength)
  {
      /* Tlc.clear() sets all the grayscale values to zero, but does not send
         them to the TLCs.  To actually send the data, call Tlc.update() */
      Tlc.clear();
      
      //fade currently lit leds
      for(i=0;i<totalLeds;i++)
      {
          ledLevel[i]=ledLevel[i]-fadeOutStep;
          if(ledLevel[i]<0)
            ledLevel[i]=0;
      }
      //set the current column to current brightness
      ledOffset=0;   //holds the current offset for the led index 
      for(i=0; i < ltrNum; i++)
      {
          if(i==currCol)
          {
            for(j=0;j<ltrLedNum[i];j++)
            {  
              ledLevel[ltrLedOrd[ledOffset+j]]=currColVal;
            }
          }
          ledOffset=ledOffset+ltrLedNum[i];            
      } 
        
      //set the TLC
      for(i=0;i<totalLeds;i++)
      {
        Tlc.set(i, ledLevel[i]);
      }    
      /* Tlc.update() sends the data to the TLCs.  This is when the LEDs will
         actually change. */
      Tlc.update();
      
      delay(delayAmount);
      //check to make sure the pattern was called from the random pattern selector
      if(runLength!=-1)
          totRunTime=totRunTime+delayAmount;
      //check to see if it is time to move to the next column
      currColTime++;
      currColVal=currColVal+fadeInStep;
      if(currColTime>currColTimeOut)
      {
        currColTime=0;
        currColVal=fadeInStep;
        currCol=currCol+dir;
        if(currCol>=ltrNum)  //when the last column is reached, start back at 0
        {
          currCol=0;
          //dir=-1;
        }
        else if(currCol==-1) //when we return to the first column
        {
            currCol=ltrNum-1;
            //dir=1; 
        }
      }
      //check to see if it is time to change direction
      currDirTime=currDirTime+delayAmount;
      if(currDirTime>currDirTimeOut)
      {
         currDirTime=0;
         dir=dir*-1; 
      }
  }
}

//snakes
void runPattern8(int runLength)
{
  int i,j;
  int delayAmount=75; //50
  int fadeOutStep=1300; //550
  int initPointVal=4000;  //the initial brightness for a point
  int numAdjPoints1;      //the number of adjacent points to the current point
  int currOffset1;        //the offset into the sLedOrd array
  int randAdjPoint1;      //the offset of the randomly selected adjacent point
  //the initial starting point
  int currPoint1=random(0,totalLeds);
  int currPoint2=random(0,totalLeds);
  int ledLevel[]={0,0,0,0,0,0,0,0,0,0, //stores the level for each led
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0};
                 
  int adjLedNum[]={3,3,5,6,3, //0-4 //number of LEDs in each column
                   3,6,6,6,4, //5-9
                   5,4,8,5,6, //10-14
                   5,4,6,5,6, //15-19
                   6,3,4,6,6, //20-24
                   6,4,7,5,5, //25-29
                   3,5,3,3,3, //30-34
                   2,7,4,7,5,5//35-40
                 };
      //the location for an index in the ltrLedOrd
  int ledOffset[]={0,3,6,11,17,20,      //0-5
                   23,29,35,41,45,      //6-10
                   50,54,62,67,73,      //11-15
                   78,82,88,93,99,      //16-20
                   105,108,112,118,124, //21-25
                   130,134,141,146,151, //26-30
                   154,159,162,165,168, //31-35
                   170,177,181,188,193  //36-40
                   };
      //the order to turn on the LEDs
  int sLedOrd[]={
                2,3,4,                 //0
                2,6,5,                 //1
                1,6,7,3,0,             //2
                0,2,7,8,9,4,           //3
                0,9,3,                 //4
                1,6,11,                //5
                1,2,7,12,11,5,         //6
                2,3,8,10,12,6,         //7
                3,9,15,14,10,7,        //8
                4,3,8,15,              //9
                7,8,14,13,12,          //10
                5,6,12,16,             //11
                6,7,10,13,18,17,16,11, //12
                10,14,19,18,12,        //13
                8,15,20,19,13,10,      //14
                9,8,14,20,21,          //15
                11,12,17,26,           //16
                16,12,18,23,22,26,     //17
                12,13,19,23,17,        //18
                13,14,20,24,23,18,     //19
                15,21,25,24,19,14,     //20
                15,20,25,              //21
                26,17,23,27,           //22
                17,18,19,24,27,22,     //23
                23,19,20,25,28,27,     //24
                21,20,24,28,29,30,     //25
                16,17,22,31,           //26
                22,23,24,28,38,37,31,  //27
                27,24,25,29,38,        //28
                28,25,30,40,38,        //29
                25,29,40,              //30
                26,27,37,36,32,        //31
                31,36,33,              //32
                32,36,34,              //33
                33,36,39,              //34
                39,40,                 //35
                31,37,38,39,34,33,32,  //36
                31,27,38,36,           //37
                37,27,28,29,40,39,36,  //38
                34,36,38,40,35,        //39
                30,29,38,39,35         //40
                };
  int totRunTime=-2;   //the total time the pattern has run
  
    
  while(currentPattern==8 && pChange==0 && totRunTime<runLength)
  {
      /* Tlc.clear() sets all the grayscale values to zero, but does not send
         them to the TLCs.  To actually send the data, call Tlc.update() */
      Tlc.clear();
      
      //fade currently lit leds
      for(i=0;i<totalLeds;i++)
      {
          ledLevel[i]=ledLevel[i]-fadeOutStep;
          if(ledLevel[i]<0)
            ledLevel[i]=0;
      }
      //set the current points intensity
      ledLevel[currPoint1]=initPointVal;
      ledLevel[currPoint2]=initPointVal;
      //find the next adjacent point to move to
      //snake 1 ************************
      //get the offset
      currOffset1=ledOffset[currPoint1];
      //get the number of adjacent points
      numAdjPoints1=adjLedNum[currPoint1];
      //pick a random point from the adjacent points
      randAdjPoint1=random(0,numAdjPoints1);
      //update the current point
      currPoint1=sLedOrd[currOffset1+randAdjPoint1];
      //snake 2 **********************
      //get the offset
      currOffset1=ledOffset[currPoint2];
      //get the number of adjacent points
      numAdjPoints1=adjLedNum[currPoint2];
      //pick a random point from the adjacent points
      randAdjPoint1=random(0,numAdjPoints1);
      //update the current point
      currPoint2=sLedOrd[currOffset1+randAdjPoint1];
              
      //set the TLC
      for(i=0;i<totalLeds;i++)
      {
        Tlc.set(i, ledLevel[i]);
      }    
      /* Tlc.update() sends the data to the TLCs.  This is when the LEDs will
         actually change. */
      Tlc.update();
      
      delay(delayAmount);
      //check to make sure the pattern was called from the random pattern selector
      if(runLength!=-1)
          totRunTime=totRunTime+delayAmount;

  }
}

void loop() 
{    
  delay(1000);
  //*********on interrupt, turn off all LEDs*********************
  //signal the LED
    /* Tlc.clear() sets all the grayscale values to zero, but does not send
         them to the TLCs.  To actually send the data, call Tlc.update() */
  Tlc.clear();
  //reset the led levels
  for(int i=0;i<totalLeds;i++)
  {
     Tlc.set(i, 0);
  }
   /* Tlc.update() sends the data to the TLCs.  This is when the LEDs will
         actually change. */
  Tlc.update();
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
    else if(currentPattern==7)
      runPattern7(-1);
    else if(currentPattern==8)
      runPattern8(-1);
}

