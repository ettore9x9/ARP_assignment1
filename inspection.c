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
#include <ncurses.h>
#include <math.h>
#include <time.h>

/*COLORS*/
#define RESET "\033[0m"
#define BHRED "\e[1;91m"
#define BHGRN "\e[1;92m"
#define BHYEL "\e[1;93m"
#define BHMAG "\e[1;95m"

int last_row = 20;
int last_col = 20;

/*FUNCTIONS*/
void setup_konsole(){

	initscr();

	addstr("This is the INSPECTION console.\n");
	addstr("Press the 'r' for resetting the hoist position\n");
	addstr("Press the 's' for stopping the hoist movement\n");
	addstr("\n");
	addstr("||===================================================||---> x\n");
    addstr("||                                                   ||\n");
    addstr("||                                                   ||\n");
    addstr("||                                                   ||\n");
    addstr("||                                                   ||\n");
    addstr("||                                                   ||\n");
    addstr("||                                                   ||\n");
    addstr("||                                                   ||\n");
    addstr("||                                                   ||\n");
    addstr("||                                                   ||\n");
    addstr("||                                                   ||\n");
    addstr("||                                                   ||\n");
    addstr("||                                                   ||\n");
    addstr("||                                                   ||\n");
    addstr("||                                                   ||\n");
    addstr("||                                                   ||\n");
    addstr("||                                                   ||\n");
    addstr("||                                                   ||\n");
    addstr("|\n");
    addstr("v\n");
    addstr("z\n");
    addstr("\n");

	refresh();
}

void printer(float x, float z){

	int row = floor( (z / 0.625) + 0.2 ) + 5;
    int col = floor( (x / 0.2) + 0.2 ) + 2;

    curs_set(0);

	for ( int i = 5; i <= last_row; i++){

	    move(i, last_col);
	    addch(' ');
	}

	for ( int i = 5; i < row; i++){
	    move(i, col);
	    addch('|');
	}

	move(row, col);
	addch('V');

    last_row = row;
    last_col = col;

   	// Print on screen dynamically with a fixed format.
    move(26, 0);
	if (x >= 0 && z >= 0){
		printw("Estimated position (X, Z) = ( %.3f, %.3f) ", x, z);

	} else if (x < 0 && z >= 0) {
		printw("Estimated position (X, Z) = (%.3f, %.3f) ", x, z);

	} else if (x >= 0 && z < 0) {
		printw("Estimated position (X, Z) = ( %.3f,%.3f) ", x, z);

	} else if (x < 0 && z < 0) {
		printw("Estimated position (X, Z) = (%.3f,%.3f) ", x, z);
	}

    curs_set(0);

    refresh();
}


//crate a LogFile
FILE *file;
void file_printer(float x, float z){
	//print on a text file
	time_t ltime = time(NULL);
	fprintf(file, "%.19s: %.3f, %.3f  \n", ctime( &ltime ), x, z);
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
    	return -1;
	}

	setup_konsole();

	//printing on the log file
	fprintf(file, "This is the LOGFILE. The following data represent the estimated hoist position\n");

	while(1){

		FD_ZERO(&rset);
	    FD_SET(fd_from_motor_x, &rset);
		FD_SET(fd_from_motor_z, &rset);
		FD_SET(0, &rset);

	    ret = select(FD_SETSIZE, &rset, NULL, NULL, &tv);

		if(ret == -1){ // An error occours.
				perror("select() on motor x\n");
				return -2;
			}

		if( FD_ISSET(0, &rset) != 0 ){ //if the standard input receives any inputs...
			alarm = getchar(); //get keyboard input
			kill(pid_wd, SIGTSTP); //Send a signal to let the watchdog know that an input occurred.
			if(alarm == 's'){ //STOP command
				kill(pid_motor_x, SIGUSR1); //SIGUSR1 signal has been used for STOP command
				kill(pid_motor_z, SIGUSR1);
				// printf("\n"BHRED"Stopping..."RESET"\n");
				}
			if(alarm == 'r'){ //RESET command
				kill(pid_motor_x, SIGUSR2); //SIGUSR2 signal has been used for RESET command
				kill(pid_motor_z, SIGUSR2);
				// printf("\n"BHRED"Resetting..."RESET"\n");
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
		return -3;
	}
	return 0;
}