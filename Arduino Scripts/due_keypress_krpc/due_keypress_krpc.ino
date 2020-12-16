#include <krpc.h>
#include <krpc/services/krpc.h>
#include <krpc/services/space_center.h>

#include <Keyboard.h>
#include <ShiftIn.h>
#include <ShiftRegister74HC595.h>

// kRPC Objects
HardwareSerial * conn;
krpc_SpaceCenter_Flight_t flight;
krpc_SpaceCenter_Control_t control;
krpc_SpaceCenter_Vessel_t vessel;
krpc_SpaceCenter_SASMode_t mode;
krpc_SpaceCenter_Resources_t resources;
krpc_SpaceCenter_Comms_t comms;
krpc_SpaceCenter_Camera_t camera;
krpc_SpaceCenter_Orbit_t orbit;
krpc_SpaceCenter_Vessel_t target;
krpc_SpaceCenter_Parts_t parts_obj;
krpc_list_object_t parts_list;
krpc_SpaceCenter_CameraMode_t cameraMode;

// # Chips
ShiftIn<8> shift_buttons;

// # Chips / DATA / CLOCK / LATCH
ShiftRegister74HC595 sr_lights (5, 6, 7, 5);
ShiftRegister74HC595 sr_resources (8, 6, 7, 8);
ShiftRegister74HC595 sr_resources2 (8, 9, 10, 11);
ShiftRegister74HC595 sr_resources3 (6, 12, 13, 14);


int x;

// Utilities
bool ignition1 = true;
bool ignition2 = true;
bool allZero = true;
const int pressK = 0;
const int holdK = 1;
unsigned long lastDebounceTime = 0;
unsigned long lightsDebounceTime = 0;
bool keyboardOn = false;

// Button Hold Utilities
int cacheAG[]{0,0,0,0,0,0,0};
int cacheSAS[]{0,0,0,0,0,0,0,0,0,0};
bool cacheNAV = false;
bool cacheBool = false;

// Number Constants
const int nKeyPresses = 42;
const int mapT = nKeyPresses + 1;
const int navT = nKeyPresses + 2;
const int sasT = nKeyPresses + 3;
const int rcsT = nKeyPresses + 4;
const int nToggles = 4;
const int nAllKeys = (nKeyPresses + nToggles);
const int nSAS = 10;
const int nAG = 7;
const int nAllShiftIn = 63;
const int nResources = 9;

// kRPC Utilities
float timewarp = 0;
int navBallMode = 1;
double commLink;
bool stateSAS;
bool stateAG;
bool sasChange = true;
bool agChange = true;
bool twChange = true;

int ASCII[] = {

       // EVA Controls
      98,     pressK, // EVA Board        (B)              <<<<< Change hotkey binding in KSP (brakes)
      80,     pressK, // EVA Jump/Let Go  (P)              <<<<< Change hotkey binding in KSP Y/O/P/; (stage)
      89,     holdK,  // EVA Up           (Y)              <<<<< Change hotkey binding in KSP Y/O/P/; (thrust)
      79,     holdK,  // EVA Down         (O)              <<<<< Change hotkey binding in KSP Y/O/P/; (thrust)
     102,     pressK, // EVA Grab         (F)
      59,     pressK, // EVA Headlamp     (;)              <<<<< Change hotkey binding in KSP Y/O/P/; (RCS translate right)
       
       // Action Groups
      49,     pressK, // AG 1             (1)
      50,     pressK, // AG 2             (2)
      51,     pressK, // AG 3             (3)
      52,     pressK, // AG 4             (4)
      53,     pressK, // AG 5             (5)
      54,     pressK, // AG 6             (6)
      55,     pressK, // AG 7             (7)
      56,     pressK, // AG 8             (8)
      57,     pressK, // AG 9             (9)
      48,     pressK, // AG 10            (10)

       // Pilot Handling
      98,     holdK,  // Brakes           (B)              <<<<< Needs own function
     120,     pressK, // Reset Trim       (X)
     134,     holdK,  // Trim             (ALT)
       8,     pressK, // Abort            (BACKSPACE)
      32,     pressK, // Stage            (SPACE)

       // Navigation
      96,     pressK, // Reset Map        (`)
     000,     pressK, // Cycle Nav        (----)   // Needs function
     179,     pressK, // Cycle Map        (TAB)
      93,     pressK, // Cycle Ships      (])
      99,     pressK, // IVA              (C)
     195,     pressK, // Toggle UI        (F2)
     118,     pressK, // Cycle Views      (V)
     194,     pressK, // Screenshot       (F1)

       // Timewarp and Camera
      47,     pressK, // Stop Timewarp    (/)
     218,     holdK,  // Camera Up        (Arrow Up)
     217,     holdK,  // Camera Down      (Arrow Down)
     215,     holdK,  // Camera Right     (Arrow Right)
     223,     holdK,  // Zoom In          (Numpad +)
     216,     holdK,  // Camera Left      (Arrow Left)
     222,     holdK,  // Zoom Out         (Numpad -)
     177,     pressK, // Escape           (ESC)
     134,     holdK,  // Phys Timewarp    (ALT)
      46,     pressK, // Timewarp +       (.)
      44,     pressK, // Timewarp -       (,)

       // Special Action Groups
     117,     pressK, // Lights           (U)
     103,     pressK, // Gear             (G)
      98,     pressK, // Brakes           (B)              <<<<< Needs own function
       
       
        // Toggles
      116,     // SAS              (T)
      114,     // RCS              (R)
      109,     // Map              (M)
      235,     // Navball          (Numpad .)
};

