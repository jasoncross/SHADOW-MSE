// =======================================================================================
//                 SHADOW :  Small Handheld Arduino Droid Operating Wand
// =======================================================================================
//                          Last Revised Date: 08/16/19
//                             Written By: KnightShade
//                        Inspired by the PADAWAN by danf
//                              Hacked By: CJE1985
//                              Modded by: IOIIOOO
// =======================================================================================
//
//         This program is free software: you can redistribute it and/or modify it .
//         This program is distributed in the hope that it will be useful,
//         but WITHOUT ANY WARRANTY; without even the implied warranty of
//         MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
// =======================================================================================
//   Note: You will need a Arduino Mega 2560 or Uno to run this sketch
//
//   This is written to be a UNIVERSAL Sketch - supporting multiple controller options
//      - Single PS3 Move Navigation
//      - Pair of PS3 Move Navigation
//
//   PS3 Bluetooth library - developed by Kristian Lauszus (kristianl@tkjelectronics.com)
//   For more information visit my blog: http://blog.tkjelectronics.dk/ or
//
// =======================================================================================
//        Updated:  12/26/2017
// ---------------------------------------------------------------------------------------
//                          User Settings
// ---------------------------------------------------------------------------------------

//Primary Controller
String PS3MoveNavigatonPrimaryMAC = "00:06:F7:C2:D6:C2"; //If using multiple controlers, designate a primary

byte joystickDeadZoneRange = 10;  // For controllers that centering problems, use the lowest number with no drift

int steeringNeutral = 93;        // Move this by one or two to set the center point for the steering servo

#define reverseSteering          // Comment this to have normal stering direction (Lots of RC cars are built with reverseSteering by default)
int steeringRightEndpoint = 160; // Move this down (below 180) if you need to set a narrower Right turning radius
int steeringLeftEndpoint = 0;    // Move this up (above 0) if you need to set a narrower Left turning radius

int driveNeutral = 91;           // Move this by one or two to set the center point for the drive ESC
int maxForwardSpeed = 105;       // Move this down (below 180, but above 90) if you need a slower forward speed
int maxReverseSpeed = 9;        // Move this up (above 0, but below 90) if you need a slower reverse speed

//#define TEST_CONROLLER         //Support coming soon
//#define SHADOW_DEBUG           //uncomment this for console DEBUG output
#define SHADOW_VERBOSE           //uncomment this for console VERBOSE output

// ---------------------------------------------------------------------------------------
//                          Drive Controller Settings
// ---------------------------------------------------------------------------------------

#define steeringPin 4            // connect this pin to steering servo for MSE (R/C mode)
#define drivePin 3               // connect this pin to ESC for forward/reverse drive (R/C mode)
#define L2Throttle               // comment this to use Joystick throttle (instead of L2 throttle)

// ---------------------------------------------------------------------------------------
//                          Sound Control Settings
// ---------------------------------------------------------------------------------------

#define MP3TxPin 2               // connect this pin to the MP3 Trigger
#define MP3RxPin 8               // This pin isn't used by the sparkfun MP3 trigger, but is used by the MDFly
//#define Sparkfun               // Use the sparkfun MP3 Trigger
#define MDFly                    // Use the MDFly MP3 Player
//#define RogueRMP3              // Use the Rogue RMP3 Player

uint8_t SoundsGroup[15] = { 1, 5, 6, 2, 7, 8, 3, 9, 10, 4, 12, 13, 14 };
uint8_t AlertSoundsGroup[15] = { 1, 1, 1 };
uint8_t DoDoSoundsGroup[15] = { 2, 7, 8 };
uint8_t ScrambleSoundsGroup[15] = { 3, 9, 10 };
uint8_t HornSoundsGroup[15] = { 4, 12, 13, 14 };
uint8_t DroidSoundsGroup[15] = { 15, 16, 17, 18, 19 };
uint8_t ZapSoundsGroup[15] = { 5, 6 };
uint8_t startupSound = 24;
uint8_t imperialMarchSong = 22;
uint8_t cantinaSong = 20;
uint8_t SWSong = 21;
uint8_t MSESong = 23;
uint8_t playSound;
uint8_t lastSound;
uint8_t soundType;
uint8_t lastSoundType;

