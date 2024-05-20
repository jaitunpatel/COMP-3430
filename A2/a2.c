
// include header files
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <sys/types.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "queue.h"

#define NANOS_PER_USEC 1000
#define USEC_PER_SEC 1000000

// Declaring prototypes
void fileRead(char *fileName, queue *queue);
bool isNumeric(const char *str);
void *scheduler_mlfq();
void *cpu_mlfq();
void run_mlfq(task *, int, queue *, queue *);
void print_report(int);
static void microsleep(unsigned int usecs);
struct timespec diff(struct timespec start, struct timespec end);

// creating locks
pthread_mutex_t scheduler_lock;
pthread_mutex_t done_lock;
pthread_mutex_t task_lock;
pthread_mutex_t time_lock;
pthread_cond_t task_condition;

// main task queue
queue *task_queue;
int total_cpu = 0;

// 4 type of tasks
queue *type_0;
queue *type_1;
queue *type_2;
queue *type_3;

// 3 priority queues
queue *priority_1;
queue *priority_2;
queue *priority_3;

int quantum_length = 50;    //quantum length
int time_allotment = 200;   //time allotment
long s_value;               //time between moving all the jobs to the topmost queue

// variables to store time at the start and end of the program
struct timespec start_time_1;
struct timespec start_time_2;

// main function
int main(int argc, char *argv[]){
    if (argc != 4) {
        printf("\nWrong arguments entered.\n");
        printf("\nSequence expected : ./a2 <number of CPUs> <value of S> <tasks file>\n\n");
        return EXIT_FAILURE;
    }

    // initializing locks
    pthread_mutex_init(&task_lock, NULL);
    pthread_mutex_init(&time_lock, NULL);
    pthread_mutex_init(&scheduler_lock, NULL);
    pthread_mutex_init(&done_lock, NULL);
    pthread_cond_init(&task_condition, NULL);

    total_cpu = atoi(argv[1]);
    pthread_t p[total_cpu];

    char *endptr;
    long s_value = strtol(argv[2], &endptr, 10);
    char *tasks_file = argv[3];

    // confirm the value of S is valid
    if (*endptr != '\0' || !isNumeric(argv[2]) || s_value <= 0 || s_value > 10000) {
        printf("\nInvalid value for S. Please enter a valid positive integer.\n\n");
        return EXIT_FAILURE;
    }

    // main task queue 
    task_queue = initialize_queue();
    type_0 = initialize_queue();
    type_1 = initialize_queue();
    type_2 = initialize_queue();
    type_3 = initialize_queue();

    // read the file and store tasks in the queue
    fileRead(tasks_file, task_queue);

    // get the time of the start of execution
    clock_gettime(CLOCK_REALTIME, &start_time_1);
    clock_gettime(CLOCK_REALTIME, &start_time_2);

    // initialize priority queues
    priority_1 = initialize_queue();
    priority_2 = initialize_queue();
    priority_3 = initialize_queue();

    // create threads
    for (int i = 0; i <= total_cpu; i++){
        if (i == 0){
            pthread_create(&p[i], NULL, &scheduler_mlfq, (void *)p[i]);
        }
        else{
            pthread_create(&p[i], NULL, &cpu_mlfq, (void *)p[i]);
        }            
    }
    for (int i = 0; i <= total_cpu; i++){
        pthread_join(p[i], NULL);
    }
    print_report(total_cpu);  // print the final print_report

    // destroy all the locks
    pthread_mutex_destroy(&task_lock);
    pthread_mutex_destroy(&time_lock);
    pthread_mutex_destroy(&scheduler_lock);
    pthread_mutex_destroy(&done_lock);
    pthread_cond_destroy(&task_condition);

    pthread_exit(NULL);
    return 0;
}

// reads the file 
void fileRead(char *fileName, queue *queue){
    FILE *tasks_file = fopen(fileName, "r");

    char line[100];
    char *token;

    while (fgets(line, 100, tasks_file)){
        // initialize allocate memory
        int counter = 0;
        token = strtok(line, " ");
        task *new_tasks = (task *)malloc(sizeof(task));

        while (token){
            if (counter == 0){
                strcpy(new_tasks->task_name, token);
            }
            else if (counter == 1){
                new_tasks->task_type = atoi(token);
            }   
            else if (counter == 2){
                new_tasks->task_length = atoi(token);
            }   
            else if (counter == 3){
                new_tasks->odds_for_IO = atoi(token);
            }
            token = strtok(NULL, " ");
            counter++;
        }

        // initialise to default values at first
        new_tasks->total_run_time = 0;
        new_tasks->turnaround_time = 0;
        new_tasks->response_time = 0;
        new_tasks->visited = 0;
        new_tasks->allotment_time_mlfq = 0;
        new_tasks->curr_priority = 0;
        enqueue_sort(queue, new_tasks);
    }
    fclose(tasks_file); 
}

// check if a string contains only numeric values
bool isNumeric(const char *str) {
    while (*str) {
        if (*str < '0' || *str > '9')
            return false;
        str++;
    }
    return true;
}

