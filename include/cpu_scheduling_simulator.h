/**
 * @file cpu_scheduling_simulator.h
 * @brief struct definitions
 *        and function prototypes of simulator, queue, priority queue
 */
#ifndef __CPU_SCHEDULING_SIM_H_
#define __CPU_SCHEDULING_SIM_H_

typedef unsigned int process_id_t;
typedef unsigned int time_len_t;
typedef double avg_time_len_t;

/* Struct that represents a process */
typedef struct Process_t {
  process_id_t pid;          /* unique identifier of a process */
  time_len_t cpu_burst_time; /* time length to be processed by CPU */
  time_len_t io_burst_time;  /* time length to be processed by I/O */
  time_len_t arrival_time;   /* time at which process has created */
  int priority;              /* the smaller value is, the higher priority is */

  time_len_t waiting_time;    /* waiting time of process */
  time_len_t turnaround_time; /* turnaround time of process */
} Process_t, *ProcessPtr_t;

/* Queue Implementation */
typedef struct ListNode {
  ProcessPtr_t process_ptr; /* pointer of a process */
  struct ListNode *next;    /* pointer used for singly linked list */
} ListNode_t, *ListNodePtr_t;

typedef struct Queue_t {
  int count;
  ListNodePtr_t front, rear;
} Queue_t;

int Queue_Init(Queue_t **queue);
int Queue_Enqueue(Queue_t *queue, ProcessPtr_t process);
int Queue_Dequeue(Queue_t *queue);
ProcessPtr_t Queue_Front(Queue_t *queue);
int Queue_IsEmpty(Queue_t *queue);

/* Priority Queue Implementation */
typedef struct TreeNode {
  ProcessPtr_t process_ptr;               /* pointer of a process */
  struct TreeNode *left, *right, *parent; /* pointer for binary tree */
} TreeNode_t, *TreeNodePtr_t;

typedef struct Priority_Queue_t {
  int count;
  TreeNodePtr_t top;
  int (*compare)(ProcessPtr_t a, ProcessPtr_t b); /* compare function pointer */
} Priority_Queue_t;

int Priority_Init(Priority_Queue_t **queue,
                  int (*compare)(ProcessPtr_t a, ProcessPtr_t b));
int Priority_Enqueue(Priority_Queue_t *queue, ProcessPtr_t process);
int Priority_Dequeue(Priority_Queue_t *queue);
ProcessPtr_t Priority_Top(Priority_Queue_t *queue);
int Priority_IsEmpty(Priority_Queue_t *queue);
int SJF_Compare(ProcessPtr_t a, ProcessPtr_t b);
int Priority_Compare(ProcessPtr_t a, ProcessPtr_t b);
int IO_Burst_Compare(ProcessPtr_t a, ProcessPtr_t b);

/* Flag values of scheduling algorithms */
#define FCFS 1
#define NON_PREEMPTIVE_SJF 2
#define PREEMPTIVE_SJF 3
#define NON_PREEMPTIVE_PRIORITY 4
#define PREEMPTIVE_PRIORITY 5
#define ROUND_ROBIN 6

/* Struct that represents a simulator */
typedef struct Simulator {
  unsigned int num_process;              /* the number of processes generated */
  Priority_Queue_t *generated_processes; /* pointer of job queue */
  Priority_Queue_t *terminated_processes; /* queue of terminated processes */

  int flag;                        /* flag of scheduling algorithm */
  ProcessPtr_t cur_cpu_burst;      /* pointer of currently running process */
  void *ready_queue;               /* pointer of ready queue */
  Priority_Queue_t *waiting_queue; /* pointer of waiting queue */

  time_len_t elapsed_time;            /* elapsed time in simulation */
  time_len_t idle_time;               /* total amount of cpu idle time */
  avg_time_len_t avg_waiting_time;    /* average waiting time */
  avg_time_len_t avg_turnaround_time; /* average turnaround time */
  time_len_t max_waiting_time;
} Simulator_t;

int Simulator_Init(Simulator_t **simulator, int flag);
int Simulator_GenerateProcess(Simulator_t **simulators, int N);
int Simulator_ArrivalTime_Compare(ProcessPtr_t a, ProcessPtr_t b);

void Simulator_Start(Simulator_t *simulator);

void Simulator_LoadReadyQueue(Simulator_t *simulator);
void Simulator_LoadReadyQueue_Priority(Simulator_t *simulator);

void Simulator_LoadProcess(Simulator_t *simulator);
void Simulator_LoadProcess_Priority(Simulator_t *simulator);

int Simulator_CPU_Burst(Simulator_t *simulator);
int Simulator_CPU_Burst_Preemptive(Simulator_t *simulator);
int Simulator_CPU_Burst_RR(Simulator_t *simulator, int *time_quantum);

void Simulator_Queue_Waiting(Queue_t *ready_queue);
void Simulator_Priority_Waiting(TreeNodePtr_t waiting_process);

void Simulator_Process_IO(TreeNodePtr_t io_process);
void Simulator_Process_IO_Queue(Simulator_t *simulator);
void Simulator_Process_IO_Priority(Simulator_t *simulator);

int Simulator_Probability();

void Simulator_FCFS(Simulator_t *simulator);
void Simulator_NonPreemptiveSJF(Simulator_t *simulator);
void Simulator_PreemptiveSJF(Simulator_t *simulator);
void Simulator_NonPreemptivePriority(Simulator_t *simulator);
void Simulator_PreemptivePriority(Simulator_t *simulator);
void Simulator_RoundRobin(Simulator_t *simulator);

void Simulator_Eval(Simulator_t *simulator);
void Simulator_Terminate(Simulator_t **simulators);

#endif