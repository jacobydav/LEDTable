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
byte ledRowCnt[] = {6,7,6,7,6,7,6};  //The number of LEDs in each row

volatile int pChange;          // interrupt?
int currentPattern;
int k;
//MSG pin setup
int analogPin = 0; // read from multiplexer using analog input 0
int strobePin = 7; // strobe is attached to digital pin 2
int resetPin = 5; // reset is attached to digital pin 3
//Overall intensity
//This will give control over the brightness. For example if the table is too
//bright at night the potentiometer on the control panel can be used to dim it.
//Every intensity value applied to an LED in any of the patterns will first
//be multiplied by a scale factor determined by the potentiometer position.
//The value read from the analog pin has a range from 0-1024.
//These values will be scaled to between 0.25-1. Where 0.25 is 1/4th normal
//brightness and 1 is normal brightness. I choose this scale because I don't
//there is any need to dim the LEDs below 1/4th brightness.
int ovIntenPin = 1; // analog pin 1
float ovIntScaleVal = 1.0;              

void setup() 
{
  //**********interrupt setup begin**************
  pinMode(2, INPUT);      // Make digital 2 an input
  // attach our interrupt pin to it's ISR
  attachInterrupt(0, patternChange, LOW);
  // we need to call this to enable interrupts
  interrupts();
  pChange=0;
  currentPattern=4;
  //**********interrupt setup end**************
  //Serial.begin(9600);
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
  /* Call Tlc.init() to setup the tlc.
     You can optionally pass an initial PWM value (0 - 4095) for all channels.*/
  Tlc.init();
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
  int recentMin[]={600,600,600,600,600,600,600}; //hold the recent min value returned by MSG
  int minMaxStep=5;
  int msgVal;
  int bandLedNum[]={7,7,5,7,7,5,3};
  //store the order of leds to use in the msg equalizer
  //there are 7 bands.  Each bands leds are offset by its value in bandLedNum
  int msgLedOrd[]={31,38,44,30,35,37,46,
                   32,39,47,35,34,43,41,
                   26,24,28,33,22,
                   16,4,12,23,14,11,7,
                   20,13,5,17,14,10,6,
                   25,21,18,29,19,
                   27,37,11};
  
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
      if(msgVal<recentMin[i])
      {
        recentMin[i]=msgVal;
      }
        
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
         
         //Shrink the gap between max and min to adjust to changes in overall volume levels
         recentMin[i]++;
         recentMax[i]--;
      }
      
      //set the TLC
      if(chanHeightTotal<maxLedsOn) //only if the total number of lit LEDs is less than 88%
      {                                 //update the lights
        for(i=0;i<totalLeds;i++)
        {
          //Adjust for the overall intensity scale factor
          int adjLedLevel = (int)(ledLevel[i]*ovIntScaleVal);
          Tlc.set(msgLedOrd[i], adjLedLevel);
        }
      }
      else   //only turn on the leds up to maxLedsOn
      {
        for(i=0;i<maxLedsOn;i++)
        {
          //Adjust for the overall intensity scale factor
          int adjLedLevel = (int)(ledLevel[i]*ovIntScaleVal);
          Tlc.set(msgLedOrd[i], adjLedLevel);
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
                  0,0,0,0,0,0,0,0,0};
  int totRunTime=-2;   //the total time the pattern has run
  //begin loop              
  while(currentPattern==3 && pChange==0 && totRunTime<runLength)
  {
    //Get an updated value for the overall intensity scale factor
    calcOvIntScaleFac();
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
    for(int j=0; j < totalLeds; j++)
      {
          ledInd = ledIndArray[j];
          //Adjust for the overall intensity scale factor
          int adjLedLevel = (int)(ledLevel[ledInd]*ovIntScaleVal);
          Tlc.set(ledInd,adjLedLevel);        
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
  int i,j;
  int dir=1;
  int delayAmount=200;
  int maxTime=int(20000.0/delayAmount); //max=20 seconds
  int minTime=int(8000.0/delayAmount);  //min=8 seconds
  
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
  
  int f1Ind=random(0,numFlowers-1);  //initial flower 1 center
  int f1TimeOut=random(minTime,maxTime);  //flower 1 timeout
  int f1Time=0;      //flower 1 current time
  //byte pedalChangeCntThresh = 1;  //After this many iterations, change the petal
  //byte f1CurrPedalChangeCnt = 1;//random(0,pedalChangeCntThresh);
  //byte f2CurrPedalChangeCnt = 1;//random(0,pedalChangeCntThresh);
  bool f1ChangePetal = false;  //change the petal every other iterations
  bool f2ChangePetal = true;  //change the petal every other iterations
  int f1CurrPetal=0;
  int f2Ind=random(0,numFlowers-1);
  //make sure that flower 2 is different than flower 1
  while(f1Ind==f2Ind)
      f2Ind=random(0,numFlowers-1);
      
  int f2TimeOut=random(7,50);  //flower 2 timeout
  int f2Time=0;      //flower 2 current time
  int f2CurrPetal=0;
  int fadeStep=(int)(250*ovIntScaleVal);  //Adjust the fadestep for the current intensity scaling factor
  
  int ledLevel[]={0,0,0,0,0,0,0,0,0,0, //stores the level for each led
                  0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0};
        
  //offsets are where to start finding the values for each flower
  //int ledOffsets[]={0,8,16,23,32,39,47,54,60,67};
  int totRunTime=-2;   //the total time the pattern has run
  
  while(currentPattern==4 && pChange==0 && totRunTime<runLength)
  {
    //Get an updated value for the overall intensity scale factor
    calcOvIntScaleFac();
      /* Tlc.clear() sets all the grayscale values to zero, but does not send
         them to the TLCs.  To actually send the data, call Tlc.update() */
      Tlc.clear();
      
      //flower 1
     if(f1Time<f1TimeOut)
     {
       //keep the central led on
       ledLevel[ledIndArray[flowerInds[f1Ind]]]=4000;
       //set the current pedal to a bright value
       ledLevel[ledIndArray[flowerInds[f1Ind]+indOffset[f1CurrPetal]]]=4000;
       //After a certain number of iterations, change the petal
      if(f1ChangePetal)
         f1CurrPetal++;       
       //check to make sure we have not reached the last pedal
       if(f1CurrPetal>=numPetals)
         f1CurrPetal=0;
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
       f1CurrPetal=0;
     }
    
    //flower 2
     if(f2Time<f2TimeOut)
     {
       //keep the central led on
       ledLevel[ledIndArray[flowerInds[f2Ind]]]=4000;
       //set the current pedal to a bright value
       ledLevel[ledIndArray[flowerInds[f2Ind]+indOffset[f2CurrPetal]]]=3000;       
       //After a certain number of iterations, change the petal
       if(f2ChangePetal)
         f2CurrPetal++;       
       //check to make sure we have not reached the last pedal
       if(f2CurrPetal>=numPetals)
         f2CurrPetal=0;
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
       f2CurrPetal=0;
     }
     
     //Fade all the leds except the flower center
     for(i=0;i<totalLeds;i++)
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
      for(i=0;i<totalLeds;i++)
      {
        int adjValue = (int)(ledLevel[i]*ovIntScaleVal);
        Tlc.set(i, adjValue);
      }    
      /* Tlc.update() sends the data to the TLCs.  This is when the LEDs will
         actually change. */
      Tlc.update();
      
      delay(delayAmount);
      f1ChangePetal = !f1ChangePetal;
      f2ChangePetal = !f2ChangePetal;
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
  //byte ledInd;
  
  int ledLevel[]={0,0,0,0,0,0,0,0,0,0,0,0, //stores the level for each led
                0,0,0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0,0};
               
  //by color fade variables
  int bcNum=8;
  int bcLedNum[]={6,6,6,6,6,6,3,6}; //orange, aqua, red, yellow, green, violet, white, blue, 
  int bcLedOrd[]={47,40,18,17,13,7,
                45,29,21,20,14,12,
                44,35,32,24,22,3,
                43,41,33,19,10,6,
                46,39,34,28,9,5,
                36,30,25,16,4,15,
                37,27,11,
                42,38,31,26,23,8};
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
        //Adjust for the overall intensity scale factor
        int adjLedLevel = (int)(ledLevel[i]*ovIntScaleVal);
        Tlc.set(i, adjLedLevel);
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
  
}

//cross swirl
void runPattern7(int runLength)
{
  
}

//snakes
void runPattern8(int runLength)
{
  
}

void calcOvIntScaleFac()
{
  //Read the potentiometer value
  int ovIntVal=analogRead(ovIntenPin);
   //Scale the value
  ovIntScaleVal = (0.75/1024.0)*ovIntVal + 0.25; 
}

void loop() 
{    
  //Get an initial value for the overall intensity scale factor
  calcOvIntScaleFac();
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

