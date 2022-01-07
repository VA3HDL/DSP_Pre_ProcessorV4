/* TODO
    Tone generator
    Soft reboot
*/
#include <SD.h>
#include <SPI.h>
#include <Audio.h>
#include <Wire.h>
#include <SerialFlash.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include <TimerOne.h>
#include <ClickEncoder.h>
#include <menu.h>
#include <menuIO/SSD1306AsciiOut.h>
#include <menuIO/serialIO.h>
#include <menuIO/clickEncoderIn.h>
#include <menuIO/chainStream.h>

#include "effect_noisegate.h"

File myFile;
int myPreset = 0;
const int chipSelect = 10;
char charRead;

constexpr int OLED_SDC = 5; //5
constexpr int OLED_SDA = 4; //4
#define I2C_ADDRESS 0x3C

using namespace Menu;

//Define your font here. Default font: lcd5x7
#define menuFont X11fixed7x14
#define fontW 7
#define fontH 14
//#define OPTIMIZE_I2C 1

SSD1306AsciiWire oled;

// Encoder /////////////////////////////////////
#define encA 4
#define encB 3
#define encBtn 5
ClickEncoder clickEncoder(encA, encB, encBtn, 2);
ClickEncoderStream encStream(clickEncoder, 1);
MENU_INPUTS(in, &encStream);
void timerIsr() {
  clickEncoder.service();
}
////////////////////////////////////////////////
/*
  AudioNoiseGate           NoiseGate;
  AudioConnection          patchCord2(audioInput, 0, NoiseGate, 0);
  AudioConnection          patchCord11(NoiseGate, 0, CompMix, 0);
*/

AudioControlSGTL5000     audioShield;
AudioInputI2S            audioInput;
AudioOutputI2S           audioOutput;  // audio shield: headphones & line-out
AudioAnalyzePeak         peakPre;
AudioAnalyzePeak         peakPost;
AudioAnalyzeFFT256       fftValues;
AudioNoiseGate           NoiseGate;
AudioFilterBiquad        EQ_1;
AudioFilterBiquad        EQ_3;
AudioFilterBiquad        EQ_2;
AudioFilterBiquad        EQ_4;
AudioFilterBiquad        EQ_5;
AudioFilterBiquad        EQ_6;
AudioFilterBiquad        EQ_7;
AudioFilterBiquad        EQ_8;
AudioMixer4              CompMix;
AudioMixer4              M4;
AudioMixer4              EQ_Mix1;
AudioMixer4              EQ_Mix2;
AudioMixer4              EQ_Mix3;
AudioConnection          patchCordc1(audioInput, 0, peakPre, 0);
// AudioConnection          patchCordc2(audioInput, 0, CompMix, 0);
AudioConnection          patchCordc2(audioInput, 0, NoiseGate, 0);
AudioConnection          patchCordc3(NoiseGate, 0, CompMix, 0);
AudioConnection          patchCordc4(audioInput, 0, M4, 1);
AudioConnection          patchCord1(CompMix, 0, EQ_1, 0);
AudioConnection          patchCord2(CompMix, 0, EQ_2, 0);
AudioConnection          patchCord3(CompMix, 0, EQ_3, 0);
AudioConnection          patchCord4(CompMix, 0, EQ_4, 0);
AudioConnection          patchCord5(CompMix, 0, EQ_5, 0);
AudioConnection          patchCord6(CompMix, 0, EQ_6, 0);
AudioConnection          patchCord7(CompMix, 0, EQ_7, 0);
AudioConnection          patchCord8(CompMix, 0, EQ_8, 0);
AudioConnection          patchCord9(EQ_1, 0, EQ_Mix1, 0);
AudioConnection          patchCord10(EQ_3, 0, EQ_Mix1, 2);
AudioConnection          patchCord11(EQ_2, 0, EQ_Mix1, 1);
AudioConnection          patchCord12(EQ_4, 0, EQ_Mix1, 3);
AudioConnection          patchCord13(EQ_5, 0, EQ_Mix2, 0);
AudioConnection          patchCord14(EQ_6, 0, EQ_Mix2, 1);
AudioConnection          patchCord15(EQ_7, 0, EQ_Mix2, 2);
AudioConnection          patchCord16(EQ_8, 0, EQ_Mix2, 3);
AudioConnection          patchCord17(EQ_Mix2, 0, EQ_Mix3, 1);
AudioConnection          patchCord18(EQ_Mix1, 0, EQ_Mix3, 0);
AudioConnection          patchCord19(EQ_Mix3, 0, M4, 0);
AudioConnection          patchCord30(M4, fftValues);
AudioConnection          patchCord31(M4, peakPost);
AudioConnection          patchCordc100(M4, 0, audioOutput, 0); // Left Channel
AudioConnection          patchCordc101(M4, 0, audioOutput, 1); // Right Channel

