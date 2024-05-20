
## Files Submitted
    - elfRead.c, process.c, threads.c, Makefile, hello.out64, config.txt, README.md

## Compile and Run the .C Files 

Compile : make 
    - The 'make' command will compile all the required files (elfRead.c, process.c, threads.c)

### Q1 Reading ELF file

    Run     : type -> elfRead hello.out64  
            : 'hello.out64' is already included in the submission. Replace it with your own file if you want to test with a different file.

    Clean   : make clean   
            : it will remove the executable "elfRead"

### Q2 Process and Thread Herding

    Run     : type -> process
            : Connect to the same machine from another terminal where process is executing. Use only Parent PID to send SIGHUP to each process
            : type -> kill -HUP PID 
            : Ex -> kill -HUP 12345  
            : To send SIGINT  to a process, use ctrl+c only to interrupt

    Run     : type -> threads
            : Connect to the same machine from another terminal where threads is executing. Use only Parent PID to send SIGHUP to each thread
            : type -> kill -HUP PID 
            : Ex -> kill -HUP 12345   
            : To send SIGINT  to a process, use ctrl+c only to interrupt

    Clean   : make clean   
              - it will remove the executables "process" & "threads"

- All the files are working as expected. I have tested them on aviary as well. You can change the value in config.txt during run time and
  program should be able to add/delete processes or threads in running state.

- Thank you!

