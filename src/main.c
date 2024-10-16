#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#define RAYGUI_IMPLEMENTATION
#include "raylib.h"
#include "../include/raygui.h"

#include "ProcessView.h"
#include "ProcessManager.h"
#include "colors.h"



#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600



pthread_mutex_t cpu_info_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef enum {
  PROCESS_VIEW,
  CPU_VIEW,
  PERFORMANCE_VEIW,
}VIEW;


int NavbarRender(Rectangle boundary, VIEW* currentView, CPU_INFO* cpu_info);

void* cpu_info_updater(void* arg) {
    CPU_INFO* info = (CPU_INFO*)arg;

    while (true) {
        CPU_INFO temp_info;
        LoadCpuInfo(&temp_info);
        // Lock the mutex before updating shared data
        pthread_mutex_lock(&cpu_info_mutex);
        
        temp_info.refreshTime = info->refreshTime;
        *info = temp_info;
        
        // Unlock the mutex after updating
        pthread_mutex_unlock(&cpu_info_mutex);

        usleep((useconds_t)temp_info.refreshTime);
        //DEBUG
        //usleep(50000);
    }

    return NULL;
}



int main(){
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "TaskMan");
  SetTargetFPS(60);


  GuiSetStyle(DEFAULT, TEXT_SIZE, 18);

  VIEW CurrentView = PROCESS_VIEW; 
  Rectangle ViewRec;

  CPU_INFO cpu_info = {0};
  cpu_info.refreshTime = 100000.0f;

  PROCESS_LIST process_list = {0};
  InitProcLs(&process_list);
  LoadProcLs(&process_list);

  // Create the CPU info updating thread
  pthread_t updater_thread;
  if (pthread_create(&updater_thread, NULL, cpu_info_updater, (void*)&cpu_info) != 0) {
    perror("Failed to create updater thread");
    return 1;
  }



  while(!WindowShouldClose()){

    //DEBUG
    //printf("Refresh time: %f ms\n", cpu_info.refreshTime);

    int WindowWidth = GetScreenWidth();
    int WindowHeight = GetScreenHeight();
    


    BeginDrawing();
    
    Rectangle NavRec = {
      0, 
      0, 
      WindowWidth/4, 
      WindowHeight
    };
    
    if(GetMouseX() < NavRec.width){
      ViewRec.x = WindowWidth/4; 
      ViewRec.y = 0; 
      ViewRec.width = WindowWidth - (WindowWidth/4); 
      ViewRec.height = WindowHeight;

      NavbarRender(NavRec, &CurrentView, &cpu_info);
    } else {  
      ViewRec.x = 0;
      ViewRec.y = 0;
      ViewRec.width = WindowWidth;
      ViewRec.height = WindowHeight;
    }

    switch(CurrentView){
      case PROCESS_VIEW:{
        ProcRender(ViewRec, &process_list);
        break;
        }
      case CPU_VIEW: {
        CpuRender(ViewRec, &cpu_info, &cpu_info_mutex);
        break;
        }
      }

      EndDrawing();
  }
  

  ClearProcLs(&process_list);
  
  pthread_cancel(updater_thread);
  pthread_join(updater_thread, NULL);
  pthread_mutex_destroy(&cpu_info_mutex);

  CloseWindow();

  return 0;
}


int NavbarRender(Rectangle boundary, VIEW* currentView, CPU_INFO* cpu_info){
  DrawRectangleRec(boundary, panelBackground);
  
  Rectangle buttonOne = {boundary.x + 20, boundary.y + 20, boundary.width-40, 50};
  if(GuiButton(buttonOne, "Processes")){
    *currentView = PROCESS_VIEW;
  }

  Rectangle buttonTwo = {boundary.x + 20, boundary.y + 80, boundary.width-40, 50};
  if(GuiButton(buttonTwo, "Cpu")){
    *currentView = CPU_VIEW;
  }
  
  Rectangle sliderRec = {boundary.x + 10, boundary.height - 50, boundary.width-20, 20};
  pthread_mutex_lock(&cpu_info_mutex);
  int slider_change = GuiSlider(
      sliderRec,
      NULL,
      NULL,
      &cpu_info->refreshTime,
      1000,
      1000000
      );
  pthread_mutex_unlock(&cpu_info_mutex);

  return 0;
}
