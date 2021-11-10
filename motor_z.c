#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

float z_position = 0;
float z_upperbound = 10;
float z_lowerbound = -10;
float z_est_pos = 0;
int step = 1;


int main(){

    int fd_z, ret;
    int command = 0;
    fd_set rset;

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    printf("I am the motor z\n");
    fd_z = open("fifo_command_to_mot_z", O_RDONLY);

while(1){
    
    FD_ZERO(&rset);
    FD_SET(fd_z, &rset);
    ret=select(FD_SETSIZE, &rset, NULL, NULL, &tv);
    if(ret==-1){
        perror("select() on motor z\n");
        fflush(stdout);
    }
    else if(FD_ISSET(fd_z, &rset)>0){
        read(fd_z, &command, sizeof(int));
    }
    if(command == 1){
        printf("Motor Z received: increase\n");
        if (z_position >= z_upperbound){
            z_position = z_upperbound;
            command = 5;
            printf("Upper limit of the work envelope reached.\n");
        } else {
            z_position += step;
        }
        fflush(stdout);
    }
    if(command == 2){
        printf("Motor Z received: decrease\n");
        if (z_position <= z_lowerbound){
            z_position = z_lowerbound;
            command = 5;
                printf("Lower limit of the work envelope reached.\n");
            } else {
                z_position -= step;
            }
        }

        if(command == 5){
            printf("Motor Z received: STOP\n");
            fflush(stdout);
        }

        // Sleep for one second, if the command does not change, than repeat again the same command.
        printf("The current Z position is: %.2f\n", z_position);
        usleep(500000);

    } // End of the while cycle.
close(fd_z);
return 0;
}