#include "KerbalSimpit.h"

KerbalSimpit mySimpit(Serial);

// Analog Utilities
rotationMessage myRotation;
translationMessage myTranslation;
int throttle;
int pitchR;
int yawR;
int rollR;
int xR;
int yR;
int zR;



bool keyA;
bool keyB;

int maxVolt = 1019;


void setup() {
  // Connect to the Server
  Serial.begin(115200);
  while (!mySimpit.init());

   // Analogs
  mySimpit.registerChannel(THROTTLE_MESSAGE);
  mySimpit.registerChannel(ROTATION_MESSAGE);
  mySimpit.registerChannel(TRANSLATION_MESSAGE);
  myRotation.mask = 7;
  myTranslation.mask = 7;

  pinMode(12, INPUT);
  pinMode(13, INPUT);

  analogReference(EXTERNAL);

}

void loop() {
  // Analog Controls
    readAnalogs();
    mySimpit.send(THROTTLE_MESSAGE, throttle);
    mySimpit.send(ROTATION_MESSAGE, myRotation);
    mySimpit.send(TRANSLATION_MESSAGE, myTranslation);
    if ((throttle > 32700) and (myRotation.roll> 32700) and (myTranslation.Z> 32700)){mySimpit.init();}

}


void readAnalogs() {
  int throttleRead = analogRead(A0);
  if (throttleRead > maxVolt){throttleRead=maxVolt;}
  throttle = map(throttleRead, 0, maxVolt, 0, 32767);

  int pitchRead = analogRead(A5);
  if (pitchRead > maxVolt){pitchRead=maxVolt;}
  pitchR = map(pitchRead, 0, maxVolt, 32767, -32768);
  myRotation.pitch = deadZone(pitchR);

  int yawRead = analogRead(A4);
  if (yawRead > maxVolt){yawRead=maxVolt;}
  yawR = map(yawRead, 0, maxVolt, -32768, 32767);
  myRotation.yaw = deadZone(yawR);

  int rollRead = analogRead(A6);
  if (rollRead > maxVolt){rollRead=maxVolt;}
  rollR = map(rollRead, 0, maxVolt, -32768, 32767);
  myRotation.roll = deadZone(rollR);

  int xRead = analogRead(A2);
  if (xRead > maxVolt){xRead=maxVolt;}
  xR = map(xRead, 0, maxVolt, 32767, -32768);
  myTranslation.Z = deadZone(xR);

  int yRead = analogRead(A1);
  if (yRead > maxVolt){yRead=maxVolt;}
  yR = map(yRead, 0, maxVolt, 32767, -32768);
  myTranslation.Y = deadZone(yR);

  int zRead = analogRead(A3);
  if (zRead > maxVolt){zRead=maxVolt;}
  zR = map(zRead, 0, maxVolt, 32767, -32768);
  myTranslation.X = deadZone(zR);
}

int deadZone(int input){
  if (input > 3000 or input < -3000){
    return input;
  } else {
    return 0;
  }
}
