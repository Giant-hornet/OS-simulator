/**
 * @file simulator.c
 * @brief function implementations of simulator
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../include/cpu_scheduling_simulator.h"

#define cur_process(x) x->cur_cpu_burst

/* when defined, I/O burst time of all processes are set to 0 */
//#define IO_DEBUG
/* when defined, arrival time of all processes are set to 0 */
// #define ARRIVAL_DEBUG
#define TIME_QUANTUM 10 /* time quantum value used in RR */
#define SIM_SPEED 1     /* speed that displays gantt chart */
#define CUTTER 10       /* the number of blocks in each line of gantt chart */

/**
 * @fn int Simulator_Init(Simulator **simulator, int flag)
 * @brief initialize variables in Simulator_t
 *
 * @param simulator pointer of Simulator *(double pointer)
 * @param flag value that indicates what scheduling algorithm simulator uses
 *
 * @return 1 if initialization successes
 */
int Simulator_Init(Simulator_t **simulator, int flag) {
  *simulator = (Simulator_t *)malloc(sizeof(Simulator_t));

  if (*simulator == NULL) {
    printf("Error log: malloc() in Simulator_Init\n");
    exit(-1);
  }

  /* Initialize variables in simulator */
  (*simulator)->num_process = 0;
  (*simulator)->cur_cpu_burst = NULL;
  (*simulator)->elapsed_time = 0;
  (*simulator)->idle_time = 0;
  (*simulator)->avg_waiting_time = 0;
  (*simulator)->avg_turnaround_time = 0;
  (*simulator)->max_waiting_time = 0;

  /*
   * Allocate data structure of ready queue(normal queue or priority queue)
   * according to given flag parameter
   */
  switch (flag) {
    case FCFS:
    case ROUND_ROBIN:
      Queue_Init((Queue_t **)&((*simulator)->ready_queue));
      break;
    case NON_PREEMPTIVE_SJF:
    case PREEMPTIVE_SJF:
      Priority_Init((Priority_Queue_t **)&((*simulator)->ready_queue),
                    SJF_Compare);
      break;
    case NON_PREEMPTIVE_PRIORITY:
    case PREEMPTIVE_PRIORITY:
      Priority_Init((Priority_Queue_t **)&((*simulator)->ready_queue),
                    Priority_Compare);
      break;
    default:
      printf("Error log: invalid flag parameter value\n");
      exit(-1);
  }

  /* Allocate memory for waiting queue and initialize */
  Priority_Init(&(*simulator)->waiting_queue, IO_Burst_Compare);

  /* Allocate memory for generated processes and initialize */
  Priority_Init(&(*simulator)->generated_processes,
                Simulator_ArrivalTime_Compare);

  /* Allocate memory for queue of terminated processes and initialize */
  Priority_Init(&(*simulator)->terminated_processes,
                Simulator_ArrivalTime_Compare);

  /* Set scheduling algorithm flag */
  (*simulator)->flag = flag;

  return 1;
}

/**
 * @fn int Simulator_GenerateProcess(Simulator_t **simulators, int N)
 * @brief generate random processes and clone them into each simulator
 *        in simulators
 *
 * @param simulators array of Simulator_t *, which ends with NULL pointer
 * @param N the number of generated processes
 *
 * @return 1 if generation successes
 */
