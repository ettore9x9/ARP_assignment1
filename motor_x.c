/* LIBRARIES */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

/* CONSTANTS */
#define X_UB 9.9   // Upper bound of x axes.
#define X_LB 0     // Lower bound of x axes.
#define STEP 0.01  // Velocity of the motor.

/* GLOBAL VARIABLES */
float x_position = X_LB;   // Real position of the x motor.
float est_pos_x = X_LB;    // Estimated position of the x motor.
int command = 0;           // Command received.
bool resetting = false;    // Boolean variable for reset the motor.
bool stop_pressed = false; // Boolean variable for stop the motor.
FILE *log_file;            // Log file.

/* FUNCTIONS HEADERS */
void signal_handler(int sig);
float float_rand(float min, float max);
void logPrint ( char * string);

/* FUNCTIONS */
void signal_handler(int sig) {
    /* Function to handle stop and reset signals. */

    if (sig == SIGUSR1) { // SIGUSR1 is the signal to stop the motor.
        command = 6; //stop command
        stop_pressed = true;
    }
    if (sig == SIGUSR2){ // SIGUSR2 is the signal to reset the motor.
        stop_pressed = false;
        resetting = true;
    }
}

float float_rand(float min, float max)
{
    // Function to generate a randomic error.
    float scale = rand() / (float)RAND_MAX;
    return min + scale * (max - min); /* [min, max] */
}

void logPrint ( char * string) {
    /* Function to print on log file adding time stamps. */

    time_t ltime = time(NULL);
    fprintf( log_file, "%.19s: %s", ctime( &ltime ), string );
    fflush(log_file);
}

/*MAIN()*/

int main()
{

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

    logPrint("motor_x   : Motor x started.\n");

    fd_set rset; //ready set of file descriptors

    /*the select() system call does not wait for file descriptors to be ready */
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    //opening pipes
    fd_x = open("fifo_command_to_mot_x", O_RDONLY);
    fd_inspection_x=open("fifo_est_pos_x", O_WRONLY);

    while (1)
    {
        FD_ZERO(&rset);
        FD_SET(fd_x, &rset);

        ret = select(FD_SETSIZE, &rset, NULL, NULL, &tv);

        if (ret == -1)
        { // An error occours.
            perror("select() on motor x\n");
            return -8;
        }
        else if (FD_ISSET(fd_x, &rset) != 0)
        {                                      // There is something to read!
            read(fd_x, &command, sizeof(int)); // Update the command.

            char str[50];
            sprintf(str, "motor_x   : command received = %d.", command);
            logPrint(str);
        }

        if (!resetting)
        {
            if (command == 3)
            {

                if (x_position > X_UB)
                {                //Upper X limit of the work envelope reached
                    command = 6; //stop command
                }
                else
                {
                    x_position += STEP; //go right
                }
            }

            if (command == 4)
            {

                if (x_position < X_LB)
                {                //Lower X limit of the work envelope reached
                    command = 6; //stop command
                }
                else
                {
                    x_position -= STEP; //go left
                }
            }

            if (command == 6)
            { //stop command
                //x_position must not change
            }
        }
        else // The motor is resetting.
        {
            if ( (!stop_pressed) && (x_position > X_LB) )
            {
                x_position -= STEP;
            }
            else
            {
                resetting = false;
                command = 0;
            }
            stop_pressed = false;
        }

        // Sleeps. If the command does not change, repeats again the same command.
        est_pos_x = x_position + float_rand(-0.005, 0.005); //compute the estimated position
        write(fd_inspection_x, &est_pos_x, sizeof(float));  //send to inspection konsole

        char str[50];
        sprintf(str, "motor_x   : x_position = %f\n", x_position);
        logPrint(str);

        usleep(20000); //sleep for 0,2 second

    } // End of the while cycle.

    /* Close pipes. */
    close(fd_x);
    close(fd_inspection_x);

    logPrint("motor_x   : Motor x ended.\n");

    fclose(log_file); // Close log file.

    return 0;
}