// ---------------------------------------------------------------------------------------
//                          Libraries
// ---------------------------------------------------------------------------------------
#include <PS3BT.h>
#include <Servo.h>


#ifdef Sparkfun
#include <MP3Trigger.h>
#include <SoftwareSerial.h>
SoftwareSerial swSerial = SoftwareSerial(MP3RxPin, MP3TxPin);
MP3Trigger MP3;
#endif

#ifdef MDFly
#include <serMP3.h>
serMP3 MP3(MP3TxPin, MP3RxPin);
#endif

// ---------------------------------------------------------------------------------------
//                          Variables
// ---------------------------------------------------------------------------------------

long previousMillis = millis();
long currentMillis = millis();
int serialLatency = 25;          // This is a delay factor in ms to prevent queueing of the Serial data.
                                 // 25ms seems approprate for HardwareSerial, values of 50ms or larger
                                 // are needed for Softare Emulation

///////Setup for USB and Bluetooth Devices////////////////////////////
USB Usb;
BTD Btd(&Usb);                   // You have to create the Bluetooth Dongle instance like so
PS3BT *PS3Nav = new PS3BT(&Btd);
PS3BT *PS3Nav2 = new PS3BT(&Btd);
//Used for PS3 Fault Detection
uint32_t msgLagTime = 0;
uint32_t lastMsgTime = 0;
uint32_t currentTime = 0;
uint32_t lastLoopTime = 0;
int badPS3Data = 0;

boolean firstMessage = true;
String output = "";

boolean isDriveMotorStopped = true;

boolean isPS3NavigatonInitialized = false;
boolean isSecondaryPS3NavigatonInitialized = false;

byte vol = 16;                   // 0 = full volume, 255 off for MP3Trigger, the MDFly use a 0-31 vol range
boolean isStickEnabled = true;

byte action = 0;
unsigned long DriveMillis = 0;

boolean wasDriving = false;  // used to track driving status

Servo steeringSignal;
Servo driveSignal;
int steeringValue, driveValue;   //will hold steering/drive values (-100 to 100)
int prevSteeringValue, prevDriveValue; //will hold steering/drive speed values (-100 to 100)


// =======================================================================================
//                          Main Program
// =======================================================================================

void setup()
{
  
  //Debug Serial for use with USB Debugging
  Serial.begin(115200);
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
  if (Usb.Init() == -1)
  {
    Serial.print(F("\r\nOSC did not start"));
    while (1); //halt
  }
  Serial.print(F("\r\nBluetooth Library Started"));
  output.reserve(200); // Reserve 200 bytes for the output string

  //Setup for PS3
  PS3Nav->attachOnInit(onInitPS3); // onInit() is called upon a new connection - you can call the function whatever you like
  PS3Nav2->attachOnInit(onInitPS3Nav2);

  // random seed
  randomSeed(analogRead(0));

#ifdef Sparkfun
  //Setup for SoftwareSerial/MP3 Trigger
  MP3.setup(&swSerial);
  swSerial.begin(MP3Trigger::serialRate());
  MP3.setVolume(vol);
  Serial.print(F("\r\nSparkfun MP3 Trigger Initialized"));
#endif

#ifdef MDFly
  MP3.begin(vol);
  Serial.print(F("\r\nMDFly MP3 Initialized"));
#endif

  Serial.print(F("\r\nMSE Drive Running"));
  steeringSignal.attach(steeringPin);
  driveSignal.attach(drivePin);

  #ifdef MDFly
        MP3.play(startupSound);
  #endif

  #ifdef Sparkfun
        MP3.trigger(startupSound);
  #endif
}

