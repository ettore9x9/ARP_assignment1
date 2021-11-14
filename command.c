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
#include <termios.h> 
#include <time.h> 
#include <stdbool.h>

/*COLORS*/
#define RESET "\033[0m"
#define BHRED "\e[1;91m"
#define BHGRN "\e[1;92m"
#define BHYEL "\e[1;93m"
#define BHMAG "\e[1;95m"

/*GLOBAL VARIABLES*/     
const int cmd_increase_z = 1; //this represents the "increase Z" command
const int cmd_decrease_z = 2; //this represents the "decrease Z" command
const int cmd_increase_x = 3; //this represents the "increase X" command
const int cmd_decrease_x = 4; //this represents the "decrease X" command
const int cmd_stop_z = 5; //this represents the "STOP Z" command
const int cmd_stop_x = 6; //this represents the "STOP X" command
int fd_x, fd_z, fd_pid; //file descriptors
pid_t command_pid, pid_wd; //process ID
bool resetting=false;
FILE * log_file;

/*FUNCTIONS*/

void interpreter();

void signal_handler(int sig) {

    if(sig == SIGUSR1){
        resetting=false;
    }   
    if(sig == SIGUSR2){
        resetting=true;
        printf(BHRED "RESETTING!"RESET"\n");
        fflush(stdout);
    }
}

void interpreter(){

    int c, c1, c2;
    c = getchar(); //get input from keyboard
    kill(pid_wd, SIGTSTP); // Send a signal to let the watchdog know that an input occurred.

    time_t ltime = time(NULL);
    fprintf(log_file, "%.19s: command   : Input received.\n", ctime( &ltime ) );
    fflush(log_file);

    if(resetting){

        if(c != 0){
            printf(BHRED "THE SYSTEM IS STILL RESETTING!" RESET "\n");
            fflush(stdout);
        }
    }
    else{
        if(c == 27){ //the ASCII numbers for arrows is a combination of three numbers, the first two (27 and 91) are always the same
            c1 = getchar();

            if (c1 == 91){ 
                c2 = getchar();

                if(c2 == 65){ //third ASCII nummber for upwards arrow
                    printf("\n" BHYEL "Decrease Z" RESET "\n");
                    write(fd_z, &cmd_decrease_z, sizeof(int));    
                }
                if(c2 == 66){ //third ASCII nummber for downwards arrow
                    printf("\n" BHYEL "Increase Z" RESET "\n");
                    write(fd_z, &cmd_increase_z, sizeof(int)); 
                }
                if(c2 == 67){ //third ASCII nummber for right arrow
                    printf("\n" BHMAG "Increase X" RESET "\n");
                    write(fd_x, &cmd_increase_x, sizeof(int));
                }
                if(c2 == 68){ //third ASCII nummber for left arrow
                    printf("\n" BHMAG "Decrease X" RESET "\n");
                    write(fd_x, &cmd_decrease_x, sizeof(int));
                }
            }
        }
        else{
            if(c == 120){ //ASCII number for 'x' keyboard key
                printf("\n" BHRED "X stop" RESET "\n");
                write(fd_x, &cmd_stop_x, sizeof(int));
            }
            if(c == 122){ //ASCII number for 'z' keyboard key
                printf("\n" BHRED "Z stop" RESET "\n");
                write(fd_z, &cmd_stop_z, sizeof(int)); 
            }
        }
        if(c!=120 && c!=122 && c!= 27){
            printf(BHMAG"Please, use the commands above!"RESET"\n");
        }
    }
}

/*MAIN()*/
int main(int argc, char * argv[]){   

    static struct termios oldt, newt;
    pid_wd= atoi(argv[1]);
    command_pid=getpid();


    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &signal_handler;
    sa.sa_flags = SA_RESTART;

    //sigaction for SIGUSR1 & SIGUSR2
    if(sigaction(SIGUSR1,&sa,NULL)==-1){
        perror("Sigaction error, SIGUSR1 command\n");
        return -14;
    }
    if(sigaction(SIGUSR2,&sa,NULL)==-1){
        perror("Sigaction error, SIGUSR2 command");
        return -15;
    }

    
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

    log_file = fopen("Log.txt", "a"); // Open the log file

    time_t ltime = time(NULL);
    fprintf(log_file, "%.19s: command   : Command console started\n", ctime( &ltime ) );
    fflush(log_file);

    printf(BHGRN "This is the COMMAND console: you can use the following commands: " RESET "\n");
    printf(BHMAG "Press the upwards Arrow for moving the hoist upwards" RESET "\n");
    printf(BHMAG "Press the downwards Arrow for moving the hoist downwards" RESET "\n");
    printf(BHYEL "Press 'X' for stopping the horizontal movement" RESET "\n");
    printf(BHYEL "Press 'Z' for stopping the vertical movement" RESET "\n");  
    fflush(stdout); 

    //opening pipes
    
    fd_z=open("fifo_command_to_mot_z", O_WRONLY);
    fd_x=open("fifo_command_to_mot_x", O_WRONLY);
    fd_pid=open("command_to_in_pid", O_WRONLY);
    write(fd_pid, &command_pid, sizeof(int));


    while(1){ 
        interpreter();
    }   
    
    //closing pipes
    close(fd_x);
    close(fd_z);
    close(fd_pid);

    /*restore the old settings*/
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);

    ltime = time(NULL);
    fprintf(log_file, "%.19s: command   : Command console ended\n", ctime( &ltime ) );
    fflush(log_file);

    fclose(log_file); // Close log file.

    return 0;
}
