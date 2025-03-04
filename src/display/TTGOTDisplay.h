#ifndef TTGOTDISPLAY_H_
#define TTGOTDISPLAY_H_

/*********************************************************************
 This is programming code for LilyGo ESP32 T-Display S3 (170x320)
      see: https://github.com/Xinyuan-LilyGO/T-Display-S3

 The code uses heavily the supplied: 

  TFT_eSPI Display library
      see: https://github.com/Bodmer/TFT_eSPI 

 Many have invested time and resources providing open source code!
 
        MIT license, check LICENSE for more information
        All text must be included in any redistribution
*********************************************************************/

// LILYGO_T_DISPLAY_ESP32_S3 uses ST7789 170*320 TFT with 8 bit paralell bus

#include "IDisplay.h"
static const char* IDISPLAY = "TTGO T-Display 170*320";

// ------------------------------------------------------------------------------------------------
// Necessary libraries for use of TFT display
// User_Setup_Select.h ---> #include <User_Setups/Setup206_LilyGo_T_Display_S3.h> 
#include <TFT_eSPI.h>

// Include Icons and fonts
#include "Orbitron_Medium_20.h"
#include "Orbitron_Bold_32.h"
#include "XBM_Icons.h"

#define MAXVALUES 24

TFT_eSPI TFT = TFT_eSPI();
TFT_eSprite simcline = TFT_eSprite(&TFT);
TFT_eSprite icon = TFT_eSprite(&TFT);
TFT_eSprite gauge = TFT_eSprite(&TFT);
TFT_eSprite hist = TFT_eSprite(&TFT);

// Color palette for grade percentage is taken from HTTPS://wwww.CylingCols.com
const uint16_t colorGradient[21] = { 0xFFFF, 0xFFF9, 0xFFF2, 0xFFEA, 0xFFE4, 0xFFA0, 0xFEA0, // FFF2, FFEA, FFE4
                                     0xFDC0, 0xFCC0, 0xFBA0, 0xFA60, 0xF920, 0xF800, 0xE800,
                                     0xD000, 0xC000, 0xB000, 0x7800, 0x5800, 0x3800, 0x1800 };
unsigned long fillColor = 0;

int values[MAXVALUES+1] = {0};
unsigned long colors[MAXVALUES+1] = {0};

#define MAXBRIGHTNESS 9
const int TFT_BRIGHTNESS[MAXBRIGHTNESS+1]={0,46,47,48,50,55,60,100,175,255};

const bool UP = true;
const bool DOWN = false; 

class TTGOTDisplay : public IDisplay {

private:

// Check if button is pressed and adjust display brightness
// Left-right depends on board orientation, assume USB-connector is pointing UP
void checkButtons(void) {
    if(digitalRead(PIN_BUTTON_1)==0 && digitalRead(PIN_BUTTON_2)==1 ) { // button #1 left pressed    
        //Serial.println("Button 1 pressed -> down!");
        setDisplayBrightness(DOWN);
    }
    if(digitalRead(PIN_BUTTON_2)==0 && digitalRead(PIN_BUTTON_1)==1) { // button #2 right pressed
        //Serial.println("Button 2 pressed -> up!");
        setDisplayBrightness(UP);
    }
}

unsigned long getColorGradient(float gradePerc) {
  uint8_t cnt = (uint8_t)round(fabs(gradePerc));
  return colorGradient[cnt];
}

