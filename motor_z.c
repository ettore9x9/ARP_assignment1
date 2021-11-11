#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

float float_rand( float min, float max )
{
    float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
    return min + scale * ( max - min );      /* [min, max] */
}

float z_position = 0;
float est_pos_z=0;
float z_upperbound = 10;
float z_lowerbound = -10;
float z_est_pos = 0;
int step = 1;
int command = 0;

void signal_handler(int sig) {

    if(sig==SIGUSR1){
        command=5;
    }   
    if(sig==SIGUSR2){
        z_position=0;
        command=5;
    }
}

int main(){

    int fd_z,fd_inspection_z, ret;
    fd_set rset;

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

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    fd_z = open("fifo_command_to_mot_z", O_RDONLY);
    fd_inspection_z=open("fifo_est_pos_z", O_WRONLY);
    
while(1){    
    FD_ZERO(&rset);
    FD_SET(fd_z, &rset);
    ret=select(FD_SETSIZE, &rset, NULL, NULL, &tv);

    if(ret==-1){
        perror("select() on motor z\n");
        fflush(stdout);
    }
    else if(FD_ISSET(fd_z, &rset)!=0){
        read(fd_z, &command, sizeof(int));
    }
    if(command == 1){
        //printf("Motor Z received: increase\n");
        if (z_position >= z_upperbound){
           
            command = 5;
           // printf("\rUpper Z limit of the work envelope reached.");
        } else {
            z_position += step;
        }
    }
    if(command == 2){
        //Motor Z received: decrease
        if (z_position <= z_lowerbound){
            z_position = z_lowerbound;
            command = 5;
            //Lower Z limit of the work envelope reached
            } else {
                z_position -= step;
            }
        }
    if(command == 5){
        //do nothing
        }


        // Sleep for a half of a second, if the command does not change, than repeat again the same command.
        printf("The current Z position is: %.2f\n", z_position);
        fflush(stdout);
        est_pos_z=z_position + float_rand(-0.05,0.05); //compute the estimated position
        write(fd_inspection_z, &est_pos_z, sizeof(float)); //send to inspection konsole
        usleep(500000);

    } // End of the while cycle.
close(fd_z);
close(fd_inspection_z);
return 0;
}