boolean readUSB()
{
  //The more devices we have connected to the USB or BlueTooth, the more often Usb.Task need to be called to eliminate latency.
  Usb.Task();
  if (PS3Nav->PS3NavigationConnected ) Usb.Task();
  if (PS3Nav2->PS3NavigationConnected ) Usb.Task();
  if ( criticalFaultDetect() )
  {
    //We have a fault condition that we want to ensure that we do NOT process any controller data!!!
    flushAndroidTerminal();
    return false;
  }
  return true;
}

void loop()
{
  //Useful to enable with serial console when having controller issues.
#ifdef TEST_CONROLLER
  testPS3Controller();
#endif

  //LOOP through functions from highest to lowest priority.

  if ( !readUSB() )
  {
    //We have a fault condition that we want to ensure that we do NOT process any controller data!!!
    return;
  }
  Drive();

  if ( !readUSB() )
  {
    //We have a fault condition that we want to ensure that we do NOT process any controller data!!!
    return;
  }

  toggleSettings();
  soundControl();
  flushAndroidTerminal();
}

void onInitPS3()
{
  String btAddress = getLastConnectedBtMAC();
  PS3Nav->setLedOn(LED1);
  isPS3NavigatonInitialized = true;
  badPS3Data = 0;
#ifdef SHADOW_DEBUG
  output += "\r\nBT Address of Last connected Device when Primary PS3 Connected: ";
  output += btAddress;
  if (btAddress == PS3MoveNavigatonPrimaryMAC)
  {
    output += "\r\nWe have our primary controller connected.\r\n";
  }
  else
  {
    output += "\r\nWe have a controller connected, but it is not designated as \"primary\".\r\n";
  }
#endif
}

void onInitPS3Nav2()
{
  String btAddress = getLastConnectedBtMAC();
  PS3Nav2->setLedOn(LED1);
  isSecondaryPS3NavigatonInitialized = true;
  badPS3Data = 0;
  if (btAddress == PS3MoveNavigatonPrimaryMAC) swapPS3NavControllers();
#ifdef SHADOW_DEBUG
  output += "\r\nBT Address of Last connected Device when Secondary PS3 Connected: ";
  output += btAddress;
  if (btAddress == PS3MoveNavigatonPrimaryMAC)
  {
    output += "\r\nWe have our primary controller connecting out of order.  Swapping locations\r\n";
  }
  else
  {
    output += "\r\nWe have a secondary controller connected.\r\n";
  }
#endif
}

String getLastConnectedBtMAC()
{
  String btAddress = "";
  for (int8_t i = 5; i > 0; i--)
  {
    if (Btd.disc_bdaddr[i] < 0x10)
    {
      btAddress += "0";
    }
    btAddress += String(Btd.disc_bdaddr[i], HEX);
    btAddress += (":");
  }
  btAddress += String(Btd.disc_bdaddr[0], HEX);
  btAddress.toUpperCase();
  return btAddress;
}

void swapPS3NavControllers()
{
  PS3BT* temp = PS3Nav;
  PS3Nav = PS3Nav2;
  PS3Nav2 = temp;
  //Correct the status for Initialization
  boolean tempStatus = isPS3NavigatonInitialized;
  isPS3NavigatonInitialized = isSecondaryPS3NavigatonInitialized;
  isSecondaryPS3NavigatonInitialized = tempStatus;
  //Must relink the correct onInit calls
  PS3Nav->attachOnInit(onInitPS3);
  PS3Nav2->attachOnInit(onInitPS3Nav2);
}

void flushAndroidTerminal()
{
  if (output != "")
  {
    if (Serial) Serial.println(output);
    output = ""; // Reset output string
  }
}

