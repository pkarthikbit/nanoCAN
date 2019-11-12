#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>        
#include <mcp2515.h> 

// General Declaration
#define FALSE 0
#define TRUE !FALSE
/****************************************************************************************************/

// Declaration for OLED
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
/****************************************************************************************************/

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
/****************************************************************************************************/

// Declaration for LOGO
const unsigned char myBitmap [] PROGMEM = {
};
/****************************************************************************************************/

// Declaration for Timer
struct timer_struct
{
  boolean st_timer;
  unsigned long t_startTime;
};

//Start timer
void start_timer(timer_struct *timerX)
{
  if(timerX->st_timer == FALSE)
  {
    timerX->t_startTime = millis();
    timerX->st_timer = TRUE; 
  }
}

//Stop timer
void stop_timer(timer_struct *timerX)
{
  timerX->t_startTime = 0;
  timerX->st_timer = FALSE;
}

//Get timer
unsigned long get_timer(timer_struct *timerX)
{
  return(millis() - timerX->t_startTime);
}
/****************************************************************************************************/