int Simulator_GenerateProcess(Simulator_t **simulators, int N) {
  unsigned char data[500]; /* array of random cpu burst time values */
  ProcessPtr_t new;        /* pointer of newly generated process */
  process_id_t pid;
  time_len_t cpu_burst_time;
  time_len_t io_burst_time;
  time_len_t arrival_time;
  int priority;

  srand(time(NULL)); /* seed for rand() */

  /*
   * Generate random pool of cpu burst time values
   * which follows the distribution similar to geometric distribution
   */
  for (int i = 0; i < 500; i++) {
    if (i < 450)
      data[i] = rand() % 10 + 1; /* 1 ~ 10, prob 0.9 */
    else if (i < 475)
      data[i] = rand() % 10 + 11; /* 11 ~ 20, prob 0.05 */
    else
      data[i] = rand() % 20 + 21; /* 21~ 40, prob 0.05 */
  }

  printf(
      "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
      "++++++++++"
      "\n");
  printf(
      "++  PID  ++  CPU_BURST_TIME  ++  IO_BURST_TIME  ++  ARRIVAL_TIME  ++  "
      "PRIORITY  "
      "++\n");
  printf(
      "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
      "++++++++++"
      "\n");

  /*
   * Generate random processes
   * and clone them into each simulator
   */
  for (int i = 0; simulators[i]; i++) simulators[i]->num_process = N;

  for (int i = 0; i < N; i++) {
    pid = i + 1;
    cpu_burst_time = data[rand() % 500]; /* pick cpu burst time value */

#ifdef IO_DEBUG
    io_burst_time = 0;
#else
    io_burst_time = rand() % 20;
#endif

#ifdef ARRIVAL_DEBUG
    arrival_time = 0;
#else
    arrival_time = rand() % (N * 3);
#endif

    priority = rand() % 41 - 20; /* -20 <= priority <= 20 */

    printf("++ %5d ++  %14d  ++  %13d  ++  %12d  ++  %8d  ++\n", pid,
           cpu_burst_time, io_burst_time, arrival_time, priority);

    for (int j = 0; simulators[j]; j++) {
      new = (ProcessPtr_t)malloc(sizeof(Process_t));

      if (new == NULL) {
        printf("Error log: malloc() in Simulator_GenerateProcess\n");
        exit(-1);
      }

      new->pid = pid;
      new->cpu_burst_time = cpu_burst_time;
      new->io_burst_time = io_burst_time;
      new->arrival_time = arrival_time;
      new->priority = priority;
      new->waiting_time = 0;
      new->turnaround_time = 0;

      Priority_Enqueue(simulators[j]->generated_processes, new);
    }
  }
  printf(
      "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
      "++++++++++++\n");

  return 1;
}

/**
 * @fn int Simulator_ArrivalTime_Compare
 * @brief compare arrival time of two processes
 *
 * @param a first process to compare
 * @param b second process to compare
 *
 * @return 1 if arrival time of the first process is smaller
 *         0 otherwise
 */
int Simulator_ArrivalTime_Compare(ProcessPtr_t a, ProcessPtr_t b) {
  return a->arrival_time != b->arrival_time ? a->arrival_time < b->arrival_time
                                            : a->pid < b->pid;
}

/**
 * @fn void Simulator_Start(Simulator_t *simulator)
 * @brief call the simulation function
 *
 * @param simulator pointer of Simulator_t
 */
void Simulator_Start(Simulator_t *simulator) {
  switch (simulator->flag) {
    case FCFS:
      Simulator_FCFS(simulator);
      break;
    case NON_PREEMPTIVE_SJF:
      Simulator_NonPreemptiveSJF(simulator);
      break;
    case PREEMPTIVE_SJF:
      Simulator_PreemptiveSJF(simulator);
      break;
    case NON_PREEMPTIVE_PRIORITY:
      Simulator_NonPreemptivePriority(simulator);
      break;
    case PREEMPTIVE_PRIORITY:
      Simulator_PreemptivePriority(simulator);
      break;
    case ROUND_ROBIN:
      Simulator_RoundRobin(simulator);
      break;
    default:
      printf("Error log: invalid flag parameter value\n");
      exit(-1);
  }

  return;
}

/**
 * @fn void Simulator_LoadReadyQueue(Simulator_t *simulator)
 * @brief take processes from job queue(or waiting queue)
 *        and put them in ready queue
 *
 * @param simulator pointer of Simulator_t
 */
