//KIS (TAB to open inventory, WASDQE to rotate, SHIFT to fine control, SPACE to reset, R to change anchor, X to attach, G to grab, 1-8 to equip, )
// Change Display, Reset Warnings
// Analogs, LEDs

#include <Keyboard.h>
#include <ShiftIn.h>
#include <KerbalSimpit.h>
#include <math.h>


// Init ShiftIn instance with a single chip
ShiftIn<1> shift;

KerbalSimpit mySimpit(SerialUSB);

boolean hasChanged();
bool allZero = true;
const int pressK = 0;
const int holdK = 1;
int timewarpCount = 0;

long BODY;
int x;
long SMA;
long R;
double MU;
double RS;

int ASCII[] = {

                  // Navigation
                 179,     // Cycle Map        (TAB)
                  96,     // Reset Map        (`)

                  // Camera Controls
                 223,     // Zoom In          (Numpad +)
                 222,     // Zoom Out         (Numpad -)
                 218,     // Camera Up        (Arrow Up)
                 217,     // Camera Down      (Arrow Down)
                 216,     // Camera Left      (Arrow Left)
                 215,     // Camera Right     (Arrow Right)
                 118,     // Cycle Views      (V)
                  99,     // IVA              (C)
                  93,     // Cycle Ships      (])
                 195,     // Toggle UI        (F2)
                 194,     // Screenshot       (F1)
                  27,     // Escape           (ESC)

                  // Timewarp
                  46,     // Timewarp +       (.)
                  44,     // Timewarp -       (,)
                  47,     // Stop Timewarp    (/)
                 134,     // Phys Timewarp    (ALT)

                  // EVA Controls
                  89,     // EVA Up           (Y)              <<<<< Change hotkey binding in KSP Y/O/P/; (thrust)
                  79,     // EVA Down         (O)              <<<<< Change hotkey binding in KSP Y/O/P/; (thrust)
                  80,     // EVA Jump/Let Go  (P)              <<<<< Change hotkey binding in KSP Y/O/P/; (stage)
                  59,     // EVA Headlamp     (;)              <<<<< Change hotkey binding in KSP Y/O/P/; (RCS translate right)
                 102,     // EVA Grab         (F)
                  98,     // EVA Board        (B)              <<<<< Change hotkey binding in KSP (brakes)

                  // Ship Controls
                  98,     // Brake Hold       (B)
                 134,     // Trim             (ALT)
                 120,     // Reset Trim       (X)
                  32,     // Stage            (SPACE)
                   8,     // Abort            (BACKSPACE)

                  // Action Groups
                  98,     // Brakes           (B)              <<<<< Needs own function
                 103,     // Gear             (G)
                 117,     // Lights           (U)
                  49,     // AG 1             (1)
                  50,     // AG 2             (2)
                  51,     // AG 3             (3)
                  52,     // AG 4             (4)
                  53,     // AG 5             (5)
                  54,     // AG 6             (6)
                  55,     // AG 7             (7)
                  56,     // AG 8             (8)
                  57,     // AG 9             (9)
                  48,     // AG 10            (10)

                  // Toggles
                 109,     // Map              (M)
                 235,     // Navball          (Numpad .)
                 114,     // RCS              (R)
                 116,     // SAS              (T)
};

