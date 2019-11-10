#include <C:\workspace\03_Source_code\nanoCAN_lib\nanoCAN_lib.h>

/****************************************************************************************************/
// Declaration for OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
/****************************************************************************************************/
// Declaration for CAN bus
MCP2515 mcp2515(10);
struct can_frame rx_canMsg;
struct can_frame tx_canMsg;
timer_struct timer1;
byte retval = MCP2515::ERROR_OK;

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

  // Clear the buffer
  display.clearDisplay();
/****************************************************************************************************/
  SPI.begin();
  mcp2515.reset();
  mcp2515.setBitrate(CAN_125KBPS, MCP_16MHZ); 
  mcp2515.setNormalMode();
}

// the loop function runs over and over again forever
void loop() 
{ 
  display.setCursor(0, 0);
  display.println("Client");
  display.display();

  tx_canMsg.can_id  = 0x7E0;           
  tx_canMsg.can_dlc = 0x08; 
  tx_canMsg.data[1] = 0x22;
  tx_canMsg.data[4] = 0x00;
  tx_canMsg.data[5] = 0x00;
  tx_canMsg.data[6] = 0x00;
  tx_canMsg.data[7] = 0x00;

  //Request - CPU time - every 2000 ms
  start_timer(&timer1);
  if((get_timer(&timer1) > 2000) ||
        (retval != MCP2515::ERROR_OK))
  {
    tx_canMsg.data[0] = 0x03;         
    tx_canMsg.data[2] = 0xF1;            
    tx_canMsg.data[3] = 0x01;

    retval = mcp2515.sendMessage(&tx_canMsg);
    stop_timer(&timer1);

    display.setCursor(0, 10);
    display.println("Tx success");
    display.display();
  }
 
  /****************************************************************************************************/
  if (mcp2515.readMessage(&rx_canMsg) == MCP2515::ERROR_OK)
  {
    if((rx_canMsg.can_id == 0x7E8) &&
        (rx_canMsg.can_dlc == tx_canMsg.can_dlc))
    {
      if(rx_canMsg.data[1] == (tx_canMsg.data[1] + 0x40))
      {
        if((rx_canMsg.data[0] == 0x07) &&
           (((rx_canMsg.data[2] << 8) & 0xFF00) | 
            ((rx_canMsg.data[3] << 0) & 0x00FF) ) == 0xF101)
        {
          // Clear the buffer
          display.clearDisplay();
          
          display.setCursor(0, 20);
          display.println(((rx_canMsg.data[4] << 24) & 0xFF000000) | 
                          ((rx_canMsg.data[5] << 16) & 0x00FF0000) | 
                          ((rx_canMsg.data[6] << 8)  & 0x0000FF00) | 
                          ((rx_canMsg.data[7] << 0)  & 0x000000FF));
          display.display();
        }        
      }
    }
  }
}
