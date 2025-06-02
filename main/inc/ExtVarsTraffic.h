

extern int Hours;
extern int Mins ;
extern int Secs;
extern int CDTime[16];
extern int CDTimeInput[16];
extern char CDTColor[16][8];
extern char CDTColorTable[4][2];
extern char CommandTable[10][11];
extern char Command[10];
extern char Mode[10];
extern char stageMode[10];
extern bool TimerSet;

extern int UART_TCPb;
extern int TCP_RECEIVED_TRA;

extern int stage;
extern int road;
extern int nextRoad;
extern int prevStage;
extern int decodeNEXT;
extern char TraInput[100];
extern char CurInput[100];
extern char NxtInput[100];
extern char NextCurInput[100];
extern char NextNxtInput[100];


extern char CurRoadColor;

extern char ATC1[311];
extern char ATC2[311];
extern char ATC3[311];
extern char ATC4[311];

extern char DEF1[311];
extern char DEF2[311];
extern char DEF3[311];
extern char DEF4[311];

extern char stage1[100];
extern char stage2[100];
extern char stage3[100];
extern char stage4[100];
extern char stage5[100];
extern char stage6[100];
extern char stage7[100];

extern char NEXT[100];

extern bool uartDebugInfo;

extern char PhaseTime[8];