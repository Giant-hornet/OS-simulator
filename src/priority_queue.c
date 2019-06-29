/**
 * @file priority_queue.c
 * @brief function implementations of priority queue
 *        nodes in queue managed by binary tree.
 */
#include <stdio.h>
#include <stdlib.h>
#include "../include/cpu_scheduling_simulator.h"

#define SWAP(a, b)                   \
  ProcessPtr_t tmp = a->process_ptr; \
  a->process_ptr = b->process_ptr;   \
  b->process_ptr = tmp;
#define parent(x) x->parent

/**
 * @fn int Priority_Init(Priority_Queue_t **queue,
 *                       int (*compare)(ProcessPtr a, ProcessPtr b))
 * @brief initialize variables in Priority_Queue_t
 *
 * @param queue pointer of Priority_Queue_t *(double pointer)
 * @param compare function pointer used for constructing heap
 *
 * @return 1 if initialization successes
 */
int Priority_Init(Priority_Queue_t **queue,
                  int (*compare)(ProcessPtr_t a, ProcessPtr_t b)) {
  *queue = (Priority_Queue_t *)malloc(sizeof(Priority_Queue_t));

  if ((*queue) == NULL) {
    printf("Error log: malloc() in Priority_Init\n");
    exit(-1);
  }

  (*queue)->count = 0;
  (*queue)->top = NULL;
  (*queue)->compare = compare;

  return 1;
}

/**
 * @fn int Priority_Enqueue(Priority_Queue_t *queue, ProcessPtr_t process)
 * @brief insert a new node in queue
 *
 * @param queue pointer of Priority_Queue_t
 * @param process ProcessPtr_t of the new node
 *
 * @return 1 if enqueue successes
 */
int Priority_Enqueue(Priority_Queue_t *queue, ProcessPtr_t process) {
  int pos; /* varaible used for finding the place at which insert the node */
  TreeNodePtr_t walker;
  TreeNodePtr_t new = (TreeNodePtr_t)malloc(sizeof(TreeNode_t));

  if (new == NULL) {
    printf("Error log: malloc() in Priority_Enqueue\n");
    exit(-1);
  }

  new->process_ptr = process;
  new->left = new->right = new->parent = NULL;

  /* The inserted node is the first node in queue. */
  if (queue->count++ == 0) {
    queue->top = new;

    return 1;
  }

  /* Find the place at which insert the new node */
  walker = queue->top;
  for (pos = 2; !(pos <= queue->count && queue->count < (pos << 1));
       pos = pos << 1)
    ;
  pos = pos >> 1;

  while (pos >= 2) {
    if (pos & queue->count)
      walker = walker->right;
    else
      walker = walker->left;

    pos = pos >> 1;
  }

  /* Insert the new node in heap */
  if (pos & queue->count) {
    walker->right = new;
    new->parent = walker;
  } else {
    walker->left = new;
    new->parent = walker;
  }

  /* Heap Adjustments */
  walker = new;
  while (walker->parent != NULL) {
    if (queue->compare(walker->process_ptr, parent(walker)->process_ptr)) {
      SWAP(walker, parent(walker));
      walker = walker->parent;
    } else {
      break;
    }
  }

  return 1;
}

/**
 * @fn int Priority_Dequeue(Priority_Queue_t *queue)
 * @brief delete a node in queue
 *
 * @param queue pointer of Queue_t
 *
 * @return 1 if dequeue successes
 */
int Priority_Dequeue(Priority_Queue_t *queue) {
  int pos; /* varaible used for finding the place from which delete the node */
  TreeNodePtr_t walker;

  if (queue->count == 0) return 0;

  if (queue->count == 1) {
    free(queue->top);
    queue->count--;
    queue->top = NULL;

    return 1;
  }

  /* Find the last node in heap */
  walker = queue->top;
  for (pos = 2; !(pos <= queue->count && queue->count < (pos << 1));
       pos = pos << 1)
    ;
  pos = pos >> 1;

  while (pos >= 2) {
    if (queue->count & pos)
      walker = walker->right;
    else
      walker = walker->left;

    pos = pos >> 1;
  }

  /* Delete the last node */
  if (pos & queue->count) {
    queue->top->process_ptr = walker->right->process_ptr;
    free(walker->right);
    walker->right = NULL;
  } else {
    queue->top->process_ptr = walker->left->process_ptr;
    free(walker->left);
    walker->left = NULL;
  }

  /* Heap Adjustments */
  queue->count--;
  walker = queue->top;
  while (1) {
    /* Walker node is leaf node */
    if (walker->left == NULL && walker->right == NULL) break;

    TreeNodePtr_t child = walker->right
                              ? queue->compare(walker->left->process_ptr,
                                               walker->right->process_ptr)
                                    ? walker->left
                                    : walker->right
                              : walker->left;
    if (queue->compare(child->process_ptr, walker->process_ptr)) {
      SWAP(child, walker);
      walker = child;
    } else {
      break;
    }
  }

  return 1;
}

/**
 * @fn int Priority_Top(Priority_Queue_t *queue)
 * @brief return the pointer of a process that is at the top of the queue
 *
 * @param queue pointer of Priority_Queue_t
 *
 * @return the pointer of a process if queue is not empty
 *         NULL                     if queue is empty
 */
ProcessPtr_t Priority_Top(Priority_Queue_t *queue) {
  if (queue->count == 0) return NULL;

  return queue->top->process_ptr;
}

/**
 * @fn int Priority_IsEmpty(Priority_Queue_t queue)
 * @brief return whether the queue is empty
 *
 * @param queue pointer of Priority_Queue_t
 *
 * @return 1 if the queue is empty
 *         0 otherwise
 */
int Priority_IsEmpty(Priority_Queue_t *queue) { return queue->count == 0; }

/**
 * @fn int SJF_Compare(ProcessPtr_t a, ProcessPtr_t b)
 * @brief compare cpu burst time of two processes
 *
 * @param a first process to compare
 * @param b second process to compare
 *
 * @return 1 if cpu burst time of the first process is shorter
 *         0 otherwise
 */
int SJF_Compare(ProcessPtr_t a, ProcessPtr_t b) {
  return a->cpu_burst_time != b->cpu_burst_time
             ? a->cpu_burst_time < b->cpu_burst_time
             : a->pid < b->pid;
}

/**
 * @fn int Priority_Compare(ProcessPtr_t a, ProcessPtr_t b)
 * @brief compare priority of two processes
 *
 * @param a first process to compare
 * @param b second process to compare
 *
 * @return 1 if priority of the first process is higher
 *         0 otherwise
 */
int Priority_Compare(ProcessPtr_t a, ProcessPtr_t b) {
  return a->priority != b->priority ? a->priority < b->priority
                                    : a->pid < b->pid;
}

/**
 * @fn int IO_Burst_Compare(ProcessPtr_t a, ProcessPtr_t b)
 * @brief compare I/O burst time of two processes
 *
 * @param a first process to compare
 * @param b second process to compare
 *
 * @return 1 if I/O burst time of the first process is shorter
 *         0 otherwise
 */
int IO_Burst_Compare(ProcessPtr_t a, ProcessPtr_t b) {
  return a->io_burst_time != b->io_burst_time
             ? a->io_burst_time < b->io_burst_time
             : a->pid < b->pid;
}