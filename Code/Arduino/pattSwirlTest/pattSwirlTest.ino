#include "src\Adafruit_TLC5947\Adafruit_TLC5947.h"
#include <avr/interrupt.h>

const int totalLeds=45;

//This is a representation of the layout of the LEDs viewed from above with the control panel above Row 1.
//There are 7 rows, Rows 1,3,5,7 have 6 LEDs
//Rows 2,4,6 have 7 LEDs
byte ledIndArray[]={  29,30,28,27,31,32, //the index for each led
                      25,26,24,42,36,38,37,
                      39,40,41,5,4,3,
                      8,1,6,7,2,45,11,
                      43,44,9,46,10,47,
                      20,23,14,17,21,18,19,
                      16,22,0,12,13,15};
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

//TLC5947 begin
// How many boards do you have chained?
#define NUM_TLC5974 2

#define data_pin   11
#define clock_pin  10
#define latch_pin   9
#define oe  -1  // set to -1 to not use the enable pin (its optional)

Adafruit_TLC5947 tlc = Adafruit_TLC5947(NUM_TLC5974, clock_pin, data_pin, latch_pin);
//TLC5947 end

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
  
  //TLC5947 init begin
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
  //TLC5947 init end
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
    
  //Each row is the indeces of the LEDs for a position in the swirl.
  //Note: These are indeces based on the physical layout. So they should be referenced to ledIndArray.
 //Do not include the center LED 
  byte swirlInds[] = { 0,1,8,15,29,36,43,44,
                       2,9,16,28,35,42,
                       3,10,16,28,34,0,
                       4,11,17,16,28,27,33,40,
                       5,4,10,16,28,34,40,39,
                       5,12,18,17,23,21,27,26,32,39,
                       18,17,23,21,27,26,
                       25,24,23,21,20,19,
                       31,30,23,21,14,13,
                       44,38,31,30,23,21,14,13,6,0};

//  byte swirlInds[] = { 42,44,38,31,16,4,12,15,
//                       46,35,32,20,14,7,
//                       43,39,32,20,13,6,
//                       47,41,34,32,20,17,10,5,
//                       45,47,39,32,20,13,5,3,
//                       45,40,33,34,26,25,17,19,9,3,
//                       33,34,26,25,17,19,
//                       28,24,26,25,21,18,
//                       22,23,26,25,30,29,
//                       15,8,22,23,26,25,30,29,36,42};
                       
  byte swirlCnt[] = {8,6,6,8,8,10,6,6,6,10};
                        
  byte numPos = 10;  //the number of swirl positions
  
  int ledLevel[]={0,0,0,0,0,0,0,0,0,0, //stores the level for each led
                  0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0};
        
  int currSwirlPos=random(0,numPos);  //initial swirl position
  Serial.print("currSwirlPos=");
  Serial.println(currSwirlPos);
  //The center of all the swirls. This is the LED in the center of the table.
  //This LED will have its own pattern. Use the index from the physical layout.
  byte centralInd = 22;
  ledLevel[ledIndArray[centralInd]] = 4000; //Start with the cenral led on.
  int centLEDFadeStep = -400;
  //Change swirl position after x iterations of the loop.
  byte posChangeThresh = 10;
  byte currposChangeCnt = 0;
  int totRunTime=-2;   //the total time the pattern has run

  //Set the LEDs to zero
  for(int j=0; j < 48; j++)
      tlc.setPWM(j,0);
      
        
  while(currentPattern==4 && pChange==0 && totRunTime<runLength)
  {
    //Get an updated value for the overall intensity scale factor
    calcOvIntScaleFac();
    
     //Get the starting index for the current swirl position
     byte startInd = 0;
     for(i=0;i<currSwirlPos;i++)
          startInd=startInd+swirlCnt[i];
     //Stop index is the start index plus the number of leds in the current swirl position
     byte stopInd = startInd+swirlCnt[currSwirlPos];
     
     //Serial.print("startInd=");
     //Serial.println(startInd);
     //Serial.print("stopInd=");
     //Serial.println(stopInd);
               
     //Turn on the leds for the current swirl position
     for(i=startInd;i<stopInd;i++)
     {
       ledLevel[ledIndArray[swirlInds[i]]] = ledLevel[ledIndArray[swirlInds[i]]] +500;
       if(ledLevel[ledIndArray[swirlInds[i]]]>4000)
         ledLevel[ledIndArray[swirlInds[i]]] = 4000;
       
       //Serial.print(swirlInds[i]);
       //Serial.print(",");
     }
     //Serial.println("");
     
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
       //if(i!=ledIndArray[centralInd] && bFade==true)
       if(bFade==true)
       {
         ledLevel[i] = ledLevel[i]-fadeStep;
         //check to make sure the value is greater than 0
         if(ledLevel[i]<0)
           ledLevel[i]=0; 

         Serial.print(ledLevel[i]);
         Serial.print(" ");
       }
       else
       {
          Serial.print(ledLevel[i]);
          Serial.print(" ");
       }
     }
     Serial.println();       

    //Fade the central LED. It will fade in and out.
    //ledLevel[ledIndArray[centralInd]]=ledLevel[ledIndArray[centralInd]]+centLEDFadeStep;
//    if(ledLevel[ledIndArray[centralInd]]>4000)
//    {
//      ledLevel[ledIndArray[centralInd]] = 0;
//      centLEDFadeStep=centLEDFadeStep*-1;  //Change fade direction
//    }
//    if(ledLevel[ledIndArray[centralInd]]<0)
//    {
//      ledLevel[ledIndArray[centralInd]] = 4000;
//      centLEDFadeStep=centLEDFadeStep*-1; //Change fade direction
//    }
     
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
  
    runPattern(-1);  
    
}

