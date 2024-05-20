
#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

// set and initialze memory to the queue
queue *initialize_queue(){
    queue *new_queue = (queue *)malloc(sizeof(queue));
    new_queue->size = 0;
    new_queue->top = NULL;
    return new_queue;
}

// ordered enqueue without task_length factor
void enqueue(queue *org_queue, task *new_task){
    node *newNode = NULL;
    newNode = malloc(sizeof(node));
    newNode->t = new_task;

    if (org_queue->top == NULL){
        newNode->next = org_queue->top;
        org_queue->top = newNode;
    }
    else{
        node *current = org_queue->top;
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wunused-but-set-variable"
        node *previous = NULL;
        #pragma clang diagnostic pop
        while (current->next != NULL){
            previous = current;
            current = current->next;
        }
        if (current->next == NULL){
            newNode->next = NULL;
            current->next = newNode;
        }
    }
    org_queue->size++;
}

// Ordered enqueue method 
void enqueue_sort(queue *org_queue, task *new_task){
    node *newNode = NULL;
    newNode = malloc(sizeof(node));
    newNode->t = new_task;

    if (org_queue->top == NULL){
        newNode->next = org_queue->top;
        org_queue->top = newNode;
    }
    else{
        node *current = org_queue->top;
        node *previous = NULL;
        while ((current->t->task_length) <= new_task->task_length && current->next != NULL){
            previous = current;
            current = current->next;
        }
        if (current->t->task_length > new_task->task_length){
            newNode->next = current;

            if (previous != NULL){
                previous->next = newNode;
            }
            else{
                org_queue->top = newNode;
            }
        }
        else if (current->next == NULL){
            newNode->next = NULL;
            current->next = newNode;
        }
    }
    org_queue->size++;
}

// Dequeue the job from front or return NULL if no jobs are available
task *dequeue(queue *org_queue){
    if (org_queue->top != NULL){
        task *delete_task = (task *)malloc(sizeof(task));
        delete_task = org_queue->top->t;

        if (org_queue->top->next != NULL){
            org_queue->top = org_queue->top->next;
        }
        else{
            org_queue->top = NULL;
        }
        org_queue->size--;
        return delete_task;
    }
    return NULL;
}

// function to move all the jobs to priority queue based on S value
void move_all(queue *source, queue *target){
    task *current = dequeue(source);
    while (current != NULL){
        enqueue(target, current);
        current->curr_priority = 1;
        current = dequeue(source);
    }
}

// prints the queue
void print_queue(queue *org_queue){
    node *temp = org_queue->top;

    while (temp != NULL){
        printf("%s %d\n", temp->t->task_name, temp->t->total_run_time);
        temp = temp->next;
    }
}

// find average turn around time 
long turnaround_time(queue *org_queue){
    long ta_time = 0;
    node *curr = org_queue->top;

    while (curr != NULL){
        ta_time += curr->t->turnaround_time;
        curr = curr->next;
    }
    ta_time = ta_time / org_queue->size;
    return ta_time;
}

// find average response time
long response_time(queue *org_queue){
    long response_time = 0;
    node *curr = org_queue->top;

    while (curr != NULL){
        response_time += curr->t->response_time;
        curr = curr->next;
    }
    response_time = response_time / org_queue->size;
    return response_time;
}

