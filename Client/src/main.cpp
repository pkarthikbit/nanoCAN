#include <Arduino.h>
#include <nanoCAN_lib.h>

/***************************************************************************************************/
//Display Menu
String disp_menu[NANOCAN_MENUCOUNT] =
{
  "Wr Time",
  "Wr Alarm",
  "Set Alarm",
  "Clr Alarm",
  "Menu_04",
  "Menu_05",
  "Menu_06",
  "Menu_07",
  "Menu_08",
  "<-Exit"
};

int menu_sel = 1, submenu_sel;
byte clk_HHMMSS[3];
timer_struct timer_nanoCANMenudisp;
timer_struct timer_nanoCANSubMenudisp;

/****************************************************************************************************/
// Declaration for OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
/****************************************************************************************************/
// Declaration for CAN bus
MCP2515 mcp2515(10);
struct can_frame rx_canMsg;
struct can_frame tx_canMsg;

timer_struct timer_0xF101_req;
timer_struct timer_0xF101_resp;

byte retval = MCP2515::ERROR_OK;
boolean tstr_req = FALSE;     //Flag to check for P2 timer. P2* = P2 and no NRC78.
/***************************************************************************************************/
// The rotary switch:
//    * encoder pin CLK
//    * encoder pin DT
// Rotary press button SW
#define pinCLK 2
#define pinDT 4
#define pinSW 3

bool pin1_st, pin2_st, pin1_stOld, pin2_stOld;
byte r_cntr, l_cntr;

/***************************************************************************************************/
bool pressSwt_raise;
bool nanoCAN_pressSwt()
{
  if((!digitalRead(pinSW)) &&
      (pressSwt_raise == false))
  {
    pressSwt_raise = true;
    return true;
  }
  else
  {
    pressSwt_raise = !digitalRead(pinSW);
    return false;
  }
}
/***************************************************************************************************/
int nanoCAN_rotarySwt()
{
  int ret_val;

  pin1_stOld = pin1_st;
  pin2_stOld = pin2_st;
  pin1_st = digitalRead(pinCLK);
  pin2_st = digitalRead(pinDT);

  if(pin1_st < pin2_st)
  {
    r_cntr++;
  }
  else if(pin1_st > pin2_st)
  {
    l_cntr++;
  }

    if(r_cntr > l_cntr)
    {
      //Clockwise
      ret_val = 1;
    }
    else if(r_cntr < l_cntr)
    {
      //Anti-clockwise
      ret_val = -1;
    }
    else
    {
      //Standstill
      ret_val = 0;
    }
    r_cntr = 0;
    l_cntr = 0;

  return ret_val;
}

/***************************************************************************************************/
void nanoCAN_Menu(int rot_key)
{
  start_timer(&timer_nanoCANMenudisp);
  
  if((rot_key != 0) && 
     (get_timer(&timer_nanoCANMenudisp) > 500))
  {
    byte i, j, k;
    menu_sel = menu_sel + rot_key;

    if((menu_sel > 0) && 
        (menu_sel < (NANOCAN_MENUCOUNT - 1)))
    {
      i = menu_sel - 1;
      j = menu_sel;
      k = menu_sel + 1;
    }
    else if((menu_sel == 0) ||
            (menu_sel > (NANOCAN_MENUCOUNT - 1)))
    {
      menu_sel = 0;
      
      i = NANOCAN_MENUCOUNT - 1;
      j = menu_sel; 
      k = menu_sel + 1;
    }
    else if((menu_sel < 0) ||
            (menu_sel == (NANOCAN_MENUCOUNT - 1)))
    {
      menu_sel = NANOCAN_MENUCOUNT - 1;

      i = menu_sel - 1;
      j = menu_sel; 
      k = 0;
    }
  
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.setTextColor(WHITE);
    display.println(disp_menu[i]);
    display.setCursor(0, 10);
    display.setTextColor(BLACK, WHITE);
    display.println(disp_menu[j]);
    display.setCursor(0, 20);
    display.setTextColor(WHITE);
    display.println(disp_menu[k]);
    display.display();
    
    stop_timer(&timer_nanoCANMenudisp);
  }
}