void Simulator_LoadReadyQueue(Simulator_t *simulator) {
  Queue_t *ready_queue = simulator->ready_queue;
  Priority_Queue_t *waiting_queue = simulator->waiting_queue;
  Priority_Queue_t *job_queue = simulator->generated_processes;
  Priority_Queue_t *terminated_processes = simulator->terminated_processes;
  ProcessPtr_t front_process;

  /*
   * Represent when process arrives at ready_queue
   * If elapsed time equals arrival time of the process in job queue,
   * pop the process from job queue and insert it in ready queue
   */
  while (!Priority_IsEmpty(job_queue) &&
         (front_process = Priority_Top(job_queue))->arrival_time ==
             simulator->elapsed_time) {
    Queue_Enqueue(ready_queue, front_process);
    Priority_Dequeue(job_queue);
  }

  /*
   * Represenet I/O of the process completed
   * If I/O burst time of the process in waiting queue is 0,
   * pop the process from waiting queue and insert it in ready queue
   */
  while (!Priority_IsEmpty(waiting_queue) &&
         (front_process = Priority_Top(waiting_queue))->io_burst_time == 0) {
    Priority_Dequeue(waiting_queue);
    Queue_Enqueue(ready_queue, front_process);
  }

  return;
}

/**
 * @fn void Simulator_LoadReadyQueue_Priority(Simulator_t *simulator)
 * @brief take processes from job queue(or waiting queue)
 *        and put them in ready queue
 *
 * @param simulator pointer of Simulator_t
 */
void Simulator_LoadReadyQueue_Priority(Simulator_t *simulator) {
  Priority_Queue_t *ready_queue = simulator->ready_queue;
  Priority_Queue_t *waiting_queue = simulator->waiting_queue;
  Priority_Queue_t *job_queue = simulator->generated_processes;
  Priority_Queue_t *terminated_processes = simulator->terminated_processes;
  ProcessPtr_t front_process;

  /*
   * Represent when process arrives at ready_queue
   * If elapsed time equals arrival time of the process in job queue,
   * pop the process from job queue and insert it in ready queue
   */
  while (!Priority_IsEmpty(job_queue) &&
         (front_process = Priority_Top(job_queue))->arrival_time ==
             simulator->elapsed_time) {
    Priority_Enqueue(ready_queue, front_process);
    Priority_Dequeue(job_queue);
  }

  /*
   * Represenet I/O of the process completed
   * If I/O burst time of the process in waiting queue is 0,
   * pop the process from waiting queue and insert it in ready queue
   */
  while (!Priority_IsEmpty(waiting_queue) &&
         (front_process = Priority_Top(waiting_queue))->io_burst_time == 0) {
    Priority_Dequeue(waiting_queue);
    Priority_Enqueue(ready_queue, front_process);
  }

  return;
}

/**
 * @fn void Simulator_LoadProcess(Simulator_t *simulator)
 * @brief get a process from ready queue and allocate CPU
 *
 * @param simulator pointer of Simulator_t
 */
void Simulator_LoadProcess(Simulator_t *simulator) {
  Queue_t *ready_queue = simulator->ready_queue;
  Priority_Queue_t *waiting_queue = simulator->waiting_queue;

  /* Get a process from ready queue and allocate CPU to the process */
  if (cur_process(simulator) == NULL) {
    if (!Queue_IsEmpty(ready_queue)) {
      cur_process(simulator) = Queue_Front(ready_queue);
      Queue_Dequeue(ready_queue);
    }
  }

  return;
}

/**
 * @fn void Simulator_LoadProcess_Priority(Simulator_t *simulator)
 * @brief get a process from ready queue and allocate CPU
 *
 * @param simulator pointer of Simulator_t
 */
void Simulator_LoadProcess_Priority(Simulator_t *simulator) {
  Priority_Queue_t *ready_queue = simulator->ready_queue;
  Priority_Queue_t *waiting_queue = simulator->waiting_queue;

  /* Get a process from ready queue and allocate CPU to the process */
  if (cur_process(simulator) == NULL) {
    if (!Priority_IsEmpty(ready_queue)) {
      cur_process(simulator) = Priority_Top(ready_queue);
      Priority_Dequeue(ready_queue);
    }
  }

  return;
}