int b;
float bandGain[] = {1, 1, 1, 1, 1, 1, 1, 1};
float ydBLevel[] = {0, 0, 0, 0, 0, 0, 0, 0};
int eqFreq[] = {150, 240, 370, 590, 900, 1300, 2000, 3300};
int freqBand;
float compGain = 1;
int compressorFlag = 1;
int equalizerFlag = 1;
int spectrumFlag = 0;
int noiseGateFlag = 0;
int myLineInLevel = 0;    // range is 0 to 15
int myLineOutLevel = 15;  // range is 13 to 31
float myVolume = 0.7;     // 0.8 is max for undistorted headphone output
float micGainSet = 35;    // 35 for the Chinese MH-1B8 mic
int maxVal = 0;
int myInput = AUDIO_INPUT_MIC;

// Default Noise Gate parameters
float myAttackTime = 0.01;
float myReleaseTime = 0.2;
float myThreshold = 35.0;
float myHoldTime = 0.0003;

const uint8_t fftOctTab[] = {
  1,  1,
  2,  2,
  3,  3,
  4,  4,
  5,  5,
  6,  7,
  8,  9,
  10, 11,
  12, 14,
  14, 16,
  17, 20,
  21, 25,
  26, 31,
  32, 38,
  39, 46,
  47, 57,
  58, 69,
  70, 85,
  86, 127
};

// Setting the screen driver for the Spectrum Display
int SCREEN_WIDTH   = 128; // OLED display width, in pixels
int SCREEN_HEIGHT  = 64; // OLED display height, in pixels
int OLED_RESET     = -1; // Reset pin # (or -1 if sharing Arduino reset pin)
int SCREEN_ADDRESS = 0x3C; ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// result doAlert(eventMask e, prompt &item);

result alert(menuOut& o, idleEvent e) {
  if (e == idling) {
    o.setCursor(0, 0);
    o.print("alert test");
    o.setCursor(0, 1);
    o.print("press [select]");
    o.setCursor(0, 2);
    o.print("to continue...");
  }
  return proceed;
}

//Equalizer on
result eqON(eventMask e) {
  M4.gain(0, 1);
  M4.gain(1, 0);
  M4.gain(2, 0);
  M4.gain(3, 0);
  Serial.println(""); Serial.print(e); Serial.println(" eqON executed, proceed menu"); Serial.flush();
  return proceed;
}

//Equalizer off
result eqOFF(eventMask e) {
  M4.gain(0, 0);
  M4.gain(1, 1);
  M4.gain(2, 0);
  M4.gain(3, 0);
  Serial.println(""); Serial.print(e); Serial.println(" eqOFF executed, proceed menu"); Serial.flush();
  return proceed;
}

//Compressor on
/* Valid values for dap_avc parameters
  maxGain; Maximum gain that can be applied
  0 - 0 dB
  1 - 6 dB
  2 - 12 dB
  lbiResponse; Integrator Response
  0 - 0 mS
  1 - 25 mS
  2 - 50 mS
  3 - 100 mS
  hardLimit
  0 - Hard limit disabled. AVC Compressor/Expander enabled.
  1 - Hard limit enabled. The signal is limited to the programmed threshold (signal saturates at the threshold)
  threshold
  floating point in range 0 to -96 dB
  attack
  floating point figure is dB/s rate at which gain is increased
  decay
  floating point figure is dB/s rate at which gain is reduced
*/
int myCompGain = 1;
int myCompResp = 1;
int myCompHard = 0;
float myCompThr = -18;
float myCompAtt = 0.5;
float myCompDec = 1.0;

result compON(eventMask e) {
  compressorFlag = 1;
  audioShield.autoVolumeControl( myCompGain    // Maximum gain that can be applied 0 - 0 dB / 1 - 6.0 dB / 2 - 12 dB
                                 , myCompResp  // Integrator Response 0 - 0 mS / 1 - 25 mS / 2 - 50 mS / 3 - 100 mS
                                 , myCompHard  // hardLimit
                                 , myCompThr   // threshold floating point in range 0 to -96 dB
                                 , myCompAtt   // attack floating point figure is dB/s rate at which gain is increased
                                 , myCompDec); // decay floating point figure is dB/s rate at which gain is reduced
  audioShield.autoVolumeEnable();
  Serial.println(""); Serial.print(e); Serial.println(" compON executed, proceed menu"); Serial.flush();
  return proceed;
}

//Compressor off
result compOFF(eventMask e) {
  compressorFlag = 0;
  audioShield.autoVolumeDisable();
  Serial.println(""); Serial.print(e); Serial.println(" compOFF executed, proceed menu"); Serial.flush();
  return proceed;
}

MENU(subLevels, "Volume Levels", showEvent, anyEvent, wrapStyle
     , EXIT(" <- Back")
     , FIELD(       myVolume, "Headphone", " ",  0,  1, 0.1, 0.01, SetLevels, updateEvent, noStyle)
     , FIELD(  myLineInLevel,   "Line In", " ",  0, 15,   1,     , SetLevels, updateEvent, noStyle)
     , FIELD( myLineOutLevel,  "Line Out", " ", 13, 31,   1,     , SetLevels, updateEvent, noStyle)
    );

