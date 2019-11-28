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

byte disp_submenu[60], req_CmnTime[3], req_time[3], req_alarm[3], rcd_time[3]= {11, 12, 13};
byte NANOCAN_SUBMENUDGTCNT[3] = {24, 60, 60};

int menu_sel = -1, submenu_sel = -1, submenuopt_sel = 1;
bool submenuopt_bypass = false, alarm_st = false;
char clk_HHMMSS[3];

timer_struct timer_pressSwt;
timer_struct timer_nanoCANMenudisp;
timer_struct timer_nanoCANSubMenudisp;
timer_struct timer_mainclk;

/****************************************************************************************************/
// Declaration for OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
/****************************************************************************************************/
// Declaration for CAN bus
MCP2515 mcp2515(10);
struct can_frame rx_canMsg;
struct can_frame tx_canMsg;

timer_struct timer_0xF101_req, timer_0xF101_resp;
timer_struct timer_0xF101Wr_req, timer_0xF101Wr_resp;
timer_struct timer_0xF102_req, timer_0xF102_resp;
timer_struct timer_0xF103_req, timer_0xF103_resp;

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
bool nanoCAN_pressSwt()
{
  if((!digitalRead(pinSW)) &&
      (get_timer(&timer_pressSwt) == millis()))
  {
    start_timer(&timer_pressSwt);
    return true;
  }
  else
  {
    if(get_timer(&timer_pressSwt) > 500)
    {
      stop_timer(&timer_pressSwt);
    }
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

    menu_sel = j;
    
    stop_timer(&timer_nanoCANMenudisp);
  }
  else
  {
    start_timer(&timer_mainclk);

    if((get_timer(&timer_mainclk) > 1000) &&
        (menu_sel == NANOCAN_MENUCOUNT -1))
    {
      display.clearDisplay();
      display.setTextSize(3);

      for(byte x=0; x < 3; x++)
      {
        display.setCursor((0 + (40 * x)), 00);
        display.setTextColor(WHITE);
        display.print(rcd_time[x]);
        if(x != 2)
        {
          display.print(":");
        }
      }

      display.setTextSize(1);
      display.setCursor(30, 25);
      display.setTextColor(BLACK, WHITE);
      if(alarm_st)
      {
        display.print("Alarm On");
      }
      else
      {
        display.print("Alarm Off");
      }
      

      display.display();
      stop_timer(&timer_mainclk);
    }
  }
  
}