// executes the taks as long as long as they are in any priority
// and signals the waiting CPUs to check for next task
void *scheduler_mlfq(){
    while (priority_1->size + priority_2->size + priority_3->size > 0 || task_queue->size > 0){
        pthread_mutex_lock(&scheduler_lock);
        pthread_cond_signal(&task_condition);
        pthread_mutex_unlock(&scheduler_lock);
    }
    return NULL;
}

// Implemented RULE 1, 3 and 5 of MLFQ in this function
void *cpu_mlfq(){
    while (priority_1->size + priority_2->size + priority_3->size > 0 || task_queue->size > 0){
        pthread_mutex_lock(&task_lock);
        struct timespec current_time;
        clock_gettime(CLOCK_REALTIME, &current_time);

        // RULE 5 - After some time period S, move all jobs in the system to the topmost queue
        if (diff(start_time_2, current_time).tv_nsec / NANOS_PER_USEC >= s_value){
            move_all(priority_2, priority_1); // move jobs from priority 2 to 1
            move_all(priority_3, priority_1); // move jobs from priority 3 to 1
            start_time_2 = current_time;
        }

        // RULE 3 - When a job enters, it is placed at the highest priority
        task *temp = dequeue(task_queue);
        if (temp != NULL){
            enqueue(priority_1, temp);
            temp->curr_priority = 1;
        }
        task *curr_task = NULL;
        queue *curr_queue = NULL;
        queue *nxt_queue = NULL;

        // RULE 1 : If Priority(A) > Priority(B), then run A
        if (priority_1->size > 0){
            curr_task = dequeue(priority_1);
            curr_queue = priority_1;
            nxt_queue = priority_2;
        }
        else if (priority_2->size > 0){
            curr_task = dequeue(priority_2);
            curr_queue = priority_2;
            nxt_queue = priority_3;
        }
        else if (priority_3->size > 0){
            curr_task = dequeue(priority_3);
            curr_queue = priority_3;
            nxt_queue = priority_3;
        }
        else if (task_queue->size > 0) {
            curr_task = dequeue(task_queue);
            curr_queue = task_queue;
            nxt_queue = priority_1;
        }
        while (curr_task == NULL){
            pthread_cond_wait(&task_condition, &scheduler_lock);
        } 
        pthread_mutex_unlock(&task_lock);
        run_mlfq(curr_task, curr_task->curr_priority, curr_queue, nxt_queue);
    }
    return NULL;
}

