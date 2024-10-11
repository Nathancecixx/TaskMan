#include "ProcessView.h"

int drawCoreHeatMap(Rectangle bounds, CPU_INFO* info);
float calculateCoreUsageDelta(const char* coreDataOne, const char* coreDataTwo); 
float calculateCoreUsageTotal(const char* coreDataOne, const char* coreDataTwo);

int ProcRender(Rectangle bounds) {
  DrawRectangleRec(bounds, primaryBackground);

    if (1) {
      const char *errorMessage = "Couldn't load processes";
      int fontSize = 15;  // Set a font size for the labelVector2
      Vector2 textSize = MeasureTextEx(GetFontDefault(), errorMessage, fontSize, 1);

      // Calculate the centered position within the boundsfloat
      float centerX = bounds.x + (bounds.width - textSize.x) / 2;
      float centerY = bounds.y + (bounds.height - textSize.y) / 2;

      // Center the label within the bounds
      GuiLabel((Rectangle){centerX, centerY, textSize.x, textSize.y}, errorMessage);
      return 1;
    }

  return 0;
}

int CpuRender(Rectangle bounds, CPU_INFO* locked_info, pthread_mutex_t* mutex){
  CPU_INFO info;

  // Lock the mutex before reading shared data
  pthread_mutex_lock(mutex);
  info = *locked_info;
  pthread_mutex_unlock(mutex);

  DrawRectangleRec(bounds, primaryBackground);

    if (!info.success) {
      const char *errorMessage = "Couldn't load cpu info";
      int fontSize = 15;  // Set a font size for the labelVector2
      Vector2 textSize = MeasureTextEx(GetFontDefault(), errorMessage, fontSize, 1);

      // Calculate the centered position within the boundsfloat
      float centerX = bounds.x + (bounds.width - textSize.x) / 2;
      float centerY = bounds.y + (bounds.height - textSize.y) / 2;

      // Center the label within the bounds
      GuiLabel((Rectangle){centerX, centerY, textSize.x, textSize.y}, errorMessage);
      return 1;
    }

    //Get the string representations of each of the cpu data points
    char runningProcessCountStr[32];
    snprintf(runningProcessCountStr, sizeof(runningProcessCountStr), "Running Processes: %d", info.processRunningCount);

    char totalProcessCountStr[32];
    snprintf(totalProcessCountStr, sizeof(totalProcessCountStr), "Total Process: %d", info.totalProcessCount);

    char blockedProcessCountStr[32];
    snprintf(blockedProcessCountStr, sizeof(blockedProcessCountStr), "Blocked Processes: %d", info.processBlocked);

    char contextSwitchingStr[32];
    snprintf(contextSwitchingStr, sizeof(contextSwitchingStr), "Context Switching: %d", info.contextSwitching);

    //Get the rectangle and size of text needed to make gui label
    Vector2 rp_size = MeasureTextEx(GetFontDefault(), runningProcessCountStr, 15, 1);
    Rectangle rp_rec = {bounds.x + 40, bounds.y + 40, rp_size.x, rp_size.y};

    Vector2 tp_size = MeasureTextEx(GetFontDefault(), totalProcessCountStr, 15, 1);
    Rectangle tp_rec = {bounds.x + 40, bounds.y + 80, tp_size.x, tp_size.y};

    Vector2 bp_size = MeasureTextEx(GetFontDefault(), blockedProcessCountStr, 15, 1);
    Rectangle bp_rec = {bounds.x + 40, bounds.y + 120, bp_size.x, bp_size.y};

    Vector2 cs_size = MeasureTextEx(GetFontDefault(), contextSwitchingStr, 15, 1);
    Rectangle cs_rec = {bounds.x + 40, bounds.y + 160, cs_size.x, cs_size.y};


    //Make the gui labels to display info
    GuiLabel(rp_rec, runningProcessCountStr);
    GuiLabel(tp_rec, totalProcessCountStr);
    GuiLabel(bp_rec, blockedProcessCountStr);
    GuiLabel(cs_rec, contextSwitchingStr);
    drawCoreHeatMap( (Rectangle) {bounds.x + 100, bounds.y + 250, bounds.width - 200, bounds.height - 400}, &info);
    
    

    return 0;

}

