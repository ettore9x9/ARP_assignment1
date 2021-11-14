/*LIBRARIES*/
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
#include <stdbool.h>

/*GLOBAL VARIABLES*/

#define Z_UB 9.9
#define Z_LB 0
#define STEP 0.01
float z_position = Z_LB; //the hoist has a starting position of (X, Z)=(0, 0)
float est_pos_z = Z_LB;
int command = 0;
bool resetting=false;
FILE * log_file;

/*FUNCTIONS*/
void signal_handler(int sig);
float float_rand( float min, float max );

void signal_handler(int sig) {

    if(sig==SIGUSR1){
        command=5; //stop command
    }   
    if(sig==SIGUSR2){
        resetting=true; 
    }
}

float float_rand( float min, float max ) {
    // Function to generate a randomic error.

    float scale = rand() / (float) RAND_MAX; 
    return min + scale * ( max - min );      /* [min, max] */
}

/*MAIN()*/

int main(){

    int fd_z,fd_inspection_z; //file descriptors
    int ret; //select() system call return value
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler =&signal_handler;
    sa.sa_flags=SA_RESTART;

    struct sigaction sa2;
    memset(&sa2, 0, sizeof(sa2));
    sa2.sa_handler =&signal_handler;
    sa2.sa_flags=SA_RESTART;

    //sigaction for SIGUSR1 & SIGUSR2
    if(sigaction(SIGUSR1,&sa,NULL)==-1){
        perror("Sigaction error, SIGUSR1 on motor z\n");
        return -10;
    }
    if(sigaction(SIGUSR2,&sa2,NULL)==-1){
        perror("Sigaction error, SIGUSR2 on motor z\n");
        return -11;
    }

    log_file = fopen("Log.txt", "a"); // Open the log file

    time_t ltime = time(NULL);
    fprintf(log_file, "%.19s: motor_z   : Motor z started.\n", ctime( &ltime ) );
    fflush(log_file);

    fd_set rset; //ready set of file descriptors

    /*the select() system call does not wait for file descriptors to be ready */
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    //opening pies
    fd_z = open("fifo_command_to_mot_z", O_RDONLY);
    fd_inspection_z=open("fifo_est_pos_z", O_WRONLY);
    
    while(1){    
        FD_ZERO(&rset);
        FD_SET(fd_z, &rset);
        ret = select(FD_SETSIZE, &rset, NULL, NULL, &tv);

        if(ret == -1){ // An error occours.
            perror("select() on motor z\n");
            fflush(stdout);
        }
        else if(FD_ISSET(fd_z, &rset) != 0){ // There is something to read!
            read(fd_z, &command, sizeof(int)); // Update the command.
            ltime = time(NULL);
            fprintf(log_file, "%.19s: motor_z   : command received = %d.\n", ctime( &ltime ), command );
            fflush(log_file);
        }

        if(!resetting){ //the system is not resetting
            if(command == 1){
                if (z_position > Z_UB){ //Upper Z limit of the work envelope reached
                    command = 5; //stop command
                } else {
                    z_position += STEP; //go upwards
                }
            }
            if(command == 2){
                if (z_position < Z_LB){ //Lower Z limit of the work envelope reached
                    command = 5; //stop command
                    } else {
                        z_position -= STEP; //go downwards
                    }
                }
            if(command == 5){ //stop command
            // z_position must not change
            }
        }
        else{ //system is resetting
                while(z_position>Z_LB){
                    z_position -= STEP;
                    est_pos_z=z_position+ float_rand(-0.005,0.005); //compute the estimated position
                    write(fd_inspection_z, &est_pos_z, sizeof(float)); //send to inspection konsole
                    usleep(20000);
                }
                if(z_position<=Z_LB){
                    resetting=false;
                    command=5;
                    
                }
        }


        // Sleeps. If the command does not change, than repeats again the same command.
        est_pos_z=z_position + float_rand(-0.005,0.005); //compute the estimated position
        write(fd_inspection_z, &est_pos_z, sizeof(float)); //send to inspection konsole

        ltime = time(NULL);
        fprintf(log_file, "%.19s: motor_z   : z_position = %f\n", ctime( &ltime ), z_position);
        fflush(log_file);

        usleep(20000);

    } // End of the while cycle.

    //closing pipes
    close(fd_z);
    close(fd_inspection_z);

    ltime = time(NULL);
    fprintf(log_file, "%.19s: motor_x   : Motor z ended.\n", ctime( &ltime ) );
    fflush(log_file);
    fclose(log_file); // Close log file.

    return 0;
}