#include "Tlc5940.h"
#include <avr/interrupt.h>

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
int delayAmount=80;   //Parameter A
int fadeStep=200;      //Parameter B

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
    
  //*********overall intensity setup begin********************
  pinMode(ovIntenPin, INPUT);
  //*********overall intensity setup end********************
  randomSeed(analogRead(2));  //seed the random number with an unconnected pin(2) read value
  
  k=0;
  
  /* Call Tlc.init() to setup the tlc.
     You can optionally pass an initial PWM value (0 - 4095) for all channels.*/
  Tlc.init(0);
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

//Swirl
void runPattern(int runLength)
{
  int i,j;  
    
  //The index into ledIndArray of the leds that are able to be flower centers.
  //Basically, this is all the LEDs that are not on one of the edges. The reason
   //I am using the index in ledIndArray is because the calculation to find the
  //surrounding petals is based on the row and column of the led array.
 //Do not include the center LED 
  byte swirlInds[] = { 42,44,38,31,16,4,12,15,
                       46,35,32,20,14,7,
                       43,39,32,20,13,6,
                       47,41,34,32,20,17,10,5,
                       45,47,39,32,20,13,5,3,
                       45,40,33,34,26,25,17,19,9,3,
                       33,34,26,25,17,19,
                       28,24,26,25,21,18,
                       22,23,26,25,30,29,
                       15,8,22,23,26,25,30,29,36,42};
  byte swirlCnt[] = {8,6,6,8,8,10,6,6,6,10};
                        
  byte numPos = 10;  //the number of flowers
  
  int ledLevel[]={0,0,0,0,0,0,0,0,0,0, //stores the level for each led
                  0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0};
        
  int currSwirlPos=random(0,numPos);  //initial swirl position
  Serial.print("currSwirlPos=");
  Serial.println(currSwirlPos);
  //The center of all the swirls. This is the LED in the center of the table.
  //This LED will have its own pattern
  byte centralInd = 27;
  //Change swirl position after x iterations of the loop.
  byte posChangeThresh = 10;
  byte currposChangeCnt = 0;
  int totRunTime=-2;   //the total time the pattern has run
  
  while(currentPattern==4 && pChange==0 && totRunTime<runLength)
  {
    //Get an updated value for the overall intensity scale factor
    calcOvIntScaleFac();
      /* Tlc.clear() sets all the grayscale values to zero, but does not send
         them to the TLCs.  To actually send the data, call Tlc.update() */
      Tlc.clear();
      
     
     //Get the starting index for the current swirl position
     byte startInd = 0;
     for(i=0;i<currSwirlPos;i++)
          startInd=startInd+swirlCnt[i];
     //Stop index is the start index plus the number of leds in the current swirl position
     byte stopInd = startInd+swirlCnt[currSwirlPos];
     
     Serial.print("startInd=");
     Serial.println(startInd);
     Serial.print("stopInd=");
     Serial.println(stopInd);
               
     //Turn on the leds for the current swirl position
     for(i=startInd;i<stopInd;i++)
     {
       ledLevel[swirlInds[i]] = ledLevel[swirlInds[i]] +500;
       if(ledLevel[swirlInds[i]]>4000)
         ledLevel[swirlInds[i]] = 4000;
       //ledLevel[swirlInds[i]] = 4000;
       Serial.print(swirlInds[i]);
       Serial.print(",");
     }
     Serial.println("");
     
     //Fade all the leds except the center
     //and the current swirl positions
     for(i=0;i<48;i++)
     {
       bool bFade = true;
       for(j=startInd;j<stopInd;j++)
       {
         if(swirlInds[j]==i)
           bFade=false;
       }
       if(i!=centralInd && bFade==true)
       {
         ledLevel[i] = ledLevel[i]-fadeStep;
         //check to make sure the value is greater than 0
         if(ledLevel[i]<0)
           ledLevel[i]=0; 
       }
     }  
     
     //Move to the next position
     currposChangeCnt++;
     if(currposChangeCnt>posChangeThresh)
     {
       currposChangeCnt=0;
       currSwirlPos++;
       if(currSwirlPos>=numPos)
         currSwirlPos=0;
       Serial.print("currSwirlPos=");
       Serial.println(currSwirlPos);
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
      
      //check to make sure the pattern was called from the random pattern selector
      if(runLength!=-1)
          totRunTime=totRunTime+delayAmount;          
  }  
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
  calcOvIntScaleFac();
  
    Serial.println("Loop start");
    pChange = 0;
  
    runPattern(15000);  
    
}

