#define NO_PORTB_PINCHANGES
#define NO_PORTC_PINCHANGES
#include <PinChangeInt.h>
#include <FlexiTimer2.h>
#include <Servo.h>

#define TOP_LIMIT     5
#define BOTTOM_LIMIT  6
#define LED_OUT       13
#define LED2_OUT      12
#define LED3_OUT      11
#define BODY_SERVO    8
#define SHUTTLE_MOTOR 9
//#define M1  A0
//#define M2  A1

String COMMANDS = "AUTDBW";
String DEBUG_COMMANDS = "LSCA";

volatile int top_state = LOW;
volatile int bottom_state = LOW;

unsigned char desiredAngle = 90;
volatile unsigned char servoAngle = 90;
char servoDir = 0;

volatile int encoderCount = 0;

Servo bodyServo;
Servo shuttleMotor;

boolean immediateMode = true;
boolean exitScript = false;
volatile boolean topLimitPressed = false;
volatile boolean bottomLimitPressed = false;

void pcInt2() 
{
  if ( PCintPort::arduinoPin == TOP_LIMIT )
  {
    topLimitPressed = true;
    bottomLimitPressed = false;
  }
  else if ( PCintPort::arduinoPin == BOTTOM_LIMIT )
  {
    topLimitPressed = false;
    bottomLimitPressed = true;
  }
}

void tmr2Int()
{
  servoAngle += servoDir;
}

void encoderInt()
{
  encoderCount++;
//  top_state = !top_state;
//  digitalWrite( LED2_OUT, top_state );
}

void setup() 
{
  // put your setup code here, to run once:
  pinMode( TOP_LIMIT, INPUT );
  pinMode( BOTTOM_LIMIT, INPUT );
  pinMode( LED_OUT, OUTPUT );
  pinMode( LED2_OUT, OUTPUT );
  pinMode( LED3_OUT, OUTPUT );
  pinMode( A0, OUTPUT );
  pinMode( A1, OUTPUT );
  pinMode( A2, OUTPUT );
//  pinMode( M1, OUTPUT );
//  pinMode( M2, OUTPUT );

  // Turn on internal pullups  
  digitalWrite(TOP_LIMIT, HIGH);
  digitalWrite(BOTTOM_LIMIT, HIGH );
  digitalWrite(A0, HIGH);
  digitalWrite(A2, HIGH);
  digitalWrite(A3, HIGH);
  
  PCintPort::attachInterrupt(TOP_LIMIT, &pcInt2, RISING);
  PCintPort::attachInterrupt(BOTTOM_LIMIT, &pcInt2, FALLING);
  
  attachInterrupt( 1, encoderInt, RISING );
  
  FlexiTimer2::set(20,tmr2Int);
  FlexiTimer2::start();
  
  bodyServo.attach(BODY_SERVO);
  shuttleMotor.attach(SHUTTLE_MOTOR);
  
  Serial.begin(9600);
}

void loop() 
{
  // put your main code here, to run repeatedly: 
  if (servoAngle > desiredAngle)
    servoDir = -1;
  else if (servoAngle < desiredAngle)
    servoDir = 1;
  else
    servoDir = 0;
    
  bodyServo.write(servoAngle);
  
  // Check for incoming serial data
  while (Serial.available() > 0)
  {
    handleSerial();
  }
}

void ledOn(char pos)
{
  // Make sure they are all OFF
  digitalWrite(A0, HIGH);
  digitalWrite(A1, HIGH);
  digitalWrite(A2, HIGH);
  // Turn on the desired one
  switch(pos)
  {
    case 0: digitalWrite(A0, LOW); break;
    case 1: digitalWrite(A1, LOW); break;
    case 2: digitalWrite(A2, LOW); break;
  }
}
void debugLimit()
{
  while (!exitScript)
  {
    Serial.print("Bottom: ");
    Serial.print(bottomLimitPressed);
    Serial.print(" Top: ");
    Serial.println(topLimitPressed);
    delay(500);
    
    if (topLimitPressed)
      ledOn(2);
    else if (bottomLimitPressed)
      ledOn(0);
    else
      ledOn(-1);

    // Check for incoming serial data
    if (Serial.available() > 0)
    {
      handleSerial();
    }
  }
  ledOn(-1);
  exitScript = false;
}

