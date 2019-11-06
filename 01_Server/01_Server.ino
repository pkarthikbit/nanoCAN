
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>        
#include <mcp2515.h> 

/****************************************************************************************************/

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/****************************************************************************************************/
MCP2515 mcp2515(10);
struct can_frame canMsg;
word tx_cntr;
byte retval;
/****************************************************************************************************/

// 'index', 128x32px
const unsigned char myBitmap [] PROGMEM = {
};

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
  mcp2515.setBitrate(CAN_125KBPS); 
  mcp2515.setNormalMode();
}

// the loop function runs over and over again forever
void loop() 
{  
  /****************************************************************************************************/
  canMsg.can_id  = ++tx_cntr;           
  canMsg.can_dlc = 8;               
  canMsg.data[0] = 1;      
  canMsg.data[1] = 2;   
  canMsg.data[2] = 0x03;            
  canMsg.data[3] = 0x04;
  canMsg.data[4] = 0x05;
  canMsg.data[5] = 0x06;
  canMsg.data[6] = 0x07;
  canMsg.data[7] = 0x08;
  
  retval = mcp2515.sendMessage(&canMsg);

  // Clear the buffer
  display.clearDisplay();
  
  if(retval == MCP2515::ERROR_OK)
  {
    display.setCursor(0, 0);
    display.println(canMsg.can_id, HEX);

    display.setCursor(30, 0);
    display.println(canMsg.can_dlc, HEX);

    display.setCursor(0, 10);
    display.println(canMsg.data[0], HEX);

    display.setCursor(30, 10);
    display.println(canMsg.data[1], HEX);

    display.setCursor(60, 10);
    display.println(canMsg.data[2], HEX);

    display.setCursor(90, 10);
    display.println(canMsg.data[3], HEX);

    display.setCursor(0, 20);
    display.println(canMsg.data[4], HEX);

    display.setCursor(30, 20);
    display.println(canMsg.data[5], HEX);

    display.setCursor(60, 20);
    display.println(canMsg.data[6], HEX);

    display.setCursor(90, 20);
    display.println(canMsg.data[7], HEX);
  }
  else
  {
    display.setCursor(0, 0);
    display.println("No Tx");
  }

  if(tx_cntr > 0x200)
  {
    display.display();
  }
  delay(50);
}