// Implemented RULE 2 and 4 of MLFQ in this function
void run_mlfq(task *curr_task, int priority_level, queue *curr_queue, queue *nxt_queue){
    time_t random_number_t;
    srand((unsigned)time(&random_number_t));
    long random_num_1 = rand() % 100;
    long random_num_2 = rand() % 50;
    struct timespec end_time;

    // get response time for each task.
    pthread_mutex_lock(&time_lock);
    if (curr_task->visited == 0){
        clock_gettime(CLOCK_REALTIME, &end_time);
        curr_task->response_time = diff(start_time_1, end_time).tv_nsec / NANOS_PER_USEC;
        curr_task->visited = 1;
    }
    pthread_mutex_unlock(&time_lock);

    // Determine whether or not a task will do I/O
    if (random_num_1 <= curr_task->odds_for_IO && random_num_2 <= curr_task->task_length){
        int execute_time = 200 - curr_task->allotment_time_mlfq;

        // If the task has utilized less allotted time than a randomly generated number, it should continue running and its priority should be lowered.
        if (execute_time < random_num_2){
            microsleep(execute_time);
            curr_task->task_length -= execute_time;
            curr_task->allotment_time_mlfq = 0;

            // RULE 4 - lower the priority after running for alloted quantum_length
            pthread_mutex_lock(&task_lock);
            enqueue(nxt_queue, curr_task);
            if (curr_task->curr_priority < 3)
                curr_task->curr_priority = priority_level + 1;
            pthread_mutex_unlock(&task_lock);
        }
        else{
            //run the I/O task
            microsleep(random_num_2);
            curr_task->total_run_time += random_num_2;
            curr_task->allotment_time_mlfq += random_num_2;
            curr_task->task_length -= random_num_2;
            pthread_mutex_lock(&task_lock);
            enqueue(curr_queue, curr_task);
            pthread_mutex_unlock(&task_lock);
        }
    }
    // RULE 2 - If Priority(A) = Priority(B), A & B runs in round-robin fashion
    else if (curr_queue->size > 0){
        if (curr_task->task_length > quantum_length){
            microsleep(quantum_length);
            curr_task->task_length -= quantum_length;
            curr_task->allotment_time_mlfq += quantum_length;

            // keep the job at same priority if allocated time is not finished yet or lower the priority
            if (curr_task->allotment_time_mlfq >= 200){
                curr_task->allotment_time_mlfq = 0;
                pthread_mutex_lock(&task_lock);
                enqueue(nxt_queue, curr_task);

                if (curr_task->curr_priority < 3){
                    curr_task->curr_priority = priority_level + 1;
                }
                pthread_mutex_unlock(&task_lock);
            }
            else{
                pthread_mutex_lock(&task_lock);
                enqueue(curr_queue, curr_task);
                pthread_mutex_unlock(&task_lock);
            }
        }
        else if (quantum_length >= curr_task->task_length){
            microsleep(curr_task->task_length);
            curr_task->task_length = 0;

            // Calculate Turnaround time - Time (completion) - Time (arrival)
            pthread_mutex_lock(&time_lock);
            clock_gettime(CLOCK_REALTIME, &end_time);
            curr_task->turnaround_time = diff(start_time_1, end_time).tv_nsec / NANOS_PER_USEC;
            pthread_mutex_unlock(&time_lock);

            // put the tasks to the done queues based on their types
            pthread_mutex_lock(&done_lock);
            if (curr_task->task_type == 0)
                enqueue(type_0, curr_task);
            else if (curr_task->task_type == 1)
                enqueue(type_1, curr_task);
            else if (curr_task->task_type == 2)
                enqueue(type_2, curr_task);
            else if (curr_task->task_type == 3)
                enqueue(type_3, curr_task);
            pthread_mutex_unlock(&done_lock);
        } 
    }
    // execute if only one task is in a queue
    else{
        int execute_time = time_allotment - curr_task->allotment_time_mlfq;
        if (execute_time < curr_task->task_length){
            microsleep(execute_time);
            curr_task->task_length -= execute_time;
            curr_task->allotment_time_mlfq = 0;

            // RULE 4 - lower the priority after running for alloted quantum_length
            pthread_mutex_lock(&task_lock);
            enqueue(nxt_queue, curr_task);
            if (curr_task->curr_priority < 3)
                curr_task->curr_priority = priority_level + 1;
            pthread_mutex_unlock(&task_lock);
        }
        else{
            if (curr_task->task_length < time_allotment){
                microsleep(curr_task->task_length);
                curr_task->task_length = 0;

                // Calculate Turnaround time - Time (completion) - Time (arrival)
                pthread_mutex_lock(&time_lock);
                clock_gettime(CLOCK_REALTIME, &end_time);
                curr_task->turnaround_time = diff(start_time_1, end_time).tv_nsec / NANOS_PER_USEC;
                pthread_mutex_unlock(&time_lock);

                // put the tasks to the done queues based on their types
                pthread_mutex_lock(&done_lock);
                if (curr_task->task_type == 0){
                    enqueue(type_0, curr_task);
                }   
                else if (curr_task->task_type == 1){
                    enqueue(type_1, curr_task);
                }   
                else if (curr_task->task_type == 2){
                    enqueue(type_2, curr_task);
                }   
                else if (curr_task->task_type == 3){
                    enqueue(type_3, curr_task);
                }
                pthread_mutex_unlock(&done_lock);
            }
            else{
                // run for its alloted time and lower the priority level
                microsleep(time_allotment);
                curr_task->task_length -= time_allotment;
                curr_task->allotment_time_mlfq = 0;
                pthread_mutex_lock(&task_lock);
                enqueue(nxt_queue, curr_task);

                if (curr_task->curr_priority < 3)
                    curr_task->curr_priority = priority_level + 1;
                pthread_mutex_unlock(&task_lock);
            }
        }
    }
}

// print the print_report and analysis
void print_report(int cpu){
    printf("\nUsing mlfq with %d CPUs.\n\n", cpu);
    printf("Average turnaround time per type: \n\n");
    printf("    Type 0 : %ld usec\n", turnaround_time(type_0));
    printf("    Type 1 : %ld usec\n", turnaround_time(type_1));
    printf("    Type 2 : %ld usec\n", turnaround_time(type_2));
    printf("    Type 3 : %ld usec\n\n", turnaround_time(type_3));

    printf("Average response time per type: \n\n");
    printf("    Type 0 : %ld usec\n", response_time(type_0));
    printf("    Type 1 : %ld usec\n", response_time(type_1));
    printf("    Type 2 : %ld usec\n", response_time(type_2));
    printf("    Type 3 : %ld usec\n\n", response_time(type_3));
}

// Code snippet provided in the assignment description
static void microsleep(unsigned int usecs){
    long seconds = usecs / USEC_PER_SEC;
    long nanos = (usecs % USEC_PER_SEC) * NANOS_PER_USEC;
    struct timespec t = {.tv_sec = seconds, .tv_nsec = nanos};
    int ret;
    do{
        ret = nanosleep(&t, &t);
        // need to loop, `nanosleep` might return before sleeping
        // for the complete time (see `man nanosleep` for details)
    } while (ret == -1 && (t.tv_sec || t.tv_nsec));
}


// check the time difference at 2 specific time periods
struct timespec diff(struct timespec start, struct timespec end){
    struct timespec temp;

    if ((end.tv_nsec - start.tv_nsec) < 0){
        temp.tv_sec = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    }
    else{
        temp.tv_sec = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return temp;
}

