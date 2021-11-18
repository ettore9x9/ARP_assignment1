/* LIBRARIES */
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
#include <time.h>
/* Defining CHECK() tool. By using this methid the code results ligher and cleaner */
#define CHECK(X) ({int __val = (X); (__val == -1 ? ({fprintf(stderr,"ERROR (" __FILE__ ":%d) -- %s\n",__LINE__,strerror(errno)); exit(-1);-1;}) : __val); })

/* GLOBAL VARIABLES */
pid_t pid_command, pid_motor_x, pid_motor_z, pid_inspection, pid_wd; // PIDs of child programs.
int wstatus;
FILE * log_file; // Log file.

/* FUNCTIONS HEADERS */
int spawn( const char * program, char ** arg_list );
void create_fifo ( const char * name );
void logPrint ( char * string) ;

/* FUNCTIONS */
int spawn( const char * program, char ** arg_list ) {
    /* Function to generate a child process, it returns the PID of the child. */

    pid_t child_pid = fork();
    if (child_pid != 0) // Father process.
        return child_pid;

    else { // Child process.
        CHECK(execvp (program, arg_list));
    }
}

void create_fifo ( const char * name ) {
    /* Function to generate a named pipe. */
    mkfifo(name, 0666);
}

void logPrint ( char * string ) {
    /* Function to print on log file adding time stamps. */

    time_t ltime = time(NULL);
    fprintf( log_file, "%.19s: %s", ctime( &ltime ), string );
    fflush(log_file);
}

/* MAIN */
int main() {

    /* Creates a log file */
    FILE * log_file_create;
    log_file_create = fopen("Log.txt", "w"); // Create the file Log.txt, if the file already exists, overwrites it.

    log_file = fopen("Log.txt", "a");

    if(!log_file){ // Error management for fopen.
        perror("Error opening file");
        return -2; //return value put at -2 just to avoid confusing with the CHECK function control.
    }

    logPrint("Master    : Log file created by master process.\n");

    // Creates all named pipes for communications.
    create_fifo("/tmp/fifo_command_to_mot_x");
    create_fifo("/tmp/fifo_command_to_mot_z");
    create_fifo("/tmp/fifo_est_pos_x");
    create_fifo("/tmp/fifo_est_pos_z");
    create_fifo("/tmp/command_to_in_pid");

    /* Executes all child processes. */
    char * arg_list_motor_x[] = { "./motor_x", NULL, NULL };
    pid_motor_x = spawn("./motor_x", arg_list_motor_x);

    char * arg_list_motor_z[] = { "./motor_z", NULL, NULL };
    pid_motor_z = spawn("./motor_z", arg_list_motor_z);

    /* Convert motors' PIDs into strings. */
    char pid_motor_x_char[20];
    char pid_motor_z_char[20];
    sprintf(pid_motor_x_char, "%d", pid_motor_x); 
    sprintf(pid_motor_z_char, "%d", pid_motor_z); 

    char * arg_list_wd[] = {"./wd", pid_motor_x_char, pid_motor_z_char, (char*)NULL };
    pid_wd = spawn("./wd", arg_list_wd); // Executes the watchdog.


    /* Convert WatchDog' PID into string. */
    char pid_wd_char[20];
    sprintf(pid_wd_char, "%d", pid_wd);

    char * arg_list_command[] = { "/usr/bin/konsole",  "-e", "./command", pid_wd_char ,(char*)NULL };
    pid_command = spawn("/usr/bin/konsole", arg_list_command);

    char * arg_list_insp[] = { "/usr/bin/konsole",  "-e", "./inspection", pid_motor_x_char, pid_motor_z_char, pid_wd_char ,(char*)NULL };
    pid_inspection = spawn("/usr/bin/konsole", arg_list_insp);

    logPrint("Master    : Created all processes.\n");

    /* Waits for child processes. */ 
    CHECK(wait(&wstatus));

    /* Deletes named pipes. */
    CHECK(unlink("/tmp/fifo_command_to_mot_x"));
    CHECK(unlink("/tmp/fifo_command_to_mot_z"));
    CHECK(unlink("/tmp/fifo_est_pos_x"));
    CHECK(unlink("/tmp/fifo_est_pos_z"));
    CHECK(unlink("/tmp/command_to_in_pid"));

    /*  If any of the child processes returns 0, then kill al the processes exept for the command konsole.
        This is because the command konsole is the only one who can terminate with status 0. In such case 
        the CHECK function would generate an error because the master process would try to kill a process
        that has already terminated.                                                                    */ 
    if(wstatus==0){ //
        CHECK(kill(pid_inspection, SIGKILL));
        CHECK(kill(pid_wd, SIGKILL));
        CHECK(kill(pid_motor_x, SIGKILL));
        CHECK(kill(pid_motor_z, SIGKILL));
    }
    else{ //In this case an error occurred, so every processes must be killed
        CHECK(kill(pid_inspection, SIGKILL));
        CHECK(kill(pid_command, SIGKILL)); //kill the command konsole
        CHECK(kill(pid_wd, SIGKILL));
        CHECK(kill(pid_motor_x, SIGKILL));
        CHECK(kill(pid_motor_z, SIGKILL));
    }


    char str[100];
    sprintf(str, "Master    : Child process terminated with status : %d\n", wstatus); //here we can check if the termionation was due to a problem or not.
                                                                                      //A negative number means a problem. Value 0 means that user just normally quitted the program.  
    logPrint(str);

    logPrint("Master    : End.\n");

    fclose(log_file); // Closes log file.
    fclose(log_file_create);

    return 0;
}