TOGGLE(myCompGain, chooseCompGain, "Comp. Gain: ", doNothing, noEvent, wrapStyle
       , VALUE( "0 dB", 0, SetCompressorParameters, updateEvent)
       , VALUE( "6 dB", 1, SetCompressorParameters, updateEvent)
       , VALUE("12 dB", 2, SetCompressorParameters, updateEvent)
      );

TOGGLE(myCompResp, chooseCompResp, "Response: ", doNothing, noEvent, wrapStyle
       , VALUE(  "0 ms", 0, SetCompressorParameters, updateEvent)
       , VALUE( "25 ms", 1, SetCompressorParameters, updateEvent)
       , VALUE( "50 ms", 2, SetCompressorParameters, updateEvent)
       , VALUE("100 ms", 3, SetCompressorParameters, updateEvent)
      );

TOGGLE(myCompHard, setHardLimit, "Hard Limit: ", doNothing, noEvent, wrapStyle
       , VALUE("On", 1, SetCompressorParameters, updateEvent)
       , VALUE("Off", 0, SetCompressorParameters, updateEvent)
      );

MENU(subComp, "Compressor cfg", showEvent, anyEvent, wrapStyle
     , EXIT(" <- Back")
     , SUBMENU(chooseCompGain)
     , SUBMENU(chooseCompResp)
     , SUBMENU(setHardLimit)
     , FIELD(  myCompThr, "Thresh.", " dB", -96, 0, 1, 0.1, SetCompressorParameters, updateEvent, noStyle)
     , FIELD(  myCompAtt, "Attack", " dB/s", 0, 10, 1, 0.1, SetCompressorParameters, updateEvent, noStyle)
     , FIELD(  myCompDec, "Decay", " dB/s", 0, 10, 1, 0.1, SetCompressorParameters, updateEvent, noStyle)
     , FIELD( micGainSet, "Mic.Gain", " dB", 0, 63, 1, , SetCompressorParameters, updateEvent, noStyle)
    );

TOGGLE(equalizerFlag, setEQ, "Equalizer: ", doNothing, noEvent, wrapStyle
       , VALUE("On", 1, eqON, noEvent)
       , VALUE("Off", 0, eqOFF, noEvent)
      );

TOGGLE(compressorFlag, setComp, "Compressor: ", doNothing, noEvent, wrapStyle
       , VALUE("On", 1, compON, noEvent)
       , VALUE("Off", 0, compOFF, noEvent)
      );

TOGGLE(myInput, selMenu, "Input: ", doNothing, noEvent, wrapStyle
       , VALUE("Mic", AUDIO_INPUT_MIC, SetInput, enterEvent)
       , VALUE("Line", AUDIO_INPUT_LINEIN, SetInput, enterEvent)
      );

MENU(subEQ, "Equalizer cfg", showEvent, anyEvent, wrapStyle
     , EXIT(" <- Back")
     , FIELD(ydBLevel[0], " 150 Hz", " db", -12, 12, 1, 0.1, EqGainSetL, updateEvent, noStyle)
     , FIELD(ydBLevel[1], " 240 Hz", " db", -12, 12, 1, 0.1, EqGainSetL, updateEvent, noStyle)
     , FIELD(ydBLevel[2], " 370 Hz", " db", -12, 12, 1, 0.1, EqGainSetL, updateEvent, noStyle)
     , FIELD(ydBLevel[3], " 600 Hz", " db", -12, 12, 1, 0.1, EqGainSetL, updateEvent, noStyle)
     , FIELD(ydBLevel[4], " 900 Hz", " db", -12, 12, 1, 0.1, EqGainSetL, updateEvent, noStyle)
     , FIELD(ydBLevel[5], "1.3 KHz", " db", -12, 12, 1, 0.1, EqGainSetL, updateEvent, noStyle)
     , FIELD(ydBLevel[6], "2.0 KHz", " db", -12, 12, 1, 0.1, EqGainSetL, updateEvent, noStyle)
     , FIELD(ydBLevel[7], "3.3 KHz", " db", -12, 12, 1, 0.1, EqGainSetL, updateEvent, noStyle)
    );

// Configuration https://github.com/neu-rah/ArduinoMenu/wiki/Menu-definition

result toggleAudioSpectrum(eventMask e) {
  if (spectrumFlag == 0) {
    spectrumFlag = 1;
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.setCursor(0, 0);
    display.cp437(true);
    display.write("-80dB.Peak meter.0dB");
    display.setCursor(0, 16);
    display.print(" 0");
    display.setCursor(0, 26);
    display.print("-2");
    display.setCursor(0, 36);
    display.print("-4");
    display.setCursor(0, 46);
    display.print("-6");
    display.setCursor(0, 56);
    display.print("-8");
    display.display();

    Serial.println();
    Serial.println("Audio Spectrum ON");
  }
  else {
    spectrumFlag = 0;
    Serial.println();
    Serial.println("Audio Spectrum OFF");
  }
  return proceed;
}