int kRPCActionGroup[] {
  // Custom Action Groups
  11,      // CAG 11
  12,      // CAG 12
  13,      // CAG 13
  14,      // CAG 14
  15,      // CAG 15
  16,      // CAG 16
  17,      // CAG 17
};

krpc_SpaceCenter_SASMode_t kRPC_SASModes[] {
  // Navigation
  KRPC_SPACECENTER_SASMODE_STABILITYASSIST,
  KRPC_SPACECENTER_SASMODE_MANEUVER,
  KRPC_SPACECENTER_SASMODE_PROGRADE,
  KRPC_SPACECENTER_SASMODE_RETROGRADE,
  KRPC_SPACECENTER_SASMODE_NORMAL,
  KRPC_SPACECENTER_SASMODE_ANTINORMAL,
  KRPC_SPACECENTER_SASMODE_RADIAL,
  KRPC_SPACECENTER_SASMODE_ANTIRADIAL,
  KRPC_SPACECENTER_SASMODE_TARGET,
  KRPC_SPACECENTER_SASMODE_ANTITARGET,
};

void setup() {
  conn = &Serial;
  krpc_open(&conn, NULL);
  krpc_connect(conn, "Flight Control");
  
  krpc_SpaceCenter_ActiveVessel(conn, &vessel);
  krpc_SpaceCenter_Vessel_Control(conn, &control, vessel);
  krpc_SpaceCenter_Vessel_Flight(conn, &flight, vessel, KRPC_NULL);
  krpc_SpaceCenter_Vessel_Resources(conn, &resources, vessel);
  krpc_SpaceCenter_Vessel_Comms(conn, &comms, vessel);
  krpc_SpaceCenter_Vessel_Parts(conn, &parts_obj, vessel);
  krpc_SpaceCenter_Parts_All(conn, &parts_list, parts_obj);
  
  shift_buttons.begin(15, 14, 16, 17);
  lastDebounceTime = millis();
}

void loop() {
  shift_buttons.update();
  if (ignition1 and ignition2){
    main_sequence();
  }
}

void main_sequence(){
  digitalWrite(LED_BUILTIN, HIGH);
  key_presses();
  toggle_presses();
  kRPC_presses();
  
  if ((keyboardOn==false) and ((millis() - lightsDebounceTime) > 100)){
    if (sasChange==true){
      SAS_Lights();
    }
    if (agChange == true){
      AG_Lights();
    }
    if(twChange == true){
      utility_Lights();
    }
    lightsDebounceTime = millis();
  }
  
  digitalWrite(LED_BUILTIN, LOW);
  //delay(200);
}

