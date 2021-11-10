#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

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

int main(){
    int fd_x, fd_inspection_x, ret;
    int command = 0;
    fd_set rset;

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    printf("I am the motor x\n");
    fflush(stdout);

    fd_x = open("fifo_command_to_mot_x", O_RDONLY);
    fd_inspection_x=open("fifo_est_pos_x", O_WRONLY);

    printf("dopo open");
    fflush(stdout);


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
        est_pos_x=x_position+ float_rand(-0.2,0.2); //compute the estimated position
        write(fd_inspection_x, &est_pos_x, sizeof(float)); //send to inspection konsole
        usleep(500000);

    } // End of the while cycle.
close(fd_x);
close(fd_inspection_x);
return 0;
}