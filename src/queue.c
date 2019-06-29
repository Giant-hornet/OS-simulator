/**
 * @file queue.c
 * @brief function implementations of queue
 *        nodes in queue managed by singly linked list.
 */
#include <stdio.h>
#include <stdlib.h>
#include "../include/cpu_scheduling_simulator.h"

/**
 * @fn int Queue_Init(Queue_t **queue)
 * @brief initialize variables in Queue_t
 *
 * @param queue pointer of Queue_t *(double pointer)
 *
 * @return 1 if initialization successes
 */
int Queue_Init(Queue_t **queue) {
  *queue = (Queue_t *)malloc(sizeof(Queue_t));

  if ((*queue) == NULL) {
    printf("Error log: malloc() in Queue_Init\n");
    exit(-1);
  }

  (*queue)->count = 0;
  (*queue)->front = (*queue)->rear = NULL;

  return 1;
}

/**
 * @fn int Queue_Enqueue(Queue_t *queue, ProcessPtr_t process)
 * @brief insert a new node in queue
 *
 * @param queue pointer of Queue_t
 * @param process ProcessPtr_t of the new node
 *
 * @return 1 if enqueue successes
 */
int Queue_Enqueue(Queue_t *queue, ProcessPtr_t process) {
  ListNodePtr_t new = (ListNodePtr_t)malloc(sizeof(ListNode_t));

  if (new == NULL) {
    printf("Error log: malloc() in Queue_Enqueue\n");
    exit(-1);
  }

  new->process_ptr = process;
  new->next = NULL;

  if (queue->count++ == 0) {
    /* The inserted node is the only node in queue. */
    queue->front = queue->rear = new;
  } else {
    queue->rear->next = new;
    queue->rear = new;
  }

  return 1;
}

/**
 * @fn int Queue_Dequeue(Queue_t *queue)
 * @brief delete a node in queue
 *
 * @param queue pointer of Queue_t
 *
 * @return 1 if dequeue successes
 */
int Queue_Dequeue(Queue_t *queue) {
  if (queue->count == 0) return 0;

  ListNodePtr_t tmp = queue->front;
  queue->front = tmp->next;
  free(tmp);

  /* There is no node in queue after deletion. */
  if (--queue->count == 0) queue->rear = NULL;

  return 1;
}

/**
 * @fn int Queue_Front(Queue_t *queue)
 * @brief return the pointer of a process that is at the front of the queue
 *
 * @param queue pointer of Queue_t
 *
 * @return the pointer of a process if queue is not empty
 *         NULL                     if queue is empty
 */
ProcessPtr_t Queue_Front(Queue_t *queue) {
  if (queue->count == 0) return NULL;

  return queue->front->process_ptr;
}

/**
 * @fn int Queue_IsEmpty(Queue_t queue)
 * @brief return whether the queue is empty
 *
 * @param queue pointer of Queue_t
 *
 * @return 1 if the queue is empty
 *         0 otherwise
 */
int Queue_IsEmpty(Queue_t *queue) { return queue->count == 0; }