void key_presses(){
  if ((millis() - lastDebounceTime) > 10) {
    for (int i = 0; i < nKeyPresses+1; i++){
      if (shift_buttons.hasChanged(i)==1){
        int k = (i*2);
        if ((shift_buttons.state(i)==1) and (ASCII[k+1]==0)){
          krpc_close(conn);
          Keyboard.begin();
          Keyboard.write(ASCII[k]);
          conKRPC();
        }
        if ((shift_buttons.state(i)==1) and (ASCII[k+1]==1)){
          krpc_close(conn);
          Keyboard.begin();
          Keyboard.press(ASCII[k]);
          keyboardOn = true;
        }
        if ((shift_buttons.state(i)==0) and (ASCII[k+1]==1)){
          Keyboard.release(ASCII[k]);
          keyboardOn = false;
          conKRPC();
        }
        if ((i > 5) and (i < 16)){
          agChange = true;
        }
        if ((i == 38) or (i == 39)){
          twChange = true;
        }
        lastDebounceTime = millis();
      }
      
      // return 0 if any button is pressed
      allZero |= shift_buttons.state(i);
    }
  }
  // end keyboard emulation if no buttons are pressed
  if (allZero) {Keyboard.end();}
}

void toggle_presses(){
  
  if (shift_buttons.hasChanged(sasT)) {
    if ((millis() - lastDebounceTime) > 50) {
      if (shift_buttons.state(sasT)==1){
        if (keyboardOn){
          Keyboard.write(116);
        }else{
          krpc_SpaceCenter_Control_set_SAS(conn, control, true);
        }
      }
      if (shift_buttons.state(sasT)==0){
        if (keyboardOn){
          Keyboard.write(116);
        }else{
          krpc_SpaceCenter_Control_set_SAS(conn, control, false);
        }
      }
      sasChange = true;
      lastDebounceTime = millis();
    }
  }
    
  if (shift_buttons.hasChanged(rcsT)) {
    if ((millis() - lastDebounceTime) > 50) {
      if (shift_buttons.state(rcsT)==1){
        if (keyboardOn){
          Keyboard.write(114);
        }else{
          krpc_SpaceCenter_Control_set_RCS(conn, control, true);
        }
      }
      if (shift_buttons.state(rcsT)==0){
        if (keyboardOn){
          Keyboard.write(114);
        }else{
          krpc_SpaceCenter_Control_set_RCS(conn, control, false);
        }
      }
      lastDebounceTime = millis();
    }
  }
  
  if (shift_buttons.hasChanged(navT)) {
    if ((millis() - lastDebounceTime) > 50) {
      if (shift_buttons.state(navT)==1){
        if (keyboardOn){
          Keyboard.write(235);
        }else{
          krpc_SpaceCenter_set_Navball(conn, true);
        }
      }
      if (shift_buttons.state(navT)==0){
        if (keyboardOn){
          Keyboard.write(235);
        }else{
          krpc_SpaceCenter_set_Navball(conn, false);
        }
      }
      lastDebounceTime = millis();
    }
  }
  
  if (shift_buttons.hasChanged(mapT)) {
    if ((millis() - lastDebounceTime) > 50) {
      krpc_close(conn);
      Keyboard.begin();
      Keyboard.write(109);
      conKRPC();

      /*
      if (shift_buttons.state(mapT)==1){
        krpc_SpaceCenter_Camera_set_Mode(conn, camera, KRPC_SPACECENTER_CAMERAMODE_MAP);
      }
      if (shift_buttons.state(mapT)==0){
        krpc_SpaceCenter_Camera_Mode(conn, &cameraMode, camera);
        if (cameraMode==KRPC_SPACECENTER_CAMERAMODE_MAP){
          krpc_close(conn);
          Keyboard.begin();
          Keyboard.write(109);
          conKRPC();
        }
      }
      */

      lastDebounceTime = millis();
    }
  }
}

