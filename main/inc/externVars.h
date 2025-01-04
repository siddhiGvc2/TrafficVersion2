#include "defs.h"

extern int HardwareTestMode;
extern int HardwareTestCount;
extern int INHInputValue;
extern int INHOutputValue;
extern int PreviousINHValue;
extern int ServerRetryCount;

#define INHIBITLevel 1
extern char WIFI_SSID_1[64];
extern char WIFI_PASS_1[64];
extern char WIFI_SSID_2[64];
extern char WIFI_PASS_2[64];
extern char WIFI_SSID_3[64];
extern char WIFI_PASS_3[64];

extern char server_ip_addr[100];
extern char ipstr[100]; // host mapped

extern int MQTTRequired;
extern char MAC_ADDRESS_ESP[40];
extern char FOTA_URL[356];
extern int16_t server_port;
extern int jumperPort;
extern int16_t caValue;
extern int16_t cashValue;
extern int sock ;

extern int sp_port;


// values used in erase pin
extern bool ErasePinStatus,LastErasePinStatus,Jumper2Status,LastJumper2Status;
extern int ErasePinDebounce,Jumper2Debounce;

// valuses used in CA command
extern int numValue;
extern int polarity;
extern int pulseWitdh;
extern int SignalPolarity;
extern char UniqueTimeStamp[100];

extern char dateTime[100];
extern char userName[100];

extern char SIPdateTime[100];
extern char SIPuserName[100];

extern char CAdateTime[100];
extern char CAuserName[100];

extern char CCdateTime[100];
extern char CCuserName[100];

extern char URLdateTime[100];
extern char URLuserName[100];

extern char FOTAdateTime[100];
extern char FOTAuserName[100];

extern char RSTdateTime[100];
extern char RSTuserName[100];

extern char SNdateTime[100];
extern char SNuserName[100];

extern char SSdateTime[100];
extern char SSuserName[100];

extern char SSdateTime[100];
extern char SSuserName[100];

extern char SLdateTime[100];
extern char SLuserName[100];

extern char PWdateTime[100];
extern char PWuserName[100];

extern char SS1dateTime[100];
extern char SS1userName[100];

extern char PW1dateTime[100];
extern char PW1userName[100];

extern char SS2dateTime[100];
extern char SS2userName[100];

extern char PW2dateTime[100];
extern char PW2userName[100];

extern char INHdateTime[100];
extern char INHuserName[100];

extern char SPdateTime[100];
extern char SPuserName[100];

extern char ERASEdateTime[100];
extern char ERASEuserName[100];

extern char PTdateTime[100];
extern char PTuserName[100];

extern char PassThruValue[100];

extern char SerialNumber[100];

extern char LastTID[100];
extern char TID[100];

extern char ErasedSerialNumber[100];
extern char QrString[100];




extern unsigned char  SwitchStatus,PreviousSwitchStatus,DebounceCount;
extern int16_t x;
extern unsigned char Test4094Count;

// used in Check input sense pins
extern uint16_t Counter,InputPin,LastValue,LastInputPin;


extern uint32_t current_interval;
extern int numberOfPulses;
extern uint16_t TotalPulses; // total received pulses for coin input
extern uint16_t PulseStoppedDelay; // time delay when it is assumed that pulses end
// uint16_t PulseCount =0; // for received pulses for coin input
// uint16_t PulseTimeOut = 0;  // received pulses time out time in ticks* delay

extern uint32_t ticks_100;

extern int pin; // output pin for Generating Pulses
extern int pulses; // number of pulses for generating pulses on output pin
extern int edges; // number of edges (pulses * 2) for generating pulses on output pin
extern int Totals[7];
extern int CashTotals[7];
extern int ledpin;
extern int ledstatus;
extern int blinkLEDNumber;
extern int SipNumber;

#define EX_UART_NUM UART_NUM_2
#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)
//extern  QueueHandle_t uart0_queue;


extern nvs_handle_t utils_nvs_handle;

extern Led_State_t led_state;
extern TCPIP_Socket_State socket_state;
extern bool connected_to_wifi_and_internet,LED4TCPPacket;
extern int WiFiNumber;
extern int serverStatus;
extern int fotaStatus;

extern int rx_event_pending;
extern int tx_event_pending;

extern bool pending_tcp_packet ;
extern char tcp_packet[200];
extern uint32_t last_update_led1;
extern uint8_t led1_gpio_state;
extern uint8_t led2_gpio_state;

extern int32_t MQTT_CONNEECTED;
extern int AckPulseReceived;
extern int ServerHBTTimeOut;
