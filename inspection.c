#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

int main(int argc, char * argv[]){
int fd_from_motor_x, fd_from_motor_z;
int ret;
int pid_motor_x=atoi(argv[1]);
int pid_motor_z=atoi(argv[2]);
int pid_wd=atoi(argv[3]);
float est_pos_x, est_pos_z;
char alarm;
fd_set rset;
struct timeval tv;
tv.tv_sec = 0;
tv.tv_usec = 0;
printf("This is the inspection konsole, these are the current hoist X & Z positions:\n");
printf("Press the 'r' button for resetting the hoist position\n");
printf("Press the 's' button for stopping the hoist movement\n");
fflush(stdout);
fd_from_motor_x=open("fifo_est_pos_x", O_RDONLY);
fd_from_motor_z=open("fifo_est_pos_z", O_RDONLY);

while(1){
	FD_ZERO(&rset);
    FD_SET(fd_from_motor_x, &rset);
	FD_SET(fd_from_motor_z, &rset);
	FD_SET(0, &rset);
    ret = select(FD_SETSIZE, &rset, NULL, NULL, &tv);

	if(ret == -1){ // An error occours.
			perror("select() on motor x\n");

		}

		if(FD_ISSET(0, &rset)!=0){
			alarm=getchar();
			kill(pid_wd, SIGTSTP);
			if(alarm=='s'){
				kill(pid_motor_x, SIGUSR1);
				kill(pid_motor_z, SIGUSR1);
				printf("\nStopping...\n");
				}
			if(alarm=='r'){
				kill(pid_motor_x, SIGUSR2);
				kill(pid_motor_z, SIGUSR2);
				printf("\nResetting...\n");
				}
		}
		if (FD_ISSET(fd_from_motor_z, &rset) != 0){
			
			read(fd_from_motor_z, &est_pos_z, sizeof(float)); // Update the command.

			
		}
		if( FD_ISSET(fd_from_motor_x, &rset) != 0) { // There is something to read!
		
			read(fd_from_motor_x, &est_pos_x, sizeof(float)); // Update the command.

		}


		// printf("The current estimated X position is: %f ", est_pos_x);
		// printf("The current estimated Z position is: %f\n", est_pos_z);
		printf("\r                                               ");
		printf("\rEstimated position (X, Z) = (%.3f,%.3f) ", est_pos_x, est_pos_z);
		fflush(stdout);
		usleep(500000);
	}
close(fd_from_motor_x);
close(fd_from_motor_z);
return 0;
}