// =======================================================================================
// //////////////////////////Process PS3 Controller Fault Detection///////////////////////
// =======================================================================================
boolean criticalFaultDetect()
{
  if (PS3Nav->PS3NavigationConnected || PS3Nav->PS3Connected)
  {
    lastMsgTime = PS3Nav->getLastMessageTime();
    currentTime = millis();
    if ( currentTime >= lastMsgTime)
    {
      msgLagTime = currentTime - lastMsgTime;
    } else
    {
#ifdef SHADOW_DEBUG
      output += "Waiting for PS3Nav Controller Data\r\n";
#endif
      badPS3Data++;
      msgLagTime = 0;
    }

    if (msgLagTime > 100 && !isDriveMotorStopped)
    {
#ifdef SHADOW_DEBUG
      output += "It has been 100ms since we heard from the PS3 Controller\r\n";
      output += "Shut downing motors, and watching for a new PS3 message\r\n";
#endif

      driveSignal.write(driveNeutral);
      steeringSignal.write(steeringNeutral);
      isDriveMotorStopped = true;
      return true;
    }
    if ( msgLagTime > 30000 )
    {
#ifdef SHADOW_DEBUG
      output += "It has been 30s since we heard from the PS3 Controller\r\n";
      output += "msgLagTime:";
      output += msgLagTime;
      output += "  lastMsgTime:";
      output += lastMsgTime;
      output += "  millis:";
      output += millis();
      output += "\r\nDisconnecting the controller.\r\n";
#endif
      PS3Nav->disconnect();
    }

    //Check PS3 Signal Data
    if (!PS3Nav->getStatus(Plugged) && !PS3Nav->getStatus(Unplugged))
    {
      // We don't have good data from the controller.
      //Wait 10ms, Update USB, and try again
      delay(10);
      Usb.Task();
      if (!PS3Nav->getStatus(Plugged) && !PS3Nav->getStatus(Unplugged))
      {
        badPS3Data++;
#ifdef SHADOW_DEBUG
        output += "\r\nInvalid data from PS3 Controller.";
#endif
        return true;
      }
    }
    else if (badPS3Data > 0)
    {
      //output += "\r\nPS3 Controller  - Recovered from noisy connection after: ";
      //output += badPS3Data;
      badPS3Data = 0;
    }
    if ( badPS3Data > 10 )
    {
#ifdef SHADOW_DEBUG
      output += "Too much bad data coming fromo the PS3 Controller\r\n";
      output += "Disconnecting the controller.\r\n";
#endif
      PS3Nav->disconnect();
    }
  }
  else if (!isDriveMotorStopped)
  {
#ifdef SHADOW_DEBUG
    output += "No Connected Controllers were found\r\n";
    output += "Shuting downing motors, and watching for a new PS3 message\r\n";
#endif

    driveSignal.write(driveNeutral);
    steeringSignal.write(steeringNeutral);
    isDriveMotorStopped = true;
    return true;
  }
  return false;
}
// =======================================================================================
// //////////////////////////END of PS3 Controller Fault Detection///////////////////////
// =======================================================================================