int pressHold[] = {

                  // Navigation
              pressK,     // Cycle Map        (TAB)
              pressK,     // Reset Map        (`)

                  // Camera Controls
               holdK,     // Zoom In          (Numpad +)
               holdK,     // Zoom Out         (Numpad -)
               holdK,     // Camera Up        (Arrow Up)
               holdK,     // Camera Down      (Arrow Down)
               holdK,     // Camera Left      (Arrow Left)
               holdK,     // Camera Right     (Arrow Right)
              pressK,     // Cycle Views      (V)
              pressK,     // IVA              (C)
              pressK,     // Cycle Ships      (])
              pressK,     // Toggle UI        (F2)
              pressK,     // Screenshot       (F1)
              pressK,     // Escape           (ESC)

                  // Timewarp
              pressK,     // Timewarp +       (.)
              pressK,     // Timewarp -       (,)
              pressK,     // Stop Timewarp    (/)
               holdK,     // Phys Timewarp    (ALT)

                  // EVA Controls
               holdK,     // EVA Up           (Y)
               holdK,     // EVA Down         (O)
              pressK,     // EVA Jump/Let Go  (P)
              pressK,     // EVA Headlamp     (;)
              pressK,     // EVA Grab         (F)
              pressK,     // EVA Board        (B)

                  // Ship Controls
               holdK,     // Brake Hold       (B)
               holdK,     // Trim             (ALT)
              pressK,     // Reset Trim       (X)
              pressK,     // Stage            (SPACE)
              pressK,     // Abort            (BACKSPACE)

                  // Action Groups
              pressK,     // Brakes           (B)
              pressK,     // Gear             (G)
              pressK,     // Lights           (U)
              pressK,     // AG 1             (1)
              pressK,     // AG 2             (2)
              pressK,     // AG 3             (3)
              pressK,     // AG 4             (4)
              pressK,     // AG 5             (5)
              pressK,     // AG 6             (6)
              pressK,     // AG 7             (7)
              pressK,     // AG 8             (8)
              pressK,     // AG 9             (9)
              pressK      // AG 10            (10)

                  // Toggles
                          // Map              (M)
                          // Navball          (Numpad .)
                          // RCS              (R)
                          // SAS              (T)
                  
};

int simpitActionGroup[] {
                  // Navigation
                 100,     // NavBall
                 101,     // SAS Assist
                 102,     // SAS Prograde
                 103,     // SAS Retrograde
                 104,     // SAS Normal
                 105,     // SAS Antinormal
                 106,     // SAS Radial Out
                 107,     // SAS Radial In
                 108,     // SAS Target
                 109,     // SAS Antitarget
                 110,     // SAS Maneuver

                  // Custom Action Groups
                 11,      // CAG 11
                 12,      // CAG 12
                 13,      // CAG 13
                 14,      // CAG 14
                 15,      // CAG 15
                 16,      // CAG 16
                 17,      // CAG 17
};

void setup() {
	SerialUSB.begin(115200);
  
	// declare pins: pLoadPin, clockEnablePin, dataPin, clockPin
	shift.begin(8, 9, 11, 12);

  while (!mySimpit.init()) {delay(100);}

  // Telemetry
  mySimpit.inboundHandler(messageHandler);
  mySimpit.registerChannel(ALTITUDE_MESSAGE);
  mySimpit.registerChannel(APSIDES_MESSAGE);
  mySimpit.registerChannel(VELOCITY_MESSAGE);
  mySimpit.registerChannel(TARGETINFO_MESSAGE);
  mySimpit.registerChannel(AIRSPEED_MESSAGE); //<<<<< Add Airspeed view toggle
  mySimpit.registerChannel(APSIDESTIME_MESSAGE);
  mySimpit.registerChannel(SOI_MESSAGE);

  // Fuels
  mySimpit.registerChannel(LF_MESSAGE);
  mySimpit.registerChannel(OX_MESSAGE);
  mySimpit.registerChannel(SF_MESSAGE);
  mySimpit.registerChannel(MONO_MESSAGE);
  mySimpit.registerChannel(ELECTRIC_MESSAGE);
  mySimpit.registerChannel(EVA_MESSAGE);    //<<<<< Add EVA guage to EVA panel

  // Control
  mySimpit.registerChannel(ROTATION_MESSAGE);
  mySimpit.registerChannel(TRANSLATION_MESSAGE);
  mySimpit.registerChannel(THROTTLE_MESSAGE);
  mySimpit.registerChannel(WHEEL_MESSAGE);   //<<<<< Add wheel functionality
}

void loop() {
	keyPresses();
  simpitPresses();
  mySimpit.update();
  //orbitalMechanics();
  // DISPLAY SHOW
  // IF ACTION GROUP
  // RESOURCES
	delay(1);
}