int drawCoreHeatMap(Rectangle bounds, CPU_INFO* info){
  //Make the usage heatmap
  int gridWidth = sqrt(info->coreCount);  
  int cellWidth = bounds.width / gridWidth;
  int cellHeight = bounds.height / gridWidth;

  for (int i = 0; i < info->coreCount; i++) {
    int row = i / gridWidth;
    int col = i % gridWidth;
    float usage = calculateCoreUsageDelta(info->cores1[i], info->cores2[i]);

    //DEBUG
    printf("USAGE: %f for cpu - %s\n", usage, info->cores2[i]);
    Rectangle cell = { bounds.x + col * cellWidth, bounds.y + row * cellHeight, cellWidth, cellHeight };

    // Color gradient based on CPU usage
    Color usageColor = ColorLerp(DARKGREEN, RED, usage);  // Lerp between red (low usage) and green (high usage)
    DrawRectangleRec(cell, usageColor);
    DrawRectangleLinesEx(cell, 0.5, BLACK);
  }
}


/* Usage == (TotalActiveTime - PreviousActiveTime) / (TotalTotalTime - PreviousTotalTime)
 *
 * TotalActiveTime == (Everything but IdleTime and ioWaitTime)
 *

    Total CPU Time = user + nice + system + idle
    Idle CPU Time = idle
    Non-idle CPU Time = user + nice + system
    CPU usage percentage = (Non-idle CPU time / Total CPU time) * 100
 */

float calculateCoreUsageTotal(const char* coreDataOne, const char* coreDataTwo) { 
    unsigned long long int userTime, systemTime, idleTime, ioWaitTime, irq, softIrq, steal;
  
    // Parse core data from both time points
    sscanf(coreDataOne, "cpu%*d %llu %llu %llu %llu %llu %llu %llu", &userTime, &systemTime, &idleTime, &ioWaitTime, &irq, &softIrq, &steal);

    unsigned long long int TotalCpuTime,IdleCpuTime,NonIdleCpuTime;
  
    TotalCpuTime = userTime + systemTime + idleTime;
    IdleCpuTime = idleTime;
    NonIdleCpuTime = userTime + systemTime;

    //if (TotalCpuTime == 0) {
        //return 0.0f; // Return 0 if no time has passed
    //}

    // Return CPU usage as a decimal
    return (NonIdleCpuTime/TotalCpuTime);
}





/* 
 * Instead of finding the exact cpu usage 
 * I find the change of usage between 2 timestamps 
 * making the heatmap more dynamic.
 *
 * Usage == 1.0 - (DeltaTotalTime/DeltaIdleTime)
 * 
 *
 * DeltaTotalTime == TotalTime1 - TotalTime2
 *
 * DeltaIdleTime == IdleTime1 - idleTime2
 *
 *
 * TotalTime == all fields added up
 *
 * IdleTime == idle + iowait
 */
float calculateCoreUsageDelta(const char* coreDataOne, const char* coreDataTwo) {
    unsigned long long int fields1[10], fields2[10];
    int numFields1, numFields2;

    // Read core data from both snapshots
    numFields1 = sscanf(coreDataOne, "cpu%*d %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                        &fields1[0], &fields1[1], &fields1[2], &fields1[3], &fields1[4],
                        &fields1[5], &fields1[6], &fields1[7], &fields1[8], &fields1[9]);

    numFields2 = sscanf(coreDataTwo, "cpu%*d %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                        &fields2[0], &fields2[1], &fields2[2], &fields2[3], &fields2[4],
                        &fields2[5], &fields2[6], &fields2[7], &fields2[8], &fields2[9]);

    // Check if parsing was successful
    if (numFields1 < 4 || numFields2 < 4) {
        // Not enough data parsed; return 0 usage
        return 0.0f;
    }

    // Sum up all the fields to get total times
    unsigned long long int totalTime1 = 0, totalTime2 = 0;
    for (int i = 0; i < numFields1; i++) {
        totalTime1 += fields1[i];
    }
    for (int i = 0; i < numFields2; i++) {
        totalTime2 += fields2[i];
    }

    // Calculate idle times (idle + iowait)
    unsigned long long int idleTime1 = fields1[3] + ((numFields1 > 4) ? fields1[4] : 0);
    unsigned long long int idleTime2 = fields2[3] + ((numFields2 > 4) ? fields2[4] : 0);

    // Calculate deltas
    unsigned long long int deltaTotalTime = totalTime2 - totalTime1;
    unsigned long long int deltaIdleTime = idleTime2 - idleTime1;

    // Avoid division by zero
    if (deltaTotalTime == 0) {
        return 0.0f;
    }

    // Calculate usage
    float usage = 1.0f - ((float)deltaIdleTime / (float)deltaTotalTime);

    // Clamp usage to a valid range (0.0 to 1.0)
    if (usage > 1.0f) usage = 1.0f;
    if (usage < 0.0f) usage = 0.0f;

    return usage;
}

