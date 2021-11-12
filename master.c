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
#include <sys/wait.h>

// Declare PIDs of the children programs as global variables.
pid_t pid_command, pid_motor_x, pid_motor_z, pid_inspection, pid_wd;
int wstatus;
/*FUNCTIONS*/

int spawn(const char * program, char ** arg_list) {
    // Function to generate a child process, it returns the PID of the child.

    pid_t child_pid = fork();
    if (child_pid != 0) // Father process.
        return child_pid;

    else { // Child process.
        execvp (program, arg_list);

        perror("exec failed");  // If it's executed, an error occurred.
    return -4;
    }
}

void create_fifo (const char * name){

    if(mkfifo(name, 0666)==-1){
        if (errno != EEXIST){
          perror("Error creating named fifo\n");
          exit(-5);
        }
    }
}

/*MAIN()*/
int main() {

    // Creates all named pipes for communications.
    create_fifo("fifo_command_to_mot_x");
    create_fifo("fifo_command_to_mot_z");
    create_fifo("fifo_est_pos_x");
    create_fifo("fifo_est_pos_z");

    // Executes all child processes.
    char * arg_list_motor_x[] = { "./motor_x", NULL, NULL };
    pid_motor_x = spawn("./motor_x", arg_list_motor_x);

    char * arg_list_motor_z[] = { "./motor_z", NULL, NULL };
    pid_motor_z = spawn("./motor_z", arg_list_motor_z);

    // Turn motors' PIDs into strings.
    char pid_motor_x_char[20];
    char pid_motor_z_char[20];
    sprintf(pid_motor_x_char, "%d", pid_motor_x); 
    sprintf(pid_motor_z_char, "%d", pid_motor_z); 

    char * arg_list_wd[] = {"./wd", pid_motor_x_char, pid_motor_z_char, (char*)NULL };
    pid_wd = spawn("./wd", arg_list_wd); // Executes the watchdog.

    char pid_wd_char[20];
    sprintf(pid_wd_char, "%d", pid_wd);

    char * arg_list_command[] = { "/usr/bin/konsole",  "-e", "./command", pid_wd_char ,(char*)NULL };
    pid_command = spawn("/usr/bin/konsole", arg_list_command);

    char * arg_list_insp[] = { "/usr/bin/konsole",  "-e", "./inspection", pid_motor_x_char, pid_motor_z_char, pid_wd_char ,(char*)NULL };
    pid_inspection = spawn("/usr/bin/konsole", arg_list_insp);

    //wait for child processes 
    wait(&wstatus);
    if(WIFEXITED(wstatus)){
        int status=WEXITSTATUS(wstatus);
        if(status<0){ //if any of child processes returns a negative number kill all of them!
            printf("Negative number occured\n");
            kill(pid_command, SIGKILL);
            kill(pid_inspection, SIGKILL);
            kill(pid_wd, SIGKILL);
            kill(pid_motor_x, SIGKILL);
            kill(pid_motor_z, SIGKILL);
    }
    }

    // Deletes named pipes.
    unlink("fifo_command_to_mot_x");
    unlink("fifo_command_to_mot_z");
    unlink("fifo_est_pos_x");
    unlink("fifo_est_pos_z");

    return 0;

}