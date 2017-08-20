//Test the snakes pattern. This will light up a random LED and then randomly "snake" out from there with a fading tail.
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
int delayAmount=100;   //Parameter A
int fadeStep=150;      //Parameter B
int snakeChangeCntThresh = 5; //Parameter C

void setup() 
{
  Serial.begin(9600);
  Serial.println("Setup begin");
  //*********overall intensity setup begin********************
  pinMode(ovIntenPin, INPUT);
  //*********overall intensity setup end********************
  //randomSeed(analogRead(2));  //seed the random number with an unconnected pin(2) read value

  currentPattern=4;
  
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
          Serial.print(" setting snakeChangeCntThresh=");
          Serial.println(tempVal);
          snakeChangeCntThresh = tempVal; 
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

//snakes pattern
//lit up an led, then randomly move to a connected led
void runPattern4(int runLength)
{
  //int delayAmount=100;
  
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
  /*Serial.println(" begin potDirList");
  for(int i=0;i<82;i++)
  {
    Serial.print(potDirList[i]);
    Serial.print(",");
  }
  Serial.println(" end of potDirList");*/
                            
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
  
  while(currentPattern==4 && pChange==0 && totRunTime<runLength)
  {       
    //Serial.print("  s1Ind=");
    //Serial.print(s1Ind);
      //snake 1     
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
        //Serial.print("   numPotDirs=");
        //Serial.println(numPotDirs);
        byte newDirInd = random(0,numPotDirs);
        int newDirBeginInd = 0;
        for(int i=0;i< borderLEDInd;i++)
          newDirBeginInd = newDirBeginInd+potDirCntList[i];
        //Serial.print("   newDirBeginInd=");
        //Serial.print(newDirBeginInd);
        s1CurrOffset = potDirList[newDirBeginInd+newDirInd];
        //Serial.print("   s1CurrOffset=");
        //Serial.println(s1CurrOffset);
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
    Serial.println("Loop start");
    pChange = 0;
  
    runPattern4(-1); 
   runPatternOnce=false; 
    
}