result ngON(eventMask e) {
  NoiseGate.enable(myAttackTime, myReleaseTime, myThreshold, myHoldTime);
  noiseGateFlag = 1;
  Serial.println();
  Serial.print(" myAttackTime: "); Serial.print(myAttackTime); Serial.print(" myReleaseTime: "); Serial.print(myReleaseTime); Serial.print(" myThreshold: "); Serial.print(myThreshold); Serial.print(" myHoldTime: "); Serial.print(myHoldTime, 4);
  Serial.println();
  Serial.println("Noise Gate ON");
  return proceed;
}

result ngOFF(eventMask e) {
  NoiseGate.disable();
  noiseGateFlag = 0;
  Serial.println();
  Serial.println("Noise Gate OFF");
  return proceed;
}

MENU(subNG, "Noise Gate cfg", showEvent, anyEvent, wrapStyle
     , EXIT(" <- Back")
     , altFIELD(decPlaces<3>::menuField,  myAttackTime,  "Attack", "", 0.0, 1.0, 0.01, 0.001, ngON, updateEvent, noStyle)
     , altFIELD(decPlaces<3>::menuField, myReleaseTime, "Release", "", 0.0, 1.0, 0.01, 0.001, ngON, updateEvent, noStyle)
     , FIELD( myThreshold, "Thresh.", "", -40.0, 50.0, 1, 0.1, ngON, updateEvent, noStyle)
     , altFIELD(decPlaces<4>::menuField,myHoldTime,"Hold","",0.0001, 0.01, 0.001, 0.0001, ngON, updateEvent, noStyle)
    );

TOGGLE(noiseGateFlag, setNG, "Noise Gate: ", doNothing, noEvent, wrapStyle
       , VALUE("On", 1, ngON, noEvent)
       , VALUE("Off", 0, ngOFF, noEvent)
      );

MENU(sdCard, "SD Card", doNothing, noEvent, wrapStyle
     , FIELD(myPreset, "Select preset", "", 0, 9, 1, , doNothing, noEvent, wrapStyle)
     , OP("Read preset", readFromFile, enterEvent)
     , OP("Save preset",  writeToFile, enterEvent)
     , OP("Del. preset",   deleteFile, enterEvent)
     , EXIT(" <- Back")
    );

MENU(mainMenu, "Main menu", doNothing, noEvent, wrapStyle
     , OP("Spectrum disp", toggleAudioSpectrum, enterEvent)
     , SUBMENU(subLevels)
     , SUBMENU(subNG)
     , SUBMENU(setNG)
     , SUBMENU(subEQ)
     , SUBMENU(setEQ)
     , SUBMENU(subComp)
     , SUBMENU(setComp)
     , SUBMENU(selMenu)
     , SUBMENU(sdCard)
     //, OP("Reboot", , enterEvent)
    );

result showEvent(eventMask e, navNode & nav, prompt & item) {
  Serial.println();
  Serial.print("event: ");
  Serial.print(e);
  return proceed;
}

/*
  MENU(mainMenu, "Main menu", doNothing, noEvent, wrapStyle
     , OP("Op1", action1, anyEvent)
     , OP("Op2", action2, enterEvent)
     , FIELD(test, "Test", "%", 0, 100, 10, 1, doNothing, noEvent, wrapStyle)
     , SUBMENU(subMenu)
     , SUBMENU(setLed)
     , OP("LED On", myLedOn, enterEvent)
     , OP("LED Off", myLedOff, enterEvent)
     , SUBMENU(selMenu)
     , SUBMENU(chooseMenu)
     , OP("Alert test", doAlert, enterEvent)
     , EXIT("<Back")
    );
*/

#define MAX_DEPTH 2

//define output device
idx_t serialTops[MAX_DEPTH] = {0};
serialOut outSerial(Serial, serialTops);

//describing a menu output device without macros
//define at least one panel for menu output
const panel panels[] MEMMODE = {{0, 0, 128 / fontW, 64 / fontH}};
navNode* nodes[sizeof(panels) / sizeof(panel)];                         //navNodes to store navigation status
panelsList pList(panels, nodes, 1);                                     //a list of panels and nodes
idx_t tops[MAX_DEPTH] = {0, 0};                                         //store cursor positions for each level
SSD1306AsciiOut outOLED(&oled, tops, pList, 8, 1 + ((fontH - 1) >> 3) ); //oled output device menu driver
menuOut* constMEM outputs[] MEMMODE = {&outOLED, &outSerial};           //list of output devices
outputsList out(outputs, sizeof(outputs) / sizeof(menuOut*));           //outputs list

//macro to create navigation control root object (nav) using mainMenu
NAVROOT(nav, mainMenu, MAX_DEPTH, in, out);

result doAlert(eventMask e, prompt & item) {
  nav.idleOn(alert);
  return proceed;
}

