#include "defs.h"

int HardwareTestMode = 0;
int HardwareTestCount = 0;
int rx_event_pending = 0;
int tx_event_pending = 0;
int sp_port;
int INHInputValue = 0;
int INHOutputValue = 0;
int PreviousINHValue = 0;
int ServerRetryCount = 0;
char WIFI_SSID_1[64];
char WIFI_PASS_1[64];
char WIFI_SSID_2[64];
char WIFI_PASS_2[64];
char WIFI_SSID_3[64];
char WIFI_PASS_3[64];

char server_ip_addr[100];
char ipstr[100]; // host mapped

char MAC_ADDRESS_ESP[40];
char FOTA_URL[356];
int16_t server_port;
int jumperPort;
int16_t caValue;
int16_t cashValue;
int sock = -1;

// values used in erase pin
bool ErasePinStatus,LastErasePinStatus,Jumper2Status,LastJumper2Status;
int ErasePinDebounce,Jumper2Debounce;

// valuses used in CA command
int numValue=0;
int polarity=0;
int pulseWitdh=0;
int SignalPolarity=0;
char UniqueTimeStamp[100];

char dateTime[100];
char userName[100];

char SIPdateTime[100];
char SIPuserName[100];

char CAdateTime[100];
char CAuserName[100];

char CCdateTime[100];
char CCuserName[100];

char URLdateTime[100];
char URLuserName[100];

char FOTAdateTime[100];
char FOTAuserName[100];

char RSTdateTime[100];
char RSTuserName[100];

char SNdateTime[100];
char SNuserName[100];

char SSdateTime[100];
char SSuserName[100];

char SLdateTime[100];
char SLuserName[100];

char PWdateTime[100];
char PWuserName[100];

char SS1dateTime[100];
char SS1userName[100];

char PW1dateTime[100];
char PW1userName[100];

char SS2dateTime[100];
char SS2userName[100];

char PW2dateTime[100];
char PW2userName[100];

char INHdateTime[100];
char INHuserName[100];

char SPdateTime[100];
char SPuserName[100];

char ERASEdateTime[100];
char ERASEuserName[100];

char PTdateTime[100];
char PTuserName[100];

char PassThruValue[100];

char SerialNumber[100];

char LastTID[100];
char TID[100];

char ErasedSerialNumber[100];

char QrString[100];

int MQTTRequired;






unsigned char  SwitchStatus,PreviousSwitchStatus,DebounceCount;
int16_t x;
unsigned char Test4094Count;

// used in Check input sense pins
uint16_t Counter,InputPin,LastValue,LastInputPin;


uint32_t current_interval = 0;
int numberOfPulses = 0;
uint16_t TotalPulses = 0; // total received pulses for coin input
uint16_t PulseStoppedDelay = 0; // time delay when it is assumed that pulses end
// uint16_t PulseCount =0; // for received pulses for coin input
// uint16_t PulseTimeOut = 0;  // received pulses time out time in ticks* delay

uint32_t ticks_100 =0;;

int pin = 0; // output pin for Generating Pulses
int pulses = 0; // number of pulses for generating pulses on output pin
int edges = 0; // number of edges (pulses * 2) for generating pulses on output pin
int Totals[7];
int CashTotals[7];
int ledpin = 0;
int ledstatus = 0;
int blinkLEDNumber = 0;
int SipNumber=0;

Led_State_t led_state = STANDBY_LED;
TCPIP_Socket_State socket_state;
bool connected_to_wifi_and_internet = false;
bool LED4TCPPacket = 0;
int WiFiNumber = 0;
int WifiX=1;
int serverStatus=0;
int fotaStatus=0;


int tcp_sock = -1;

bool pending_tcp_packet = false;
char tcp_packet[200];
uint32_t last_update_led1 = 0;
uint8_t led1_gpio_state = 0;
uint8_t led2_gpio_state = 0;
int AckPulseReceived = 0;


nvs_handle_t utils_nvs_handle;
int ServerHBTTimeOut = 0; // added on 251224