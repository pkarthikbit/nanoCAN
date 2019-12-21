# nanoCAN
Server Client implementation with Arduino Nano + mcp2515 can bus module + 0,96" oled i2c 
non RTOS, C++, UDS protocol used

Server <-----CAN-----> Client

Server has RTC.
Client has rotary switch, LED.

Main menu has below options:
1. Exit
2. Wr Time
3. Wr Alarm
4. Set Alarm
5. Clr Alarm
6. Greeting

Selecting Exit displays the time, alarm status and alarm time.

Wr Time, alarm, Set/Clr alarm updates the WDBI to the server.

Real time is read by a RDBI.

P2 timing implemented.

no flowcontrol.

display, CAN, RTC drivers from other GIT hub.

rotary, press switch driver logic is indigenous.

no powersaver implemented as CAN wakeup wasn't working. But when server is disconnected, client display will go off and again on when client is connected.

Being alarm to be fail proof, client send the alarm time and status periodic.

when alarm rings, press button snooze and switches it off.
