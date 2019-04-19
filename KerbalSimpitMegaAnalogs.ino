// Need to change sprintf to fit displays

#include "KerbalSimpit.h"

#include <SPI.h>
#include <bitBangedSPI.h>
#include <MAX7219.h>

KerbalSimpit mySimpit(Serial);

// Analog Utilities
rotationMessage myRotation;
translationMessage myTranslation;
int throttleRead;
int pitchRead;
int yawRead;
int rollRead;
int xRead;
int yRead;
int zRead;
int throttle;
int pitchR;
int yawR;
int rollR;
int xR;
int yR;
int zR;

// Telemetry Strings
char orbitalVelocity_str [15];
char surfaceVelocity_str [15];
char verticalVelocity_str [15];
char apoapsis_str [15];
char periapsis_str [15];
char meanAltitude_str [15];
char apoapsisTime_str [15];
char periapsisTime_str [15];
char srfAltitude_str [15];
char relativeVelocity_str [15];
char targetDistance_str [15];
char airspeed_str [15];
char machNumber_str [15];

// Other Utilities
int displayMode = 1;
int dim = 0;
int dimmerValue;

// Debug Utilities
char throttle_str [15];
char pitch_str [15];
char yaw_str [15];
char roll_str [15];
char x_str [15];
char y_str [15];
char z_str [15];

// MAX7219 Master LOAD / CLK
int LOAD = 6;
int CLK = 7;

// # Chips / LOAD / DIN / CLK
MAX7219 display1 (1, LOAD, 8, CLK);
MAX7219 display2 (1, LOAD, 9, CLK);
MAX7219 display3 (1, LOAD, 10, CLK);
MAX7219 display4 (1, LOAD, 11, CLK);
MAX7219 display5 (1, LOAD, 12, CLK);
MAX7219 display6 (1, LOAD, 13, CLK);
MAX7219 display7 (1, LOAD, 14, CLK);
MAX7219 display8 (1, LOAD, 15, CLK);
MAX7219 display9 (1, LOAD, 16, CLK);

// Hard Coded Pins
const int displayChangePin = 17;
const int ignitionKeysPinL = 18;
const int ignitionKeysPinR = 19;
const int dimmerPin = A7;

// Analog Pins
int throttlePin = A0;
int pitchPin = A1;
int yawPin = A2;
int rollPin = A3;
int xPin = A4;
int yPin = A5;
int zPin = A6;

void setup() {

  // Connect to the Server
  Serial.begin(115200);
  while (!mySimpit.init()) {delay(100);}

  // Telemetry
  mySimpit.inboundHandler(messageHandler);
  mySimpit.registerChannel(ALTITUDE_MESSAGE);
  mySimpit.registerChannel(APSIDES_MESSAGE);
  mySimpit.registerChannel(VELOCITY_MESSAGE);
  mySimpit.registerChannel(TARGETINFO_MESSAGE);
  mySimpit.registerChannel(AIRSPEED_MESSAGE);
  mySimpit.registerChannel(APSIDESTIME_MESSAGE);
  mySimpit.registerChannel(SOI_MESSAGE);

  // Analogs
  mySimpit.registerChannel(THROTTLE_MESSAGE);
  mySimpit.registerChannel(ROTATION_MESSAGE);
  mySimpit.registerChannel(TRANSLATION_MESSAGE);
  myRotation.mask = 7;
  myTranslation.mask = 7;

  // MAX7219
  display1.begin ();
  display2.begin ();
  display3.begin ();
  display4.begin ();
  display5.begin ();
  display6.begin ();
  display7.begin ();
  display8.begin ();
  display9.begin ();

  // Hard Coded Pins
  pinMode(displayChangePin, INPUT);
  pinMode(ignitionKeysPinL, INPUT);
  pinMode(ignitionKeysPinR, INPUT);
}

void loop() {

  // Dimmer
  dimmerValue = analogRead(dimmerPin);
  int dim = map(dimmerValue, 0, 1023, 0, 15);
  display1.setIntensity (dim);
  display2.setIntensity (dim);
  display3.setIntensity (dim);
  display4.setIntensity (dim);
  display5.setIntensity (dim);
  display6.setIntensity (dim);
  display7.setIntensity (dim);
  display8.setIntensity (dim);
  display9.setIntensity (dim);

  // Analog Mode
  if ((digitalRead(ignitionKeysPinL) == HIGH) and (digitalRead(ignitionKeysPinR) == HIGH)) {

    // Analog Controls
    readAnalogs();
    mySimpit.send(THROTTLE_MESSAGE, throttle);
    mySimpit.send(ROTATION_MESSAGE, myRotation);
    mySimpit.send(TRANSLATION_MESSAGE, myTranslation);
  
    // Get Data from Simpit
    mySimpit.update();
  
    // Change Display Mode
    if (digitalRead(displayChangePin) ==  HIGH) {
      displayMode = (displayMode + 1);
      if (displayMode > 2) {
        displayMode = 1;
      }
    }
  
    // Display Mode 1
    if (displayMode == 1) {
      display1.sendString(orbitalVelocity_str);
      display2.sendString(surfaceVelocity_str);
      display3.sendString(verticalVelocity_str);
      display4.sendString(apoapsis_str);
      display5.sendString(periapsis_str);
      display6.sendString(meanAltitude_str);
      display7.sendString(apoapsisTime_str);
      display8.sendString(periapsisTime_str);
      display9.sendString(srfAltitude_str);
      
    }
  
    // Display Mode 2
    if (displayMode == 2) {
      display1.sendString(relativeVelocity_str);
      display2.sendString(targetDistance_str);
      display3.sendString(verticalVelocity_str);
      display4.sendString(apoapsis_str);
      display5.sendString(periapsis_str);
      display6.sendString(airspeed_str);
      display7.sendString(apoapsisTime_str);
      display8.sendString(periapsisTime_str);
      display9.sendString(machNumber_str);
    }
  }

  // Low Power Mode
  if ((digitalRead(ignitionKeysPinL) == LOW) and (digitalRead(ignitionKeysPinR) == LOW)) {
    display1.sendString("            ");
    display2.sendString("            ");
    display3.sendString("            ");
    display4.sendString("            ");
    display5.sendString("            ");
    display6.sendString("            ");
    display7.sendString("            ");
    display8.sendString("            ");
    display9.sendString("            ");
    delay(10);
  }

  // Debug Mode
  else {
      debugMode();
      mySimpit.init();
  }
  
  delay(1);
}

