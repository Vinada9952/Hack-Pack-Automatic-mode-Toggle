#include <Arduino.h>
#include <Servo.h>
#include <IRremote.hpp>
#include <avr/wdt.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>



#define left 0x8
#define right 0x5A
#define up 0x18
#define down 0x52
#define ok 0x1C
#define cmd1 0x45
#define cmd2 0x46
#define cmd3 0x47
#define cmd4 0x44
#define cmd5 0x40
#define cmd6 0x43
#define cmd7 0x7
#define cmd8 0x15
#define cmd9 0x9
#define cmd0 0x19
#define star 0x16
#define hashtag 0xD
#define secret1 0xC
#define secret2 0x5E
#define secret3 0x42
#define secret4 0x4A


Servo yawServo;
Servo pitchServo;
Servo rollServo;

LiquidCrystal_I2C lcd(0x27, 16, 2);

uint8_t pitchServoVal = 100;

#define pitchMoveSpeed 8
#define yawMoveSpeed 90
#define yawStopSpeed 90
#define rollMoveSpeed 90
#define rollStopSpeed 90

uint16_t yawPrecision = 150;
#define rollPrecision 158

#define pitchMax 150
#define pitchMin 33

#define pinEchoDistance 2
#define pinTrigDistance 3

uint8_t savedDistance;
uint8_t waitVal = 0;
uint8_t shootNumber = 1;

bool pause = false;

void displayLCD(bool automode = false);


void setup() {

  yawServo.attach(10);
  pitchServo.attach(11);
  rollServo.attach(12);

  lcd.init();
  lcd.backlight();

  pinMode(pinEchoDistance, INPUT);
  pinMode(pinTrigDistance, OUTPUT);


  IrReceiver.begin(9, ENABLE_LED_FEEDBACK);

  homeServos();
}


void loop() {
  uint16_t command = getIRCommand();

  switch (command) {  //this is where the commands are handled

    case up:  //pitch up
      upMove(1);
      break;

    case down:  //pitch down
      downMove(1);
      break;

    case left:  //fast counterclockwise rotation
      leftMove(1);
      break;

    case right:  //fast clockwise rotation
      rightMove(1);
      break;

    case ok:  //firing routine
      fire();
      //// Serial.println("FIRE");
      break;

    case star:
      fireAll();
      delay(50);
      break;

    case hashtag:
      setAutoMode();
      break;

    case cmd0:
      yawPrecision = 50;
      break;

    case cmd1:
      yawPrecision = 100;
      break;

    case cmd2:
      yawPrecision = 150;
      break;

    case cmd3:
      yawPrecision = 200;
      break;

    case cmd4:
      yawPrecision = 250;
      break;

    case cmd5:
      yawPrecision = 300;
      break;

    case cmd6:
      yawPrecision = 350;
      break;

    case cmd7:
      yawPrecision = 400;
      break;

    case cmd8:
      yawPrecision = 450;
      break;

    case cmd9:
      yawPrecision = 500;
      break;
  }


  displayLCD(false);
  delay(5);
}

void leftMove(uint8_t moves) {
  for (uint8_t i = 0; i < moves; i++) {
    yawServo.write(yawStopSpeed + yawMoveSpeed);
    delay(yawPrecision);
    yawServo.write(yawStopSpeed);
    delay(5);
  }
}

void rightMove(uint8_t moves) {
  for (uint8_t i = 0; i < moves; i++) {
    yawServo.write(yawStopSpeed - yawMoveSpeed);
    delay(yawPrecision);
    yawServo.write(yawStopSpeed);
    delay(5);
  }
}

void upMove(uint8_t moves) {
  for (uint8_t i = 0; i < moves; i++) {
    if ((pitchServoVal + pitchMoveSpeed) < pitchMax) {
      pitchServoVal = pitchServoVal + pitchMoveSpeed;
      pitchServo.write(pitchServoVal);
      delay(50);
    }
  }
}

void downMove(uint8_t moves) {
  for (uint8_t i = 0; i < moves; i++) {
    if ((pitchServoVal - pitchMoveSpeed) > pitchMin) {
      pitchServoVal = pitchServoVal - pitchMoveSpeed;
      pitchServo.write(pitchServoVal);
      delay(50);
    }
  }
}

void fire() {
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("!FIRE!");
  rollServo.write(rollStopSpeed + rollMoveSpeed);
  delay(rollPrecision);
  rollServo.write(rollStopSpeed);
  delay(5);
  lcd.clear();
}

