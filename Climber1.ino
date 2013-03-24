#define NO_PORTB_PINCHANGES
#define NO_PORTC_PINCHANGES
#include <PinChangeInt.h>
#include <FlexiTimer2.h>
#include <Servo.h>
#include <EEPROM.h>

#define TOP_LIMIT     5
#define BOTTOM_LIMIT  6
#define LED_OUT       13
#define LED2_OUT      12
#define LED3_OUT      11
#define BODY_SERVO    8
#define SHUTTLE_MOTOR 9
//#define M1  A0
//#define M2  A1
#define SHUTTLE_UP_SPEED    2260
#define SHUTTLE_DOWN_SPEED  700
#define SHUTTLE_STOP_SPEED  1500

String COMMANDS = "AUDWTBE";
String IMMEDIATE_COMMANDS = "AUDWTBE";
String DEBUG_COMMANDS = "LSCA";
String ADMIN_COMMANDS = "LSP";

volatile int top_state = LOW;
volatile int bottom_state = LOW;

unsigned char desiredAngle = 90;
volatile unsigned char servoAngle = 90;
char servoDir = 0;

volatile int encoderCount = 0;

Servo bodyServo;
Servo shuttleMotor;

boolean immediateMode = false;
boolean exitScript = false;
volatile boolean topLimitPressed = false;
volatile boolean bottomLimitPressed = false;
volatile boolean inBodyMove = false;

int eepromCounter = 0;

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
  if (inBodyMove)
    servoAngle += servoDir;
}