void readAnalogs() {
  int throttleRead = analogRead(A0);
  int throttle = map(throttleRead, 0, 1023, 0, 32767);

  int pitchRead = analogRead(A1);
  int pitchR = map(pitchRead, 0, 1023, -32768, 32767);
  myRotation.pitch = pitchR;

  int yawRead = analogRead(A2);
  int yawR = map(yawRead, 0, 1023, -32768, 32767);
  myRotation.yaw = yawR;

  int rollRead = analogRead(A3);
  int rollR = map(rollRead, 0, 1023, -32768, 32767);
  myRotation.roll = rollR;

  int xRead = analogRead(A4);
  int xR = map(xRead, 0, 1023, -32768, 32767);
  myTranslation.X = xR;

  int yRead = analogRead(A5);
  int yR = map(yRead, 0, 1023, -32768, 32767);
  myTranslation.Y = yR;

  int zRead = analogRead(A6);
  int zR = map(zRead, 0, 1023, -32768, 32767);
  myTranslation.Z = zR;
}

void messageHandler(byte messageType, byte msg[], byte msgSize) {
  switch(messageType) {
  case ALTITUDE_MESSAGE:
    if (msgSize == sizeof(altitudeMessage)) {
      altitudeMessage myAltitude;
      myAltitude = parseAltitude(msg);
      dtostrf(myAltitude.sealevel, 8, 0, meanAltitude_str);
      dtostrf(myAltitude.surface, 8, 0, srfAltitude_str);
    } break;
    
  case APSIDES_MESSAGE:
    if (msgSize == sizeof(apsidesMessage)) {
      apsidesMessage myApsides;
      myApsides = parseApsides(msg);
      dtostrf(myApsides.apoapsis, 8, 0, apoapsis_str);
      dtostrf(myApsides.periapsis, 8, 0, periapsis_str);
    } break;
    
  case VELOCITY_MESSAGE:
    if (msgSize == sizeof(velocityMessage)) {
      velocityMessage myVelocity;
      myVelocity = parseVelocity(msg);
      dtostrf(myVelocity.orbital, 8, 0, orbitalVelocity_str);
      dtostrf(myVelocity.surface, 8, 0, surfaceVelocity_str);
      dtostrf(myVelocity.vertical, 8, 0, verticalVelocity_str);
    } break;

  case TARGETINFO_MESSAGE:
    if (msgSize == sizeof(targetMessage)) {
      targetMessage myTarget;
      myTarget = parseTarget(msg);
      dtostrf(myTarget.distance, 8, 0, targetDistance_str);
      dtostrf(myTarget.velocity, 8, 0, relativeVelocity_str);
    } break;
    
  case AIRSPEED_MESSAGE:
    if (msgSize == sizeof(airspeedMessage)) {
      airspeedMessage myAirspeed;
      myAirspeed = parseAirspeed(msg);
      dtostrf(myAirspeed.IAS, 8, 0, airspeed_str);
      dtostrf(myAirspeed.mach, 8, 0, machNumber_str);
    } break;

  case APSIDESTIME_MESSAGE:
    if (msgSize == sizeof(apsidesTimeMessage)) {
      apsidesTimeMessage myApsidesTime;
      myApsidesTime = parseApsidesTime(msg);
      dtostrf(myApsidesTime.apoapsis, 8, 0, apoapsisTime_str);
      dtostrf(myApsidesTime.periapsis, 8, 0, periapsisTime_str);
    } break;
  }
}

void debugMode() {

  // Get Analog Data
  readAnalogs();
  sprintf(throttle_str, "%9.2f", throttleRead);
  sprintf(pitch_str, "%9.2f", pitchRead);
  sprintf(yaw_str, "%9.2f", yawRead);
  sprintf(roll_str, "%9.2f", rollRead);
  sprintf(x_str, "%9.2f", xRead);
  sprintf(y_str, "%9.2f", yRead);
  sprintf(z_str, "%9.2f", zRead);

  // Display Analog Data
  display1.sendString(throttle_str);
  display2.sendString(pitch_str);
  display3.sendString(yaw_str);
  display4.sendString(roll_str);
  display5.sendString(x_str);
  display6.sendString(y_str);
  display7.sendString(z_str);
  display8.sendString("debug");
  display9.sendString("mode");
  
}
