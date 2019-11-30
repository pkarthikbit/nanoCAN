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

tmElements_t myTime, newTime, alarm_t;
bool alarm_st, alarmOn_st;
timer_struct timer_alarm;
/****************************************************************************************************/

void RDBI_0xF101(struct can_frame *fill_canMsg)
{             
  fill_canMsg->data[0] = 0x06;

  RTC.read(myTime);
      
  fill_canMsg->data[4] = myTime.Hour;
  fill_canMsg->data[5] = myTime.Minute;
  fill_canMsg->data[6] = myTime.Second;
  fill_canMsg->data[7] = 0;
}

void WDBI_0xF101(struct can_frame *fill_canMsg)
{             
  newTime.Hour   = fill_canMsg->data[4];
  newTime.Minute = fill_canMsg->data[5];
  newTime.Second = fill_canMsg->data[6];

  RTC.write(newTime);
}

void WDBI_0xF102(struct can_frame *fill_canMsg)
{             
  alarm_t.Hour   = fill_canMsg->data[4];
  alarm_t.Minute = fill_canMsg->data[5];
  alarm_t.Second = fill_canMsg->data[6];
}

void WDBI_0xF103(struct can_frame *fill_canMsg)
{             
  alarm_st = fill_canMsg->data[4];
}

void setup() {
  /* Pin 01 - Red light - Low On */
  /* Pin 13 - build in LED - High On */
  pinMode(01, OUTPUT);
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
        
        retval = mcp2515.sendMessage(&tx_canMsg);
  
        if(retval == MCP2515::ERROR_OK)
        {
          //Serial.println("Time reply success");
        }   
      }
      //WDBI received
      else if(rx_canMsg.data[1] == 0x2E)
      {
        tx_canMsg.data[1] = (rx_canMsg.data[1] + 0x40);      
  
        //RDBI - 0xF101 requested
        if(((((rx_canMsg.data[2] << 8) & 0xFF00) | 
            ((rx_canMsg.data[3] << 0) & 0x00FF) ) == 0xF101) &&
            (rx_canMsg.data[0] == 0x06))
        {
          tx_canMsg.data[0] = 0x03;
          tx_canMsg.data[2] = rx_canMsg.data[2];   
          tx_canMsg.data[3] = rx_canMsg.data[3]; 
          
          WDBI_0xF101(&rx_canMsg);

          retval = mcp2515.sendMessage(&tx_canMsg);

          if(retval == MCP2515::ERROR_OK)
          {
            Serial.println("Time got success");
          } 
        }       
        else if(((((rx_canMsg.data[2] << 8) & 0xFF00) | 
            ((rx_canMsg.data[3] << 0) & 0x00FF) ) == 0xF102) &&
            (rx_canMsg.data[0] == 0x06))
        {
          tx_canMsg.data[0] = 0x03;
          tx_canMsg.data[2] = rx_canMsg.data[2];   
          tx_canMsg.data[3] = rx_canMsg.data[3]; 
          
          WDBI_0xF102(&rx_canMsg);

          retval = mcp2515.sendMessage(&tx_canMsg);

          if(retval == MCP2515::ERROR_OK)
          {
            Serial.println("Alarm got success");
          } 
        }       
        else if(((((rx_canMsg.data[2] << 8) & 0xFF00) | 
            ((rx_canMsg.data[3] << 0) & 0x00FF) ) == 0xF103) &&
            (rx_canMsg.data[0] == 0x04))
        {
          tx_canMsg.data[0] = 0x03;
          tx_canMsg.data[2] = rx_canMsg.data[2];   
          tx_canMsg.data[3] = rx_canMsg.data[3]; 
          
          WDBI_0xF103(&rx_canMsg);

          retval = mcp2515.sendMessage(&tx_canMsg);

          if(retval == MCP2515::ERROR_OK)
          {
            Serial.println("Alarm set/clr success");
          } 
        }       
      }
    }
  }

  if(alarm_st)
  {
    RTC.read(myTime);

    if(((alarm_t.Hour == myTime.Hour) &&
        (alarm_t.Minute == myTime.Minute) &&
          (myTime.Second > alarm_t.Second)) ||
          (alarmOn_st))
    {
      alarmOn_st = true;
      start_timer(&timer_alarm);
      
      if(get_timer(&timer_alarm) <= 1000)
      {
        digitalWrite(1, LOW);   // turn the LED on (HIGH is the voltage level)
      }
      else if((get_timer(&timer_alarm) > 1000) &&
         (get_timer(&timer_alarm) < 2000))
      {
        digitalWrite(1, HIGH); 
      }
      else
      {
        stop_timer(&timer_alarm);
      }
    }
  }
  else
  {
    alarmOn_st = false;
    stop_timer(&timer_alarm);
  }
}
