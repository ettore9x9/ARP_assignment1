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

/*FUNCTIONS*/
void printer(float x, float z){
	// Print on screen dinamically with a fixed format.

	printf("\r                                               ");
	if (x >= 0 && z >= 0){
		printf("\rEstimated position (X, Z) = ( %.3f, %.3f) ", x, z);

	} else if (x < 0 && z >= 0) {
		printf("\rEstimated position (X, Z) = (%.3f, %.3f) ", x, z);

	} else if (x >= 0 && z < 0) {
		printf("\rEstimated position (X, Z) = ( %.3f,%.3f) ", x, z);

	} else if (x < 0 && z < 0) {
		printf("\rEstimated position (X, Z) = (%.3f,%.3f) ", x, z);
	}

	fflush(stdout);
}


//crate a LogFile
FILE *file;
void file_printer(float x, float z){
	fprintf(file, "(X, Z)= ( %.3f, %.3f) \n", x, z);
}


/*MAIN()*/
int main(int argc, char * argv[]){
	
	int fd_from_motor_x, fd_from_motor_z; //file descriptors
	int ret; //select() system call return value
	/*process IDs*/
	pid_t pid_motor_x = atoi(argv[1]);
	pid_t pid_motor_z = atoi(argv[2]);
	pid_t pid_wd = atoi(argv[3]);
	float est_pos_x, est_pos_z; // estimate hoist X and Z positions
	char alarm; //char that will contain the 'stop' or 'reset' command
	fd_set rset; //set of ready file descriptors
	/*the select() system call does not wait for file descriptors to be ready */
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	/*opening pipes*/
	fd_from_motor_x=open("fifo_est_pos_x", O_RDONLY);
	fd_from_motor_z=open("fifo_est_pos_z", O_RDONLY);

	file=fopen("Log.txt", "w");
	if(!file){
		perror("Error file");
    	exit(-1);
	}
	printf("This is the inspection konsole, these are the current hoist X & Z positions:\n");
	printf("Press the 'r' button for resetting the hoist position\n");
	printf("Press the 's' button for stopping the hoist movement\n");
	//printing on the log file
	fprintf(file, "This is the LOGFILE. The following data represent the estimated hoist position\n");
	fflush(stdout);

	while(1){

		FD_ZERO(&rset);
	    FD_SET(fd_from_motor_x, &rset);
		FD_SET(fd_from_motor_z, &rset);
		FD_SET(0, &rset);

	    ret = select(FD_SETSIZE, &rset, NULL, NULL, &tv);

		if(ret == -1){ // An error occours.
				perror("select() on motor x\n");
			}

		if( FD_ISSET(0, &rset) != 0 ){ //if the standard input receives any inputs...
			alarm = getchar(); //get keyboard input
			kill(pid_wd, SIGTSTP); //Send a signal to let the watchdog know that an input occurred.
			if(alarm == 's'){ //STOP command
				kill(pid_motor_x, SIGUSR1); //SIGUSR1 signal has been used for STOP command
				kill(pid_motor_z, SIGUSR1);
				printf("\nStopping...\n");
				}
			if(alarm == 'r'){ //RESET command
				kill(pid_motor_x, SIGUSR2); //SIGUSR2 signal has been used for RESET command
				kill(pid_motor_z, SIGUSR2);
				printf("\nResetting...\n");
				}
			}
			if ( FD_ISSET(fd_from_motor_z, &rset) != 0 ){
				read(fd_from_motor_z, &est_pos_z, sizeof(float)); // Update the command.
			}
			if( FD_ISSET(fd_from_motor_x, &rset) != 0 ) { // There is something to read!			
				read(fd_from_motor_x, &est_pos_x, sizeof(float)); // Update the command.
			}
			printer(est_pos_x, est_pos_z);
			file_printer(est_pos_x, est_pos_z);
			usleep(15000); //sleep for 0,5 seconds
	} // End of while.

	//closing pipes
	close(fd_from_motor_x);
	close(fd_from_motor_z);
	//deleting file
	int return_file;
	if(return_file=remove("Log.txt")!=0){
		perror("Could not create a file\n");
	}

	return 0;
}