/**
 * @fn int Simulator_CPU_Burst(Simulator_t *simulator)
 * @brief running process is processed
 *        and display gantt chart
 *
 * @param simulator pointer of Simulator_t
 *
 * @return 1 if all processes terminated
 *         0 otherwise
 */
int Simulator_CPU_Burst(Simulator_t *simulator) {
  Priority_Queue_t *waiting_queue = simulator->waiting_queue;
  Priority_Queue_t *terminated_processes = simulator->terminated_processes;

  for (int i = 0; i < SIM_SPEED; i++)
    ;

  if (cur_process(simulator)) { /* running process exists */
    cur_process(simulator)->cpu_burst_time--;
    cur_process(simulator)->turnaround_time++;
    printf("[ %6d ]", cur_process(simulator)->pid); /* display gantt chart */

    if (cur_process(simulator)->cpu_burst_time) {
      if (cur_process(simulator)->io_burst_time) {
        if (cur_process(simulator)->cpu_burst_time == 1 ||
            Simulator_Probability()) {
          /*
           * cur_process(simulator) has both CPU and I/O works.
           * I/O request occured randomly
           * move cur_process(simulator) from CPU to waiting queue
           */
          Priority_Enqueue(waiting_queue, cur_process(simulator));
          cur_process(simulator) = NULL;
        }
      }
    } else { /* Process has completely processed, move to terminated_process */
      Priority_Enqueue(terminated_processes, cur_process(simulator));
      cur_process(simulator) = NULL;

      /* All processes terminated */
      if (terminated_processes->count == simulator->num_process) {
        printf("\n-> Simulation End.\n\n");
        Simulator_Eval(simulator);

        return 1;
      }
    }
  } else { /* CPU is in idle */
    simulator->idle_time++;
    printf("[  IDLE  ]");
  }

  /* Limit the number of blocks in each line of gantt chart */
  if ((simulator->elapsed_time + 1) % CUTTER == 0) {
    printf("\n");
  }

  return 0;
}

/**
 * @fn int Simulator_CPU_Burst_Preemptive(Simulator_t *simulator)
 * @brief running process is processed
 *        and display gantt chart
 *
 * @param simulator pointer of Simulator_t
 *
 * @return 1 if all processes terminated
 *         0 otherwise
 */
int Simulator_CPU_Burst_Preemptive(Simulator_t *simulator) {
  Priority_Queue_t *ready_queue = simulator->ready_queue;
  Priority_Queue_t *waiting_queue = simulator->waiting_queue;
  Priority_Queue_t *terminated_processes = simulator->terminated_processes;
  int (*compare)(ProcessPtr_t a, ProcessPtr_t b) = ready_queue->compare;

  for (int i = 0; i < SIM_SPEED; i++)
    ;

  /*
   * Find whether the process
   * by which currently running process can be preempted exists
   */
  if (cur_process(simulator) && !Priority_IsEmpty(ready_queue) &&
      compare(ready_queue->top->process_ptr, cur_process(simulator))) {
    /* Currently running process is preempted */
    Priority_Enqueue(ready_queue, cur_process(simulator));
    cur_process(simulator) = NULL;

    /* Allocate CPU to another process */
    Simulator_LoadProcess_Priority(simulator);
  }

  if (cur_process(simulator)) { /* running process exists */
    cur_process(simulator)->turnaround_time++;
    cur_process(simulator)->cpu_burst_time--;
    printf("[ %6d ]", cur_process(simulator)->pid); /* display gantt chart */

    if (cur_process(simulator)->cpu_burst_time) {
      if (cur_process(simulator)->io_burst_time) {
        if (cur_process(simulator)->cpu_burst_time == 1 ||
            Simulator_Probability()) {
          /*
           * cur_process(simulator) has both CPU and I/O works.
           * I/O request occured randomly
           * move cur_process(simulator) from CPU to waiting queue
           */
          Priority_Enqueue(waiting_queue, cur_process(simulator));
          cur_process(simulator) = NULL;
        }
      }
    } else { /* Process has completely processed, move to terminated_process */
      Priority_Enqueue(terminated_processes, cur_process(simulator));
      cur_process(simulator) = NULL;

      /* All processes terminated */
      if (terminated_processes->count == simulator->num_process) {
        printf("\n-> Simulation End.\n\n");
        Simulator_Eval(simulator);

        return 1;
      }
    }
  } else { /* CPU is in idle */
    simulator->idle_time++;
    printf("[  IDLE  ]");
  }

  /* Limit the number of blocks in each line of gantt chart */
  if ((simulator->elapsed_time + 1) % CUTTER == 0) {
    printf("\n");
  }

  return 0;
}

