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

//For serial input
#define INPUT_STRING_MAX 10
char inputString[INPUT_STRING_MAX];         // a string to hold incoming data
int inputStringIndex = 0;                    //the current position in the char array

bool runPatternOnce = true;

//Global parameters for the pattern
//These can be set by serial input
int delayAmount=100;   //Parameter A
int fadeStep=100;      //Parameter B
byte pedalChangeCntThresh=2;  //Parameter C

void setup() 
{
  Serial.begin(9600);
  Serial.println("Setup begin");
  //**********interrupt setup begin**************
  pinMode(2, INPUT);      // Make digital 2 an input
  // attach our interrupt pin to it's ISR
  //attachInterrupt(0, patternChange, LOW);
  // we need to call this to enable interrupts
  interrupts();
  pChange=0;
  currentPattern=4;
  //**********interrupt setup end**************
  
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
  Tlc.init(0);
}

// The interrupt hardware calls this 
void patternChange()
{
  //pChange=1;
}

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() 
{
  while (Serial.available()) 
  {
    // get the new byte:
    char inChar = (char)Serial.read();
    
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if(inChar=='\n')
    {
      inputString[inputStringIndex] = '\0'; //Null terminate the string
      if(inputStringIndex>1)
      {
        //Command format = A0123 to set parameter A to value 123
        //The parameters are hard coded to set what A, B, etc corrospond
        //to which parameters.
        
        //Extract the value
        int tempVal=-1;
        char valStr[5];
        strncpy(valStr, inputString+1, 4);
        valStr[4] = '\0'; // place the null terminator
        char * pEnd;
        tempVal = strtol(valStr,&pEnd,10);
        //Extract the parameter value
        if(inputString[0]=='A')
        {
          Serial.print(" setting delayAmount=");
          Serial.println(tempVal);
          delayAmount = tempVal;          
        }
        else if(inputString[0]=='B')
        {
          Serial.print(" setting fadeStep=");
          Serial.println(tempVal);
          fadeStep = tempVal; 
        }
        else if(inputString[0]=='C')
        {
          Serial.print(" setting pedalChangeCntThresh=");
          Serial.println(tempVal);
          pedalChangeCntThresh = tempVal; 
        }
        else if(inputString[0]=='D')
        {
          Serial.print(" setting pChange=");
          Serial.println(tempVal);
          pChange = tempVal; 
        }
        
      }
      // clear the string:
      strcpy(inputString, "X");
      inputStringIndex = 0;
    }
    else
    {
      inputString[inputStringIndex] = inChar;
      inputStringIndex++;
      if(inputStringIndex>INPUT_STRING_MAX) //Check that we don't overrun the allocated memory for the string
        inputStringIndex=0;
    }
  }
}

//flowers pattern
//lit up a central led, then have the surrounding leds
//move in a circle around it
void runPattern4(int runLength)
{
  int i,j;
  int dir=1;
  //int delayAmount=100;
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
       //Serial.print(flowerInds[f1Ind]);
       //Serial.print(" ,");
       //Serial.print(ledIndArray[flowerInds[f1Ind]]);
       //Serial.print(" ,");
       //set the current petal to a bright value
       ledLevel[ledIndArray[flowerInds[f1Ind]+indOffset[f1CurrPetal]]]=3000;
       //Serial.print(flowerInds[f1Ind]+indOffset[f1CurrPetal]);
       //Serial.print(" ,"); 
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
       //Serial.print(f1Time);
       //Serial.print(" ,");
       //Serial.print(f1TimeOut);
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
     
     //Serial.println("");
     //Serial.println(freeRam());
     
    //flower 2
     if(f2Time<f2TimeOut)
     {
       //keep the central led on
       ledLevel[ledIndArray[flowerInds[f2Ind]]]=4000;
       //Serial.print("     ");
       //Serial.print(flowerInds[f2Ind]);
       //Serial.print(" ,");
       //Serial.print(ledIndArray[flowerInds[f2Ind]]);
       //Serial.print(" ,");
       //Serial.print(" : ");
      //Serial.print(totRunTime);
      //Serial.print(" : ");
      //Serial.print(f2CurrPetal);
      // Serial.print(" ,");
       //set the current pedal to a bright value
       ledLevel[ledIndArray[flowerInds[f2Ind]+indOffset[f2CurrPetal]]]=3000;
       //Serial.print(f2CurrPetal);
       //Serial.print(" ,");
       //Serial.print(indOffset[f2CurrPetal]);
       //Serial.print(" ,");
       //Serial.print(flowerInds[f2Ind]+indOffset[f2CurrPetal]);
       //Serial.print(" ,");
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
      //Serial.print(" : ");
      //Serial.print(totRunTime);
      //Serial.print(" : ");       
       //check to make sure we have not reached the last pedal
       if(f2CurrPetal>=numPetals)
         f2CurrPetal=0;
       if(f2CurrPetal<0)
         f2CurrPetal=numPetals-1;
       //increase the time
       f2Time++;
       //Serial.print(f2Time);
       //Serial.print(" ,");
       //Serial.println(f2TimeOut);
     }
    else
     {
       //Serial.print("F2 time out ");
       //Serial.print(f2Time);
       //Serial.print(" ,");
       //Serial.println(f2TimeOut);
       //get the next flower values
       f2Ind=random(0,numFlowers-1);  //initial flower 2 center
       while(f1Ind==f2Ind)
         f2Ind=random(0,numFlowers-1);
         
       f2TimeOut=random(minTime,maxTime);  //flower 2 timeout
       f2Time=0;      //flower 2 current time
       f2CurrPetal=random(0,numPetals);
       f2CurrDir=random(0,2);  //Determines the direction of travel for the petals. 0 = CCW, 1=CW.
     }
     
     //Serial.println("");
     //Serial.println(freeRam());
     //Serial.print(" f1Ind=");
     //Serial.print(f1Ind);
     //Serial.print(" f2Ind=");
     //Serial.print(f2Ind);
     //Serial.print(" ");
     
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
        Tlc.set(i, adjValue);
      }    
      /* Tlc.update() sends the data to the TLCs.  This is when the LEDs will
         actually change. */
      Tlc.update();
      
      delay(delayAmount);
      //f1ChangePetal = !f1ChangePetal;
      //f2ChangePetal = !f2ChangePetal;
      //check to make sure the pattern was called from the random pattern selector
      if(runLength!=-1)
          totRunTime=totRunTime+delayAmount;
          
      //Serial.print("totRunTime= ");
      //Serial.println(totRunTime);
  }
  //Serial.println("pattern ended");  
  //Serial.print("Current pattern= ");
  //Serial.println(currentPattern);
  //Serial.print("pChange= ");
  //Serial.println(pChange);
  //Serial.print("totRunTime= ");
//Serial.println(totRunTime);
  //Serial.print("runLength= ");
  //Serial.println(runLength);
}

void calcOvIntScaleFac()
{
  //Read the potentiometer value
  int ovIntVal=analogRead(ovIntenPin);
  //Serial.print("ovIntVal=");
  //Serial.println(ovIntVal);
   //Scale the value
  ovIntScaleVal = (0.75/1024.0)*ovIntVal + 0.25; 
}

int freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void loop() 
{    
  //Get an initial value for the overall intensity scale factor
  //calcOvIntScaleFac();
  //delay(1000);
  
    Serial.println("Loop start");
    pChange = 0;
  
    runPattern4(-1); 
   runPatternOnce=false; 
    
}

