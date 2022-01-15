/* TODO
   Tone generator
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

#include "effect_dynamics.h"

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

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

AudioControlSGTL5000     audioShield;
AudioInputI2S            audioInput;
AudioOutputI2S           audioOutput;
AudioAnalyzePeak         peakPre;
AudioAnalyzePeak         peakPost;
AudioAnalyzeFFT256       fftValues;
AudioEffectDynamics      Dynamics;
AudioFilterBiquad        EQ_1;
AudioFilterBiquad        EQ_2;
AudioFilterBiquad        EQ_3;
AudioFilterBiquad        EQ_4;
AudioFilterBiquad        EQ_5;
AudioFilterBiquad        EQ_6;
AudioFilterBiquad        EQ_7;
AudioFilterBiquad        EQ_8;
AudioMixer4              EQ_MixIn;
AudioMixer4              EQ_Mix1;
AudioMixer4              EQ_Mix2;
AudioMixer4              EQ_MixOut;
AudioConnection          patchCord1(audioInput, 0, peakPre, 0);
AudioConnection          patchCord2(audioInput, 0, EQ_MixIn, 0);
AudioConnection          patchCord3(audioInput, 0, EQ_MixOut, 2);
AudioConnection          patchCord4(EQ_MixIn, 0, EQ_1, 0);
AudioConnection          patchCord5(EQ_MixIn, 0, EQ_2, 0);
AudioConnection          patchCord6(EQ_MixIn, 0, EQ_3, 0);
AudioConnection          patchCord7(EQ_MixIn, 0, EQ_4, 0);
AudioConnection          patchCord8(EQ_MixIn, 0, EQ_5, 0);
AudioConnection          patchCord9(EQ_MixIn, 0, EQ_6, 0);
AudioConnection          patchCord10(EQ_MixIn, 0, EQ_7, 0);
AudioConnection          patchCord11(EQ_MixIn, 0, EQ_8, 0);
AudioConnection          patchCord15(EQ_1, 0, EQ_Mix1, 0);
AudioConnection          patchCord16(EQ_2, 0, EQ_Mix1, 1);
AudioConnection          patchCord14(EQ_3, 0, EQ_Mix1, 2);
AudioConnection          patchCord13(EQ_4, 0, EQ_Mix1, 3);
AudioConnection          patchCord12(EQ_5, 0, EQ_Mix2, 0);
AudioConnection          patchCord17(EQ_6, 0, EQ_Mix2, 1);
AudioConnection          patchCord18(EQ_7, 0, EQ_Mix2, 2);
AudioConnection          patchCord19(EQ_8, 0, EQ_Mix2, 3);
AudioConnection          patchCord20(EQ_Mix1, 0, EQ_MixOut, 0);
AudioConnection          patchCord21(EQ_Mix2, 0, EQ_MixOut, 1);
AudioConnection          patchCord22(EQ_MixOut, 0, Dynamics, 0);
AudioConnection          patchCord23(Dynamics, fftValues);
AudioConnection          patchCord24(Dynamics, peakPost);
AudioConnection          patchCord25(Dynamics, 0, audioOutput, 0);
AudioConnection          patchCord26(Dynamics, 0, audioOutput, 1);

//Auto Volume Control (AVC) on
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

//DEFAULT FLAGS
#define AVCFLAG 0;
#define EQUALIZERFLAG 0;
#define NOISEGATEFLAG 0;
#define MYINPUT AUDIO_INPUT_MIC;

//DEFAULT AVC LEVELS
#define AVCGAIN 1.0f;
#define MYAVCGAIN 1;
#define MYAVCRESP 1;
#define MYAVCHARD 0;
#define MYAVCTHR -18.0f;
#define MYAVCATT 0.5f;
#define MYAVCDEC 1.0f;

//DEFAULT LEVELS
#define MYLINEINLEVEL 0;
#define MYLINEOUTLEVEL 29;
#define MYVOLUME 0.7f;
#define MICGAINSET 35;

// DEFAULT NOISE GATE PARAMETERS
#define MYNGATTACKTIME 0.01f;
#define MYNGRELEASETIME 0.2f;
#define MYNGTHRESHOLD -110.0f;
#define MYNGHYSTERISIS 6.0f;
#define MYNGHOLDTIME 0.0003f;

// DEFAULT PROCESSOR PARAMETERS
#define PROCFLAG 0;
#define MYPRCTHRESHOLD -30.0f;
#define MYPRCATTACK 0.03f;
#define MYPRCRELEASE 0.5f;
#define MYPRCRATIO 8.0f;
#define MYPRCKNEEWIDTH 6.0f;

// DEFAULT LIMITER PARAMETERS
#define LIMFLAG 0;
#define MYLIMTHRESHOLD -3.0f;
#define MYLIMATTACK 0.03f;
#define MYLIMRELEASE 0.03f;

// DEFAULT AUTO MAKEUP GAIN PARAMETERS
#define AMGFLAG 0;
#define MYAMGHEADROOM 6.0f;

// DEFAULT MAKEUP GAIN PARAMETERS
#define MUPFLAG 0;
#define MYMUPGAIN 0.0f;

int b;
int maxVal = 0;
int spectrumFlag = 0;

float AVCgain = AVCGAIN;
int AVCFlag = AVCFLAG;

int myAVCGain = MYAVCGAIN;
int myAVCResp = MYAVCRESP;
int myAVCHard = MYAVCHARD;
float myAVCThr = MYAVCTHR;
float myAVCAtt = MYAVCATT;
float myAVCDec = MYAVCDEC;

int freqBand;
const int eqFreq[] = {150, 240, 370, 590, 900, 1300, 2000, 3300};

float bandGain[] = {1, 1, 1, 1, 1, 1, 1, 1};
float ydBLevel[] = {0, 0, 0, 0, 0, 0, 0, 0};

int equalizerFlag = EQUALIZERFLAG;
int noiseGateFlag = NOISEGATEFLAG;
int myLineInLevel = MYLINEINLEVEL;    // range is 0 to 15
int myLineOutLevel = MYLINEOUTLEVEL;  // range is 13 to 31
float myVolume = MYVOLUME;     // 0.8 is max for undistorted headphone output
float micGainSet = MICGAINSET;    // 35 for the Chinese MH-1B8 mic
int myInput = MYINPUT;

// Default Noise Gate parameters
float myNGattackTime = MYNGATTACKTIME;
float myNGreleaseTime = MYNGRELEASETIME;
float myNGthreshold = MYNGTHRESHOLD;
float myNGhysterisis = MYNGHYSTERISIS;
float myNGholdTime = MYNGHOLDTIME;

// Default Processor parameters
int procFlag = PROCFLAG;
float myPRCthreshold = MYPRCTHRESHOLD;
float myPRCattack = MYPRCATTACK;
float myPRCrelease = MYPRCRELEASE;
float myPRCratio = MYPRCRATIO;
float myPRCkneeWidth = MYPRCKNEEWIDTH;

// Default Limiter parameters
int limFlag = LIMFLAG;
float myLIMthreshold = MYLIMTHRESHOLD;
float myLIMattack = MYLIMATTACK;
float myLIMrelease = MYLIMRELEASE;

// Default Auto Makeup Gain parameters
int amgFlag = AMGFLAG;
float myAMGheadroom = MYAMGHEADROOM;

// Default Makeup Gain parameters
int mupFlag = MUPFLAG;
float myMUPgain = MYMUPGAIN;

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
eventMask evt;

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
  EQ_MixOut.gain(0, 1);
  EQ_MixOut.gain(1, 1);
  EQ_MixOut.gain(2, 0);
  EQ_MixOut.gain(3, 0);
  //Serial.Println(""); Serial.print(e); Serial.println(" eqON executed, proceed menu"); Serial.flush();
  return proceed;
}

//Equalizer off
result eqOFF(eventMask e) {
  EQ_MixOut.gain(0, 0);
  EQ_MixOut.gain(1, 0);
  EQ_MixOut.gain(2, 1);
  EQ_MixOut.gain(3, 0);
  //Serial.Println(""); Serial.print(e); Serial.println(" eqOFF executed, proceed menu"); Serial.flush();
  return proceed;
}

result AVCon(eventMask e) {
  AVCFlag = 1;
  audioShield.autoVolumeControl( myAVCGain    // Maximum gain that can be applied 0 - 0 dB / 1 - 6.0 dB / 2 - 12 dB
                                 , myAVCResp  // Integrator Response 0 - 0 mS / 1 - 25 mS / 2 - 50 mS / 3 - 100 mS
                                 , myAVCHard  // hardLimit
                                 , myAVCThr   // threshold floating point in range 0 to -96 dB
                                 , myAVCAtt   // attack floating point figure is dB/s rate at which gain is increased
                                 , myAVCDec); // decay floating point figure is dB/s rate at which gain is reduced
  audioShield.autoVolumeEnable();
  //Serial.Println(""); Serial.print(e); Serial.println(" AVCon executed, proceed menu"); Serial.flush();
  return proceed;
}

//AVC off
result AVCoff(eventMask e) {
  AVCFlag = 0;
  audioShield.autoVolumeDisable();
  //Serial.Println(""); Serial.print(e); Serial.println(" AVCoff executed, proceed menu"); Serial.flush();
  return proceed;
}

MENU(subLevels, "Vol/Lim Levels", showEvent, anyEvent, wrapStyle
     , EXIT(" <- Back")
     , FIELD(     micGainSet,     "Mic.Gain", " ",   0, 63,   1,     , SetLevels, updateEvent, noStyle)
     , FIELD(       myVolume,    "Headphone", " ",   0,  1, 0.1, 0.01, SetLevels, updateEvent, noStyle)
     , FIELD(  myLineInLevel,      "Line In", " ",   0, 15,   1,     , SetLevels, updateEvent, noStyle)
     , FIELD( myLineOutLevel,     "Line Out", " ",  13, 31,   1,     , SetLevels, updateEvent, noStyle)
     , FIELD(  myAMGheadroom, "AMG Headroom", " ",   0, 60,   1,     , SetLevels, updateEvent, noStyle)
     , FIELD(      myMUPgain,  "Makeup Gain", " ", -12, 24,   1,     , SetLevels, updateEvent, noStyle)
    );

TOGGLE(myAVCGain, chooseAVCgain, "AVC Gain: ", doNothing, noEvent, wrapStyle
       , VALUE( "0 dB", 0, SetAVCParameters, updateEvent)
       , VALUE( "6 dB", 1, SetAVCParameters, updateEvent)
       , VALUE("12 dB", 2, SetAVCParameters, updateEvent)
      );

TOGGLE(myAVCResp, chooseAVCresp, "Response: ", doNothing, noEvent, wrapStyle
       , VALUE(  "0 ms", 0, SetAVCParameters, updateEvent)
       , VALUE( "25 ms", 1, SetAVCParameters, updateEvent)
       , VALUE( "50 ms", 2, SetAVCParameters, updateEvent)
       , VALUE("100 ms", 3, SetAVCParameters, updateEvent)
      );

TOGGLE(myAVCHard, setHardLimit, "Hard Limit: ", doNothing, noEvent, wrapStyle
       , VALUE("On", 1, SetAVCParameters, updateEvent)
       , VALUE("Off", 0, SetAVCParameters, updateEvent)
      );

MENU(subAVC, "Auto Vol Ctl cfg", showEvent, anyEvent, wrapStyle
     , EXIT(" <- Back")
     , SUBMENU(chooseAVCgain)
     , SUBMENU(chooseAVCresp)
     , SUBMENU(setHardLimit)
     , FIELD(   myAVCThr, "Thresh.", " dB", -96, 0, 1,    , SetAVCParameters, updateEvent, noStyle)
     , FIELD(   myAVCAtt, "Attack", " dB/s", 0, 10, 1, 0.1, SetAVCParameters, updateEvent, noStyle)
     , FIELD(   myAVCDec,  "Decay", " dB/s", 0, 10, 1, 0.1, SetAVCParameters, updateEvent, noStyle)
    );

TOGGLE(equalizerFlag, setEQ, "Equalizer: ", doNothing, noEvent, wrapStyle
       , VALUE("On", 1, eqON, noEvent)
       , VALUE("Off", 0, eqOFF, noEvent)
      );

TOGGLE(AVCFlag, setAVC, "Auto Vol Ctl: ", doNothing, noEvent, wrapStyle
       , VALUE("On", 1, AVCon, noEvent)
       , VALUE("Off", 0, AVCoff, noEvent)
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

    //Serial.Println();
    //Serial.Println("Audio Spectrum ON");
  }
  else {
    spectrumFlag = 0;
    //Serial.Println();
    //Serial.Println("Audio Spectrum OFF");
  }
  return proceed;
}

result ngON(eventMask e) {
  Dynamics.gate(myNGthreshold, myNGattackTime, myNGreleaseTime, myNGhysterisis);
  noiseGateFlag = 1;
  //Serial.Println();
  //Serial.Print(" myNGattackTime: "); Serial.print(myNGattackTime); Serial.print(" myNGreleaseTime: "); Serial.print(myNGreleaseTime); Serial.print(" myNGthreshold: "); Serial.print(myNGthreshold);
  //Serial.Print(" myNGholdTime: "); Serial.print(myNGholdTime, 4); Serial.print(" myNGhysterisis: "); Serial.print(myNGhysterisis);
  //Serial.Println();
  //Serial.Println("Noise Gate ON");
  return proceed;
}

result ngOFF(eventMask e) {
  noiseGateFlag = 0;
  Dynamics.gate( -110.0f, myNGattackTime, myNGreleaseTime, myNGhysterisis);
  //Serial.Println();
  //Serial.Println("Noise Gate OFF");
  return proceed;
}

MENU(subNG, "Noise Gate cfg", showEvent, anyEvent, wrapStyle
     , EXIT(" <- Back")
     , altFIELD(decPlaces<3>::menuField,  myNGattackTime,  "Attack", "", 0.0, 1.0, 0.01, 0.001, ngON, updateEvent, noStyle)
     , altFIELD(decPlaces<3>::menuField, myNGreleaseTime, "Release", "", 0.0, 1.0, 0.01, 0.001, ngON, updateEvent, noStyle)
     , FIELD(myNGthreshold, "Thresh.", "", -110.0, 50.0, 1, , ngON, updateEvent, noStyle)
     , FIELD(myNGhysterisis, "Hysteresis", "", 0.0f, 6.0f, 1, 0.1, ngON, updateEvent, noStyle)
     , altFIELD(decPlaces<4>::menuField, myNGholdTime, "Hold", "", 0.0001, 0.01, 0.001, 0.0001, ngON, updateEvent, noStyle)
    );

TOGGLE(noiseGateFlag, setNG, "Noise Gate: ", doNothing, noEvent, wrapStyle
       , VALUE("On", 1, ngON, noEvent)
       , VALUE("Off", 0, ngOFF, noEvent)
      );

result procON(eventMask e) {
  Dynamics.compression( myPRCthreshold, myPRCattack, myPRCrelease, myPRCratio, myPRCkneeWidth);
  procFlag = 1;
  //Serial.Println();
  //Serial.Print(" myPRCthreshold: "); Serial.print(myPRCthreshold); Serial.print(" myPRCattack: "); Serial.print(myPRCattack , 4);
  //Serial.Print(" myPRCrelease: "); Serial.print(myPRCrelease, 4); Serial.print(" myPRCratio: "); Serial.print(myPRCratio); Serial.print(" myPRCkneeWidth: "); Serial.print(myPRCkneeWidth);
  //Serial.Println();
  //Serial.Println("Processor ON");
  return proceed;
}

result procOFF(eventMask e) {
  Dynamics.compression( 0.0f, 0.03f, 0.5f, 1.0f, 6.0f);
  procFlag = 0;
  //Serial.Println();
  //Serial.Print(" myPRCthreshold: "); Serial.print(myPRCthreshold); Serial.print(" myPRCattack: "); Serial.print(myPRCattack, 4);
  //Serial.Print(" myPRCrelease: "); Serial.print(myPRCrelease, 4); Serial.print(" myPRCratio: "); Serial.print(myPRCratio); Serial.print(" myPRCkneeWidth: "); Serial.print(myPRCkneeWidth);
  //Serial.Println();
  //Serial.Println("Processor OFF");
  return proceed;
}

MENU(subProc, "Processor cfg", showEvent, anyEvent, wrapStyle
     , EXIT(" <- Back")
     , FIELD( myPRCthreshold, "Thresh.", "", -110.0, 0.0, 1, , procON, updateEvent, noStyle)
     , altFIELD(decPlaces<3>::menuField,  myPRCattack,  "Attack", "", 0.0, 1.0, 0.01, 0.001, procON, updateEvent, noStyle)
     , altFIELD(decPlaces<3>::menuField, myPRCrelease, "Release", "", 0.0, 1.0, 0.01, 0.001, procON, updateEvent, noStyle)
     , FIELD(     myPRCratio,      "Ratio", "", 1.0, 60.0, 1, , procON, updateEvent, noStyle)
     , FIELD( myPRCkneeWidth, "Knee Width", "", 0.0, 32.0, 1, , procON, updateEvent, noStyle)
    );

TOGGLE(procFlag, setProc, "Processor: ", doNothing, noEvent, wrapStyle
       , VALUE("On", 1, procON, noEvent)
       , VALUE("Off", 0, procOFF, noEvent)
      );

result limON(eventMask e) {
  Dynamics.limit( myLIMthreshold, myLIMattack, myLIMrelease);
  limFlag = 1;
  //Serial.Println();
  //Serial.Print(" myLIMthreshold: "); Serial.print(myLIMthreshold); Serial.print(" myLIMattack: "); Serial.print(myLIMattack, 4); Serial.print(" myLIMrelease: "); Serial.print(myLIMrelease, 4);
  //Serial.Println();
  //Serial.Println("Limiter ON");
  return proceed;
}

result limOFF(eventMask e) {
  Dynamics.limit( myLIMthreshold, myLIMattack, myLIMrelease);
  limFlag = 0;
  //Serial.Println();
  //Serial.Print(" myLIMthreshold: "); Serial.print(myLIMthreshold); Serial.print(" myLIMattack: "); Serial.print(myLIMattack, 4); Serial.print(" myLIMrelease: "); Serial.print(myLIMrelease, 4);
  //Serial.Println();
  //Serial.Println("Limiter OFF");
  return proceed;
}

MENU(subLim, "Limiter cfg", showEvent, anyEvent, wrapStyle
     , EXIT(" <- Back")
     , FIELD( myLIMthreshold, "Thresh.", "", -110.0, 0.0, 1, , limON, updateEvent, noStyle)
     , altFIELD(decPlaces<3>::menuField,  myLIMattack,  "Attack", "", 0.0, 4.0, 0.01, 0.001, limON, updateEvent, noStyle)
     , altFIELD(decPlaces<3>::menuField, myLIMrelease, "Release", "", 0.0, 4.0, 0.01, 0.001, limON, updateEvent, noStyle)
    );

TOGGLE(limFlag, setLim, "Limiter: ", doNothing, noEvent, wrapStyle
       , VALUE("On", 1, limON, noEvent)
       , VALUE("Off", 0, limOFF, noEvent)
      );

MENU(sdCard, "SD Card", doNothing, noEvent, wrapStyle
     , FIELD(myPreset, "Select preset", "", 0, 9, 1, , doNothing, noEvent, wrapStyle)
     , OP("Read preset", readFromFile, enterEvent)
     , OP("Save preset",  writeToFile, enterEvent)
     , OP("Del. preset",   deleteFile, enterEvent)
     , OP("Load default", resetDefault, enterEvent)
     , EXIT(" <- Back")
    );

result amgON(eventMask e) {
  Dynamics.makeupGain(0.0f);
  mupFlag = 0;
  Dynamics.autoMakeupGain(myAMGheadroom);
  amgFlag = 1;
  //Serial.Println();
  //Serial.Print(" myAMGheadroom: "); Serial.print(myAMGheadroom);
  //Serial.Println();
  //Serial.Println("Auto Makeup Gain ON and Makeup Gain OFF");
  return proceed;
}

result amgOFF(eventMask e) {
  Dynamics.autoMakeupGain(0.0f);
  amgFlag = 0;
  //Serial.Println();
  //Serial.Print(" myAMGheadroom: "); Serial.print(0.0f);
  //Serial.Println();
  //Serial.Println("Auto Makeup Gain OFF");
  return proceed;
}

TOGGLE(amgFlag, setAMG, "Auto MU Gain: ", doNothing, noEvent, wrapStyle
       , VALUE("On", 1, amgON, noEvent)
       , VALUE("Off", 0, amgOFF, noEvent)
      );

result mupON(eventMask e) {
  Dynamics.autoMakeupGain(0.0f);
  amgFlag = 0;
  Dynamics.makeupGain(myMUPgain);
  mupFlag = 1;
  //Serial.Println();
  //Serial.Print(" myMUPgain: "); Serial.print(myMUPgain);
  //Serial.Println();
  //Serial.Println("Makeup Gain ON and Auto Makeup OFF");
  return proceed;
}

result mupOFF(eventMask e) {
  Dynamics.makeupGain(0.0f);
  mupFlag = 0;
  //Serial.Println();
  //Serial.Print(" myMUPgain: "); Serial.print(0.0f);
  //Serial.Println();
  //Serial.Println("Makeup Gain OFF");
  return proceed;
}

TOGGLE(mupFlag, setMUP, "Makeup Gain: ", doNothing, noEvent, wrapStyle
       , VALUE("On", 1, mupON, noEvent)
       , VALUE("Off", 0, mupOFF, noEvent)
      );

MENU(mainMenu, "Main menu", doNothing, noEvent, wrapStyle
     , OP("Spectrum disp", toggleAudioSpectrum, enterEvent)
     , SUBMENU(subLevels)
     , SUBMENU(subNG)
     , SUBMENU(setNG)
     , SUBMENU(subProc)
     , SUBMENU(setProc)
     , SUBMENU(subLim)
     , SUBMENU(setLim)
     , SUBMENU(setAMG)
     , SUBMENU(setMUP)
     , SUBMENU(subEQ)
     , SUBMENU(setEQ)
     , SUBMENU(subAVC)
     , SUBMENU(setAVC)
     , SUBMENU(selMenu)
     , SUBMENU(sdCard)
    );

result showEvent(eventMask e, navNode & nav, prompt & item) {
  //Serial.Println();
  //Serial.Print("event: ");
  //Serial.Print(e);
  return proceed;
}

// Configuration https://github.com/neu-rah/ArduinoMenu/wiki/Menu-definition

#define MAX_DEPTH 2

// define output device
//idx_t serialTops[MAX_DEPTH] = {0};
//serialOut outSerial(Serial, serialTops);

//describing a menu output device without macros
//define at least one panel for menu output
const panel panels[] MEMMODE = {{0, 0, 128 / fontW, 64 / fontH}};
navNode* nodes[sizeof(panels) / sizeof(panel)];                         //navNodes to store navigation status
panelsList pList(panels, nodes, 1);                                     //a list of panels and nodes
idx_t tops[MAX_DEPTH] = {0, 0};                                         //store cursor positions for each level
SSD1306AsciiOut outOLED(&oled, tops, pList, 8, 1 + ((fontH - 1) >> 3) ); //oled output device menu driver
// menuOut* constMEM outputs[] MEMMODE = {&outOLED, &outSerial};           //list of output devices
menuOut* constMEM outputs[] MEMMODE = {&outOLED};           //list of output devices
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

  //Serial.begin(115200);
  //Serial.Println("");
  /*
    while (!Serial) {
      // wait for Arduino Serial Monitor to be ready
    }
  */
  //Serial.Println("DSP Pre-Processor booting");

  Wire.begin();
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(menuFont);

  oled.clear();
  oled.setCursor(0, 0);
  oled.println("VA3HDL - Aurora,ON");
  oled.println("Version 4.0");

  Timer1.initialize(500);
  Timer1.attachInterrupt(timerIsr);

  AudioMemory(100);
  audioShield.enable();

  //Serial.Println("SD Card active");
  oled.println("SD Card active");

  if (SD.begin(chipSelect))
  {
    //Serial.Println("SD card is present");
    oled.println("SD card is present");
    //Serial.Println("Reading settings");
    oled.println("Reading settings");
    readFromFile();
  }
  else
  {
    //Serial.Println("SD card missing or failure");
    oled.println("SD card missing or failure");
    while (1); //wait here forever
  }

  delay(2000);
  oled.clear();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    //Serial.Println(F("SSD1306 allocation failed"));
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
      //Serial.Println();
      //Serial.Println("Exiting Spectrum");
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

  EQ_MixIn.gain(0, 1);

  if (equalizerFlag == 1) eqON(evt); else eqOFF(evt);

  SetupFilters();         // Setup Equalizer bands
  EqGainSetL();           // Setup Equalizer levels
  SetAVCParameters();     // Setup AVC parameters

  /* Dynamics - Order of events:
    1. gate -> timeToAlpha -> timeToAlpha
    2. compression -> computeMakeupGain -> timeToAlpha -> timeToAlpha
    3. limit -> computeMakeupGain -> timeToAlpha -> timeToAlpha
    4. makeupgain
    5. autoMakeupGain -> computeMakeupGain
  */

  if (noiseGateFlag == 1) ngON(evt); else ngOFF(evt);

  if (procFlag == 1) procON(evt); else procOFF(evt);

  if (limFlag == 1) limON(evt); else limOFF(evt);

  if (mupFlag == 1) mupON(evt); else mupOFF(evt);

  if (amgFlag == 1) amgON(evt); else amgOFF(evt);

  audioShield.unmuteHeadphone();
  audioShield.unmuteLineout();
}

