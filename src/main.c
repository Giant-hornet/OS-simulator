/**
 * @file main.c
 * @brief main function
 */
#include <stdio.h>
#include "../include/cpu_scheduling_simulator.h"

int main() {
  int N;
  Simulator_t *simulator[7];

  Simulator_Init(&simulator[0], FCFS);
  Simulator_Init(&simulator[1], NON_PREEMPTIVE_SJF);
  Simulator_Init(&simulator[2], PREEMPTIVE_SJF);
  Simulator_Init(&simulator[3], NON_PREEMPTIVE_PRIORITY);
  Simulator_Init(&simulator[4], PREEMPTIVE_PRIORITY);
  Simulator_Init(&simulator[5], ROUND_ROBIN);
  simulator[6] = NULL;

  printf("Enter the number of processes: ");
  scanf("%d", &N);

  Simulator_GenerateProcess(simulator, N);

  for (int i = 0; i < 6; i++) Simulator_Start(simulator[i]);

  Simulator_Terminate(simulator);

  return 0;
}