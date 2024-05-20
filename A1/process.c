
//Included some required header files
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/mman.h>

// Global variables for configuration
#define MAX 1000
int num_workers = 0;
int new_num_workers = 0;
int total_child_processes = 0;
char config_file_path[] = "config.txt";
pid_t active_processes[MAX];

//method declarations
int read_config_file();
void worker_process();
struct SharedData* shared_data;
void handle_sigint();
void handle_sighup();

struct SharedData {
    int value;
};

// Function to create and initialize shared memory
// since processes don't shared memory amongst themselves by default
// Shared memory is used to store data which will be consistent across PARENT and CHILD process
// Talked with prof about this implementation
struct SharedData* create_shared_memory() {
    struct SharedData* shared_data = mmap(NULL, sizeof(struct SharedData), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_data == MAP_FAILED) {
        perror("Error creating shared memory");
        exit(1);
    }
    shared_data->value = 0;
    return shared_data;
}

// main program
// reads config file and wait for signals
int main() {
    shared_data = create_shared_memory();
    // Set up signal handlers
    signal(SIGINT, handle_sigint);
    signal(SIGHUP, handle_sighup);
    shared_data->value = read_config_file();
    printf("\nPARENT PID : %d <--- use this ID to send SIGHUP\n", getpid());
    
    printf("\n----->Starting %d worker processes...\n\n", shared_data->value);
    // Fork worker processes
    for (int i = 0; i < shared_data->value; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("Error creating worker process");
            exit(1); 
        } else if (pid == 0) {
            // Child process
            printf("%d process is starting\n", getpid());
            worker_process();
            exit(0);
        }
        else{
            active_processes[total_child_processes] = pid;
            total_child_processes++;
        }
    }
    // The main process waits for all child processes to exit
    int status;
    pid_t child_pid;
    while ((child_pid = waitpid(-1, &status, 0)) > 0) {
        printf("Worker process with PID main %d exited.\n", child_pid);
        for (int i = 0; i < shared_data->value; i++) {
            if (active_processes[i] == child_pid) {
                active_processes[i] = 0;  // Set to 0 to mark as inactive
                break;
            }
        }
        // Print remaining active processes
        printf("\n----->Remaining active processes: \n");
        for (int i = 0; i < shared_data->value; i++) {
            if (active_processes[i] != 0) {
                printf("%d process is active\n", active_processes[i]);
            }
        }    
    }
    printf("\n----------->All worker processes have exited. Exiting the main process<-----------\n\n");
    return 0;
}

// reading config file
int read_config_file() {
    int returnValue = 0;
    FILE* config_file = fopen(config_file_path, "r");
    if (config_file == NULL) {
        perror("Error opening configuration file");
        exit(1);
    }
    fscanf(config_file, "%d", &returnValue);
    fclose(config_file);
    return returnValue;
}

// Function to handle SIGINT (Ctrl+C) signal
void handle_sigint() {
    printf("\nReceived SIGINT. Cleaning up and exiting.\n");
    // Terminate all child processes
    for (int i = 0; i < num_workers; i++) {
        kill(-getpgid(0), SIGTERM);  // Send SIGTERM to all processes in the process group
    }
    // Wait for all child processes to exit
    int status;
    pid_t child_pid;
    while ((child_pid = waitpid(-1, &status, 0)) > 0) {
        printf("Worker process with PID %d exited.\n", child_pid);
    }
    exit(0);
}

// Function to handle SIGHUP signal
void handle_sighup() {
    printf("\nReceived SIGHUP to PARENT with ID %d\n",getpid());
    // Read the new configuration from the file
    int temp = shared_data->value;
    shared_data->value = read_config_file();
    int diff = shared_data->value - temp;
    // Compare the number of processes with the previous value
    if (temp < shared_data->value) {
        // Add 'diff' number of processes to the active list
        printf("\n----->Adjusting number of process from %d to %d\n", temp, shared_data->value);
        printf("\nAdding %d new processes to the active list.\n", diff);

        for (int i = 0; i < diff; i++) {
            pid_t pid = fork();
            signal(SIGHUP, handle_sighup);
            if (pid == -1) {
                perror("Error creating worker process");
                exit(1);
            } else if (pid == 0) {
                // Child process
                printf("%d process is starting\n", getpid());
                worker_process();
                exit(0);
            } else {
                active_processes[total_child_processes] = pid;
                total_child_processes++;
            }
        }
    }
    else if (temp > shared_data->value){
        printf("\n----->Adjusting number of process from %d to %d\n", temp, shared_data->value);
        diff = -diff;
        for(int i = 0; i < diff; i++){
            //printf("\n\nActive process = %d\n\n", active_processes[total_child_processes - 1]);
            kill(active_processes[total_child_processes - 1], SIGTERM);
            //printf("Killed process as requested by supervisor.\n");
            total_child_processes--;
        }
    }
}

// Worker process function (empty while loop)
void worker_process() {
    while (1) {}
}