//when menu is suspended
result idle(menuOut & o, idleEvent e) {
  o.clear();
  if (&o == &outOLED) {
    if (e == idling) {
      o.println("OLED");
      o.println("Suspended menu");
    }
  } else
    switch (e) {
      case idleStart: o.println("suspending menu!"); break;
      case idling: o.println("suspended..."); break;
      case idleEnd: o.println("resuming menu."); break;
    }
  return proceed;
}

void MyDelay(unsigned long ms)
{
  unsigned long currentMillis = millis();
  unsigned long previousMillis = millis();

  while (currentMillis - previousMillis < ms) {
    currentMillis = millis();
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void setup(void) {
  pinMode(2, INPUT);
  pinMode(encBtn, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  Serial.println("DSP Pre-Processor booting");
  Serial.flush();

  Wire.begin();
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(menuFont);

  oled.clear();
  oled.setCursor(0, 0);
  oled.println("VA3HDL - Aurora,ON");
  oled.println("Version 2.0 NG");

  Timer1.initialize(500);
  Timer1.attachInterrupt(timerIsr);

  AudioMemory(100);
  audioShield.enable();

  Serial.println("SD Card active");
  oled.println("SD Card active");

  if (SD.begin(chipSelect))
  {
    Serial.println("SD card is present");
    oled.println("SD card is present");
    Serial.println("Reading settings");
    oled.println("Reading settings");
    readFromFile();
  }
  else
  {
    Serial.println("SD card missing or failure");
    oled.println("SD card missing or failure");
    while (1); //wait here forever
  }

  delay(2000);
  oled.clear();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.clearDisplay();

  nav.idleTask = idle; //point a function to be used when menu is suspended
  nav.useUpdateEvent = true;
}

elapsedMillis fps;
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void loop()
{
  nav.poll();
  if (spectrumFlag == 1) {
    displayAudioSpectrum();
    b = clickEncoder.getButton();
    if (b == ClickEncoder::Clicked) {
      spectrumFlag = 0;
      display.clearDisplay();
      display.display();
      nav.refresh();
      Serial.println();
      Serial.println("Exiting Spectrum");
    }
  }
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void SetAudioShield() {
  audioShield.muteHeadphone();
  audioShield.muteLineout();

  audioShield.inputSelect(myInput);
  audioShield.micGain(micGainSet);

  audioShield.lineInLevel(myLineInLevel);
  audioShield.lineOutLevel(myLineOutLevel);

  audioShield.volume(myVolume);
  audioShield.adcHighPassFilterEnable();
  audioShield.dacVolume(1);
  audioShield.dacVolumeRamp();

  audioShield.audioPreProcessorEnable();
  audioShield.audioPostProcessorEnable();

  CompMix.gain(0, 1);

  if (equalizerFlag == 1) {
    M4.gain(0, 1);  // EQ enabled
    M4.gain(1, 0);  // Passthrough disabled
  }
  else
  {
    M4.gain(0, 0);  // EQ disabled
    M4.gain(1, 1);  // Passthrough enabled
  }
  M4.gain(2, 0);
  M4.gain(3, 0);

  SetupFilters();               // Setup Equalizer bands
  EqGainSetL();                 // Setup Equalizer levels
  SetCompressorParameters();     // Setup Compressor parameters

  if (noiseGateFlag == 1)
    NoiseGate.enable(myAttackTime, myReleaseTime, myThreshold, myHoldTime);

  audioShield.unmuteHeadphone();
  audioShield.unmuteLineout();
}

void SetCompressorParameters() {
  Serial.println();
  Serial.print(" Comp Gain: "); Serial.print(myCompGain); Serial.print(" Comp Resp: "); Serial.print(myCompResp); Serial.print(" Hardlimit: "); Serial.print(myCompHard);
  Serial.print(" Threshold: "); Serial.print( myCompThr); Serial.print(" Attack: "); Serial.print(myCompAtt); Serial.print(" Decay: "); Serial.print(myCompDec);
  Serial.print(" Mic Gain: "); Serial.print(micGainSet);

  if (compressorFlag == 1) {
    audioShield.autoVolumeControl( myCompGain    // Maximum gain that can be applied 0 - 0 dB / 1 - 6.0 dB / 2 - 12 dB
                                   , myCompResp  // Integrator Response 0 - 0 mS / 1 - 25 mS / 2 - 50 mS / 3 - 100 mS
                                   , myCompHard  // hardLimit
                                   , myCompThr   // threshold floating point in range 0 to -96 dB
                                   , myCompAtt   // attack floating point figure is dB/s rate at which gain is increased
                                   , myCompDec); // decay floating point figure is dB/s rate at which gain is reduced
    audioShield.autoVolumeEnable();
  }

  audioShield.micGain(micGainSet);
}

void SetLevels() {
  Serial.println();
  Serial.print(" Headphone: "); Serial.print(myVolume); Serial.print(" Line In Level: "); Serial.print(myLineInLevel); Serial.print(" Line Out Level: "); Serial.print(myLineOutLevel);
  audioShield.volume(myVolume);
  audioShield.lineInLevel(myLineInLevel);
  audioShield.lineOutLevel(myLineOutLevel);
}

void SetInput() {
  if (myInput == AUDIO_INPUT_MIC)
  {
    audioShield.lineInLevel(0);
    audioShield.micGain(micGainSet);
  }
  else
  {
    audioShield.lineInLevel(myLineInLevel);
    audioShield.micGain(0);
  }
  SetAudioShield();
  Serial.println(); Serial.print("Input: "); Serial.print(myInput);
}

/*****
  Purpose:  Function to set gains for the 8-Band Receive EQ
  Note: Gain Values in dB +/- max of about 20dB either way
*****/
void EqGainSetL() {
  for (int freqBand = 0; freqBand < 8; freqBand++) {
    bandGain[freqBand] = pow(10, (ydBLevel[freqBand] / 20));
    if (ydBLevel[freqBand] <= -12) {
      bandGain[freqBand] = 0;
    }
    Serial.println();
    Serial.print("Eq. band: "); Serial.print(freqBand); Serial.print(" dB: "); Serial.print(ydBLevel[freqBand]); Serial.print(" Gain: "); Serial.print(bandGain[freqBand]);
  }

  EQ_Mix1.gain(0, bandGain[0]);     //  140
  EQ_Mix1.gain(1, bandGain[1]);     //  240
  EQ_Mix1.gain(2, bandGain[2]);     //  370
  EQ_Mix1.gain(3, bandGain[3]);     //  590

  EQ_Mix2.gain(0, bandGain[4]);     //  900
  EQ_Mix2.gain(1, bandGain[5]);     //  1300
  EQ_Mix2.gain(2, bandGain[6]);     //  2000
  EQ_Mix2.gain(3, bandGain[7]);     //  3300

  EQ_Mix3.gain(0, 1);  // Combine Mixers
  EQ_Mix3.gain(1, 1);
  EQ_Mix3.gain(2, 0);
}

/*****
  Purpose:  Function to set Filter Parameters
  Return value:  void

  Note: Gain Values in dB +/- max of about 20dB either way  8-14-18
*****/
void SetupFilters() {
  eqFreq[0] = 150;
  eqFreq[1] = 240;
  eqFreq[2] = 370;
  eqFreq[3] = 590;
  eqFreq[4] = 900;
  eqFreq[5] = 1300;
  eqFreq[6] = 2000;
  eqFreq[7] = 3300;
  EQ_1.setBandpass(0, eqFreq[0], .9);  // Band 1 140Hz, Q =.9
  EQ_1.setBandpass(1, eqFreq[0], .9);
  EQ_1.setBandpass(2, eqFreq[0], .9);
  EQ_1.setBandpass(3, eqFreq[0], .9);

  EQ_2.setBandpass(0, eqFreq[1], .9);  // Band 2 240Hz
  EQ_2.setBandpass(1, eqFreq[1], .9);
  EQ_2.setBandpass(2, eqFreq[1], .9);
  EQ_2.setBandpass(3, eqFreq[1], .9);

  EQ_3.setBandpass(0, eqFreq[2], .9);  // Band 3 370Hz
  EQ_3.setBandpass(1, eqFreq[2], .9);
  EQ_3.setBandpass(2, eqFreq[2], .9);
  EQ_3.setBandpass(3, eqFreq[2], .9);

  EQ_4.setBandpass(0, eqFreq[3], .9);  // Band 4 590Hz
  EQ_4.setBandpass(1, eqFreq[3], .9);
  EQ_4.setBandpass(2, eqFreq[3], .9);
  EQ_4.setBandpass(3, eqFreq[3], .9);

  EQ_5.setBandpass(0, eqFreq[4], .9);  // Band 5 900Hz
  EQ_5.setBandpass(1, eqFreq[4], .9);
  EQ_5.setBandpass(2, eqFreq[4], .9);
  EQ_5.setBandpass(3, eqFreq[4], .9);

  EQ_6.setBandpass(0, eqFreq[5], .9);  // Band 6 1300Hz
  EQ_6.setBandpass(1, eqFreq[5], .9);
  EQ_6.setBandpass(2, eqFreq[5], .9);
  EQ_6.setBandpass(3, eqFreq[5], .9);

  EQ_7.setBandpass(0, eqFreq[6], .9);  // Band 7 2000Hz
  EQ_7.setBandpass(1, eqFreq[6], .9);
  EQ_7.setBandpass(2, eqFreq[6], .9);
  EQ_7.setBandpass(3, eqFreq[6], .9);

  EQ_8.setBandpass(0, eqFreq[7], .9);  // Band 8 3300 Hz
  EQ_8.setBandpass(1, eqFreq[7], .9);
  EQ_8.setBandpass(2, eqFreq[7], .9);
  EQ_8.setBandpass(3, eqFreq[7], .9);
}

void displayAudioSpectrum() {
  const int nBars = sizeof(fftOctTab) / 2 ;
  const int barWidth = 6;
  const int posX = 128 - nBars * barWidth;
  const int posY = 64;
  const int minHeight = 1;
  const int maxHeight = 50;

  static uint16_t bar = 0;

  float n;
  int16_t val;

  float peak;
  int peakM = 0;
  int mVal = 0;

  if (fps > 24) {
    fps = 0;
    if (peakPre.available()) {
      peak = peakPre.read();
      peakM = map(peak, 0.0, 1.0, 0, 128);
      display.drawFastHLine(0, 12, 128, SSD1306_BLACK);
    }
    display.drawFastHLine(0, 12, peakM, SSD1306_WHITE);

    if (peakPost.available()) {
      peak = peakPost.read();
      peakM = map(peak, 0.0, 1.0, 0, 128);
      display.drawFastHLine(0, 15, 128, SSD1306_BLACK);
    }
    display.drawFastHLine(0, 15, peakM, SSD1306_WHITE);

    if (fftValues.available()) {
      n = fftValues.read(fftOctTab[bar * 2], fftOctTab[bar * 2 + 1]);
      val = log10f(n) * 60 + 252;
      if (val > maxVal) maxVal = val;
      mVal = map(val, 0, maxVal, minHeight, maxHeight);
    }

    int x = posX + bar * barWidth;

    display.drawFastVLine(x, 13, 51, SSD1306_BLACK);
    display.drawFastVLine(x, posY - mVal, posY, SSD1306_WHITE);
    //Serial.println();
    //Serial.print(" maxVal: "); Serial.print(maxVal); Serial.print(" bar: "); Serial.print(bar); Serial.print(" val: "); Serial.print(val); Serial.print(" mVal: "); Serial.print(mVal); Serial.print(" x: "); Serial.print(x);
    if (++bar >= nBars) bar = 0;
    display.display();
  }
}

/* VARIABLES to SAVE to SD Card - Default values
  float compGain = 1;
  int compressorFlag = 1;
  int equalizerFlag = 1;
  int spectrumFlag = 0;
  int myLineInLevel = 3;
  int myLineOutLevel = 20;
  float myVolume = 0.8;
  float micGainSet = 50;
  int myCompGain = 1;
  int myCompResp = 1;
  int myCompHard = 0;
  float myCompThr = -5;
  float myCompAtt = 0.5;
  float myCompDec = 0.5;
*/

void readFromFile()
{
  byte i = 0;
  char inputString[100];
  char fileName[13];
  char filePref[] = "settings";
  char fileExtn[] = ".txt";
  sprintf(fileName, "%s%i%s", filePref, myPreset, fileExtn);
  Serial.println();
  // Check to see if the file exists:
  if (!SD.exists(fileName)) {
    Serial.print(fileName);
    Serial.println(" doesn't exist.");
  } else {
    Serial.print("Reading from: "); Serial.println(fileName);
    myFile = SD.open(fileName);
  }
  int line = 0;
  while (myFile.available())
  {
    char inputChar = myFile.read(); // Gets one byte from serial buffer
    if (inputChar == '\n') //end of line (or 10)
    {
      inputString[i] = 0;  //terminate the string correctly
      Serial.print("Input string: "); Serial.print(inputString);
      switch (line) {
        case 0:
          compGain = atof(inputString);
          Serial.print(" Input compGain: "); Serial.println(compGain);
          break;
        case 1:
          compressorFlag = atoi(inputString);
          Serial.print(" Input compressorFlag: "); Serial.println(compressorFlag);
          break;
        case 2:
          equalizerFlag = atoi(inputString);
          Serial.print(" Input equalizerFlag: "); Serial.println(equalizerFlag);
          break;
        case 3:
          myLineInLevel = atoi(inputString);
          Serial.print(" Input myLineInLevel: "); Serial.println(myLineInLevel);
          break;
        case 4:
          myLineOutLevel = atoi(inputString);
          Serial.print(" Input myLineOutLevel: "); Serial.println(myLineOutLevel);
          break;
        case 5:
          myVolume = atof(inputString);
          Serial.print(" Input myVolume: "); Serial.println(myVolume);
          break;
        case 6:
          micGainSet = atof(inputString);
          Serial.print(" Input micGainSet: "); Serial.println(micGainSet);
          break;
        case 7:
          myCompGain = atoi(inputString);
          Serial.print(" Input myCompGain: "); Serial.println(myCompGain);
          break;
        case 8:
          myCompResp = atoi(inputString);
          Serial.print(" Input myCompResp: "); Serial.println(myCompResp);
          break;
        case 9:
          myCompHard = atoi(inputString);
          Serial.print(" Input myCompHard: "); Serial.println(myCompHard);
          break;
        case 10:
          myCompThr = atof(inputString);
          Serial.print(" Input myCompThr: "); Serial.println(myCompThr);
          break;
        case 11:
          myCompAtt = atof(inputString);
          Serial.print(" Input myCompAtt: "); Serial.println(myCompAtt);
          break;
        case 12:
          myCompDec = atof(inputString);
          Serial.print(" Input myCompDec: "); Serial.println(myCompDec);
          break;
        case 13:
          myInput = atoi(inputString);
          Serial.print(" Input myInput: "); Serial.println(myInput);
          break;
        case 14:
          ydBLevel[0] = atof(inputString);
          Serial.print(" Input ydBLevel[0]: "); Serial.println(ydBLevel[0]);
          break;
        case 15:
          ydBLevel[1] = atof(inputString);
          Serial.print(" Input ydBLevel[1]: "); Serial.println(ydBLevel[1]);
          break;
        case 16:
          ydBLevel[2] = atof(inputString);
          Serial.print(" Input ydBLevel[2]: "); Serial.println(ydBLevel[2]);
          break;
        case 17:
          ydBLevel[3] = atof(inputString);
          Serial.print(" Input ydBLevel[3]: "); Serial.println(ydBLevel[3]);
          break;
        case 18:
          ydBLevel[4] = atof(inputString);
          Serial.print(" Input ydBLevel[4]: "); Serial.println(ydBLevel[4]);
          break;
        case 19:
          ydBLevel[5] = atof(inputString);
          Serial.print(" Input ydBLevel[5]: "); Serial.println(ydBLevel[5]);
          break;
        case 20:
          ydBLevel[6] = atof(inputString);
          Serial.print(" Input ydBLevel[6]: "); Serial.println(ydBLevel[6]);
          break;
        case 21:
          ydBLevel[7] = atof(inputString);
          Serial.print(" Input ydBLevel[7]: "); Serial.println(ydBLevel[7]);
          break;
        case 22:
          myAttackTime = atof(inputString);
          Serial.print(" Input myAttackTime: "); Serial.println(myAttackTime);
          break;
        case 23:
          myReleaseTime = atof(inputString);
          Serial.print(" Input myReleaseTime: "); Serial.println(myReleaseTime);
          break;
        case 24:
          myThreshold = atof(inputString);
          Serial.print(" Input myThreshold: "); Serial.println(myThreshold);
          break;
        case 25:
          myHoldTime = atof(inputString);
          Serial.print(" Input myHoldTime: "); Serial.println(myHoldTime, 4);
          break;
        case 26:
          noiseGateFlag = atoi(inputString);
          Serial.print(" Input noiseGateFlag: "); Serial.println(noiseGateFlag);
          break;
      }
      line++;
      i = 0;
    }
    else
    {
      inputString[i] = inputChar; // Store it
      i++; // Increment where to write next
      if (i > sizeof(inputString))
      {
        Serial.println("Incoming string longer than array allows");
        Serial.println(sizeof(inputString));
        while (1);
      }
    }
  }
  oled.clear();
  oled.setCursor(0, 0);
  oled.println("Settings read");
  oled.println(fileName);
  // Apply settings loaded
  Serial.println("Applying settings loaded to the AudioShield");
  SetAudioShield();
}

void writeToFile()
{
  Serial.println();
  char fileName[13];
  char filePref[] = "settings";
  char fileExtn[] = ".txt";
  sprintf(fileName, "%s%i%s", filePref, myPreset, fileExtn);
  deleteFile();
  myFile = SD.open(fileName, FILE_WRITE);
  if (myFile) // it opened OK
  {
    Serial.print("Writing to "); Serial.println(fileName);
    myFile.println(compGain);
    myFile.println(compressorFlag);
    myFile.println(equalizerFlag);
    myFile.println(myLineInLevel);
    myFile.println(myLineOutLevel);
    myFile.println(myVolume);
    myFile.println(micGainSet);
    myFile.println(myCompGain);
    myFile.println(myCompResp);
    myFile.println(myCompHard);
    myFile.println(myCompThr);
    myFile.println(myCompAtt);
    myFile.println(myCompDec);
    myFile.println(myInput);
    myFile.println(ydBLevel[0]);
    myFile.println(ydBLevel[1]);
    myFile.println(ydBLevel[2]);
    myFile.println(ydBLevel[3]);
    myFile.println(ydBLevel[4]);
    myFile.println(ydBLevel[5]);
    myFile.println(ydBLevel[6]);
    myFile.println(ydBLevel[7]);
    myFile.println(myAttackTime);
    myFile.println(myReleaseTime);
    myFile.println(myThreshold);
    myFile.println(myHoldTime, 4);
    myFile.println(noiseGateFlag);
    myFile.close();
    Serial.println("Done");
    oled.clear();
    oled.setCursor(0, 0);
    oled.println("Settings saved");
    oled.println(fileName);
  }
  else
    Serial.println("Error opening file");
}

void deleteFile()
{
  Serial.println();
  char fileName[13];
  char filePref[] = "settings";
  char fileExtn[] = ".txt";
  sprintf(fileName, "%s%i%s", filePref, myPreset, fileExtn);
  //delete a file:
  if (SD.exists(fileName))
  {
    Serial.print("Removing "); Serial.println(fileName);
    SD.remove(fileName);
    Serial.println("Done");
  } else {
    Serial.print(fileName); Serial.println(" does not exist");
  }
}