/**
 * @fn int Simulator_CPU_Burst_RR(Simulator_t *simulator)
 * @brief running process is processed
 *        and display gantt chart
 *
 * @param simulator pointer of Simulator_t
 *
 * @return 1 if all processes terminated
 *         0 otherwise
 */
int Simulator_CPU_Burst_RR(Simulator_t *simulator, int *time_quantum) {
  Queue_t *ready_queue = simulator->ready_queue;
  Priority_Queue_t *waiting_queue = simulator->waiting_queue;
  Priority_Queue_t *terminated_processes = simulator->terminated_processes;

  for (int i = 0; i < SIM_SPEED; i++)
    ;

  if (cur_process(simulator)) { /* running process exists */
    cur_process(simulator)->cpu_burst_time--;
    cur_process(simulator)->turnaround_time++;
    printf("[ %6d ]", cur_process(simulator)->pid); /* display gantt chart */
    (*time_quantum)++;

    if (cur_process(simulator)->cpu_burst_time) {
      if (cur_process(simulator)->io_burst_time) {
        if (cur_process(simulator)->cpu_burst_time == 1 ||
            Simulator_Probability()) {
          /*
           * cur_process(simulator) has both CPU and I/O works.
           * I/O request occured randomly
           * move cur_process(simulator) from CPU to waiting queue
           */
          Priority_Enqueue(waiting_queue, cur_process(simulator));
          cur_process(simulator) = NULL;
          *time_quantum = 0;
        }
      }

      if (*time_quantum == TIME_QUANTUM) {
        Queue_Enqueue(ready_queue, cur_process(simulator));
        cur_process(simulator) = NULL;
        *time_quantum = 0;
      }
    } else { /* Process has completely processed, move to terminated_process */
      Priority_Enqueue(terminated_processes, cur_process(simulator));
      cur_process(simulator) = NULL;
      *time_quantum = 0;

      /* All processes terminated */
      if (terminated_processes->count == simulator->num_process) {
        printf("\n-> Simulation End.\n\n");
        Simulator_Eval(simulator);

        return 1;
      }
    }
  } else { /* CPU is in idle */
    simulator->idle_time++;
    printf("[  IDLE  ]");
  }

  /* Limit the number of blocks in each line of gantt chart */
  if ((simulator->elapsed_time + 1) % CUTTER == 0) {
    printf("\n");
  }

  return 0;
}

/**
 * @fn void Simulator_Queue_Waiting(Queue_t *ready_queue)
 * @brief both waiting time and turnaround time of all processes in ready
 * queue are increased by 1
 *
 * @param ready_queue pointer of Queue_t
 */
void Simulator_Queue_Waiting(Queue_t *ready_queue) {
  ListNodePtr_t tmp = ready_queue->front;

  if (tmp == NULL) return;

  do {
    tmp->process_ptr->waiting_time++;
    tmp->process_ptr->turnaround_time++;
  } while (tmp = tmp->next);

  return;
}

/**
 * @fn void Simulator_Priority_Waiting(TreeNodePtr_t waiting_process)
 * @brief both waiting time and turnaround time of all processes in ready
 * queue are increased by 1
 *
 * @param waiting_process pointer of tree node in ready queue
 */