 // Setup GAUGE graph conform Road Inclination of last gradePercentValue -------------------------------------
#define gaugeX    0
#define gaugeY   26
#define gaugeW  170
#define gaugeH  170
#define gaugeIR  47
#define gaugeOR  68

void ShowGaugePresentation(float gradePerc) { 
  gauge.fillSprite(TFT_BLACK);
  gauge.fillSmoothRoundRect(0, 0, gaugeW, gaugeH, 4, TFT_DARKGREY);
  gauge.setFreeFont(&Orbitron_Bold_32); //&Orbitron_Medium_20);
  gauge.fillSmoothCircle(85, 85, gaugeIR, TFT_BLACK, TFT_DARKGREY); 
  gauge.setTextColor(TFT_CYAN, TFT_BLACK, true);
  char tmp[7]; // To be preferred above gauge.drawFloat()
  dtostrf(gradePerc, 5, 1, tmp); 
  gauge.drawString(tmp, 30, 65); //36
  gauge.setTextFont(1);
  gauge.setTextColor(TFT_CYAN, TFT_DARKGREY, true);
  gauge.drawString("0", 5, 80);
  gauge.drawString("10", 80, 5);
  gauge.drawString("20", 156, 80);
  gauge.drawString("-10", 80, 156);
  gauge.drawSmoothArc(85, 85, gaugeOR, gaugeIR, 0, 270, TFT_LIGHTGREY, TFT_DARKGREY); //, true); // NO  markers
  /*
  for(int j=0;j<27;j++){ // every 10 degrees a marker
    if(j<9) 
      gauge.drawSmoothArc(85, 85, gaugeOR, gaugeIR, (j*10), ((j+1)*10)-2, TFT_LIGHTGREY, TFT_DARKGREY); 
      //gauge.drawSmoothArc(85, 85, gaugeOR, gaugeIR, (j*10), ((j+1)*10)-2, getColorGradient((float)(9-j)), TFT_DARKGREY);
    else
      gauge.drawSmoothArc(85, 85, gaugeOR, gaugeIR, (j*10), ((j+1)*10)-2, TFT_LIGHTGREY, TFT_DARKGREY);
      //gauge.drawSmoothArc(85, 85, gaugeOR, gaugeIR, (j*10), ((j+1)*10)-2, getColorGradient((float)(j-8)), TFT_DARKGREY);
  }
  */
  /*
  for(int j=0;j<3;j++){ // every 90 degrees a marker
    gauge.drawSmoothArc(85, 85, gaugeOR, 45, (j*90), ((j+1)*90)-2, TFT_LIGHTGREY, TFT_DARKGREY);
  }
  */
  fillColor = getColorGradient(gradePerc);
  int steps = (int)round(gradePerc)*9; // 180/20 --> 9 degree per 1 step percent value
  if(steps == 0) { // Special attention to level plus or minus zero, show in both directions!
    gauge.drawSmoothArc(85, 85, gaugeOR, gaugeIR, 80, 100, fillColor, TFT_DARKGREY); //, true);
  } else if(steps > 0) {
      gauge.drawSmoothArc(85, 85, gaugeOR, gaugeIR, 90, 90+steps, fillColor, TFT_DARKGREY); //, true);
      //gauge.drawSmoothArc(85, 85, 50, gaugeIR, 90, 90+steps, TFT_BLACK, TFT_DARKGREY);
    } else {
      gauge.drawSmoothArc(85, 85, gaugeOR, 47, 90+steps, 90, fillColor, TFT_DARKGREY); //, true);
      //gauge.drawSmoothArc(85, 85, 50, gaugeIR, 90+steps, 90, TFT_BLACK, TFT_DARKGREY); //, true);
    } 
  for(int i=0;i<20;i++) { // Legenda + pointer
    gauge.fillRect( (150), (160-i*3), 10, 3, getColorGradient((float)i) );
    if( i==(int)round(fabs(gradePerc)) ) {
      gauge.drawWedgeLine((135), (160-i*3), (145), (160-i*3), 3, 1, TFT_CYAN, TFT_DARKGREY);
    }
  }
  gauge.pushSprite(gaugeX, gaugeY);
  yield(); // Mind background tasks
}

