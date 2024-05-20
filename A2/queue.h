
typedef struct TASK{
    char task_name[100];        // task name
    int task_type;              // task type
    int task_length;            // Task length
    int odds_for_IO;            // Odds of IO
    int total_run_time;         // total run time
    long turnaround_time;       // turnaround time
    long response_time;         // response time
    int visited;                // check if visited once or not
    int allotment_time_mlfq;    // time alloted
    int curr_priority;          // priority level
} task;

// Struct for a node in a linked list.
typedef struct NODE{
    task *t;
    struct NODE *next;
} node;

// Struct for queue
typedef struct QUEUE{
    node *top;
    int size;
} queue;

queue *initialize_queue();          // initialize the queue
void enqueue_sort(queue *, task *); // add the values in a sorting order
void enqueue(queue *, task *);      // enqueue at the end of the queue
task *dequeue(queue *);             // remove the first task
void print_queue(queue *);          // print the tasks in the queue
long turnaround_time(queue *);      // turnaround time of the tasks
long response_time(queue *);        // response time of the tasks
void move_all(queue *, queue *);    // move tasks from one queue to another
