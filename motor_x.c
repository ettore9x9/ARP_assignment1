/*LIBRARIES*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>

/*GLOBAL VARIABLES*/

#define X_UB 9.9
#define X_LB 0
#define STEP 0.01
float x_position = X_LB; //the hoist has a starting position of (X, Z)=(0, 0)
float est_pos_x = X_LB;
int command = 0;
FILE * log_file;

/*FUNCTIONS*/
void signal_handler(int sig);
float float_rand( float min, float max );

void signal_handler(int sig) {

    if(sig == SIGUSR1){
        command = 6; //stop command
    }   
    if(sig == SIGUSR2){
        x_position = 0; //reset the x_position
        command = 6; //and then stop the hoist movement
    }
}

float float_rand( float min, float max ) {
    // Function to generate a randomic error.
    float scale = rand() / (float) RAND_MAX;
    return min + scale * ( max - min );      /* [min, max] */
}

/*MAIN()*/

int main(){

    int fd_x, fd_inspection_x; //file descriptors
    int ret; //select() system call return value
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &signal_handler;
    sa.sa_flags = SA_RESTART;

    struct sigaction sa2;
    memset(&sa2, 0, sizeof(sa2));
    sa2.sa_handler =&signal_handler;
    sa2.sa_flags=SA_RESTART;

    //sigaction for SIGUSR1 & SIGUSR2
    if(sigaction(SIGUSR1,&sa,NULL)==-1){
        perror("Sigaction error, SIGUSR1 motor x\n");
        return -6;
    }
    if(sigaction(SIGUSR2,&sa2,NULL)==-1){
        perror("Sigaction error, SIGUSR2 motor x");
        return -7;
    }

    log_file = fopen("Log.txt", "a"); // Open the log file

    time_t ltime = time(NULL);
    fprintf(log_file, "%.19s: motor_x   : Motor x started.\n", ctime( &ltime ) );
    fflush(log_file);

    fd_set rset; //ready set of file descriptors

    /*the select() system call does not wait for file descriptors to be ready */
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    //opening pipes
    fd_x = open("fifo_command_to_mot_x", O_RDONLY);
    fd_inspection_x=open("fifo_est_pos_x", O_WRONLY);

    while(1){
        FD_ZERO(&rset);
        FD_SET(fd_x, &rset);

        ret = select(FD_SETSIZE, &rset, NULL, NULL, &tv);

        if(ret == -1){ // An error occours.
            perror("select() on motor x\n");
            return -8;
        }
        else if( FD_ISSET(fd_x, &rset) != 0 ){ // There is something to read!
            read(fd_x, &command, sizeof(int)); // Update the command.

            ltime = time(NULL);
            fprintf(log_file, "%.19s: motor_x   : command received = %d.\n", ctime( &ltime ), command );
            fflush(log_file);
        }

        if(command == 3){
            if (x_position > X_UB){ //Upper X limit of the work envelope reached
                command = 6; //stop command
            } else {
                x_position += STEP; //go right
            }
        }

        if(command == 4){ 
            if (x_position < X_LB){ //Lower X limit of the work envelope reached
                command = 6; //stop command
            } else {
                x_position -= STEP; //go left
            }
        }

        if(command == 6){ //stop command
        //x_position must not change
        }

        // Sleeps. If the command does not change, repeats again the same command.
        est_pos_x=x_position+ float_rand(-0.005,0.005); //compute the estimated position
        write(fd_inspection_x, &est_pos_x, sizeof(float)); //send to inspection konsole

        ltime = time(NULL);
        fprintf(log_file, "%.19s: motor_x   : x_position = %f\n", ctime( &ltime ), x_position);
        fflush(log_file);

        usleep(20000); //sleep for 0,2 second

    } // End of the while cycle.

    //closing pipes
    close(fd_x);
    close(fd_inspection_x);

    ltime = time(NULL);
    fprintf(log_file, "%.19s: motor_x   : Motor x ended.\n", ctime( &ltime ) );
    fflush(log_file);
    fclose(log_file); // Close log file.

    return 0;
}