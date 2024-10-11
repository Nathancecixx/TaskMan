#include "ProcessManager.h"

int InitCpuInfo(CPU_INFO* info){
  if(info == NULL)
    return false;
/*
  info->userTime = NULL;
  info->systemTime = NULL;
  info->idleTime = NULL;
  info->ioWaitTime = NULL;
  info->irq = NULL;
  info->softIrq = NULL;
  info->steal = NULL;
  info->bootTime = 0;
*/
  //info->cores1 = NULL;
  //info->cores2 = NULL;
  //info->prevIdleTime = NULL;
  info->coreCount = 0;

  info->contextSwitching = 0;
  info->totalProcessCount = 0;
  info->processRunningCount = 0;
  info->processBlocked = 0;

  info->success = false;
  info->refreshTime = 100000.0f;

  return true;
}

int LoadCpuInfo(CPU_INFO* info){ 
  info->success = false;

  //Read individual core info first by itself to be able to compare it
  FILE* file1 = fopen("/proc/stat", "r");
 
  if(file1 == NULL){
    perror("Error opening \'/proc/stat\' file");
    return 1;
  }

  char line1[256];
  info->coreCount = 0;

  while(fgets(line1, sizeof(line1), file1)){
    char label1[32];
    sscanf(line1, "%s", label1);

    if (strncmp(label1, "cpu", 3) == 0) {
      //Dont care aboyt general cpu data this pass
      if (strcmp(label1, "cpu") == 0) {
        continue;
      }
      char user[20], nice[20], system[20], idle[20], ioWait[20], irq[20], softIrq[20], steal[20];
      sscanf(line1, "%*s %s %s %s %s %s %s %s %s", user, nice, system, idle, ioWait, irq, softIrq, steal);
      
        strcpy(info->cores1[info->coreCount], line1);
        info->coreCount++;
    }
  }
 
  //Reset and wait interval to get current values
  fclose(file1);
  usleep(CORE_INTERVAL_TIME);  

  FILE* file = fopen("/proc/stat", "r");
 
  if(file == NULL){
    perror("Error opening \'/proc/stat\' file");
    return 1;
  }

  char line[256];
  info->coreCount = 0;

  while(fgets(line, sizeof(line), file)){
    char label[32];
    // Parse the label from the linesscanfsscanf
    sscanf(line, "%s", label);

    // Check if the line starts with "cpu" for CPU data
    if (strncmp(label, "cpu", 3) == 0) {
      char user[20], nice[20], system[20], idle[20], ioWait[20], irq[20], softIrq[20], steal[20];
      sscanf(line, "%*s %s %s %s %s %s %s %s %s", user, nice, system, idle, ioWait, irq, softIrq, steal);
      if (strcmp(label, "cpu") == 0) {
        strcpy(info->userTime, user);
        strcpy(info->systemTime, system);
        strcpy(info->idleTime, idle);
        strcpy(info->ioWaitTime, ioWait);
        strcpy(info->irq, irq);
        strcpy(info->softIrq, softIrq);
        strcpy(info->steal, steal);
      } else {
        // Store core data
        strcpy(info->cores2[info->coreCount], line);
        info->coreCount++;
      }
    } else if (strcmp(label, "ctxt") == 0) {
      sscanf(line, "%*s %d", &info->contextSwitching);
    } else if (strcmp(label, "btime") == 0) {
      sscanf(line, "%*s %d", &info->bootTime);
    } else if (strcmp(label, "processes") == 0) {
      sscanf(line, "%*s %d", &info->totalProcessCount);
    } else if (strcmp(label, "procs_running") == 0) {
      sscanf(line, "%*s %d", &info->processRunningCount);
    } else if (strcmp(label, "procs_blocked") == 0) {
      sscanf(line, "%*s %d", &info->processBlocked);
    }
      
  }

  info->success = true;

  fclose(file);
  return 0;
}
