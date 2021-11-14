#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>

/*it checks the other processes, and sends a reset command in
case all processes do nothing (no computation, no motion, no input/output) for 60 seconds*/
#define PERIOD 60

int timer = PERIOD;
FILE * log_file;

// FUNCTIONS
void signal_handler(int sig);

void signal_handler(int sig) {
    if(sig==SIGTSTP){
        timer=PERIOD; //update the timer variable
    }   
}

int main(int argc, char * argv[]){

    //motors PIDs
    int pid_motor_x=atoi(argv[1]);
    int pid_motor_z=atoi(argv[2]);

    log_file = fopen("Log.txt","a"); // Open the log file

    time_t ltime = time(NULL);
    fprintf(log_file, "%.19s: wd        : Watchdog started\n", ctime( &ltime ) );
    fflush(log_file);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler =&signal_handler;
    sa.sa_flags=SA_RESTART;
    if(sigaction(SIGTSTP,&sa,NULL)==-1){
        perror("Sigaction error, SIGTSTP in WatchDog\n");
        return -9;
    }

    while(1){

        while (timer >= 0){ //waiting...
            sleep(1);
            timer--; 
        }
    //if 60 seconds are over then send signals to motors
        kill(pid_motor_x, SIGUSR2);
        kill(pid_motor_z, SIGUSR2);

        ltime = time(NULL);
        fprintf(log_file, "%.19s: wd        : Reset for time ellapsing\n", ctime( &ltime ) );
        fflush(log_file);

        timer=PERIOD; //update timer variable
    }

    ltime = time(NULL);
    fprintf(log_file, "%.19s: wd        : Watchdog ended.\n", ctime( &ltime ) );
    fflush(log_file);
    fclose(log_file); // Close log file.

    return(0);
}