boolean ps3Drive(PS3BT* myPS3 = PS3Nav)
{
  if (isPS3NavigatonInitialized) {
    // Additional fault control.  Do NOT send additional commands if no controllers have initialized.
    if (!isStickEnabled) {
      #ifdef SHADOW_VERBOSE
      if ( abs(myPS3->getAnalogHat(LeftHatY) - 128) > joystickDeadZoneRange)
      {
        output += "Drive Stick is disabled\r\n";
      }
      #endif

      driveSignal.write(driveNeutral);
      steeringSignal.write(steeringNeutral);
      isDriveMotorStopped = true;
      
    } else if (!myPS3->PS3NavigationConnected) {
      driveSignal.write(driveNeutral);
      steeringSignal.write(steeringNeutral);
      isDriveMotorStopped = true;

    } else {
      int stickX = myPS3->getAnalogHat(LeftHatX);
      #ifdef L2Throttle
      int stickY = myPS3->getAnalogButton(L2);
      #else
      int stickY = myPS3->getAnalogHat(LeftHatY);
      #endif

      #ifdef L2Throttle
      // map the steering direction
      if (((stickX <= 128 - joystickDeadZoneRange) || (stickX >= 128 + joystickDeadZoneRange))) {
        #ifdef reverseSteering
          steeringValue = map(stickX, 0, 255, steeringRightEndpoint, steeringLeftEndpoint);
        #else
          steeringValue = map(stickX, 0, 255, steeringLeftEndpoint, steeringRightEndpoint);
        #endif
      } else {
        steeringValue = steeringNeutral;
        driveValue = driveNeutral;
      }

      // map the drive direction
      if (myPS3->getButtonPress(L1) && myPS3->getAnalogButton(L2)) {
        // These values must cross 90 (as that is stopped)
        // The closer these values are the more speed control you get
        driveValue = map(stickY, 0, 255, 90, maxReverseSpeed);
      } else if (myPS3->getAnalogButton(L2)) {
        // These values must cross 90 (as that is stopped)
        // The closer these values are the more speed control you get
        driveValue = map(stickY, 0, 255, 90, maxForwardSpeed);
      }
      #else
      if (((stickX <= 128 - joystickDeadZoneRange) || (stickX >= 128 + joystickDeadZoneRange)) ||
          ((stickY <= 128 - joystickDeadZoneRange) || (stickY >= 128 + joystickDeadZoneRange))) {
            #ifdef reverseSteering
              steeringValue = map(stickX, 0, 255, steeringRightEndpoint, steeringLeftEndpoint);
            #else
              steeringValue = map(stickX, 0, 255, steeringLeftEndpoint, steeringRightEndpoint);
            #endif
            // These values must cross 90 (as that is stopped)
            // The closer these values are the more speed control you get
            driveValue = map(stickY, 0, 255, maxForwardSpeed, maxReverseSpeed);
      } else {
        // stop all movement
        steeringValue = steeringNeutral;
        driveValue = driveNeutral;
      }
      #endif

      driveSignal.write(driveValue);
      steeringSignal.write(steeringValue);

      return true; //we sent a drive command
    }
  }
  return false;
}


void ps3ToggleSettings(PS3BT* myPS3 = PS3Nav)
{
  if (myPS3->getButtonPress(PS) && myPS3->getButtonClick(L3))
  {
    //Quick Shutdown of PS3 Controller
    output += "\r\nDisconnecting the controller.\r\n";
    myPS3->disconnect();
  }


  //// enable / disable right stick & play sound
  if (myPS3->getButtonPress(PS) && myPS3->getButtonClick(CROSS))
  {
 #ifdef SHADOW_DEBUG
          output += "Disabling the DriveStick\r\n";
  #endif
    isStickEnabled = false;
    processSoundCommand('5');
  }
  if (myPS3->getButtonPress(PS) && myPS3->getButtonClick(CIRCLE))
  {
#ifdef SHADOW_DEBUG
    output += "Enabling the DriveStick\r\n";
#endif
    isStickEnabled = true;
    processSoundCommand('6');
  }
}

