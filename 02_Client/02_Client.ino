
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>        
#include <mcp2515.h> 
/****************************************************************************************************/
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
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
/****************************************************************************************************/
// Declaration for CAN bus
MCP2515 mcp2515(10);
struct can_frame canMsg;
byte retval = MCP2515::ERROR_OK;
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

void start_timer(timer_struct *timerX)
{
  if(timerX->st_timer == FALSE)
  {
    timerX->t_startTime = millis();
    timerX->st_timer = TRUE; 
  }
}

void stop_timer(timer_struct *timerX)
{
  timerX->t_startTime = 0;
  timerX->st_timer = FALSE;
}

unsigned long get_timer(timer_struct *timerX)
{
  return(millis() - timerX->t_startTime);
}

 timer_struct timer1;
/****************************************************************************************************/
void setup() 
{
  /* Pin 01 - Red light - Low On */
  /* Pin 13 - build in LED - High On */
  pinMode(13, OUTPUT);
  /****************************************************************************************************/
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  display.drawBitmap(0, 0, myBitmap, 128, 32, WHITE); //Logo display
  display.display();

/****************************************************************************************************/
  SPI.begin();
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS, MCP_16MHZ); 
  mcp2515.setNormalMode();
}

// the loop function runs over and over again forever
void loop() 
{ 
  //Request - CPU time - every 2sec
  start_timer(&timer1);
  if((get_timer(&timer1) > 2000) ||
        (retval != MCP2515::ERROR_OK))
  {
    canMsg.can_id  = 0x7E0;           
    canMsg.can_dlc = 0x03;               
    canMsg.data[0] = 0x22;      
    canMsg.data[1] = 0xF1;   
    canMsg.data[2] = 0x01;            
    canMsg.data[3] = 0x00;
    canMsg.data[4] = 0x00;
    canMsg.data[5] = 0x00;
    canMsg.data[6] = 0x00;
    canMsg.data[7] = 0x00;

    retval = mcp2515.sendMessage(&canMsg);
    stop_timer(&timer1);
  }
 
  /****************************************************************************************************/
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK)
  {
    if((canMsg.can_id, HEX) == 0x7E0)
    {
      if((canMsg.data[0], HEX) == 0x22)
      {
        if(((canMsg.data[1], HEX) == 0xF1) &&
              ((canMsg.data[2], HEX) == 0x01) && 
                  ((canMsg.can_dlc, HEX) == 0x5))
        {
          display.setCursor(0, 0);
          display.println(canMsg.can_id, HEX);
        }        
      }
    }
  }
  else
  {

  }
  display.display();
}
