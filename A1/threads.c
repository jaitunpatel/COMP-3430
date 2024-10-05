// testing test branch again again
//Included some required header files
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

// Global variables 
#define MAX 1000
int arrayThreads[MAX];
char config_file_path[] = "config.txt";
int totalThreads; 
int sigint_received = 0;
pthread_t worker[MAX];

//method declarations
void read_config_file();
void handle_sighup();
void handle_Threads(int num);
void handle_sigint();

//main program
//reads config file and wait for signals
int main(){
    totalThreads = 0;
    printf("\n----->Thread with Parent ID %d is started...\n", getpid());
    //send a signal to re read the config file.
    signal(SIGHUP, handle_sighup);  
    signal(SIGINT, handle_sigint); 
    read_config_file();  

    //empyt while loop to wait for signals 
    while(totalThreads > 0 && !sigint_received) {
        sleep(1); 
    }
    return 0;
}

//reading config file
void read_config_file(){
    FILE *file = fopen(config_file_path, "r");
    if (file == NULL) {
        printf("Error: Unable to open config file.\n");
        return;
    }
    // Read the number from the file
    int returnValue = 0;
    if (fscanf(file, "%d", &returnValue) != 1) {
        printf("Error: Unable to read number from config file.\n");
        fclose(file);
        return;
    }
    fclose(file);
    printf("\nNumber  of threads set in configuration file: %d\n", returnValue);
    handle_Threads(returnValue);
}

// directed the execution to readConfigFile
void handle_sighup(){
    printf("\nReceived a SIGHUP signal. Killing a thread...\n");
    read_config_file();
}

// threads working here
void  *work(){
    while (!sigint_received) {
        if (sigint_received) {
            printf("Received a SIGINT signal. Exiting thread.\n");
            pthread_exit(NULL); // Exit the thread
        }
    }
    return NULL;
}

// creating threads according to the number from the config file.
void handle_Threads(int num){
    int temp = totalThreads;
    if(totalThreads < num){
        for(int i = temp; i < num; i++){
            printf("New Thread %d is starting\n", i + 1);

            // Check if thread creation is successful
            // and increment totalThreads only if thread creation is successful
            if (pthread_create(&worker[i], NULL, &work, "Thread") == 0){
                totalThreads++;
            } 
            else {
                printf("Failed to create thread %d\n", i + 1);
            }
        }
    }
    else{
        int threads_to_cancel = (totalThreads - num);
        printf("Current active threads : %d\n", totalThreads);
        printf("Threads to cancel : %d \n", threads_to_cancel);
        int temp = totalThreads;
        for (int i = totalThreads - 1; i > (temp - 1) - threads_to_cancel; i--) {
            printf("Pausing thread %d\n", i + 1);

            // Cancel the excess thread
            pthread_cancel(worker[i]); 
            totalThreads--; 
            worker[i] = 0;
        }
    }
}

// Function to handle SIGINT (Ctrl+C) signal
void handle_sigint() {
    printf("\nReceived a SIGINT signal. Cleaning up and exiting...\n");
    sigint_received = 1; // Set the flag to indicate SIGINT is received
    // Cancel all threads
    for (int i = 0; i < totalThreads; i++) {
        pthread_cancel(worker[i]);
    }
    totalThreads = 0;
}



