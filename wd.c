#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define PERIOD 30

int timer = PERIOD;

void signal_handler(int sig) {
    if(sig==SIGTSTP){
        timer=PERIOD;
    }   
}

int main(int argc, char * argv[]){

    int pid_motor_x=atoi(argv[1]);
    int pid_motor_z=atoi(argv[2]);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler =&signal_handler;
    sa.sa_flags=SA_RESTART;
    sigaction(SIGTSTP,&sa,NULL);

    while(1){

        while (timer >= 0){
            sleep(1);
            timer--;
        }

        kill(pid_motor_x, SIGUSR2);
        kill(pid_motor_z, SIGUSR2);
        timer=PERIOD;
    }
    return(0);
}