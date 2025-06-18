## 030625
1.CurTimeDisplyed by *Data: command if wifi is connected.
2.when we set Mode using *SERVER# & *FIXED# it will change immedialtely & stage will change after amber
3. In Fixed mode calculate Red Timings and display
4. When server mode is set, change RED timings  to ATC, change timings after amber
5. When foxed mode is set. chnage timings after amber

## 1170625
1. change baud rate to 9600 for uart 13:18 - 13:20
2. send command every second as *R22,A10,R20,R30# every second 13:20 - 13:40 
3. worked on STM8 code 13:40 - 17:30
4. Change UART command to *SERVER,R22,A10,R20,R30#
5. Set time to 0 in servermode

pending

9. Wifi bug (Second ssid not connected)

## 180625
6. In server mode - green not being sent (11:40 - 12:10)
8. time 0 is also sent (12:10 - 12:15)
7. in server mode - red time is calculated once and sent (12:15 - 12:20 )
9. TherE was  difference of 1 second when shifting from anber to next green, solved by doing three for loops - 1 to reduce value, second to calculate new time, third to display
Test - differeng times for green and ambed for all 4 roads in fixed and server mode
tes
