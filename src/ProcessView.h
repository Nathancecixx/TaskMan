#include <stdio.h>

#include "raylib.h"
#include "colors.h"

#include "ProcessManager.h"


//int InitProcView();

//int UpdateProcView(PROCESS_MANAGER* pm);

int ProcRender(Rectangle boundary);

int CpuRender(Rectangle bounds, CPU_INFO* info, pthread_mutex_t* mutex);
//int DeInitProcVeiw();
