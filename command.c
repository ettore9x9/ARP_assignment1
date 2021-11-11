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

const int cmd_increase_z = 1;
const int cmd_decrease_z = 2;
const int cmd_increase_x = 3;
const int cmd_decrease_x = 4;
const int cmd_stop_z = 5;
const int cmd_stop_x = 6;

int fd_x, fd_z;

pid_t pid_wd;

void interpreter(){

    int c, c1, c2;
    c = getchar(); 
    kill(pid_wd, SIGTSTP); // Send a signal to the watchdog.

    if(c == 27){
        c1 = getchar();

        if (c1 == 91){
            c2 = getchar();
            if(c2 == 65){
                //su
                printf("\nIncrease Z\n");
                write(fd_z, &cmd_increase_z, sizeof(int)); 
            }
            if(c2 == 66){
                //giu
                printf("\nDecrease Z\n");
                write(fd_z, &cmd_decrease_z, sizeof(int));    
            }
            if(c2 == 67){
                //dx
                printf("\nIncrease X\n");
                write(fd_x, &cmd_increase_x, sizeof(int));
            }
            if(c2 == 68){
                //sx
                printf("\nDecrease X\n");
                write(fd_x, &cmd_decrease_x, sizeof(int));
            }
        }
    }
    else{
        if(c == 120){
            //x stop
            printf("\nX stop\n");
            write(fd_x, &cmd_stop_x, sizeof(int));
        }
        if(c == 122){
            //z stop
            printf("\nZ stop\n");
            write(fd_z, &cmd_stop_z, sizeof(int)); 
        }
    }
}

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

    fd_z=open("fifo_command_to_mot_z", O_WRONLY);
    fd_x=open("fifo_command_to_mot_x", O_WRONLY);

    while(1){ 
        interpreter();
    }   
        
    close(fd_x);
    close(fd_z);

    /*restore the old settings*/
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);

    return 0;
}