void kRPC_presses(){
  
  // Action Groups 11-17
  for (int i = 47; i < 54; i++){
    if (shift_buttons.hasChanged(i)==1){
      if (shift_buttons.state(i)==1){
        if (keyboardOn){
          cacheAG[i-47] = 1;
          cacheBool = true;
        }else{
          krpc_SpaceCenter_Control_ToggleActionGroup(conn, control, kRPCActionGroup[i-47]);
          agChange = true;
        }
      }
    }
  }
  
  // SAS
  for (int i = 54; i < 64; i++){
    if (shift_buttons.hasChanged(i)==1){
      if (shift_buttons.state(i)==1){
        if (keyboardOn){
          cacheSAS[i-54] = 1;
          cacheBool = true;
        }else{
          krpc_SpaceCenter_Control_set_SASMode(conn, control, kRPC_SASModes[i-54]);
          sasChange = true;
        }
      }
    }
  }
  
  // NavBall Modes
  if (shift_buttons.hasChanged(22)==1){
    if (shift_buttons.state(22)==1){
      if (keyboardOn){
          cacheNAV = 1;
          cacheBool = true;
      }else{
      
        if (navBallMode == 1) {
          krpc_SpaceCenter_Control_set_SpeedMode(conn, control, KRPC_SPACECENTER_SPEEDMODE_ORBIT);
        }
        if (navBallMode == 2) {
          krpc_SpaceCenter_Control_set_SpeedMode(conn, control, KRPC_SPACECENTER_SPEEDMODE_SURFACE);
        }
        if (navBallMode == 3) {
          krpc_SpaceCenter_Control_set_SpeedMode(conn, control, KRPC_SPACECENTER_SPEEDMODE_TARGET);
        }
  
        navBallMode = (navBallMode + 1);
        if (navBallMode > 3) {
          navBallMode = 1;
        }
      }
    }
  }

  if ((keyboardOn==false) and (cacheBool)){
    for (int i = 0; i < 7; i++){
      if (cacheAG[i] == 1){
        krpc_SpaceCenter_Control_ToggleActionGroup(conn, control, kRPCActionGroup[i+10]);
        cacheAG[i] = 0;
        agChange = true;
      }
    }
    for (int i = 0; i < 10; i++){
      if (cacheSAS[i] == 1){
        krpc_SpaceCenter_Control_set_SASMode(conn, control, kRPC_SASModes[i]);
        sasChange = true;
        cacheSAS[i] = 0;
      }
    }
    if (cacheNAV){
      if (navBallMode == 1) {
        krpc_SpaceCenter_Control_set_SpeedMode(conn, control, KRPC_SPACECENTER_SPEEDMODE_ORBIT);
      }
      if (navBallMode == 2) {
        krpc_SpaceCenter_Control_set_SpeedMode(conn, control, KRPC_SPACECENTER_SPEEDMODE_SURFACE);
      }
      if (navBallMode == 3) {
        krpc_SpaceCenter_Control_set_SpeedMode(conn, control, KRPC_SPACECENTER_SPEEDMODE_TARGET);
      }

      navBallMode = (navBallMode + 1);
      if (navBallMode > 3) {
        navBallMode = 1;
      }
      cacheNAV = 0;
    }
    cacheBool = false;
  }
}

void SAS_Lights() {
  
  krpc_SpaceCenter_Control_SAS(conn, &stateSAS, control);
  if (stateSAS == false){
    for (int i = 0; i < 10; i++){
      sr_lights.set(i, LOW);
    }
  }

  krpc_SpaceCenter_Control_SASMode(conn, &mode, control);
  if (stateSAS == true){
    for (int i = 0; i < 10; i++){
      if (mode == kRPC_SASModes[i]) {
        sr_lights.set(i, HIGH);
      } else {
        sr_lights.set(i, LOW);
      }
    }
  }
  
  sasChange = false;
}

void AG_Lights() {
  // Lights for all custom action groups
  for (int i = 0; i < 10; i++) {
    krpc_SpaceCenter_Control_GetActionGroup(conn, &stateAG, control, i);
    if (stateAG) {
      sr_lights.set(i+9, HIGH);
    } else {
      sr_lights.set(i+9, LOW);
    }
  }

  // Lights for standard action groups (Gear, Lights, Brakes)
  krpc_SpaceCenter_Control_Gear(conn, &stateAG, control);
  if (stateAG) {
    sr_lights.set(28, HIGH);
  } else {
    sr_lights.set(28, LOW);
  }
  krpc_SpaceCenter_Control_Lights(conn, &stateAG, control);
  if (stateAG) {
    sr_lights.set(27, HIGH);
  } else {
    sr_lights.set(27, LOW);
  }
  krpc_SpaceCenter_Control_Brakes(conn, &stateAG, control);
  if (stateAG) {
    sr_lights.set(29, HIGH);
  } else {
    sr_lights.set(29, LOW);
  }
  
  agChange = false;
}

void utility_Lights(){
  // Timewarp Light
  krpc_SpaceCenter_WarpFactor(conn, &timewarp);
  if ((int) timewarp != 0) {
    sr_lights.set(30, HIGH);
  }
  else {
    sr_lights.set(30, LOW);
  }
}






void conKRPC(){
  krpc_open(&conn, NULL);
  krpc_connect(conn, "Flight Control"); 
}