void fireAll() {
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("!FIRE ALL!");
  rollServo.write(rollStopSpeed + rollMoveSpeed);
  delay(rollPrecision * 12);
  rollServo.write(rollStopSpeed);
  delay(5);
  lcd.clear();
}

void homeServos() {
  yawServo.write(yawStopSpeed);
  delay(20);
  rollServo.write(rollStopSpeed);
  delay(100);
  pitchServo.write(100);
  delay(100);
  pitchServoVal = 100;
}

uint8_t mesureDistance() {

  digitalWrite(pinTrigDistance, LOW);
  delayMicroseconds(2);
  digitalWrite(pinTrigDistance, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrigDistance, LOW);

  unsigned long time = pulseIn(pinEchoDistance, HIGH);

  uint8_t distance = (time * 0.0343) / 2;

  return distance;
}

uint8_t mesureAverageDistance(uint8_t n = 5) {
  uint32_t sum = 0;
  uint8_t validMesurement = 0;

  for (uint8_t i = 0; i < n; i++) {
    uint8_t d = mesureDistance();
    if (d > 0) {
      sum += d;
      validMesurement++;
    }
    delay(20);
  }

  if (validMesurement == 0) return -1;
  return (uint8_t)(sum / validMesurement);
}

void displayLCD(bool automode = false) {
  lcd.setCursor(0, 0);
  if (automode) {
    if (pause) {
      lcd.print("Pause  ");
      lcd.print(yawPrecision / 50);
      lcd.setCursor(11, 0);
      lcd.print(mesureAverageDistance());
      lcd.print("cm ");
      lcd.setCursor(0, 1);
      lcd.print(waitVal / 2);
      if (waitVal % 2 == 1) lcd.print(".5s");
      else lcd.print(".0s");
      lcd.setCursor(8, 1);
      lcd.print(shootNumber);
      lcd.print(" shoots");
    } else {
      lcd.print("Auto");
      lcd.setCursor(12, 0);
      lcd.print(waitVal / 2);
      if (waitVal % 2 == 1) lcd.print(".5s");
      else lcd.print(".0s");
      lcd.setCursor(0, 1);
      lcd.print(mesureAverageDistance());
      lcd.print("cm ");
      lcd.setCursor(8, 1);
      lcd.print(shootNumber);
      lcd.print(" shoots");
    }
  } else {
    lcd.print("Manual");
    lcd.setCursor(0, 1);
    lcd.print("speed:");
    lcd.print(yawPrecision / 50);
    lcd.print(' ');
  }
}


void setAutoMode() {
  lcd.clear();
  shootNumber = 1;
  pause = false;
  uint8_t currentDistance = mesureAverageDistance();
  savedDistance = currentDistance;
  while (true) {
    switch (getIRCommand()) {

      case cmd1:
        waitVal = 1;
        break;

      case cmd2:
        waitVal = 2;
        break;

      case cmd3:
        waitVal = 3;
        break;

      case cmd4:
        waitVal = 4;
        break;

      case cmd5:
        waitVal = 5;
        break;

      case cmd6:
        waitVal = 6;
        break;

      case cmd7:
        waitVal = 7;
        break;

      case cmd8:
        waitVal = 8;
        break;

      case cmd9:
        waitVal = 9;
        break;

      case cmd0:
        waitVal = 0;
        break;

      case star:
        shootNumber++;
        if (shootNumber == 7)
          shootNumber = 1;
        break;

      case hashtag:
        lcd.clear();
        return;

      case ok:
        lcd.clear();
        currentDistance = mesureAverageDistance();
        savedDistance = mesureAverageDistance();
        pause = !pause;
        break;
    }
    if (!pause) {
      uint8_t currentDistance = mesureAverageDistance();
      if (abs(currentDistance - savedDistance) > 10) {
        delay(waitVal * 500);
        for (uint8_t i = 0; i < shootNumber; i++)
          fire();
        delay(waitVal * 500);
      }
      savedDistance = currentDistance;
    }
    displayLCD(true);
    delay(10);
  }
}


int getIRCommand() {
  if (IrReceiver.decode()) {

    IrReceiver.printIRResultShort(&Serial);
    IrReceiver.printIRSendUsage(&Serial);
    if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
      IrReceiver.printIRResultRawFormatted(&Serial, true);
    }
    IrReceiver.resume();

    uint16_t command = IrReceiver.decodedIRData.command;
    return command;
  } else {
    return 0;
  }
}