void debugIndicators()
{
  while (!exitScript)
  {
    ledOn(0);
    delay(500);
    ledOn(1);
    delay(500);
    ledOn(2);
    delay(500);

    // Check for incoming serial data
    if (Serial.available() > 0)
    {
      handleSerial();
    }
  }
  ledOn(-1);  // Invalid number turns all off and none back on
  exitScript = false;
}

void debugShuttle()
{
}

void debugArm()
{
}

void handleSerial()
{
  static char lastChar;
  char c = Serial.read();
  
  if ( c != '\r' && c != '\n' & c != ' ' )
  {
    Serial.print("Received: ");
    Serial.print(c);
    
    if ( c == 'Q' )
    {
      exitScript = true;
    }
    else if ( lastChar == '-' )  // debug command
    {
//      d = Serial.read();
      Serial.print("  lastChar: ");
      Serial.print(lastChar);
      Serial.print("  Debug: ");
      Serial.print(c);
      
      switch(c)
      {
        case 'L':  // print limit switches
          debugLimit();
          break;
          
        case 'S':  // run shuttle and display counter
          debugShuttle();
          break;
          
        case 'A':  // Move arm
          debugArm();
          break;
          
        case 'I':  // Indicator LEDs
          debugIndicators();
          break;
      }
    }
    else if ( COMMANDS.indexOf(c) > -1 )
    {
      int value = Serial.parseInt();
      Serial.print("   int: ");
      Serial.println(value);
    }
    else
      Serial.println();
      
    lastChar = c;
  }
  else
    return;
  
  if (immediateMode && COMMANDS.indexOf(c) > -1)
  {
    switch(c)
    {
      case 'A':  // move to angle
        break;
      
      case 'U':  // shuttle up count or top limit
        break;
        
      case 'T':  // shuttle up to top limit
        break;
        
      case 'D':  // shuttle down count or bottom limit
        break;
        
      case 'B':  // shuttle down to bottom limit
        break;
        
      case 'W':  // Wait seconds
        break;
    }
  }
  
  /*
  // Decide what to do with input
  switch (c)
  {
    case 'L':  // Load script.  Script from PC follows
      break;
      
    case 'R':  // Script that follows is reset script
      break;
      
    case 'S':  // Store current buffer as script (regular or reset)
      break;
      
    case 'E':  // End of script
      break;
      
    case 'I':  // Change to Immediate mode
      break;
      
    case 'N':  // Change to Not Immediate mode
      break;
      
    case 'G':  // Run current buffer as script
      break;
      
    case 'X':  // Exit any currently running script
      break;
      
    case 'A':  // Move to angle
      break;
      
    case 'U':  // Shuttle up count or to limit
      break;
      
    case 'T':  // Shuttle to top limit switch
      break;
      
    case 'D':  // Shuttle down count or to limit
      break;
      
    case 'W':  // Delay # of seconds
      break;
  }
  */
}
  /*
  if (latest_interrupted_pin == TOP_LIMIT)
    digitalWrite(LED_OUT, LOW);
  else if (latest_interrupted_pin == BOTTOM_LIMIT)
    digitalWrite(LED_OUT, HIGH);
  */  
  /*
  bodyServo.write(30);
  delay(2000);
  bodyServo.write(150);
  delay(2000);
  */

/*  
  if (servoAngle == desiredAngle)
  {
    if (desiredAngle == 30)
    {
      desiredAngle = 150;
      shuttleMotor.write(95);
      digitalWrite(M1, LOW);
      digitalWrite(M2, LOW);
      digitalWrite(M1, HIGH);
    }
    else
    {
      desiredAngle = 30;
      shuttleMotor.write(75);
      digitalWrite(M1, LOW);
      digitalWrite(M2, LOW);
      digitalWrite(M2, HIGH);
    }
  }
*/  