/***************************************************************************************************/
void nanoCAN_SubMenu()
{
  if(menu_sel == 0) //Menu_0 - Wr Time
  {
    switch(submenu_sel)
    {
      case 0:
      {
        //do nothing
      }
      break;
      
      case 1:
      {
        int rot_st = nanoCAN_rotarySwt();

        start_timer(&timer_nanoCANSubMenudisp);

        if((rot_st != 0) &&
            (get_timer(&timer_nanoCANSubMenudisp) > 500))
        {
          if(rot_st> 0)
          {
            clk_HHMMSS[0]++;

            if(clk_HHMMSS[0] > 12)
            {
              clk_HHMMSS[0] = 0;
            }
          }
          else if(rot_st < 0)
          {
            clk_HHMMSS[0]--;

            if(clk_HHMMSS[0] < 0)
            {
              clk_HHMMSS[0] = 12;
            }
          }
          display.clearDisplay();
          display.setCursor(00, 10);
          display.setTextColor(BLACK, WHITE);
          display.println(clk_HHMMSS[0]);
          display.setCursor(00, 20);
          display.println("HH");
          display.display();
          stop_timer(&timer_nanoCANSubMenudisp);
        }
      }
      break;

      case 2:
      {
        int rot_st = nanoCAN_rotarySwt();
        
        stop_timer(&timer_nanoCANSubMenudisp);
        start_timer(&timer_nanoCANSubMenudisp);

        if((rot_st != 0) &&
            (get_timer(&timer_nanoCANSubMenudisp) > 500))
        {
          if(rot_st> 0)
          {
            clk_HHMMSS[1]++;

            if(clk_HHMMSS[1] > 60)
            {
              clk_HHMMSS[1] = 0;
            }
          }
          else if(rot_st < 0)
          {
            clk_HHMMSS[1]--;

            if(clk_HHMMSS[1] < 0)
            {
              clk_HHMMSS[1] = 60;
            }
          }

          display.clearDisplay();
          display.setCursor(00, 10);
          display.setTextColor(WHITE, BLACK);
          display.println(clk_HHMMSS[0]);
          display.setCursor(00, 20);
          display.println("HH");

          display.setCursor(30, 10);
          display.setTextColor(BLACK, WHITE);
          display.println(clk_HHMMSS[1]);
          display.setCursor(30, 20);
          display.println("MM");

          display.display();
          stop_timer(&timer_nanoCANSubMenudisp);
        }
      }
      break;

      case 3:
      {
        int rot_st = nanoCAN_rotarySwt();
        
        stop_timer(&timer_nanoCANSubMenudisp);
        start_timer(&timer_nanoCANSubMenudisp);

        if((rot_st != 0) &&
          (get_timer(&timer_nanoCANSubMenudisp) > 500))
        {
          if(rot_st> 0)
          {
            clk_HHMMSS[2]++;

            if(clk_HHMMSS[2] > 60)
            {
              clk_HHMMSS[2] = 0;
            }
          }
          else if(rot_st < 0)
          {
            clk_HHMMSS[2]--;

            if(clk_HHMMSS[2] < 0)
            {
              clk_HHMMSS[2] = 60;
            }
          }

          display.clearDisplay();
          display.setCursor(00, 10);
          display.setTextColor(WHITE, BLACK);
          display.println(clk_HHMMSS[0]);
          display.setCursor(00, 20);
          display.println("HH");

          display.setCursor(30, 10);
          display.setTextColor(WHITE, BLACK);
          display.println(clk_HHMMSS[1]);
          display.setCursor(30, 20);
          display.println("MM");

          display.setCursor(60, 10);
          display.setTextColor(BLACK, WHITE);
          display.println(clk_HHMMSS[2]);
          display.setCursor(60, 20);
          display.println("SS");

          display.display();
          stop_timer(&timer_nanoCANSubMenudisp);
        }
      } 
      break;

      default:
      {
        display.clearDisplay();
        display.setCursor(00, 10);
        display.setTextColor(WHITE, BLACK);
        display.println(clk_HHMMSS[0]);
        display.setCursor(00, 20);
        display.println("HH");

        display.setCursor(30, 10);
        display.setTextColor(WHITE, BLACK);
        display.println(clk_HHMMSS[1]);
        display.setCursor(30, 20);
        display.println("MM");

        display.setCursor(60, 10);
        display.setTextColor(WHITE, BLACK);
        display.println(clk_HHMMSS[2]);
        display.setCursor(60, 20);
        display.println("SS");

        display.setCursor(0, 30);
        display.setTextColor(BLACK, WHITE);
        display.println(disp_menu[NANOCAN_MENUCOUNT-1]);

        display.display();
        stop_timer(&timer_nanoCANSubMenudisp);
        submenu_sel = 0;
      }
      break;
    }
  }
}