void SetAVCParameters() {
  //Serial.Println();
  //Serial.Print(" Comp Gain: "); Serial.print(myAVCGain); Serial.print(" Comp Resp: "); Serial.print(myAVCResp); Serial.print(" Hardlimit: "); Serial.print(myAVCHard);
  //Serial.Print(" Threshold: "); Serial.print( myAVCThr); Serial.print(" Attack: "); Serial.print(myAVCAtt); Serial.print(" Decay: "); Serial.print(myAVCDec);
  //Serial.Print(" Mic Gain: "); Serial.print(micGainSet);

  if (AVCFlag == 1) {
    audioShield.autoVolumeControl( myAVCGain    // Maximum gain that can be applied 0 - 0 dB / 1 - 6.0 dB / 2 - 12 dB
                                   , myAVCResp  // Integrator Response 0 - 0 mS / 1 - 25 mS / 2 - 50 mS / 3 - 100 mS
                                   , myAVCHard  // hardLimit
                                   , myAVCThr   // threshold floating point in range 0 to -96 dB
                                   , myAVCAtt   // attack floating point figure is dB/s rate at which gain is increased
                                   , myAVCDec); // decay floating point figure is dB/s rate at which gain is reduced
    audioShield.autoVolumeEnable();
  }
}

void SetLevels() {
  //Serial.Println();
  //Serial.Print(" Headphone: "); Serial.print(myVolume); Serial.print(" Line In Level: "); Serial.print(myLineInLevel); Serial.print(" Line Out Level: "); Serial.print(myLineOutLevel);
  //Serial.Print(" myAMGheadroom: "); Serial.print(myAMGheadroom); Serial.print(" myMUPgain: "); Serial.print(myMUPgain);
  audioShield.volume(myVolume);
  audioShield.micGain(micGainSet);
  audioShield.lineInLevel(myLineInLevel);
  audioShield.lineOutLevel(myLineOutLevel);
  if (amgFlag == 1) Dynamics.autoMakeupGain(myAMGheadroom);
  if (mupFlag == 1) Dynamics.makeupGain(myMUPgain);
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
  //Serial.Println(); Serial.print("Input: "); Serial.print(myInput);
}

