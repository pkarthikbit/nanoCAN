
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
struct can_frame rx_canMsg;
struct can_frame tx_canMsg;
word tx_cntr;
byte retval;
/****************************************************************************************************/

// 'index', 128x32px
const unsigned char myBitmap [] PROGMEM = {
};

void RDBI_0xF101(struct can_frame *fill_canMsg)
{
  word t_0XF101 = millis()/1000;
  
  fill_canMsg->can_dlc = 0x05;               
  fill_canMsg->data[0] = 0x62;      
  fill_canMsg->data[1] = 0xF1;   
  fill_canMsg->data[2] = 0x01;            
  fill_canMsg->data[3] = ((t_0XF101) & 0xFF00);
  fill_canMsg->data[4] = ((t_0XF101) & 0x00FF);
  fill_canMsg->data[5] = 0x00;
  fill_canMsg->data[6] = 0x00;
  fill_canMsg->data[7] = 0x00;
}

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
  delay(1000);

  display.clearDisplay();
/****************************************************************************************************/
  SPI.begin();
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS); 
  mcp2515.setNormalMode();
}

// the loop function runs over and over again forever
void loop() 
{  
  display.setCursor(0, 0);
  display.println("Server"); 
  display.display();
  
  /****************************************************************************************************/
  if (mcp2515.readMessage(&rx_canMsg) == MCP2515::ERROR_OK)
  {
    if(rx_canMsg.can_id == 0x7E0)
    { 
      display.setCursor(0, 10);
      display.println("Rx Success"); 
      display.display();
      
      //Response CANID
      tx_canMsg.can_id  = 0x7E8;

      //RDBI received
      if((rx_canMsg.data[0] == 0x22) && 
                  (rx_canMsg.can_dlc == 0x3))
      {
        //RDBI - 0xF101 requested
        if((rx_canMsg.data[1] == 0xF1) &&
              (rx_canMsg.data[2] == 0x01))
        {
            RDBI_0xF101(&tx_canMsg);
        }        
      }
    }

    retval = mcp2515.sendMessage(&tx_canMsg);
  
    if(retval == MCP2515::ERROR_OK)
    {
          display.clearDisplay();
          display.setCursor(0, 20);
          display.println((tx_canMsg.data[3] & 0xFF00) | (tx_canMsg.data[4] & 0x00FF));
          display.display();
    }
  }
}