/***************************************************************************************************/
//RDBI - 0xF101
void RDBI_0xF101()
{
  //Request - CPU time - every 2000 ms
  start_timer(&timer_0xF101_req);
  if(((get_timer(&timer_0xF101_req) > 2000) ||
        (retval != MCP2515::ERROR_OK)) &&
        (tstr_req == FALSE))
  {
    tx_canMsg.data[0] = 0x03;         
    tx_canMsg.data[2] = 0xF1;            
    tx_canMsg.data[3] = 0x01;

    retval = mcp2515.sendMessage(&tx_canMsg);
    stop_timer(&timer_0xF101_req);
    start_timer(&timer_0xF101_resp);
    tstr_req = TRUE;
  }

   /****************************************************************************************************/
  if((tstr_req != FALSE) &&
    (get_timer(&timer_0xF101_resp) < 100))
  {
    if (mcp2515.readMessage(&rx_canMsg) == MCP2515::ERROR_OK)
    {   
      if((rx_canMsg.can_id == 0x7E8) &&
          (rx_canMsg.can_dlc == tx_canMsg.can_dlc))
      {
        if(rx_canMsg.data[1] == (tx_canMsg.data[1] + 0x40))
        {
          if((rx_canMsg.data[0] == 0x07) &&
              ((((rx_canMsg.data[2] << 8) & 0xFF00) | 
                ((rx_canMsg.data[3] << 0) & 0x00FF) ) == 0xF101))
          {
            //Serial.println(((rx_canMsg.data[4] << 24) & 0xFF000000) | 
                           // ((rx_canMsg.data[5] << 16) & 0x00FF0000) | 
                           // ((rx_canMsg.data[6] << 8)  & 0x0000FF00) | 
                           // ((rx_canMsg.data[7] << 0)  & 0x000000FF));
          }        
        }
      }
    }
  }
  else
  {
    tstr_req = FALSE;
    stop_timer(&timer_0xF101_resp);   
  }
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
/****************************************************************************************************/
  pinMode(pinCLK, INPUT);
  pinMode(pinDT, INPUT);
  pinMode(pinSW, INPUT);

}

// the loop function runs over and over again forever
void loop() 
{ 
  if(submenu_sel == 0)
  {
    nanoCAN_Menu(nanoCAN_rotarySwt());
  }
/****************************************************************************************************/  
  if(nanoCAN_pressSwt())
  {
    submenu_sel++;
  } 
  Serial.println(submenu_sel);

  nanoCAN_SubMenu();

  tx_canMsg.can_id  = 0x7E0;           
  tx_canMsg.can_dlc = 0x08; 

  //RDBI request
  tx_canMsg.data[1] = 0x22;
  tx_canMsg.data[4] = 0x00;
  tx_canMsg.data[5] = 0x00;
  tx_canMsg.data[6] = 0x00;
  tx_canMsg.data[7] = 0x00;

  RDBI_0xF101();
/****************************************************************************************************/
}