/***************************************************************************************************/
void nanoCAN_SubMenu(int rot_key)
{ 
  start_timer(&timer_nanoCANSubMenudisp);
  
  if((submenuopt_bypass != false) ||
    ((rot_key != 0) && 
     (get_timer(&timer_nanoCANSubMenudisp) > 500)))
  {
    if(submenuopt_bypass != false)
    {
      submenuopt_bypass = false;
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.setTextColor(WHITE);
    display.println(disp_menu[menu_sel]);
    display.setCursor(0, 20);
    display.setTextColor(WHITE);
    display.println(disp_menu[NANOCAN_MENUCOUNT - 1]);

    for(byte x=0; x < submenu_sel; x++)
    {
      display.setCursor((60 + (20 * x)), 10);
      display.setTextColor(WHITE);
      display.println(req_CmnTime[x]);
    }
    
    byte i, j, k;
    submenuopt_sel = submenuopt_sel + rot_key;

    if((submenuopt_sel > 0) && 
        (submenuopt_sel < (NANOCAN_SUBMENUDGTCNT[submenu_sel] - 1)))
    {
      i = submenuopt_sel - 1;
      j = submenuopt_sel;
      k = submenuopt_sel + 1;
    }
    else if((submenuopt_sel == 0) ||
            (submenuopt_sel > (NANOCAN_SUBMENUDGTCNT[submenu_sel] - 1)))
    {
      submenuopt_sel = 0;
      
      i = NANOCAN_SUBMENUDGTCNT[submenu_sel] - 1;
      j = submenuopt_sel; 
      k = submenuopt_sel + 1;
    }
    else if((submenuopt_sel < 0) ||
            (submenuopt_sel == (NANOCAN_SUBMENUDGTCNT[submenu_sel] - 1)))
    {
      submenuopt_sel = NANOCAN_SUBMENUDGTCNT[submenu_sel] - 1;

      i = submenuopt_sel - 1;
      j = submenuopt_sel; 
      k = 0;
    }

    display.setCursor((60 + (20 * submenu_sel)), 0);
    display.setTextColor(WHITE);
    display.println(disp_submenu[i]);
    display.setCursor((60 + (20 * submenu_sel)), 10);
    display.setTextColor(BLACK, WHITE);
    display.println(disp_submenu[j]);
    display.setCursor((60 + (20 * submenu_sel)), 20);
    display.setTextColor(WHITE);
    display.println(disp_submenu[k]);
    display.display();

    submenuopt_sel = j;
    req_CmnTime[submenu_sel] = disp_submenu[submenuopt_sel];

    stop_timer(&timer_nanoCANSubMenudisp);
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
          if((rx_canMsg.data[0] == 0x06) &&
              ((((rx_canMsg.data[2] << 8) & 0xFF00) | 
                ((rx_canMsg.data[3] << 0) & 0x00FF) ) == 0xF101))
          {
            rcd_time[0] = rx_canMsg.data[4];
            rcd_time[1] = rx_canMsg.data[5];
            rcd_time[2] = rx_canMsg.data[6];
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


/***************************************************************************************************/
//WDBI - 0xF101
void WDBI_0xF101()
{
  //Request - CPU time - every 2000 ms
  start_timer(&timer_0xF101Wr_req);
  if(((get_timer(&timer_0xF101Wr_req) > 2000) ||
        (retval != MCP2515::ERROR_OK)) &&
        (tstr_req == FALSE))
  {
    tx_canMsg.data[0] = 0x06;         
    tx_canMsg.data[2] = 0xF1;            
    tx_canMsg.data[3] = 0x01;
    tx_canMsg.data[4] = req_time[0];
    tx_canMsg.data[5] = req_time[1];
    tx_canMsg.data[6] = req_time[2];
    tx_canMsg.data[7] = 0x00;

    retval = mcp2515.sendMessage(&tx_canMsg);
    stop_timer(&timer_0xF101Wr_req);
    start_timer(&timer_0xF101Wr_resp);
    tstr_req = TRUE;
  }

   /****************************************************************************************************/
  if((tstr_req != FALSE) &&
    (get_timer(&timer_0xF101Wr_resp) < 100))
  {
    if (mcp2515.readMessage(&rx_canMsg) == MCP2515::ERROR_OK)
    {   
      if((rx_canMsg.can_id == 0x7E8) &&
          (rx_canMsg.can_dlc == tx_canMsg.can_dlc))
      {
        if(rx_canMsg.data[1] == (tx_canMsg.data[1] + 0x40))
        {
          if((rx_canMsg.data[0] == 0x03) &&
              ((((rx_canMsg.data[2] << 8) & 0xFF00) | 
                ((rx_canMsg.data[3] << 0) & 0x00FF) ) == 0xF101))
          {
            //Wr Time success
          }        
        }
      }
    }
  }
  else
  {
    tstr_req = FALSE;
    stop_timer(&timer_0xF101Wr_resp);   
  }
}

/***************************************************************************************************/
//WDBI - 0xF102
void WDBI_0xF102()
{
  //Request - CPU time - every 2000 ms
  start_timer(&timer_0xF102_req);
  if(((get_timer(&timer_0xF102_req) > 2000) ||
        (retval != MCP2515::ERROR_OK)) &&
        (tstr_req == FALSE))
  {
    tx_canMsg.data[0] = 0x06;         
    tx_canMsg.data[2] = 0xF1;            
    tx_canMsg.data[3] = 0x02;
    tx_canMsg.data[4] = req_alarm[0];
    tx_canMsg.data[5] = req_alarm[1];
    tx_canMsg.data[6] = req_alarm[2];
    tx_canMsg.data[7] = 0x00;

    retval = mcp2515.sendMessage(&tx_canMsg);
    stop_timer(&timer_0xF102_req);
    start_timer(&timer_0xF102_resp);
    tstr_req = TRUE;
  }

   /****************************************************************************************************/
  if((tstr_req != FALSE) &&
    (get_timer(&timer_0xF102_resp) < 100))
  {
    if (mcp2515.readMessage(&rx_canMsg) == MCP2515::ERROR_OK)
    {   
      if((rx_canMsg.can_id == 0x7E8) &&
          (rx_canMsg.can_dlc == tx_canMsg.can_dlc))
      {
        if(rx_canMsg.data[1] == (tx_canMsg.data[1] + 0x40))
        {
          if((rx_canMsg.data[0] == 0x03) &&
              ((((rx_canMsg.data[2] << 8) & 0xFF00) | 
                ((rx_canMsg.data[3] << 0) & 0x00FF) ) == 0xF102))
          {
            //Wr Alarm success
          }        
        }
      }
    }
  }
  else
  {
    tstr_req = FALSE;
    stop_timer(&timer_0xF102_resp);   
  }
}

/***************************************************************************************************/
//WDBI - 0xF103
void WDBI_0xF103()
{
  //Request - CPU time - every 2000 ms
  start_timer(&timer_0xF103_req);
  if(((get_timer(&timer_0xF103_req) > 2000) ||
        (retval != MCP2515::ERROR_OK)) &&
        (tstr_req == FALSE))
  {
    tx_canMsg.data[0] = 0x04;         
    tx_canMsg.data[2] = 0xF1;            
    tx_canMsg.data[3] = 0x03;
    tx_canMsg.data[4] = alarm_st;
    tx_canMsg.data[5] = 0x00;
    tx_canMsg.data[6] = 0x00;
    tx_canMsg.data[7] = 0x00;

    retval = mcp2515.sendMessage(&tx_canMsg);
    stop_timer(&timer_0xF103_req);
    start_timer(&timer_0xF103_resp);
    tstr_req = TRUE;
  }

   /****************************************************************************************************/
  if((tstr_req != FALSE) &&
    (get_timer(&timer_0xF103_resp) < 100))
  {
    if (mcp2515.readMessage(&rx_canMsg) == MCP2515::ERROR_OK)
    {   
      if((rx_canMsg.can_id == 0x7E8) &&
          (rx_canMsg.can_dlc == tx_canMsg.can_dlc))
      {
        if(rx_canMsg.data[1] == (tx_canMsg.data[1] + 0x40))
        {
          if((rx_canMsg.data[0] == 0x03) &&
              ((((rx_canMsg.data[2] << 8) & 0xFF00) | 
                ((rx_canMsg.data[3] << 0) & 0x00FF) ) == 0xF103))
          {
            //Wr Alarm set/ clr success
          }        
        }
      }
    }
  }
  else
  {
    tstr_req = FALSE;
    stop_timer(&timer_0xF102_resp);   
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
/****************************************************************************************************/
  for(byte i = 0; i < 60; i++)
  {
    disp_submenu[i] = i;
  }
}

// the loop function runs over and over again forever
void loop() 
{ 
  if(submenu_sel == -1)
  {
    nanoCAN_Menu(nanoCAN_rotarySwt());
  }
/****************************************************************************************************/  
  if(nanoCAN_pressSwt())
  {
    submenu_sel++;
    if(submenu_sel >= 3)
    {
      submenu_sel = -1;

      if(menu_sel == 0)
      {
        for(byte x=0; x <3; x++)
        {
          req_time[x] = req_CmnTime[x];
        }
      }
      else if(menu_sel == 1)
      {
        for(byte x=0; x <3; x++)
        {
          req_alarm[x] = req_CmnTime[x];
        }
      }

      for(byte x=0; x <3; x++)
      {
        Serial.print(req_CmnTime[x]);
        Serial.print(":");
      }
    }
  } 

  if(submenu_sel >= 0)
  {
    if((menu_sel == 0) ||
        (menu_sel == 1))
    {
      submenuopt_bypass = true;
      nanoCAN_SubMenu(nanoCAN_rotarySwt());
    }
    else if(menu_sel == 2)
    {
      alarm_st = true;
      submenu_sel = -1;
    }
    else if(menu_sel == 3)
    {
      alarm_st = false;
      submenu_sel = -1;
    }
    else
    {
      submenu_sel = -1;
    }
    
  }

  tx_canMsg.can_id  = 0x7E0;           
  tx_canMsg.can_dlc = 0x08; 

  //RDBI request
  tx_canMsg.data[1] = 0x22;
  tx_canMsg.data[4] = 0x00;
  tx_canMsg.data[5] = 0x00;
  tx_canMsg.data[6] = 0x00;
  tx_canMsg.data[7] = 0x00;

  RDBI_0xF101();

  //WDBI request
  tx_canMsg.data[1] = 0x2E;
  WDBI_0xF101();
  WDBI_0xF102();
  WDBI_0xF103();
/****************************************************************************************************/
}
