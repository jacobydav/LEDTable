//Test adjusting the overall intensity of LEDs based on a potentiometer value.
//This will give control over the brightness. For example if the table is too
//bright at night the potentiometer on the control panel can be used to dim it.
//Every intensity value applied to an LED in any of the patterns will first
//be multiplied by a scale factor determined by the potentiometer position.
//The value read from the analog pin has a range from 0-1024.
//These values will be scaled to between 0.25-1. Where 0.25 is 1/4th normal
//brightness and 1 is normal brightness. I choose this scale because I don't
//there is any need to dim the LEDs below 1/4th brightness.

//Overall intensity
//The overall intensity will be set by a potentiometer. The value read
int ovIntenPin = 1; // analog pin 1

void setup() 
{
  Serial.begin(9600);
  pinMode(ovIntenPin, INPUT);
}

void loop() 
{
  int ovIntVal=analogRead(ovIntenPin);
  //Scale the value
  float ovIntScaleVal = (0.75/1024.0)*ovIntVal + 0.25;
  
  Serial.print(ovIntVal);
  Serial.print("     ");
  Serial.println(ovIntScaleVal);
  
  delay(500);
}