void EqGainSetL() {
  for (int freqBand = 0; freqBand < 8; freqBand++) {
    bandGain[freqBand] = pow(10, (ydBLevel[freqBand] / 20));
    if (ydBLevel[freqBand] <= -12) {
      bandGain[freqBand] = 0;
    }
    //Serial.Println();
    //Serial.Print("Eq. band: "); Serial.print(freqBand); Serial.print(" dB: "); Serial.print(ydBLevel[freqBand]); Serial.print(" Gain: "); Serial.print(bandGain[freqBand]);
  }

  EQ_Mix1.gain(0, bandGain[0]);     //  140
  EQ_Mix1.gain(1, bandGain[1]);     //  240
  EQ_Mix1.gain(2, bandGain[2]);     //  370
  EQ_Mix1.gain(3, bandGain[3]);     //  590

  EQ_Mix2.gain(0, bandGain[4]);     //  900
  EQ_Mix2.gain(1, bandGain[5]);     //  1300
  EQ_Mix2.gain(2, bandGain[6]);     //  2000
  EQ_Mix2.gain(3, bandGain[7]);     //  3300

  EQ_MixOut.gain(0, 1);  // Combine Mixers
  EQ_MixOut.gain(1, 1);
  EQ_MixOut.gain(2, 0);
}

