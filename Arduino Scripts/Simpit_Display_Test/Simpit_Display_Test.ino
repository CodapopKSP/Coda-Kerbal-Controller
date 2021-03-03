#include "KerbalSimpit.h"

#include <SPI.h>
#include <bitBangedSPI.h>
#include <MAX7219.h>

#include <ShiftRegister74HC595.h>

// parameters: <number of shift registers> (data pin, clock pin, latch pin)
ShiftRegister74HC595<2> sr(8, 10, 9);

KerbalSimpit mySimpit(Serial);

unsigned long myTime;
unsigned long oldTime = 0;
char stringTime [15];

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


// # Chips / LOAD / DIN / CLK
MAX7219 display1 (1, 15, 14, 16);
MAX7219 display2 (1, 18, 17, 19);
MAX7219 display3 (1, 21, 20, 22);
MAX7219 display4 (1, 24, 23, 25);
MAX7219 display5 (1, 27, 26, 28);
MAX7219 display6 (1, 30, 29, 31);
MAX7219 display7 (1, 33, 32, 34);
MAX7219 display8 (1, 36, 35, 37);
MAX7219 display9 (1, 39, 38, 40);

void setup() {
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
  display1.sendString("        ");
  display2.sendString("        ");
  display3.sendString("        ");
  display4.sendString("        ");
  display5.sendString("        ");
  display6.sendString("        ");
  display7.sendString("        ");
  display8.sendString("        ");
  display9.sendString("        ");

  display1.setIntensity (8);
  display2.setIntensity (3);
  display3.setIntensity (8);
  display4.setIntensity (8);
  display5.setIntensity (3);
  display6.setIntensity (8);
  display7.setIntensity (8);
  display8.setIntensity (3);
  display9.setIntensity (8);

  sr.setAllLow();

  Serial.begin(115200);
  while (!mySimpit.init()) {delay(100);}

  // Telemetry
  mySimpit.inboundHandler(messageHandler);
  mySimpit.registerChannel(VELOCITY_MESSAGE);
  mySimpit.registerChannel(TARGETINFO_MESSAGE);
  mySimpit.registerChannel(APSIDES_MESSAGE);
  //mySimpit.registerChannel(APSIDESTIME_MESSAGE);
  mySimpit.registerChannel(ALTITUDE_MESSAGE);
  //mySimpit.registerChannel(AIRSPEED_MESSAGE);
}

void loop (){
  // Get Data from Simpit
  mySimpit.update();
  //myTime = millis();
  //if (oldTime < myTime - 100){
      display1.sendString(orbitalVelocity_str);
      display2.sendString(surfaceVelocity_str);
      display3.sendString(verticalVelocity_str);
      display4.sendString(apoapsis_str);
      display5.sendString(periapsis_str);
      display6.sendString(meanAltitude_str);
      //display7.sendString(apoapsisTime_str);
      //display8.sendString(periapsisTime_str);
      display9.sendString(srfAltitude_str);
    
    //oldTime = myTime;
  //}
}

void messageHandler(byte messageType, byte msg[], byte msgSize) {
  switch(messageType) {

  case VELOCITY_MESSAGE:
    if (msgSize == sizeof(velocityMessage)) {
      velocityMessage myVelocity;
      myVelocity = parseVelocity(msg);
      dtostrf(myVelocity.orbital, 8, 0, orbitalVelocity_str);
      dtostrf(myVelocity.surface, 8, 0, surfaceVelocity_str);
      dtostrf(myVelocity.vertical, 8, 0, verticalVelocity_str);
    }

  case TARGETINFO_MESSAGE:
    if (msgSize == sizeof(targetMessage)) {
      targetMessage myTarget;
      myTarget = parseTarget(msg);
      dtostrf(myTarget.distance, 8, 0, targetDistance_str);
      dtostrf(myTarget.velocity, 8, 0, relativeVelocity_str);
    }

  case APSIDES_MESSAGE:
    if (msgSize == sizeof(apsidesMessage)) {
      apsidesMessage myApsides;
      myApsides = parseApsides(msg);
      dtostrf(myApsides.apoapsis, 8, 0, apoapsis_str);
      dtostrf(myApsides.periapsis, 8, 0, periapsis_str);
    }
  /*
  case APSIDESTIME_MESSAGE:
    if (msgSize == sizeof(apsidesTimeMessage)) {
      apsidesTimeMessage myApsidesTime;
      myApsidesTime = parseApsidesTime(msg);
      dtostrf(myApsidesTime.apoapsis, 4, 0, apoapsisTime_str);
      dtostrf(myApsidesTime.periapsis, 4, 0, periapsisTime_str);
      
    }
  */
  case ALTITUDE_MESSAGE:
    if (msgSize == sizeof(altitudeMessage)) {
      altitudeMessage myAltitude;
      myAltitude = parseAltitude(msg);
      dtostrf(myAltitude.sealevel, 8, 0, meanAltitude_str);
      dtostrf(myAltitude.surface, 4, 0, srfAltitude_str);
    }
  /*
  case AIRSPEED_MESSAGE:
    if (msgSize == sizeof(airspeedMessage)) {
      //airspeedMessage myAirspeed;
      myAirspeed = parseAirspeed(msg);
      //dtostrf(myAirspeed.IAS, 8, 0, airspeed_str);
      //dtostrf(myAirspeed.mach, 8, 0, machNumber_str);
    }
  */
  }
}
