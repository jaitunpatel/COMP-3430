
## User Instructions

To compile :  make         
To run     :  ./a2 4 3200 tasks.txt (feel free to change any values or different file to read)

           : 4 - # of CPUs
           : 3200 - Value of S, the time between moving all the jobs to the topmost queue
           : tasks.txt - file to read
           : Program should not take more than 4-5 sec to execute
      
To clean the file : make clean                 


## files included in the submission

- a2.c    
- queue.c
- queue.h
- tasks.txt  
- Makefile       
- README.md

## Report and Analysis : 

Average turnaround time per type: 

| CPU Count | Time Allotment (S)  |     Type 0     |     Type 1     |     Type 2     |     Type 3     |
|-----------|---------------------|----------------|----------------|----------------|----------------|
| 1         | 200 usec            | 146247 usec    | 521523 usec    | 428962 usec    | 509026 usec    |
|           | 800 usec            | 146682 usec    | 528981 usec    | 454516 usec    | 526632 usec    |
|           | 1600 usec           | 152945 usec    | 539786 usec    | 498521 usec    | 508478 usec    |
|           | 3200 usec           | 142872 usec    | 507816 usec    | 499064 usec    | 517657 usec    |
|-----------|---------------------|----------------|----------------|----------------|----------------|
| 2         | 200 usec            | 74349 usec     | 266707 usec    | 560464 usec    | 479025 usec    |
|           | 800 usec            | 73494 usec     | 259511 usec    | 560720 usec    | 465188 usec    |
|           | 1600 usec           | 74858 usec     | 266302 usec    | 437748 usec    | 480546 usec    |
|           | 3200 usec           | 83005 usec     | 291543 usec    | 526041 usec    | 446800 usec    |
|-----------|---------------------|----------------|----------------|----------------|----------------|
| 8         | 200 usec            | 19984 usec     | 68804 usec     | 268202 usec    | 262271 usec    |
|           | 800 usec            | 18168 usec     | 66759 usec     | 267139 usec    | 294015 usec    |
|           | 1600 usec           | 23617 usec     | 79097 usec     | 302784 usec    | 272452 usec    |
|           | 3200 usec           | 18586 usec     | 68095 usec     | 265964 usec    | 273975 usec    |

Average response time per type:

| CPU Count | Time Allotment (S)  |     Type 0     |     Type 1     |     Type 2     |     Type 3     |
|-----------|---------------------|----------------|----------------|----------------|----------------|
| 1         | 200 usec            | 2459 usec      | 6595 usec      | 14001 usec     | 13022 usec     |
|           | 800 usec            | 2424 usec      | 6585 usec      | 13941 usec     | 12966 usec     |
|           | 1600 usec           | 2432 usec      | 6620 usec      | 14242 usec     | 13220 usec     |
|           | 3200 usec           | 2472 usec      | 6604 usec      | 13824 usec     | 12872 usec     |
|-----------|---------------------|----------------|----------------|----------------|----------------|
| 2         | 200 usec            | 1098 usec      | 2961 usec      | 6225 usec      | 5785 usec      |
|           | 800 usec            | 1340 usec      | 2905 usec      | 5727 usec      | 5358 usec      |
|           | 1600 usec           | 1348 usec      | 3017 usec      | 5570 usec      | 5244 usec      |
|           | 3200 usec           | 1122 usec      | 3087 usec      | 6656 usec      | 6179 usec      | 
|-----------|---------------------|----------------|----------------|----------------|----------------|
| 8         | 200 usec            | 400 usec       | 800 usec       | 1491 usec      | 1402 usec      |
|           | 800 usec            | 540 usec       | 1115 usec      | 2040 usec      | 1920 usec      |
|           | 1600 usec           | 451 usec       | 709 usec       | 1200 usec      | 1136 usec      |
|           | 3200 usec           | 494 usec       | 1049 usec      | 1983 usec      | 1860 usec      |


## How does the value of S affect turnaround time and response time? Is the difference in turnaround time and response time what you expected to see as S and the number of CPUs change? Why or why not?
- Overall average turnaround time is higher than average response time because turnaorund time is counted when the task completes its entire task_length on cpu whereas response time counts the time between the arrival of job and its first run time. 

- Increasing the value of S means that tasks are allowed to run for longer periods without being preempted and moved to lower priority queues. As a result, tasks might spend more time waiting in higher priority queues before being executed, potentially increasing their turnaround time.

- Response time may remain relatively stable or slightly decreases. With a higher value of S, tasks have more time to execute before being preempted, leading to quicker responses for new tasks and potentially for tasks that have already been waiting in the queue.

- Increasing the number of CPUs will lead to decrease in turnaround time as tasks spend less time in lower priority queue, allowing to be executed more promptly with available CPUs.Also, increase in response time is observed as the tasks are spending more time waiting in queues.

- Adjusting the number of cpu affects a lot for both turnaround time and response time and in each policy, Because, when you make more cpus available for the tasks to get executed, the cpus(threads) can run parallely and able to execute more tasks in less time.   


## How does adjusting the S value in the system affect the turnaround time or response time for long-running and I/O tasks specifically? Does it appear to be highly correlated?
- Increasing the value of S leads to longer turnaround time for long-running tasks because the tasks remain in lower priority queues for extended periods before being executed which results in delayed completion times. The I/O tasks spend significant portion of their time waiting for I/O operations to complete, the quantum length have less influence on their turnaround times compared to CPU-bound tasks.

-Increasing the value of S leads to longer response times as tasks remain in lower priority just like long-running tasks resulting in delayed completion times. In additon, the response time for I/O-bound tasks increase with larger value of S.

- The relationship between adjusting the S value and turnaround time/response time for both long-running and I/O-bound tasks is not highly correlated because the effect of S on these metrics depends on various factors such as task characteristics, system load, and scheduling algorithms.
