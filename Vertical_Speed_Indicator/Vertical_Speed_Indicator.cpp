#include "Vertical_Speed_Indicator.h"
#include "allocateMem.h"
#include "commandmessenger.h"
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
#include <vsi_main_gauge.h>
#include <vsi_needle.h>
#include <logo.h>

#define PANEL_COLOR 0x7BEE

    TFT_eSPI tft = TFT_eSPI(); // Invoke custom library
TFT_eSprite VSImainSpr = TFT_eSprite(&tft);       // Main Sprite
TFT_eSprite VSINeedleSpr = TFT_eSprite(&tft);     // Vertical Speed

/* **********************************************************************************
    This is just the basic code to set up your custom device.
    Change/add your code as needed.
********************************************************************************** */

Vertical_Speed_Indicator::Vertical_Speed_Indicator(uint8_t Pin1, uint8_t Pin2)
{
    _pin1 = Pin1;
    _pin2 = Pin2;
}

void Vertical_Speed_Indicator::begin()
{

  Serial.begin(115200);
  tft.begin();
  tft.setRotation(screenRotation);
  tft.fillScreen(PANEL_COLOR);
  tft.setPivot(240, 160);
  tft.setSwapBytes(true);
  tft.pushImage(160, 80, 160, 160, logo);
  delay(3000);
  tft.setViewport(0, 0, 480, 320, false);
  tft.fillScreen(TFT_BLACK);

  VSImainSpr.createSprite(320, 320);
  VSImainSpr.setSwapBytes(true);
  VSImainSpr.fillSprite(TFT_BLACK);
  VSImainSpr.pushImage(0, 0, 320, 320, vsi_main_gauge);
  VSImainSpr.setPivot(160, 160);

  VSINeedleSpr.createSprite(vsi_needle_width, vsi_needle_height);
  VSINeedleSpr.setSwapBytes(true);
  VSINeedleSpr.fillScreen(TFT_BLACK);
  VSINeedleSpr.pushImage(0, 0, vsi_needle_width, vsi_needle_height, vsi_needle);
  VSINeedleSpr.setPivot(vsi_needle_width/2, 134);
  
}

void Vertical_Speed_Indicator::attach(uint16_t Pin3, char *init)
{
    _pin3 = Pin3;
}

void Vertical_Speed_Indicator::detach()
{
    if (!_initialised)
        return;
    _initialised = false;
}

void Vertical_Speed_Indicator::set(int16_t messageID, char *setPoint)
{
    /* **********************************************************************************
        Each messageID has it's own value
        check for the messageID and define what to do.
        Important Remark!
        MessageID == -2 will be send from the board when PowerSavingMode is set
            Message will be "0" for leaving and "1" for entering PowerSavingMode
        MessageID == -1 will be send from the connector when Connector stops running
        Put in your code to enter this mode (e.g. clear a display)

    ********************************************************************************** */
    // int32_t  data = atoi(setPoint);
    // uint16_t output;

    // do something according your messageID
    switch (messageID) {
   case -1:
        setPowerSaveMode(true);
        // tbd., get's called when Mobiflight shuts down
        break;
    case -2:
        if(atoi(setPoint) == 1)
          setPowerSaveMode(true);
        else setPowerSaveMode(false);
        // tbd., get's called when PowerSavingMode is entered
        break;
    case 0:
        // output = (uint16_t)data;
        // data   = output;
        setVerticalSpeed(atof(setPoint));
        break;
    case 1:
        setInstrumentBrightnessRatio(atof(setPoint));
        break;
    case 100:
        /* code */
        setScreenRotation(atoi(setPoint));
        break;
    default:
        break;
    }
}

void Vertical_Speed_Indicator::update()
{
    // Do something which is required regulary
//   if(!powerSaveFlag)
//   {
//     analogWrite(TFT_BL, instrumentBrightness);
//     if(prevScreenRotation != screenRotation)
//     {
//         tft.setRotation(screenRotation);
//         prevScreenRotation = screenRotation;
//     }
//     drawVSI();
//    }
//    else digitalWrite(TFT_BL, LOW);

    analogWrite(TFT_BL, instrumentBrightness);
    if(prevScreenRotation != screenRotation)
    {
        tft.setRotation(screenRotation);
        prevScreenRotation = screenRotation;
    }
    drawVSI();

}

void Vertical_Speed_Indicator::drawVSI()
{
  //Limit to -2000 to 2000 ft/sec
  if (VSIValue > 2000)
    VSIValue = 2000;
  else if (VSIValue < - 2000)
    VSIValue = -2000;

  VSIAngle = scaleValue(VSIValue, -2000, 2000, 102, 438); // The needle starts at -90 degrees
  // put your main code here, to run repeatedly:
  VSImainSpr.pushImage(0, 0,320, 320, vsi_main_gauge);
  VSINeedleSpr.setSwapBytes(true);
  VSINeedleSpr.pushRotated(&VSImainSpr, VSIAngle, TFT_BLACK);
  VSImainSpr.pushSprite(80, 0);
  VSImainSpr.fillSprite(TFT_BLACK);
}


void Vertical_Speed_Indicator::setVerticalSpeed(float value)
{
    VSIValue = value;
}

void Vertical_Speed_Indicator::setPowerSaveMode(bool enabled)
{
    if(enabled)
    {
        digitalWrite(TFT_BL, LOW);
        powerSaveFlag = true;
    }
    else
    {
        analogWrite(TFT_BL, instrumentBrightness);
        powerSaveFlag = false;
    }
}

void Vertical_Speed_Indicator::setInstrumentBrightnessRatio(float ratio)
{
  instrumentBrightnessRatio = ratio;
  instrumentBrightness = round(scaleValue(instrumentBrightnessRatio, 0, 1, 0, 255));
}

void Vertical_Speed_Indicator::setScreenRotation(int rotation)
{
  if(rotation == 1 || rotation == 3)
    screenRotation = rotation;
}

// Scale function
float Vertical_Speed_Indicator::scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}