  // Setup Histogram/Road Graph of 24 previous readings -------------------------------------------------------
#define histX    0
#define histY  200 
#define histW  170
#define histH  120 

void ShowRoadProfile(float gradePerc) {
  values[MAXVALUES]=gradePerc;  // Fill the last values with gradePercentValue value 
  fillColor = getColorGradient(gradePerc);
  colors[MAXVALUES]=fillColor;          // Fill last colors with gradient color
  int avValues = 0;                     // define average
  // Shift values to the left, the oldest value[0] falls off
  for(int i=0;i<MAXVALUES;i++) {
    values[i]=values[i+1];
    avValues += values[i]; // Sum the values
    colors[i]=colors[i+1];
  }
  avValues = abs(avValues)/MAXVALUES; // take average of sum
  //--------------------------------------------------
  hist.fillSprite(TFT_BLACK);
  hist.fillSmoothRoundRect(0, 0, histW, histH, 4, TFT_DARKGREY); 
  hist.setFreeFont(&Orbitron_Medium_20);
  hist.setTextColor(TFT_YELLOW, TFT_DARKGREY, true);
  hist.setTextFont(1);
  // Draw 12 vertical axis every 12 pixels
  for(int i=1;i<13;i++) { 
      hist.drawLine((20)+(i*12), 10, (20)+(i*12), (histH-20), TFT_BLACK);  
    if((i*12)%24==0)
      if(i*2<10)
        hist.drawString("0"+String(i*2), (12)+(i*12), (histH-12)); 
      else
        hist.drawString(String(i*2), (12)+(i*12), (histH-12)); 
  }
  // Draw 4 horizontal axis every 30 pixels bottom up
  for(int i=0;i<4;i++) {
    hist.drawLine((20), (histH-20)-(i*30), histW-7, (histH-20)-(i*30), TFT_BLACK);
    // if(i<4) hist.drawString(String(abs(i*10-10)), (4), (histH-24)-(i*30)); // 4 values
    if(i == 1) hist.drawString(String(0), (4), (histH-24)-(30)); // 0 value
  }
  // Draw vertical Y axis
  hist.drawLine((20), (10), (20), (histH-20), TFT_WHITE);
  //--------------------------------------------------------------------------------------------------
  float factor = 0;  // Factor to stay within screen/graph boundaries
  // Multiply factor is a function of avValues and determines how "high" or "low" the line graph will rise
  if(avValues<=5) factor=23; // 15
    else if(avValues<=10) factor=12; // 8
      else if(avValues<=15) factor=8; // 5
        else if(avValues<=19) factor=6; // 4
          else factor=4;
  float heightVal = 0.0;  // Instantiate vertical height of road profile value
  for(int i=0;i<MAXVALUES;i++) {
    float Radians = atanf(float(values[i])/100);
    heightVal += 2*sin(Radians)*factor; 
    int heightVal1 = round(heightVal); 
    //int heightVal1 = (int)(heightVal < 0 ? (heightVal - 0.5f) : (heightVal + 0.5f)); // apply 'own' rounding of float to int value
    Radians = atanf(float(values[i+1])/100);
    float heightVal2 = heightVal + 2*sin(Radians)*factor; 
    float fstep = (heightVal2-heightVal)/5;
    for(int j=0;j<5;j++) {
      int step = round(j*fstep);
      //int step = (int)(j*fstep < 0 ? (j*fstep - 0.5f) : (j*fstep + 0.5f));  // apply 'own' rounding of float to int value
      if(heightVal>=0){
        hist.drawFastVLine((22+j)+(i*6), (histH-50)-(heightVal1+step), (heightVal1+step), colors[i]);
      } else {
        hist.drawFastVLine((22+j)+(i*6), (histH-49), -(heightVal1+step), colors[i]); // shift below zero axis --> add +1 to Y
      } 
    }
  }
  // Draw histo-Xaxis at zero crossing
  hist.drawLine((20), (histH-50), histW-7, (histH-50), TFT_WHITE);  
  // ----------------------------------------------------------------------------------------------------
  hist.pushSprite(histX, histY);
  yield(); // Mind background tasks
}

public:
TTGOTDisplay() {   
    }

void setDisplayBrightness(bool updown) override {
  static uint8_t count = MAXBRIGHTNESS;
  if(updown) count++;
  else if(count>0) count--;
  if(count>MAXBRIGHTNESS) count = MAXBRIGHTNESS;
  analogWrite(PIN_BACKLIGHT, TFT_BRIGHTNESS[count]);
}

