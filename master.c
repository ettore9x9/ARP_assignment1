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

pid_t pid_command, pid_motor_x, pid_motor_z, pid_inspection, pid_wd;
char buffer [20];

int spawn(const char * program, char ** arg_list) {
  pid_t child_pid = fork();
  if (child_pid != 0)
    return child_pid;

  else {
    execvp (program, arg_list);
    perror("exec failed");
    return 1;
  }
}

void create_fifo (const char * name){
    if(mkfifo(name, 0666)==-1){
        if (errno != EEXIST){
        perror("Error creating named fifo\n");
        exit (1);
        }
    }
}

int main() {


create_fifo("fifo_command_to_mot_x");
create_fifo("fifo_command_to_mot_z");
create_fifo("fifo_est_pos_x");
create_fifo("fifo_est_pos_z");

char * arg_list_1[] = { "./motor_x", NULL, NULL };
pid_motor_x = spawn("./motor_x", arg_list_1);

char * arg_list_2[] = { "./motor_z", NULL, NULL };
pid_motor_z = spawn("./motor_z", arg_list_2);

char pid_motor_x_char[20];
char pid_motor_z_char[20];
sprintf(pid_motor_x_char, "%d", pid_motor_x); 
sprintf(pid_motor_z_char, "%d", pid_motor_z); 

char * arg_list_3[] = {"./wd", pid_motor_x_char, pid_motor_z_char, (char*)NULL };
pid_wd = spawn("./wd", arg_list_3);

char pid_wd_char[20];
sprintf(pid_wd_char, "%d", pid_wd);

char * arg_list_4[] = { "/usr/bin/konsole",  "-e", "./command", pid_wd_char ,(char*)NULL };
pid_command = spawn("/usr/bin/konsole", arg_list_4);

char * arg_list_5[] = { "/usr/bin/konsole",  "-e", "./inspection", pid_motor_x_char, pid_motor_z_char, pid_wd_char ,(char*)NULL };
pid_inspection = spawn("/usr/bin/konsole", arg_list_5);


wait(NULL);

unlink("fifo_command_to_mot_x");
unlink("fifo_command_to_mot_z");
unlink("fifo_est_pos_x");
unlink("fifo_est_pos_z");
return 0;

}