void Simulator_Priority_Waiting(TreeNodePtr_t waiting_process) {
  if (waiting_process == NULL) return;

  Simulator_Priority_Waiting(waiting_process->left);
  waiting_process->process_ptr->waiting_time++;
  waiting_process->process_ptr->turnaround_time++;
  Simulator_Priority_Waiting(waiting_process->right);

  return;
}

/**
 * @fn void Simulator_Process_IO(TreeNodePtr_t io_process)
 * @brief I/O burst time of all processes in waiting queue
 *        are decremented by 1
 *        turnaround time of all process in waiting queue
 *        are increased by 1
 *
 * @param io_process pointer of tree node in waiting_queue
 */
void Simulator_Process_IO(TreeNodePtr_t io_process) {
  if (io_process == NULL) return;

  Simulator_Process_IO(io_process->left);
  io_process->process_ptr->io_burst_time--;
  io_process->process_ptr->turnaround_time++;
  Simulator_Process_IO(io_process->right);

  return;
}

/**
 * @fn void Simulator_Process_IO_Queue(Simulator_t *simulator)
 * @brief represents randomly completion of I/O works
 *        all processes in waiting queue randomly make I/O interrupts.
 *
 * @param simulator pointer of Simulator_t
 */
void Simulator_Process_IO_Queue(Simulator_t *simulator) {
  Queue_t *ready_queue = simulator->ready_queue;
  Priority_Queue_t *waiting_queue = simulator->waiting_queue;
  Priority_Queue_t *tmp;
  ProcessPtr_t top_process;

  if (Priority_IsEmpty(waiting_queue)) return;

  Priority_Init(&tmp, IO_Burst_Compare);

  while (top_process = Priority_Top(waiting_queue)) {
    if (Simulator_Probability() && top_process->cpu_burst_time > 1) {
      Priority_Dequeue(waiting_queue);
      Queue_Enqueue(ready_queue, top_process);
    } else {
      Priority_Dequeue(waiting_queue);
      Priority_Enqueue(tmp, top_process);
    }
  }

  simulator->waiting_queue = tmp;
  free(waiting_queue);

  return;
}

/**
 * @fn void Simulator_Process_IO_Priority(Simulator_t *simulator)
 * @brief represents randomly completion of I/O works
 *        all processes in waiting queue randomly make I/O interrupts.
 *
 * @param simulator pointer of Simulator_t
 */
void Simulator_Process_IO_Priority(Simulator_t *simulator) {
  Priority_Queue_t *ready_queue = simulator->ready_queue;
  Priority_Queue_t *waiting_queue = simulator->waiting_queue;
  Priority_Queue_t *tmp;
  ProcessPtr_t top_process;

  if (Priority_IsEmpty(waiting_queue)) return;

  Priority_Init(&tmp, IO_Burst_Compare);

  while (top_process = Priority_Top(waiting_queue)) {
    if (Simulator_Probability() && top_process->cpu_burst_time > 1) {
      Priority_Dequeue(waiting_queue);
      Priority_Enqueue(ready_queue, top_process);
    } else {
      Priority_Dequeue(waiting_queue);
      Priority_Enqueue(tmp, top_process);
    }
  }

  simulator->waiting_queue = tmp;
  free(waiting_queue);

  return;
}

/**
 * @fn int Simulator_Probability()
 * @brief return 1 or 0
 *
 * @return 1 for probability 0.53
 *         0 for probability 0.47
 */
int Simulator_Probability() {
  int N = 0;

  for (int i = 0; i < 100; i++) {
    if (rand() % 2) {
      N++;
    }
  }

  return N >= 50;
}

/**
 * @fn void Simulator_FCFS(Simulator_t *simulator)
 * @brief simulate CPU scheduling with FCFS algorithm
 *
 * @param simulator pointer of Simulator_t
 */
void Simulator_FCFS(Simulator_t *simulator) {
  Queue_t *ready_queue = simulator->ready_queue;

  printf("\n# FCFS Algorithm\n\n");
  while (1) {
    Simulator_LoadReadyQueue(simulator);

    Simulator_LoadProcess(simulator);
    Simulator_Queue_Waiting(ready_queue);
    Simulator_Process_IO(simulator->waiting_queue->top);
    Simulator_Process_IO_Queue(simulator);

    if (Simulator_CPU_Burst(simulator)) return;

    simulator->elapsed_time++;
  }
}

