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

int fd_x, fd_z;
int command_increase_z=1;
int command_decrease_z=2;
int command_increase_x=3;
int command_decrease_x=4;
int command_stop_z=5;
int command_stop_x=6;

void get_send_kb_input(int c, int c1, int c2){
    c=getchar(); 
    if(c==27){
        c1=getchar();
        c2=getchar();
        if(c2==65){
            //su
            printf("\nIncrease Z\n");
            fd_z=open("fifo_command_to_mot_z", O_WRONLY);
            write(fd_z, &command_increase_z, sizeof(int));
            //sleep(1);
            //close(fd_z);    
        }
        if(c2==66){
            //giu
            printf("\nDecrease Z\n");
             fd_z=open("fifo_command_to_mot_z", O_WRONLY);
            write(fd_z, &command_decrease_z, sizeof(int));
            //sleep(1);
           // close(fd_z);      
        }
        if(c2==67){
            //dx
            printf("\nIncrease X\n");
            fd_x=open("fifo_command_to_mot_x", O_WRONLY);
            write(fd_x, &command_increase_x, sizeof(int));
            //sleep(1);
            //close(fd_x);
        }
        if(c2==68){
            //sx
            printf("\nDecrease X\n");
            fd_x=open("fifo_command_to_mot_x", O_WRONLY);
            write(fd_x, &command_decrease_x, sizeof(int));
            //sleep(1);
            //close(fd_x);
        }
    }
    else{
        if(c==120){
            //x stop
            printf("\nX stop\n");
            fd_x=open("fifo_command_to_mot_x", O_WRONLY);
            write(fd_x, &command_stop_x, sizeof(int));
            //sleep(1);
            //close(fd_x);
        }
        if(c==122){
            //z stop
            printf("\nZ stop\n");
            fd_z=open("fifo_command_to_mot_z", O_WRONLY);
            write(fd_z, &command_stop_z, sizeof(int));
            //sleep(1);
            //close(fd_z);    
        }
    } 
}

int main(){   
    int c, c1, c2;   
    static struct termios oldt, newt;
    
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

    // if(fd_z=open("fifo_command_to_mot_z", O_WRONLY)==-1){
    //     perror("Error opening fifo to motor z\n");
    // }
    // if(fd_x=open("fifo_command_to_mot_x", O_WRONLY)==-1){
    //     perror("Error opening fifo to motor x\n");
    // }
   
    //fd_x=open("fifo_command_to_mot_x", O_WRONLY);
    while(1){ 
    get_send_kb_input(c, c1, c2);
    }   
    
    close(fd_x);
    close(fd_z);

    /*restore the old settings*/
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
    return 0;
}