void encoderInt()
{
  encoderCount++;
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

  // Turn on internal pullups  ?
  digitalWrite(TOP_LIMIT, HIGH);
  digitalWrite(BOTTOM_LIMIT, HIGH );
  digitalWrite(A0, HIGH);
  digitalWrite(A1, HIGH);
  digitalWrite(A2, HIGH);
  
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
//  updateBodyServo();

  Serial.println("Press bottom limit switch to run script");
  
  while(1)
  {
    // Check for incoming serial data
    if (Serial.available() > 0)
      handleSerial();
  
    if (bottomLimitPressed && !immediateMode)
    {  
      bottomLimitPressed = false;
      playScript();

      while(1)
      {
        delay(1000);
      }
    }
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
  Serial.println();
  Serial.println();
  while (!exitScript)
  {
    encoderCount = 0;
    shuttleMotor.writeMicroseconds(SHUTTLE_DOWN_SPEED);
    ledOn(0);
    delay(2000);
    Serial.print("Encoder count: ");
    Serial.println(encoderCount);
    
    encoderCount = 0;
    shuttleMotor.writeMicroseconds(SHUTTLE_UP_SPEED);
    ledOn(2);
    delay(2000);
    Serial.print("Encoder count: ");
    Serial.println(encoderCount);
    
    // Check for incoming serial data
    if (Serial.available() > 0)
    {
      handleSerial();
    }
  }
  shuttleMotor.writeMicroseconds(SHUTTLE_STOP_SPEED);
  ledOn(-1);  // Invalid number turns all off and none back on
  exitScript = false;
}

void debugArm()
{
  Serial.println();
  Serial.println();
  while (!exitScript)
  {
    bodyServo.write(70);
    ledOn(0);
    delay(2000);
    bodyServo.write(110);
    ledOn(2);
    delay(2000);
    // Check for incoming serial data
    if (Serial.available() > 0)
    {
      handleSerial();
    }
  }
  bodyServo.write(90);
  ledOn(-1);  // Invalid number turns all off and none back on
  exitScript = false;
}

void updateBodyServo()
{
  if (servoAngle > desiredAngle)
    servoDir = -1;
  else if (servoAngle < desiredAngle)
    servoDir = 1;
  else
    servoDir = 0;
    
  bodyServo.write(servoAngle);
}

void moveToAngle( int value )
{
  desiredAngle = value;
  
  inBodyMove = true;
  
  while ( servoAngle != desiredAngle )
  {
    updateBodyServo();
  }
  
  inBodyMove = false;
}

void shuttleUpCount( int value )
{
  bottomLimitPressed = false;
  encoderCount = 0;
  ledOn(2);
  while (!exitScript && encoderCount < value && !topLimitPressed)
  {
    shuttleMotor.writeMicroseconds(SHUTTLE_UP_SPEED);
  }
  shuttleMotor.writeMicroseconds(SHUTTLE_STOP_SPEED);
  ledOn(-1);  // Invalid number turns all off and none back on
  Serial.print("Encoder count: ");
  Serial.println(encoderCount);
  exitScript = false;
}

void shuttleDownCount( int value )
{
  topLimitPressed = false;
  encoderCount = 0;
  ledOn(0);
  while (!exitScript && encoderCount < value && !bottomLimitPressed)
  {
    shuttleMotor.writeMicroseconds(SHUTTLE_DOWN_SPEED);
  }
  shuttleMotor.writeMicroseconds(SHUTTLE_STOP_SPEED);
  ledOn(-1);  // Invalid number turns all off and none back on
  Serial.print("Encoder count: ");
  Serial.println(encoderCount);
  exitScript = false;
}

void wait( int seconds )
{
  ledOn(1);
  delay(seconds * 1000);
  ledOn(-1);
}

void executeCommand( char command, int value )
{
    switch(command)
    {
      case 'A':  // move to angle
        moveToAngle(value);
        break;
      
      case 'U':  // shuttle up count or top limit
        shuttleUpCount(value);
        break;
        
      case 'T':  // shuttle up to top limit
        shuttleUpCount(32767);
        break;
        
      case 'D':  // shuttle down count or bottom limit
        shuttleDownCount(value);
        break;
        
      case 'B':  // shuttle down to bottom limit
        shuttleDownCount(32767);
        break;
        
      case 'W':  // Wait seconds
        wait(value);
        break;
    }
}

void storeCommand( char command, int value )
{
  EEPROM.write(eepromCounter++, command);
  EEPROM.write(eepromCounter++, lowByte(value));
  EEPROM.write(eepromCounter++, highByte(value));
  
  if ( command == 'E' )
  {
    Serial.print("EEPROM bytes written: ");
    Serial.println(eepromCounter);
  }
}

void showScript()
{
  char c = ' ';
  char low;
  char high;
  int count = 0;
  
  Serial.println();
  Serial.println("Script start");
  while (c != 'E' && count < 1023)
  {
    c = EEPROM.read(count++);
    low = EEPROM.read(count++);
    high = EEPROM.read(count++);
    Serial.print(c);
    Serial.println(word(high,low),DEC);
  }
  Serial.println("Script end");
}

void playScript()
{
  char c = ' ';
  char low;
  char high;
  int value = 0;
  int count = 0;
  
  Serial.println();
  Serial.println("Script start");
  while (c != 'E' && count < 1023)
  {
    c = EEPROM.read(count++);
    low = EEPROM.read(count++);
    high = EEPROM.read(count++);
    value = word(high,low);
    Serial.print(c);
    Serial.println(value,DEC);
    executeCommand(c, value);
  }
  Serial.println("Script end");
}

void handleSerial()
{
  static char lastChar;
  int value;
  
  char c = Serial.read();
  
  if ( c != '\r' && c != '\n' & c != ' ' )
  {
    Serial.print("\r\nReceived: ");
    Serial.print(c);
    Serial.print("  lastChar: ");
    Serial.print(lastChar);
    
    if ( c == 'Q' )          // Quit any running scripts
      exitScript = true;
    else if ( c == 'I' )      // Change to Immediate Mode
    {
      Serial.println("\nImmediate Mode");
      immediateMode = true;
    }
    else if ( c == 'N' )      // Change out of immediate mode
    {
      Serial.println("\nNormal Mode");
      immediateMode = false;
    }
    else if ( lastChar == '-' )  // debug command
    {
      exitScript = false;
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
    else if ( c == 'L' )      // Script follows
      eepromCounter = 0;
    else if ( c == 'S' )      // Show script stored in EEPROM
      showScript();
    else if ( c == 'P' )      // Play (execute) script stored in EEPROM
      playScript();
    else if ( COMMANDS.indexOf(c) > -1 )
    {
      value = Serial.parseInt();
      Serial.print("   int: ");
      Serial.println(value);
    }
    else
      Serial.println();
      
    lastChar = c;
  }
  else
    return;
  
  if (IMMEDIATE_COMMANDS.indexOf(c) > -1)
  {
    if (immediateMode)
    {
      executeCommand(c, value);
      exitScript = false;
    }
    else
      storeCommand(c, value);
  }
}