    // Setup Top ICON Bar --------------------------------------------------------------------------------------
#define iconX    0
#define iconY    0
#define iconW  170
#define iconH   22

void ShowIconsOnTopBar(bool isTrainer, bool isLaptop, bool isPhone) override {
  static bool blinkState = true;
  // Show Icons on Top Bar
  icon.fillSprite(TFT_BLACK);
  icon.fillSmoothRoundRect(iconX, iconY, iconW, iconH, 4, TFT_DARKGREY);
  // Show BLE icon during BLE advertising and/or BLE scanning 
  if(!isTrainer || !(isLaptop ^ isPhone)) { 
    // Blink when not all devices are connected
    if(blinkState) icon.drawXBitmap(16, 2, bluetooth_icon16x16, 16, 16, TFT_SKYBLUE, TFT_DARKGREY);
    else icon.drawXBitmap(16, 2, bluetooth_icon16x16, 16, 16, TFT_BLACK, TFT_DARKGREY);
    blinkState = !blinkState;
  }
  if (isTrainer) { // show icon
    icon.drawXBitmap(136, 2, power_icon16x16, 16, 16, TFT_MAGENTA, TFT_DARKGREY);
  }
  if (isLaptop) { // show icon
    icon.drawXBitmap(56, 2, zwift_icon16x16, 16, 16, TFT_ORANGE, TFT_DARKGREY);
  }
  if (isPhone) { // show icon Phone
    icon.drawXBitmap(96, 2, mobile_icon16x16, 16, 16, TFT_WHITE, TFT_DARKGREY);
  }
  icon.pushSprite(iconX, iconY);
  checkButtons();
  yield(); // Mind background tasks
}

#define winX    0
#define winY   69 // centered in Gauge sprite window
#define winW  170
#define winH   84 //3*20 + padding of 6*4 (4 pixels extra space at above Line1, 8 between 2 lines (16) and 4 below Line3

void ShowMessageWindow(const String& Line1, const String& Line2, const String& Line3, uint16_t Pause) override {
  // Clear and set window to display 3 lines of info -> centered
  int posX = 0;
  TFT.fillSmoothRoundRect(winX, winY, winW, winH, 4, TFT_NAVY);
  TFT.drawSmoothRoundRect(winX, winY, 5, 3, winW-1, winH, TFT_LIGHTGREY, TFT_NAVY);
  TFT.setFreeFont(&Orbitron_Medium_20);
  TFT.setTextColor(TFT_YELLOW, TFT_NAVY, true);
  if (Line1) {
    posX = round( (winW - TFT.textWidth(Line1)) / 2 );
    TFT.drawString(Line1, winX+posX, (winY+4) );
  }
  if (Line2) {
    posX = round( (winW - TFT.textWidth(Line2)) / 2 );
    TFT.drawString(Line2, winX+posX, (winY+8)+TFT.fontHeight() );
  }
  if (Line3) {
    posX = round( (winW - TFT.textWidth(Line3)) / 2 );
    TFT.drawString(Line3, winX+posX, (winY+12)+2*TFT.fontHeight() );
  }
  yield(); // Mind background tasks
  if(Pause > 0) vTaskDelay(Pause/portTICK_PERIOD_MS); // Pause indicated time in ms
}

void initDisplay(void) override {
  // Setup T-Display 170x320
  TFT.init();
  TFT.fillScreen(TFT_BLACK);
  TFT.setRotation(0);  // 0 == USB downward 2 == USB Upward facing
  TFT.setSwapBytes(true);  
  // Enable backlight pin, initially on
  pinMode(PIN_BACKLIGHT, OUTPUT);
  analogWrite(PIN_BACKLIGHT, TFT_BRIGHTNESS[MAXBRIGHTNESS]);
  TFT.drawXBitmap(0, 50, mountain170x136, 170, 136, TFT_LIGHTGREY, TFT_BLACK);
  delay(1000);

  icon.createSprite(iconW, iconH);
  icon.setSwapBytes(true);

  gauge.createSprite(gaugeW, gaugeH);
  gauge.setSwapBytes(true);

  hist.createSprite(histW, histH);
  hist.setSwapBytes(true);
 
// ------------------
// dynamic presentation of SIMCLINE                              .
  const String messageStr = "LILYGO T-display S3  > Simcline <";
  TFT.setFreeFont(&Orbitron_Light_24); // Notice this font has kerning !!
  uint16_t MsgPixWidth = TFT.textWidth(messageStr)+10; // add extra kerning pixels to width
  uint16_t MsgPixHeight = TFT.fontHeight() + 6; // add padding pixels !
  uint16_t maxScroll = MsgPixWidth - TFT.width();
  simcline.createSprite(MsgPixWidth, MsgPixHeight); 
  simcline.setSwapBytes(true);
  simcline.fillSprite(TFT_BLACK);
  simcline.setTextColor(TFT_YELLOW, TFT_BLACK, true);
  simcline.setFreeFont(&Orbitron_Light_24);
  simcline.setScrollRect(0, 0, MsgPixWidth, MsgPixHeight, TFT_BLACK); // Set Scroll area
  simcline.drawString(messageStr, 0, 0);
  simcline.pushSprite(0,220); // Show the message and pause
  delay(750);
  for (uint16_t i = 0; i < (maxScroll); i++) {
    simcline.scroll(-1, 0);     // scroll dX text 1 pixel left, dY up/down default is 0
    simcline.pushSprite(0,220);
    delay(25);
  }
  simcline.deleteSprite();
  delay(1000);
// ------------------
}

void ShowRoadGrade(float gradePerc) override {
  ShowGaugePresentation(gradePerc);
  ShowRoadProfile(gradePerc);
}

}; // TTGOTDisplay class -----------------------------------------------------------------------

// Create the Display object of type TTGOTDisplay
TTGOTDisplay* pDisplay = new TTGOTDisplay();

#endif // TTGOTDISPLAY_H