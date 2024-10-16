#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>
#include <stdbool.h>

#define PROC_STAT_FILE_LOC "/proc/stat"

#define CORE_INTERVAL_TIME 500000 

#define MIN_REFRESH_TIME_MS 50.0f
#define MAX_REFRESH_TIME_MS 100.0f

#define PROC_LS_CHUNK 20


typedef struct {
  
  char userTime[20];
  char systemTime[20];
  char idleTime[20];
  char ioWaitTime[20];
  char irq[20];
  char softIrq[20];
  char steal[20];
  int bootTime;

  char cores1[64][256];
  char cores2[64][256];
  char prevIdleTime[64][256];
  int coreCount;

  int contextSwitching;
  int totalProcessCount;
  int processRunningCount;
  int processBlocked;

  bool success;
  float refreshTime;
} CPU_INFO;

typedef struct {
    char* memTotal;
    char* memFree;
    char* memAvailable;
    char* swapMemUsage;
} MEM_INFO;

typedef struct {
  int id;
  char name[255];
} PROCESS_INFO;

typedef struct {
  PROCESS_INFO* list;
  char* processNameList;
  int currentSize;
  int maxCapacity;
} PROCESS_LIST;



int InitCpuInfo(CPU_INFO* info);
int LoadCpuInfo(CPU_INFO* info);

//int LoadMemInfo(PROCESS_MANAGER* pm);

int InitProcLs(PROCESS_LIST* pl);
int LoadProcLs(PROCESS_LIST* pl);
int AddProcessLs(PROCESS_LIST* pl, PROCESS_INFO info);
int ClearProcLs(PROCESS_LIST* pl);
int PrintProcessLs(PROCESS_LIST* pl);



#endif // PROCESS_MANAGER_H
