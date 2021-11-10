#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

float x_position = 0;
float x_upperbound = 10;
float x_lowerbound = -10;
int step = 1;

int main(){

    int fd_x, ret;
    int command = 0;
    fd_set rset;

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    printf("I am the motor x\n");
    fd_x = open("fifo_command_to_mot_x", O_RDONLY);

    while(1){
        FD_ZERO(&rset);
        FD_SET(fd_x, &rset);
        ret = select(FD_SETSIZE, &rset, NULL, NULL, &tv);

        if(ret == -1){ // An error occours.
            perror("select() on motor x\n");
        }

        else if( FD_ISSET(fd_x, &rset) > 0 ){ // There is something to read!
            read(fd_x, &command, sizeof(int)); // Update the command.
        }
        
        if(command == 3){
            //printf("Motor X received: increase\n");
            if (x_position >= x_upperbound){
                x_position = x_upperbound;
                command = 6;
                //printf("Upper limit of the work envelope reached.\n");
            } else {
                x_position += step;
            }
            fflush(stdout);
        }

        if(command == 4){
            printf("Motor X received: decrease\n");
            if (x_position <= x_lowerbound){
                x_position = x_lowerbound;
                command = 6;
                printf("Lower limit of the work envelope reached.\n");
            } else {
                x_position -= step;
            }
        }

        if(command == 6){
            printf("Motor X received: STOP\n");
            fflush(stdout);
        }

        // Sleep for one second, if the command does not change, than repeat again the same command.
        printf("\rThe current X position is: %.2f", x_position);
        usleep(500000);

    } // End of the while cycle.
close(fd_x);
return 0;
}