void keyPresses() {
  if(shift.update()) // read in all values. returns true if any button has changed
    {displayValues();}
    
  // Keypresses
  for (int i = 0; i < 42; i++){
    if (shift.hasChanged(i)==1){
      if ((shift.state(i)==1) and (pressHold[i]==0)){
        Keyboard.begin();
        Keyboard.write(ASCII[i]);
        timewarpMod(i);
      }
      if ((shift.state(i)==1) and (pressHold[i]==1)){
        Keyboard.begin();
        Keyboard.press(ASCII[i]);}
      if ((shift.state(i)==0) and (pressHold[i]==1)){
        Keyboard.release(ASCII[i]);}
    }
    // return 0 if any button is pressed
    allZero |= shift.state(i);
  }
  // end keyboard emulation if no buttons are pressed
  if (allZero) {Keyboard.end();}

  // Toggles
  // i is the number of normal keypresses
  for (int i = 42; i < 46; i++){
    if (shift.hasChanged(i)==1){
        Keyboard.begin();
        Keyboard.write(ASCII[i]);
    }
  }
}

void simpitPresses() {
  // i is the number of keypresses + toggles
  // Navball and SAS
  for (int i = 46; i < 57; i++){
    if (shift.hasChanged(i)==1){
      if (shift.state(i)==1){
        mySimpit.activateCAG(simpitActionGroup[i-46]);
      }
    }
  }
  // Action Groups 11-17
  for (int i = 57; i < 64; i++){
    if (shift.hasChanged(i)==1){
      mySimpit.toggleCAG(simpitActionGroup[i-46]);
    }
  }
}

void displayValues() {
  // print out all 8 buttons
  for (int i = 0; i < shift.getDataWidth(); i++)
    SerialUSB.print(shift.state(i)); // get state of button i
  SerialUSB.println();
}

void timewarpMod(int twPress) {
  if (twPress==46) {timewarpCount==(timewarpCount+1);}
  if (twPress==44) {timewarpCount==(timewarpCount-1);}
  if (twPress==47) {timewarpCount==0;}
  if (timewarpCount<0) {timewarpCount==0;}
}

void messageHandler(byte messageType, byte msg[], byte msgSize) {
  switch(messageType) {
  case ALTITUDE_MESSAGE:
    if (msgSize == sizeof(altitudeMessage)) {
      altitudeMessage myAltitude;
      myAltitude = parseAltitude(msg);
      // display.set(myAltitude.sealevel);
      // display.set(myAltitude.surface);
    } break;
    
  case APSIDES_MESSAGE:
    if (msgSize == sizeof(apsidesMessage)) {
      apsidesMessage myApsides;
      myApsides = parseApsides(msg);
      // display.set(myApsides.apoapsis);
      // display.set(myApsides.periapsis);
    } break;
    
  case VELOCITY_MESSAGE:
    if (msgSize == sizeof(velocityMessage)) {
      velocityMessage myVelocity;
      myVelocity = parseVelocity(msg);
      // display.set(myVelocity.orbital);
      // display.set(myVelocity.surface);
      // display.set(myVelocity.vertical);
    } break;

  case TARGETINFO_MESSAGE:
    if (msgSize == sizeof(targetMessage)) {
      targetMessage myTarget;
      myTarget = parseTarget(msg);
      // display.set(myTarget.distance);
      // display.set(myTarget.velocity);
    } break;
    
    case AIRSPEED_MESSAGE:
    if (msgSize == sizeof(airspeedMessage)) {
      airspeedMessage myAirspeed;
      myAirspeed = parseAirspeed(msg);
      // display.set(myAirspeed.IAS);
      // display.set(myAirspeed.mach);
    } break;

    case APSIDESTIME_MESSAGE:
    if (msgSize == sizeof(apsidesTimeMessage)) {
      apsidesTimeMessage myApsidesTime;
      myApsidesTime = parseApsidesTime(msg);
      // display.set(myApsidesTime.apoapsis);
      // display.set(myApsidesTime.periapsis);
    } break;
  }
}

