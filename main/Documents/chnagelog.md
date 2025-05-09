## 1.22I - 010525
## Modify
- All MQTT commands to have user name and time stamp similar to TCP commands
#3 020525
-- send TC command when coin inserted / hardware.c
-- created a new function SendTCResponse / mqtt.c
-- created new variable TCPRequired, Send response only if TCPRequired is set / vars. externvars, calls
-- create one more sub topic as GVC/KP/BROADCAST mqtt.c
-- if HBT-OK not received in X minutes and wifi is okay and MQTT is connected the reset the device
-- HBT-D is the command from device and HBT-S is the command from server
-- Pulse wisth is between 20 and 250 msec / hardware.c

#4 030525
-- stack for publish_mqtt task increased from 5k to 8k
#5 050525
-- do not reply when *HBT-S# is received / command.c
-- QOS option added on Subscribe
-- compare topic with broadcast as well serial number to allow action
-- ??? some times data not being senses by MQTT
-- ??? on 3rd May 2:52:00 pm one pulse missed while generating or receiving
-- blink leds when MQTT received moved 3 lines from analysePacketTcp.c to command.c

## 1.23- 090525
#1 090525
-- Add setting for MQTT_BROKER1, MQTT_BROKER2, MQTT_BROKER3. changed in /main/inc/defs
-- Change mqtt server as per *SIP:x# changed in /main/src/SaveRecallNVS.c
-- *MQTT:user:password# command added & saved in memory  changed in /main/src/commands.c
-- sent TC-D at sendTCresponse  changed in /main/src/mqttRoutine.c
-- *MQTT?# command added changed in /main/src/commands/c
--- & saved MQTT default user & pASSWORD values in memory. changed in/main/src/SaveRecallNvs.c
-- remove comparison of topic with expected topic  case MQTT_EVENT_DATA:/mqttRoutines.c
-- when INH input changes, send TCP/MQTT/UARt