/**
 * @fn void Simulator_NonPreemptiveSJF(Simulator_t *simulator)
 * @brief simulate CPU scheduling with Non-Preemptive SJF algorithm
 *
 * @param simulator pointer of Simulator_t
 */
void Simulator_NonPreemptiveSJF(Simulator_t *simulator) {
  Priority_Queue_t *ready_queue = simulator->ready_queue;

  printf("\n# Non-Preemptive SJF Algorithm\n\n");
  while (1) {
    Simulator_LoadReadyQueue_Priority(simulator);

    Simulator_LoadProcess_Priority(simulator);
    Simulator_Priority_Waiting(ready_queue->top);
    Simulator_Process_IO(simulator->waiting_queue->top);
    Simulator_Process_IO_Priority(simulator);

    if (Simulator_CPU_Burst(simulator)) return;

    simulator->elapsed_time++;
  }
  return;
}

/**
 * @fn void Simulator_PreemptiveSJF(Simulator_t *simulator)
 * @brief simulate CPU scheduling with Preemptive SJF algorithm
 *
 * @param simulator pointer of Simulator_t
 */
void Simulator_PreemptiveSJF(Simulator_t *simulator) {
  Priority_Queue_t *ready_queue = simulator->ready_queue;

  printf("\n# Preemptive SJF Algorithm\n\n");
  while (1) {
    Simulator_LoadReadyQueue_Priority(simulator);

    Simulator_LoadProcess_Priority(simulator);
    Simulator_Priority_Waiting(ready_queue->top);
    Simulator_Process_IO(simulator->waiting_queue->top);
    Simulator_Process_IO_Priority(simulator);

    if (Simulator_CPU_Burst_Preemptive(simulator)) return;

    simulator->elapsed_time++;
  }
  return;
}

/**
 * @fn void Simulator_NonPreemptivePriority(Simulator_t *simulator)
 * @brief simulate CPU scheduling with Non-Preemptive Priority algorithm
 *
 * @param simulator pointer of Simulator_t
 */
void Simulator_NonPreemptivePriority(Simulator_t *simulator) {
  Priority_Queue_t *ready_queue = simulator->ready_queue;

  printf("\n# Non-Preemptive Priority Algorithm\n\n");
  while (1) {
    Simulator_LoadReadyQueue_Priority(simulator);

    Simulator_LoadProcess_Priority(simulator);
    Simulator_Priority_Waiting(ready_queue->top);
    Simulator_Process_IO(simulator->waiting_queue->top);
    Simulator_Process_IO_Priority(simulator);

    if (Simulator_CPU_Burst(simulator)) return;

    simulator->elapsed_time++;
  }
  return;
}

/**
 * @fn void Simulator_PreemptivePriority(Simulator_t *simulator)
 * @brief simulate CPU scheduling with Preemptive Priority algorithm
 *
 * @param simulator pointer of Simulator_t
 */
void Simulator_PreemptivePriority(Simulator_t *simulator) {
  Priority_Queue_t *ready_queue = simulator->ready_queue;

  printf("\n# Preemptive Priority Algorithm\n\n");
  while (1) {
    Simulator_LoadReadyQueue_Priority(simulator);

    Simulator_LoadProcess_Priority(simulator);
    Simulator_Priority_Waiting(ready_queue->top);
    Simulator_Process_IO(simulator->waiting_queue->top);
    Simulator_Process_IO_Priority(simulator);

    if (Simulator_CPU_Burst_Preemptive(simulator)) return;

    simulator->elapsed_time++;
  }

  return;
}

/**
 * @fn void Simulator_RoundRobin(Simulator_t *simulator)
 * @brief simulate CPU scheduling with Round Robin algorithm
 *
 * @param simulator pointer of Simulator_t
 */
