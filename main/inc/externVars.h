extern int INHInputValue;
extern int INHOutputValue;
extern int PreviousINHValue;
#define INHIBITLevel 1
extern char WIFI_SSID_1[64];
extern char WIFI_PASS_1[64];
extern char WIFI_SSID_2[64];
extern char WIFI_PASS_2[64];
extern char WIFI_SSID_3[64];
extern char WIFI_PASS_3[64];

extern char server_ip_addr[100];
extern char ipstr[100]; // host mapped

extern char MAC_ADDRESS_ESP[40];
extern char FOTA_URL[356];
extern int16_t server_port;
extern int jumperPort;
extern int16_t caValue;
extern int16_t cashValue;
extern int sock ;

extern int sp_port;
#define ESP_MAXIMUM_RETRY       2
#define ESP_RETRY_GAP           2000
#define Production          1
#define NVS_INH_KEY           "INH"
#define NVS_SSID_1_KEY        "SSID1"
#define NVS_PASS_1_KEY        "PASS1"
#define NVS_SSID_2_KEY        "SSID2"
#define NVS_PASS_2_KEY        "PASS2"
#define NVS_SSID_3_KEY        "SSID3"
#define NVS_PASS_3_KEY        "PASS3"

#define NVS_SERVER_IP_KEY     "SERVER"
#define NVS_SERVER_PORT_KEY   "PORT"
#define NVS_SERVER_PORT_KEY_JUMPER "JUMPERPORT" 
#define NVS_OTA_URL_KEY   "OTA_URL"
#define NVS_CA_KEY          "CA"
#define NVS_CASHTOTAL_KEY   "CASHTOTAL"
#define NVS_VENDTOTAL_KEY   "VENDTOTAL"
#define NVS_CASH1_KEY   "CASH1"
#define NVS_CASH2_KEY   "CASH2"
#define NVS_CASH3_KEY   "CASH3"
#define NVS_CASH4_KEY   "CASH4"
#define NVS_CASH5_KEY   "CASH5"
#define NVS_CASH6_KEY   "CASH6"
#define NVS_CASH7_KEY   "CASH7"

#define DEFAULT_SSID1  "GVCSYS1"
#define DEFAULT_PASS1  "GVC3065V"
#define DEFAULT_SSID2  "GVCSYS2"
#define DEFAULT_PASS2  "GVC3065V"
#define DEFAULT_SSID3  "GVCSYS3"
#define DEFAULT_PASS3  "GVC3065V"


#define NVS_SIP_USERNAME     "USERNAME_SIP"
#define NVS_SIP_DATETIME   "DATETIME_SIP"

#define NVS_CA_USERNAME     "USERNAME_CA"
#define NVS_CA_DATETIME   "DATETIME_CA"

#define NVS_CC_USERNAME     "USERNAME_CC"
#define NVS_CC_DATETIME   "DATETIME_CC"

#define NVS_URL_USERNAME     "USERNAME_URL"
#define NVS_URL_DATETIME   "DATETIME_URL"

#define NVS_FOTA_USERNAME     "USERNAME_FOTA"
#define NVS_FOTA_DATETIME   "DATETIME_FOTA"

#define NVS_RST_USERNAME     "USERNAME_RST"
#define NVS_RST_DATETIME   "DATETIME_RST"

#define NVS_SS_USERNAME     "USERNAME_SS"
#define NVS_SS_DATETIME   "DATETIME_SS"

#define NVS_PW_USERNAME     "USERNAME_PW"
#define NVS_PW_DATETIME   "DATETIME_PW"

#define NVS_SS1_USERNAME     "USERNAME_SS1"
#define NVS_SS1_DATETIME   "DATETIME_SS1"

#define NVS_PW1_USERNAME     "USERNAME_PW1"
#define NVS_PW1_DATETIME   "DATETIME_PW1"

#define NVS_INH_USERNAME     "USERNAME_INH"
#define NVS_INH_DATETIME   "DATETIME_INH"

#define NVS_SP_USERNAME     "USERNAME_SP"
#define NVS_SP_DATETIME   "DATETIME_SP"

#define DEFAULT_SERVER_IP_ADDR_TRY "gvc.co.in"
#define DEFAULT_SERVER_IP_ADDR "gvc.co.in"
#define DEFAULT_SERVER_PORT    6666
#define DEFAULT_FOTA_URL  "http://gvc.co.in/esp/firmware.bin"
#define FWVersion "*GVCSYS-27JUNE24T5#"
#define HBTDelay    300000
#define LEDR    13
#define LEDG    12

#define JUMPER  15
#define JUMPER2  18


#define ErasePin 0
#define ICH1    33
#define ICH2    32
#define ICH3    35
#define ICH4    34
#define ICH5    26
#define ICH6    27
#define ICH7    25
#define INH     23

#define STRB    19
#define CLK     22
#define DAT    21

#define CINHO   14
#define CINHI   23

#define L1      2
#define L2      5
#define L3      4
#define LedHBT  L1
#define LedTCP  L2

#define SDA     21
#define SCL     22

#define MKM_IC_UART UART_NUM_2
#define MKM_IC_UART_TX 17
#define MKM_IC_UART_RX 16


// values used in erase pin
extern bool ErasePinStatus,LastErasePinStatus;
extern int ErasePinDebounce;

// valuses used in CA command
extern int numValue;
extern int polarity;
extern int pulseWitdh;
extern int SignalPolarity;
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

extern char SSdateTime[100];
extern char SSuserName[100];

extern char PWdateTime[100];
extern char PWuserName[100];

extern char SS1dateTime[100];
extern char SS1userName[100];

extern char PW1dateTime[100];
extern char PW1userName[100];

extern char INHdateTime[100];
extern char INHuserName[100];

extern char SPdateTime[100];
extern char SPuserName[100];






extern unsigned char  SwitchStatus,PreviousSwitchStatus,DebounceCount;
extern int16_t x;
extern unsigned char Test4094Count;

// used in Check input sense pins
extern uint16_t Counter,InputPin,LastValue,LastInputPin;


extern uint32_t current_interval;
extern uint32_t numberOfPulses;
extern uint16_t TotalPulses; // total received pulses for coin input
extern uint16_t PulseStoppedDelay; // time delay when it is assumed that pulses end
// uint16_t PulseCount =0; // for received pulses for coin input
// uint16_t PulseTimeOut = 0;  // received pulses time out time in ticks* delay

extern uint32_t ticks_100;
extern int LastTID;
extern int TID; // Transaction ID
extern int pin; // output pin for Generating Pulses
extern int pulses; // number of pulses for generating pulses on output pin
extern int edges; // number of edges (pulses * 2) for generating pulses on output pin
extern int Totals[7];
extern int CashTotals[7];
extern int ledpin;
extern int ledstatus;
extern int blinkLEDNumber;

#define EX_UART_NUM UART_NUM_2
#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)
extern  QueueHandle_t uart0_queue;


extern nvs_handle_t utils_nvs_handle;
