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
#include<stdio.h>
#include <termios.h>   

/*GLOBAL VARIABLES*/     
const int cmd_increase_z = 1; //this represents the "increase Z" command
const int cmd_decrease_z = 2; //this represents the "decrease Z" command
const int cmd_increase_x = 3; //this represents the "increase X" command
const int cmd_decrease_x = 4; //this represents the "decrease X" command
const int cmd_stop_z = 5; //this represents the "STOP Z" command
const int cmd_stop_x = 6; //this represents the "STOP X" command
int fd_x, fd_z; //file descriptors
pid_t pid_wd; //process ID

/*FUNCTIONS*/
void interpreter(){

    int c, c1, c2;
    c = getchar(); //get input from keyboard
    kill(pid_wd, SIGTSTP); // Send a signal to let the watchdog know that an input occurred.

    if(c == 27){ //the ASCII numbers for arrows is a combination of three numbers, the first two (27 and 91) are always the same
        c1 = getchar();

        if (c1 == 91){ 
            c2 = getchar();
            if(c2 == 65){ //third ASCII nummber for upwards arrow
                printf("\nIncrease Z\n");
                write(fd_z, &cmd_increase_z, sizeof(int)); 
            }
            if(c2 == 66){ //third ASCII nummber for downwards arrow
                printf("\nDecrease Z\n");
                write(fd_z, &cmd_decrease_z, sizeof(int));    
            }
            if(c2 == 67){ //third ASCII nummber for right arrow
                printf("\nIncrease X\n");
                write(fd_x, &cmd_increase_x, sizeof(int));
            }
            if(c2 == 68){ //third ASCII nummber for left arrow
                printf("\nDecrease X\n");
                write(fd_x, &cmd_decrease_x, sizeof(int));
            }
        }
    }
    else{
        if(c == 120){ //ASCII number for 'x' keyboard key
            printf("\nX stop\n");
            write(fd_x, &cmd_stop_x, sizeof(int));
        }
        if(c == 122){ //ASCII number for 'z' keyboard key
            printf("\nZ stop\n");
            write(fd_z, &cmd_stop_z, sizeof(int)); 
        }
    }
}

/*MAIN()*/
int main(int argc, char * argv[]){   

    static struct termios oldt, newt;
    pid_wd= atoi(argv[1]);

    /*tcgetattr gets the parameters of the current terminal
    STDIN_FILENO will tell tcgetattr that it should write the settings
    of stdin to oldt*/
    tcgetattr( STDIN_FILENO, &oldt);

    /*now the settings will be copied*/
    newt = oldt;

    /*ICANON normally takes care that one line at a time will be processed
    that means it will return if it sees a "\n" or an EOF or an EOL*/
    newt.c_lflag &= ~(ICANON);          

    /*Those new settings will be set to STDIN
    TCSANOW tells tcsetattr to change attributes immediately. */
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);

    printf("Use the following commands\n: ");
    printf("Press the upwards Arrow for moving the hoist upwards\n: ");
    printf("Press the downwards Arrow for moving the hoist downwards\n: ");
    printf("Press 'X' for stopping the horizontal movement\n: ");
    printf("Press 'Z' for stopping the vertical movement\n: ");  
    fflush(stdout); 

    //opening pipes
    fd_z=open("fifo_command_to_mot_z", O_WRONLY);
    fd_x=open("fifo_command_to_mot_x", O_WRONLY);

    while(1){ 
        interpreter();
    }   
    
    //closing pipes
    close(fd_x);
    close(fd_z);

    /*restore the old settings*/
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);

    return 0;
}
