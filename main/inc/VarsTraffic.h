

int Hours;
int Mins;
int Secs;

int CDTime[16];
char CDTColor[16][8]={"R","A","G","G"};
int CDTimeInput[16];
char CDTColorTable[4][2] = {"X","G","A","R"};
char CommandTable[10][11] = {"XXX","FIX","VA","FoFL","ErFL","ATCS","MNL","PDET","MANO","XXX"};
char Command[10]="FIX";
char Mode[10]="FIXED";
char stageMode[10]="FIXED";
bool TimerSet=0;

int UART_TCPb=0;
int TCP_RECEIVED_TRA=0;

int stage=0;
int road;
int nextRoad;
int prevStage=0;
int decodeNEXT=0;
char TraInput[100];
char CurInput[100];
char NxtInput[100];
char NextCurInput[100];
char NextNxtInput[100];


char CurRoadColor='R';

char ATC1[311];
char ATC2[311];
char ATC3[311];
char ATC4[311];

char DEF1[311];
char DEF2[311];
char DEF3[311];
char DEF4[311];

char stage1[200];
char stage2[200];
char stage3[200];
char stage4[200];
char stage5[200];
char stage6[200];
char stage7[200];

char NEXT[100];

bool uartDebugInfo=false;

char PhaseTime[8];