void orbitalMechanics() {


  
  //Check SOI
  if BODY = n {int x = n;}
  unsigned long R  = planetData[x+1];
  unsigned long MU = planetData[x+2];
  unsigned long RS = planetData[x+3];

  //Calculate Data
  long SMA        = (((AP + R) + (PE + R))/2);
  long Period     = (2*PI*sqrt((SMA*SMA*SMA) / MU));
  float SurfaceHV  = (sqrt((SV*SV) - (VV*VV)));
  float OrbitalHV  = (sqrt((OV*OV) - (VV*VV)));
  float RotateV    = (RS*AL/R);
  float Inclin     = (ARCCOS(((OrbitalHV*OrbitalHV)+(RotateV*RotateV)-(SurfaceHV*SurfaceHV))/(2*OrbitalHV*RotateV)));
  float ImpactV    = (sqrt(2*MU/(R+(AL-AS))-(MU/SMA));
  //long ImpactT    = ();

  //if PE < planetData[x+4] {turn on unstable orbit warning}
  //if AL < planetData[x+1] {turn on atmosphere detection light}

  //Take max of Inclin to determine AN/DN. Period/2 = time to next node.
}

double planetData[] = {

  //Body    //Radius        //Standard G Parameter   //Rotational Speed   //Atmosphere Height
  Kerbol,    261600000,     1.1723328e18,            3804.8,              -10000,

  Moho,      250000,        1.6860938e11,            1.2982,              -10000,

  Eve,       700000,        8.1717302e12,            54.636,              90000,
  Gilly,     13000,         8289449.8,               2.8909,              -10000,

  Kerbin,    600000,        3.5316000e12,            174.94,              70000,
  Mun,       200000,        6.5138398e10,            9.0416,              -10000,
  Minmus,    60000,         1.7658000e9,             9.3315,              -10000,

  Duna,      320000,        3.0136321e11,            30.688,              50000,
  Ike,       130000,        1.8568369e10,            12.467,              -10000,

  Dres,      138000,        2.1484489e10,            24.916,              -10000,

  Jool,      6000000,       2.8252800e14,            1047.2,              200000,
  Laythe,    500000,        1.9620000e12,            59.297,              50000,
  Vall,      300000,        2.0748150e11,            17.789,              -10000,
  Tylo,      600000,        2.8252800e12,            17.789,              -10000,
  Bop,       65000,         2.4868349e9,             0.75005,             -10000,
  Pol,       44000,         7.2170208e8,             0.30653,             -10000,

  Sarnus,    5300000,       8.2089702e13,            1168.5,              580000,
  Hale,      6000,          811990.62,               1.6005,              -10000,
  Ovok,      26000,         13258591,                5.5490,              -10000,
  Eeloo,     210000,        7.4410815e10,            22.783,              -10000,
  Slate,     540000,        1.9788564e12,            17.601,              -10000,
  Tekto,     280000,        1.9244099e11,            2.6410,              95000,

  Urlum,     2177000,       1.1944574e13,            333.62,              325000,
  Polta,     220000,        9.0181953e10,            18.931,              -10000,
  Priax,     74000,         3.3831766e9,             6.3678,              -10000,
  Wal,       370000,        4.9673624e11,            2.3031,              -10000,
  Tal,       22000,         2.1358884e8,             2.8283,              -10000,

  Neidon,    2145000,       1.4167882e13,            334.84,              260000,
  Thatmo,    286000,        1.8609758e11,            5.8640,              35000,
  Nissee,    30000,         3.9716933e8,             6.7501,              -10000,

  Plock,     189000,        5.1844895e10,            11.170,              -10000,
};

//void throttleControl() {
//  int throttle = analogRead(2);
//  int scaled_y_reading = map(throttle, 25, 1023, -32768, 32767);
//  mySimpit.send(THROTTLE_MESSAGE, scaled_y_reading);
//  delay(25);
//}

//void rotationControl() {}

//void translationControl() {}

//void planetData() {}