void SetupFilters() {
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

void readFromFile()
{
  byte i = 0;
  char inputString[100];
  char fileName[13];
  char filePref[] = "settings";
  char fileExtn[] = ".txt";
  sprintf(fileName, "%s%i%s", filePref, myPreset, fileExtn);
  //Serial.Println();
  //Serial.end();
  String msgBuffer;
  // Check to see if the file exists:
  if (!SD.exists(fileName)) {
    //Serial.print(fileName);
    //Serial.println(" doesn't exist.");
    msgBuffer.concat(fileName);
    msgBuffer.concat(" doesn't exist.\n");
  } else {
    //Serial.print("Reading from: "); Serial.println(fileName);
    msgBuffer.concat("Reading from: "); msgBuffer.concat(fileName); msgBuffer.concat("\n");
    myFile = SD.open(fileName);
  }
  int line = 0;
  while (myFile.available())
  {
    char inputChar = myFile.read(); // Gets one byte from serial buffer
    if (inputChar == '\n') //end of line (or 10)
    {
      inputString[i] = 0;  //terminate the string correctly
      msgBuffer.concat("Input string: "); msgBuffer.concat(inputString);
      switch (line) {
        case 0:
          AVCgain = atof(inputString);
          msgBuffer.concat(" Input AVCgain: "); msgBuffer.concat(String(AVCgain)); msgBuffer.concat("\n");
          break;
        case 1:
          AVCFlag = atoi(inputString);
          msgBuffer.concat(" Input AVCFlag: "); msgBuffer.concat(String(AVCFlag)); msgBuffer.concat("\n");
          break;
        case 2:
          equalizerFlag = atoi(inputString);
          msgBuffer.concat(" Input equalizerFlag: "); msgBuffer.concat(String(equalizerFlag)); msgBuffer.concat("\n");
          break;
        case 3:
          myLineInLevel = atoi(inputString);
          msgBuffer.concat(" Input myLineInLevel: "); msgBuffer.concat(String(myLineInLevel)); msgBuffer.concat("\n");
          break;
        case 4:
          myLineOutLevel = atoi(inputString);
          msgBuffer.concat(" Input myLineOutLevel: "); msgBuffer.concat(String(myLineOutLevel)); msgBuffer.concat("\n");
          break;
        case 5:
          myVolume = atof(inputString);
          msgBuffer.concat(" Input myVolume: "); msgBuffer.concat(String(myVolume)); msgBuffer.concat("\n");
          break;
        case 6:
          micGainSet = atof(inputString);
          msgBuffer.concat(" Input micGainSet: "); msgBuffer.concat(String(micGainSet)); msgBuffer.concat("\n");
          break;
        case 7:
          myAVCGain = atoi(inputString);
          msgBuffer.concat(" Input myAVCGain: "); msgBuffer.concat(String(myAVCGain)); msgBuffer.concat("\n");
          break;
        case 8:
          myAVCResp = atoi(inputString);
          msgBuffer.concat(" Input myAVCResp: "); msgBuffer.concat(String(myAVCResp)); msgBuffer.concat("\n");
          break;
        case 9:
          myAVCHard = atoi(inputString);
          msgBuffer.concat(" Input myAVCHard: "); msgBuffer.concat(String(myAVCHard)); msgBuffer.concat("\n");
          break;
        case 10:
          myAVCThr = atof(inputString);
          msgBuffer.concat(" Input myAVCThr: "); msgBuffer.concat(String(myAVCThr)); msgBuffer.concat("\n");
          break;
        case 11:
          myAVCAtt = atof(inputString);
          msgBuffer.concat(" Input myAVCAtt: "); msgBuffer.concat(String(myAVCAtt)); msgBuffer.concat("\n");
          break;
        case 12:
          myAVCDec = atof(inputString);
          msgBuffer.concat(" Input myAVCDec: "); msgBuffer.concat(String(myAVCDec)); msgBuffer.concat("\n");
          break;
        case 13:
          myInput = atoi(inputString);
          msgBuffer.concat(" Input myInput: "); msgBuffer.concat(String(myInput)); msgBuffer.concat("\n");
          break;
        case 14:
          ydBLevel[0] = atof(inputString);
          msgBuffer.concat(" Input ydBLevel[0]: "); msgBuffer.concat(String(ydBLevel[0])); msgBuffer.concat("\n");
          break;
        case 15:
          ydBLevel[1] = atof(inputString);
          msgBuffer.concat(" Input ydBLevel[1]: "); msgBuffer.concat(String(ydBLevel[1])); msgBuffer.concat("\n");
          break;
        case 16:
          ydBLevel[2] = atof(inputString);
          msgBuffer.concat(" Input ydBLevel[2]: "); msgBuffer.concat(String(ydBLevel[2])); msgBuffer.concat("\n");
          break;
        case 17:
          ydBLevel[3] = atof(inputString);
          msgBuffer.concat(" Input ydBLevel[3]: "); msgBuffer.concat(String(ydBLevel[3])); msgBuffer.concat("\n");
          break;
        case 18:
          ydBLevel[4] = atof(inputString);
          msgBuffer.concat(" Input ydBLevel[4]: "); msgBuffer.concat(String(ydBLevel[4])); msgBuffer.concat("\n");
          break;
        case 19:
          ydBLevel[5] = atof(inputString);
          msgBuffer.concat(" Input ydBLevel[5]: "); msgBuffer.concat(String(ydBLevel[5])); msgBuffer.concat("\n");
          break;
        case 20:
          ydBLevel[6] = atof(inputString);
          msgBuffer.concat(" Input ydBLevel[6]: "); msgBuffer.concat(String(ydBLevel[6])); msgBuffer.concat("\n");
          break;
        case 21:
          ydBLevel[7] = atof(inputString);
          msgBuffer.concat(" Input ydBLevel[7]: "); msgBuffer.concat(String(ydBLevel[7])); msgBuffer.concat("\n");
          break;
        case 22:
          myNGattackTime = atof(inputString);
          msgBuffer.concat(" Input myNGattackTime: "); msgBuffer.concat(String(myNGattackTime, 4)); msgBuffer.concat("\n");
          break;
        case 23:
          myNGreleaseTime = atof(inputString);
          msgBuffer.concat(" Input myNGreleaseTime: "); msgBuffer.concat(String(myNGreleaseTime, 4)); msgBuffer.concat("\n");
          break;
        case 24:
          myNGthreshold = atof(inputString);
          msgBuffer.concat(" Input myNGthreshold: "); msgBuffer.concat(String(myNGthreshold)); msgBuffer.concat("\n");
          break;
        case 25:
          myNGholdTime = atof(inputString);
          msgBuffer.concat(" Input myNGholdTime: "); msgBuffer.concat(String(myNGholdTime, 4)); msgBuffer.concat("\n");
          break;
        case 26:
          noiseGateFlag = atoi(inputString);
          msgBuffer.concat(" Input noiseGateFlag: "); msgBuffer.concat(String(noiseGateFlag)); msgBuffer.concat("\n");
          break;
        case 27:
          myNGhysterisis = atof(inputString);
          msgBuffer.concat(" Input myNGhysterisis: "); msgBuffer.concat(String(myNGhysterisis)); msgBuffer.concat("\n");
          break;
        case 28:
          procFlag = atoi(inputString);
          msgBuffer.concat(" Input procFlag: "); msgBuffer.concat(String(procFlag)); msgBuffer.concat("\n");
          break;
        case 29:
          myPRCthreshold = atof(inputString);
          msgBuffer.concat(" Input myPRCthreshold: "); msgBuffer.concat(String(myPRCthreshold)); msgBuffer.concat("\n");
          break;
        case 30:
          myPRCattack = atof(inputString);
          msgBuffer.concat(" Input myPRCattack: "); msgBuffer.concat(String(myPRCattack, 4)); msgBuffer.concat("\n");
          break;
        case 31:
          myPRCrelease = atof(inputString);
          msgBuffer.concat(" Input myPRCrelease: "); msgBuffer.concat(String(myPRCrelease, 4)); msgBuffer.concat("\n");
          break;
        case 32:
          myPRCratio = atof(inputString);
          msgBuffer.concat(" Input myPRCratio: "); msgBuffer.concat(String(myPRCratio)); msgBuffer.concat("\n");
          break;
        case 33:
          myPRCkneeWidth = atof(inputString);
          msgBuffer.concat(" Input myPRCkneeWidth: "); msgBuffer.concat(String(myPRCkneeWidth)); msgBuffer.concat("\n");
          break;
        case 34:
          limFlag = atoi(inputString);
          msgBuffer.concat(" Input limFlag: "); msgBuffer.concat(String(limFlag)); msgBuffer.concat("\n");
          break;
        case 35:
          myLIMthreshold = atof(inputString);
          msgBuffer.concat(" Input myLIMthreshold: "); msgBuffer.concat(String(myLIMthreshold)); msgBuffer.concat("\n");
          break;
        case 36:
          myLIMattack = atof(inputString);
          msgBuffer.concat(" Input myLIMattack: "); msgBuffer.concat(String(myLIMattack, 4)); msgBuffer.concat("\n");
          break;
        case 37:
          myLIMrelease = atof(inputString);
          msgBuffer.concat(" Input myLIMrelease: "); msgBuffer.concat(String(myLIMrelease, 4)); msgBuffer.concat("\n");
          break;
        case 38:
          amgFlag = atoi(inputString);
          msgBuffer.concat(" Input amgFlag: "); msgBuffer.concat(String(amgFlag)); msgBuffer.concat("\n");
          break;
        case 39:
          myAMGheadroom = atof(inputString);
          msgBuffer.concat(" Input myAMGheadroom: "); msgBuffer.concat(String(myAMGheadroom)); msgBuffer.concat("\n");
          break;
        case 40:
          mupFlag = atoi(inputString);
          msgBuffer.concat(" Input mupFlag: "); msgBuffer.concat(String(mupFlag)); msgBuffer.concat("\n");
          break;
        case 41:
          myMUPgain = atof(inputString);
          msgBuffer.concat(" Input myMUPgain: "); msgBuffer.concat(String(myMUPgain)); msgBuffer.concat("\n");
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
        msgBuffer.concat("Incoming string longer than array allows");
        msgBuffer.concat(sizeof(inputString));
        while (1);
      }
    }
  }
  oled.clear();
  oled.setCursor(0, 0);
  oled.println("Settings read");
  oled.println(fileName);
  // Apply settings loaded
  //Serial.begin(115200);
  //Serial.Println(msgBuffer);
  //Serial.Println("Applying settings loaded to the AudioShield");
  SetAudioShield();
}

void writeToFile()
{
  //Serial.Println();
  //Serial.end();
  char fileName[13];
  char filePref[] = "settings";
  char fileExtn[] = ".txt";
  sprintf(fileName, "%s%i%s", filePref, myPreset, fileExtn);
  deleteFile();
  myFile = SD.open(fileName, FILE_WRITE);
  if (myFile) // it opened OK
  {
    myFile.println(AVCgain);
    myFile.println(AVCFlag);
    myFile.println(equalizerFlag);
    myFile.println(myLineInLevel);
    myFile.println(myLineOutLevel);
    myFile.println(myVolume);
    myFile.println(micGainSet);
    myFile.println(myAVCGain);
    myFile.println(myAVCResp);
    myFile.println(myAVCHard);
    myFile.println(myAVCThr);
    myFile.println(myAVCAtt);
    myFile.println(myAVCDec);
    myFile.println(myInput);
    myFile.println(ydBLevel[0]);
    myFile.println(ydBLevel[1]);
    myFile.println(ydBLevel[2]);
    myFile.println(ydBLevel[3]);
    myFile.println(ydBLevel[4]);
    myFile.println(ydBLevel[5]);
    myFile.println(ydBLevel[6]);
    myFile.println(ydBLevel[7]);
    myFile.println(myNGattackTime, 4);
    myFile.println(myNGreleaseTime, 4);
    myFile.println(myNGthreshold);
    myFile.println(myNGholdTime, 4);
    myFile.println(noiseGateFlag);
    myFile.println(myNGhysterisis);
    myFile.println(procFlag);
    myFile.println(myPRCthreshold);
    myFile.println(myPRCattack, 4);
    myFile.println(myPRCrelease, 4);
    myFile.println(myPRCratio);
    myFile.println(myPRCkneeWidth);
    myFile.println(limFlag);
    myFile.println(myLIMthreshold);
    myFile.println(myLIMattack, 4);
    myFile.println(myLIMrelease, 4);
    myFile.println(amgFlag);
    myFile.println(myAMGheadroom);
    myFile.println(mupFlag);
    myFile.println(myMUPgain);
    myFile.close();
    //Serial.begin(115200);
    //Serial.Print("Writing to "); Serial.println(fileName);
    //Serial.Println("Done");
    oled.clear();
    oled.setCursor(0, 0);
    oled.println("Settings saved");
    oled.println(fileName);
  }
  else
  {
    //Serial.begin(115200);
    //Serial.Println("Error opening file");
  }
}

void deleteFile()
{
  //Serial.Println();
  //Serial.end();
  char fileName[13];
  char filePref[] = "settings";
  char fileExtn[] = ".txt";
  sprintf(fileName, "%s%i%s", filePref, myPreset, fileExtn);
  //delete a file:
  if (SD.exists(fileName))
  {
    SD.remove(fileName);
    //Serial.begin(115200);
    //Serial.Print("Removing "); Serial.println(fileName);
  } else {
    //Serial.begin(115200);
    //Serial.Print(fileName); Serial.println(" does not exist");
  }
}

void resetDefault(void)
{
  AVCgain = AVCGAIN;
  AVCFlag = AVCFLAG;

  myAVCGain = MYAVCGAIN;
  myAVCResp = MYAVCRESP;
  myAVCHard = MYAVCHARD;
  myAVCThr = MYAVCTHR;
  myAVCAtt = MYAVCATT;
  myAVCDec = MYAVCDEC;

  bandGain[0] = 1;
  bandGain[1] = 1;
  bandGain[2] = 1;
  bandGain[3] = 1;
  bandGain[4] = 1;
  bandGain[5] = 1;
  bandGain[6] = 1;
  bandGain[7] = 1;

  ydBLevel[0] = 0;
  ydBLevel[1] = 0;
  ydBLevel[2] = 0;
  ydBLevel[3] = 0;
  ydBLevel[4] = 0;
  ydBLevel[5] = 0;
  ydBLevel[6] = 0;
  ydBLevel[7] = 0;

  equalizerFlag = EQUALIZERFLAG;
  noiseGateFlag = NOISEGATEFLAG;
  myLineInLevel = MYLINEINLEVEL;
  myLineOutLevel = MYLINEOUTLEVEL;
  myVolume = MYVOLUME;
  micGainSet = MICGAINSET;
  myInput = MYINPUT;

  // Default Noise Gate parameters
  myNGattackTime = MYNGATTACKTIME;
  myNGreleaseTime = MYNGRELEASETIME;
  myNGthreshold = MYNGTHRESHOLD;
  myNGhysterisis = MYNGHYSTERISIS;
  myNGholdTime = MYNGHOLDTIME;

  // Default Processor parameters
  procFlag = PROCFLAG;
  myPRCthreshold = MYPRCTHRESHOLD;
  myPRCattack = MYPRCATTACK;
  myPRCrelease = MYPRCRELEASE;
  myPRCratio = MYPRCRATIO;
  myPRCkneeWidth = MYPRCKNEEWIDTH;

  // Default Limiter parameters
  limFlag = LIMFLAG;
  myLIMthreshold = MYLIMTHRESHOLD;
  myLIMattack = MYLIMATTACK;
  myLIMrelease = MYLIMRELEASE;

  // Default Auto Makeup Gain parameters
  amgFlag = AMGFLAG;
  myAMGheadroom = MYAMGHEADROOM;

  // Default Makeup Gain parameters
  mupFlag = MUPFLAG;
  myMUPgain = MYMUPGAIN;
  
  SetAudioShield();
  
  oled.clear();
  oled.setCursor(0, 0);
  oled.println("Defaults loaded");
}