void processSoundCommand(char soundCommand)
{
  switch (soundCommand)
  {
    case '-':
#ifdef SHADOW_DEBUG
      output += "Volume Down\r\n";
#endif

      if (vol > 0)
      {

#ifdef MDFly
        vol--;
        MP3.setVol(vol);
#endif

#ifdef Sparkfun
        vol = vol-5;
        if (vol < 0 ) { vol = 0 ;}
        MP3.setVolume(vol);
#endif

      }
      break;
    case '+':
#ifdef SHADOW_DEBUG
      output += "Volume Up\r\n";
#endif

#ifdef MDFly
      // The TDB380 and 381 use a 0-31 vol range
      if (vol < 31) {
          vol++;
          MP3.setVol(vol);
      }
#endif

#ifdef Sparkfun
        // The MP3Trigger use a 0-255 vol range
        if (vol < 255) {
          vol = vol+5;
          if (vol > 255 ) { vol = 255 };
          MP3.setVolume(vol);
        }
#endif

        break;

      case '1':
#ifdef SHADOW_DEBUG
        output += "Sound Button ";
        output += soundCommand;
        output += " - Play Scream\r\n";
#endif

  do { 
    playSound = random(0,3);
  } while (playSound == lastSound);

        #ifdef SHADOW_DEBUG
          output += "Play Sound ";
          output += playSound;
          output += "\r\n";
          output += "LastSound ";
          output += lastSound;
          output += "\r\n";
        #endif
    
    #ifdef MDFly
        MP3.play(AlertSoundsGroup[playSound]);
    #endif

    #ifdef Sparkfun
        MP3.trigger(AlertSoundsGroup[playSound]);
    #endif

  lastSound = playSound;

      break;
      case '2':
#ifdef SHADOW_DEBUG
        output += "Sound Button ";
        output += soundCommand;
        output += " - Play Doo Doo.\r\n";
#endif

        do { 
          playSound = random(0,3);
        } while (playSound == lastSound);

        #ifdef SHADOW_DEBUG
          output += "Play Sound ";
          output += playSound;
          output += "\r\n";
          output += "LastSound ";
          output += lastSound;
          output += "\r\n";
        #endif

        #ifdef MDFly
          MP3.play(DoDoSoundsGroup[playSound]);
        #endif

        #ifdef Sparkfun
          MP3.trigger(DoDoSoundsGroup[playSound]);
        #endif
        
        lastSound = playSound;

        break;
      case '3':
#ifdef SHADOW_DEBUG
        output += "Sound Button ";
        output += soundCommand;
        output += " - Play Scramble\r\n";
#endif
        do { 
          playSound = random(0,3);
        } while (playSound == lastSound);

        #ifdef SHADOW_DEBUG
          output += "Play Sound ";
          output += playSound;
          output += "\r\n";
          output += "LastSound ";
          output += lastSound;
          output += "\r\n";
        #endif

        #ifdef MDFly
          MP3.play(ScrambleSoundsGroup[playSound]);
        #endif

        #ifdef Sparkfun
          MP3.trigger(ScrambleSoundsGroup[playSound]);
        #endif
        
        lastSound = playSound;

        break;
        case '4':
#ifdef SHADOW_DEBUG
        output += "Sound Button ";
        output += soundCommand;
        output += " - Play Horns\r\n";
#endif
        do { 
          playSound = random(0,4);
        } while (playSound == lastSound);

        #ifdef SHADOW_DEBUG
          output += "Play Sound ";
          output += playSound;
          output += "\r\n";
          output += "LastSound ";
          output += lastSound;
          output += "\r\n";
        #endif

        #ifdef MDFly
          MP3.play(HornSoundsGroup[playSound]);
        #endif

        #ifdef Sparkfun
          MP3.trigger(HornSoundsGroup[playSound]);
        #endif
        
        lastSound = playSound;

        break;
      case '5':
#ifdef SHADOW_DEBUG
        output += "Sound Button ";
        output += soundCommand;
        output += " - Play Zap Sound.\r\n";
#endif

        do { 
          playSound = random(0,2);
        } while (playSound == lastSound);

        #ifdef SHADOW_DEBUG
          output += "Play Sound ";
          output += playSound;
          output += "\r\n";
          output += "LastSound ";
          output += lastSound;
          output += "\r\n";
        #endif

        #ifdef MDFly
          MP3.play(ZapSoundsGroup[playSound]);
        #endif

        #ifdef Sparkfun
          MP3.trigger(ZapSoundsGroup[playSound]);
        #endif
        
        lastSound = playSound;

        break;
      case '6':
#ifdef SHADOW_DEBUG
        output += "Sound Button ";
        output += soundCommand;
        output += " - Play Droid Sound.\r\n";
#endif

        do { 
          playSound = random(0,5);
        } while (playSound == lastSound);

        #ifdef SHADOW_DEBUG
          output += "Play Sound ";
          output += playSound;
          output += "\r\n";
          output += "LastSound ";
          output += lastSound;
          output += "\r\n";
        #endif

        #ifdef MDFly
          MP3.play(DroidSoundsGroup[playSound]);
        #endif

        #ifdef Sparkfun
          MP3.trigger(DroidSoundsGroup[playSound]);
        #endif
        
        lastSound = playSound;
        break;
        
      case '7':
      #ifdef SHADOW_DEBUG
        output += "Sound Button ";
        output += soundCommand;
        output += " - Play Imperial March.\r\n";
      #endif

      #ifdef MDFly
        MP3.play(imperialMarchSong);
      #endif

      #ifdef Sparkfun
        MP3.trigger(imperialMarchSong);
      #endif

        break;
      case '8':
      #ifdef SHADOW_DEBUG
        output += "Sound Button ";
        output += soundCommand;
        output += " - Play Cantina Song.\r\n";
      #endif

      #ifdef MDFly
        MP3.play(cantinaSong);
      #endif

      #ifdef Sparkfun
        MP3.trigger(cantinaSong);
      #endif

        break;
      case '9':
      #ifdef SHADOW_DEBUG
        output += "Sound Button ";
        output += soundCommand;
        output += " - Play SW Theme Song.\r\n";
      #endif

      #ifdef MDFly
        MP3.play(SWSong);
      #endif

      #ifdef Sparkfun
        MP3.trigger(SWSong);
      #endif

        break;
      case '0':
      #ifdef SHADOW_DEBUG
        output += "Sound Button ";
        output += soundCommand;
        output += " - Play MSE Song.\r\n";
      #endif

      #ifdef MDFly
        MP3.play(MSESong);
      #endif

      #ifdef Sparkfun
        MP3.trigger(MSESong);
      #endif

        break;
      case 'A':

        break;
      case 'B':

        break;
      case 'C':
        #ifdef SHADOW_DEBUG
      output += "Volume Max\r\n";
      #endif
      
      #ifdef MDFly
              vol = 30;
              MP3.setVol(vol);
      #endif
      
      #ifdef Sparkfun
              vol = 255;
              MP3.setVolume(vol);
      #endif

        break;
      case 'D':
      #ifdef SHADOW_DEBUG
      output += "Volume Mid\r\n";
      #endif
      
      #ifdef MDFly
              vol = 16;
              MP3.setVol(vol);
      #endif
      
      #ifdef Sparkfun
              vol = 127;
              MP3.setVolume(vol);
      #endif
        break;
      case 'E':

        break;
      case 'F':

        break;
      case 'R':
#ifdef SHADOW_DEBUG
        output += "Sound Button ";
        output += soundCommand;
        output += " - Play Random Mouse Sound. L3\r\n";
#endif

    do {
      soundType = random(1,4);
    } while (soundType == lastSoundType);

    if (soundType == 1) {
      playSound = random(0,3);
      #ifdef MDFly
        MP3.play(DoDoSoundsGroup[playSound]);
      #endif
      #ifdef Sparkfun
        MP3.trigger(DoDoSoundsGroup[playSound]);
      #endif
      
    } else if (soundType == 2) {
      playSound = random(0,3);
      #ifdef MDFly
        MP3.play(ScrambleSoundsGroup[playSound]);
      #endif
      #ifdef Sparkfun
        MP3.trigger(ScrambleSoundsGroup[playSound]);
      #endif
      
    } else if (soundType == 3) {
       playSound = random(0,4);
      #ifdef MDFly
        MP3.play(HornSoundsGroup[playSound]);
      #endif
      #ifdef Sparkfun
        MP3.trigger(HornSoundsGroup[playSound]);
      #endif
      
    } else if (soundType == 4) {
      playSound = random(0,5);
      #ifdef MDFly
        MP3.play(DroidSoundsGroup[playSound]);
      #endif
      #ifdef Sparkfun
        MP3.trigger(DroidSoundsGroup[playSound]);
      #endif
    }

    lastSoundType = soundType;

        break;
      default:
#ifdef SHADOW_DEBUG
        output += "Invalid Sound Command\r\n";
#endif

        output += "Invalid Sound Command\r\n";

#ifdef MDFly
        MP3.play(60);
#endif

#ifdef Sparkfun
        MP3.trigger(60);
#endif
      }
  }

  void ps3soundControl(PS3BT* myPS3 = PS3Nav, int controllerNumber = 1)
  {
    if (!(myPS3->getButtonPress(L1) || myPS3->getButtonPress(L2) || myPS3->getButtonPress(PS))) {
      if (myPS3->getButtonClick(UP))          processSoundCommand('2'); // DoDos
      else if (myPS3->getButtonClick(RIGHT))  processSoundCommand('3'); // Scrambles
      else if (myPS3->getButtonClick(DOWN))   processSoundCommand('4'); // Horns
      else if (myPS3->getButtonClick(LEFT))   processSoundCommand('6'); // Droids
      else if (myPS3->getButtonClick(CROSS))  processSoundCommand('5'); // Zaps
      else if (myPS3->getButtonClick(CIRCLE)) processSoundCommand('1'); // Scream
      else if (myPS3->getButtonClick(L3))     processSoundCommand('R'); // Random
    }
    else if (myPS3->getButtonPress(L1)) {
      if (myPS3->getButtonClick(UP))          processSoundCommand('7'); // Imperial March
      else if (myPS3->getButtonClick(RIGHT))  processSoundCommand('8'); // Cantina
      else if (myPS3->getButtonClick(DOWN))   processSoundCommand('9'); // Star Wars Theme
      else if (myPS3->getButtonClick(LEFT))   processSoundCommand('0'); // MSE Droid Song
      else if (myPS3->getButtonClick(CIRCLE)) processSoundCommand('A');
      else if (myPS3->getButtonClick(CROSS))  processSoundCommand('B');
      else if (myPS3->getButtonClick(L3))     processSoundCommand('R'); 
    }
    else if (myPS3->getButtonPress(PS)) {
      if (myPS3->getButtonClick(UP))          processSoundCommand('+'); // volume up
      else if (myPS3->getButtonClick(DOWN))   processSoundCommand('-'); // volume down
      else if (myPS3->getButtonClick(LEFT))   processSoundCommand('D'); // volume max
      else if (myPS3->getButtonClick(RIGHT))  processSoundCommand('C'); // volume mid
      else if (myPS3->getButtonClick(CIRCLE)) processSoundCommand('E'); // Enable Drive
      else if (myPS3->getButtonClick(CROSS))  processSoundCommand('F'); // Disable Drive
      else if (myPS3->getButtonClick(L3))     processSoundCommand('R');
    }
    else if (myPS3->getButtonPress(L2)) {
      if (myPS3->getButtonClick(UP))          processSoundCommand('7');
      else if (myPS3->getButtonClick(RIGHT))  processSoundCommand('8');
      else if (myPS3->getButtonClick(DOWN))   processSoundCommand('9');
      else if (myPS3->getButtonClick(LEFT))   processSoundCommand('0');
      else if (myPS3->getButtonClick(CIRCLE)) processSoundCommand('A');
      else if (myPS3->getButtonClick(CROSS))  processSoundCommand('B');
      else if (myPS3->getButtonClick(L3))     processSoundCommand('R');
    }
  }

  void Drive()
  {
    //Flood control prevention
    if ((millis() - previousMillis) < serialLatency) return;
    if (PS3Nav->PS3NavigationConnected) ps3Drive(PS3Nav);
    //TODO:  Drive control must be mutually exclusive - for safety
    //Future: I'm not ready to test that before FanExpo
    //if (PS3Nav2->PS3NavigationConnected) ps3Drive(PS3Nav2);
  }

  void toggleSettings()
  {
    if (PS3Nav->PS3NavigationConnected) ps3ToggleSettings(PS3Nav);
    if (PS3Nav2->PS3NavigationConnected) ps3ToggleSettings(PS3Nav2);
  }

  void soundControl()
  {
    if (PS3Nav->PS3NavigationConnected) ps3soundControl(PS3Nav, 1);
    if (PS3Nav2->PS3NavigationConnected) ps3soundControl(PS3Nav2, 2);
  }