void Simulator_RoundRobin(Simulator_t *simulator) {
  Queue_t *ready_queue = simulator->ready_queue;
  int time_quantum = 0;

  printf("\n# Round Robin Algorithm\n\n");
  while (1) {
    Simulator_LoadReadyQueue(simulator);

    Simulator_LoadProcess(simulator);
    Simulator_Queue_Waiting(ready_queue);
    Simulator_Process_IO(simulator->waiting_queue->top);
    Simulator_Process_IO_Queue(simulator);

    if (Simulator_CPU_Burst_RR(simulator, &time_quantum)) return;

    simulator->elapsed_time++;
  }
  return;
}

/**
 * @fn void Simulator_Eval(Simulator_t *simulator)
 * @brief print total execution time,
 *                    CPU utilization,
 *                    average waiting time,
 *                    and average turnaround time
 *
 * @param simulator pointer of Simulator_t
 */
void Simulator_Eval(Simulator_t *simulator) {
  Priority_Queue_t *termninated_processes = simulator->terminated_processes;
  ProcessPtr_t tmp;
  avg_time_len_t CPU_utilization =
      (double)(simulator->elapsed_time + 1 - simulator->idle_time) /
      simulator->elapsed_time;

  /* Calculate average waiting time and average turnaround time */
  while (tmp = Priority_Top(termninated_processes)) {
    Priority_Dequeue(termninated_processes);

    simulator->avg_waiting_time += tmp->waiting_time;
    simulator->avg_turnaround_time += tmp->turnaround_time;

    if (tmp->waiting_time > simulator->max_waiting_time)
      simulator->max_waiting_time = tmp->waiting_time;
  }

  simulator->avg_waiting_time /= simulator->num_process;
  simulator->avg_turnaround_time /= simulator->num_process;

  printf("-> Execution time: %d\n", simulator->elapsed_time + 1);
  printf("-> CPU Utilization: %.3f\n", CPU_utilization);
  printf("-> Average waiting time: %.3f\n", simulator->avg_waiting_time);
  printf("-> Average turnaround time: %.3f\n", simulator->avg_turnaround_time);

  return;
}

/**
 * void Simulator_Terminate(Simulator_t **simulators)
 * @brief compare execution time,
 *                CPU utilization,
 *                average waiting time,
 *                average turnaround time of all scheduling algorithms
 *
 * @param simulators array of Simulator_t *, which ends with NULL pointer
 */
void Simulator_Terminate(Simulator_t **simulators) {
  printf("\n# Summary\n\n");
  printf(
      "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
      "++"
      "+++++++++++++++\n");
  printf(
      "+++++++++++++++++++++++++++++++  CPU Util  ++   Avg WT   ++   AVG TT  "
      " "
      "++   Max WT   ++\n");

  for (int i = 0; simulators[i]; i++) {
    switch (simulators[i]->flag) {
      case FCFS:
        printf("++            FCFS           ++  ");
        break;
      case NON_PREEMPTIVE_SJF:
        printf("++     Non-Preemptive SJF    ++  ");
        break;
      case PREEMPTIVE_SJF:
        printf("++       Preemptive SJF      ++  ");
        break;
      case NON_PREEMPTIVE_PRIORITY:
        printf("++  Non-Preemptive Priority  ++  ");
        break;
      case PREEMPTIVE_PRIORITY:
        printf("++     Preemptive Priority   ++  ");
        break;
      case ROUND_ROBIN:
        printf("++        Round Robin        ++  ");
        break;
      default:
        printf("Error log: invalid flag parameter value\n");
        exit(-1);
    }

    printf(
        "%8.3f  ++ %10.3f ++ %10.3f ++  %8d "
        " ++\n",
        (double)(simulators[i]->elapsed_time - simulators[i]->idle_time) /
            simulators[i]->elapsed_time,
        simulators[i]->avg_waiting_time, simulators[i]->avg_turnaround_time,
        simulators[i]->max_waiting_time);
  }

  printf(
      "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
      "++"
      "+++++++++++++++\n");

  return;
}