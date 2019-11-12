#include <nanoCAN_lib.h>

/****************************************************************************************************/
// Declaration for OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/****************************************************************************************************/
MCP2515 mcp2515(10);
struct can_frame rx_canMsg;
struct can_frame tx_canMsg;

byte retval;
/****************************************************************************************************/
// Init the DS1302
// Set pins:  RST, DATA, CLK
DS1302RTC RTC(7, 8, 9);

tmElements_t myTime;
/****************************************************************************************************/

void RDBI_0xF101(struct can_frame *fill_canMsg)
{
  unsigned long tmr_0xF101 = (millis()/1000);
                 
  fill_canMsg->data[0] = 0x07;
      
  fill_canMsg->data[4] = ((tmr_0xF101 & 0xFF000000) >> 24);
  fill_canMsg->data[5] = ((tmr_0xF101 & 0x00FF0000) >> 16);
  fill_canMsg->data[6] = ((tmr_0xF101 & 0x0000FF00) >> 8);
  fill_canMsg->data[7] = ((tmr_0xF101 & 0x000000FF) >> 0);

  RTC.read(myTime);

  Serial.print(myTime.Hour, DEC);
  Serial.print(':');
  Serial.print(myTime.Minute,DEC);
  Serial.print(':');
  Serial.print(myTime.Second,DEC);
  Serial.print('-');
  Serial.print(myTime.Day,DEC);
  Serial.print('/');
  Serial.print(myTime.Month,DEC);
  Serial.print('/');
  Serial.println(1970 + (myTime.Year,DEC));
}

void setup() {
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

void loop() {
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
      tx_canMsg.can_dlc = rx_canMsg.can_dlc; 

      //RDBI received
      if((rx_canMsg.data[0] == 0x03) && 
                  (rx_canMsg.data[1] == 0x22))
      {
        tx_canMsg.data[1] = (rx_canMsg.data[1] + 0x40);      
  
        //RDBI - 0xF101 requested
        if((((rx_canMsg.data[2] << 8) & 0xFF00) | 
            ((rx_canMsg.data[3] << 0) & 0x00FF) ) == 0xF101)
        {
            tx_canMsg.data[2] = rx_canMsg.data[2];   
            tx_canMsg.data[3] = rx_canMsg.data[3]; 
            RDBI_0xF101(&tx_canMsg);
        }        
      }
    }

    retval = mcp2515.sendMessage(&tx_canMsg);
  
    if(retval == MCP2515::ERROR_OK)
    {
          display.clearDisplay();
          display.setCursor(0, 20);
          display.println(((tx_canMsg.data[4] << 24) & 0xFF000000) | 
                          ((tx_canMsg.data[5] << 16) & 0x00FF0000) | 
                          ((tx_canMsg.data[6] << 8)  & 0x0000FF00) | 
                          ((tx_canMsg.data[7] << 0)  & 0x000000FF));
          display.display();
    }
  }
}