#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>

float float_rand( float min, float max )
{
    float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
    return min + scale * ( max - min );      /* [min, max] */
}

float x_position = 0;
float est_pos_x=0;
float upper=0.3;
float lower=-0.3;
float x_upperbound = 10;
float x_lowerbound = -10;
int step = 1;
int fd_x, fd_inspection_x, ret;
int command = 0;


void signal_handler(int sig) {

    if(sig==SIGUSR1){
        command=6;
    }   
    if(sig==SIGUSR2){
        x_position=0;
        command=6;
    }
}

int main(){

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler =&signal_handler;
    sa.sa_flags=SA_RESTART;

    struct sigaction sa2;
    memset(&sa2, 0, sizeof(sa2));
    sa2.sa_handler =&signal_handler;
    sa2.sa_flags=SA_RESTART;

    sigaction(SIGUSR1,&sa,NULL);
    sigaction(SIGUSR2,&sa2,NULL);

    fd_set rset;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    fd_x = open("fifo_command_to_mot_x", O_RDONLY);
    fd_inspection_x=open("fifo_est_pos_x", O_WRONLY);

    while(1){
        FD_ZERO(&rset);
        FD_SET(fd_x, &rset);
         fflush(stdout);
        ret = select(FD_SETSIZE, &rset, NULL, NULL, &tv);

        if(ret == -1){ // An error occours.
            perror("select() on motor x\n");
        }

        else if( FD_ISSET(fd_x, &rset) != 0 ){ // There is something to read!
            read(fd_x, &command, sizeof(int)); // Update the command.
        }
        
        if(command == 3){
            //printf("Motor X received: increase\n");
            if (x_position >= x_upperbound){
                
                command = 6;
               // printf("Upper X limit of the work envelope reached.\n");
            } else {
                x_position += step;
            }

        }

        if(command == 4){
            //printf("Motor X received: decrease\n");
            if (x_position <= x_lowerbound){
                x_position = x_lowerbound;
                command = 6;
               // printf("Lower X limit of the work envelope reached.\n");
            } else {
                x_position -= step;
            }
        }

        if(command == 6){
        }

        // Sleep fora a half of one second, if the command does not change, than repeat again the same command.
        printf("The current X position is: %.2f\n", x_position);
        fflush(stdout);
        est_pos_x=x_position+ float_rand(-0.05,0.05); //compute the estimated position
        write(fd_inspection_x, &est_pos_x, sizeof(float)); //send to inspection konsole
        usleep(500000);

    } // End of the while cycle.
close(fd_x);
close(fd_inspection_x);
return 0;
}