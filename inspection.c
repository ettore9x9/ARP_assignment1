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
#include <ncurses.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

/* COLORS */
#define RESET "\033[0m"
#define BHRED "\e[1;91m"
#define BHGRN "\e[1;92m"
#define BHYEL "\e[1;93m"
#define BHMAG "\e[1;95m"

/* GLOBAL VARIABLES */
int last_row = 20;
int last_col = 20;
FILE *log_file;
time_t start_time;

/* FUNCTIONS HEADERS */
void setup_console();
void printer( float x, float z );
void logPrint ( char * string );

/* FUNCTIONS */
void setup_console() {
	/* Function to initialize the console.
	To refresh the same console, the ncurses library is used. */

	initscr(); // Init the console screen.

	/* Print the base structure of the user interface. */
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

	refresh(); // Send changes to the console.
}

void printer( float x, float z ) {
	/* Function to print dinamically the position of the hoist in the console. */

	int row = floor((z / 0.625) + 0.2) + 5; // Row of the hoist.
	int col = floor((x / 0.2) + 0.2) + 2;   // Column of the hoist.

	curs_set(0); // Hide the cursor.

	for (int i = 5; i <= last_row; i++) { // Overwrite with blank spaces the old characters.
		move(i, last_col);
		addch(' ');
	}

	for (int i = 5; i < row; i++) { // Write the new chain on the screen.
		move(i, col);
		addch('|');
	}

	move(row, col); // Write the hoist.
	addch('V');

	last_row = row; // Update position.
	last_col = col;

	/* Print on screen dynamically with a fixed format. */
	move(26, 0);
	if (x >= 0 && z >= 0)
	{
		printw("Estimated position (X, Z) = ( %.3f, %.3f)\n", x, z);
	}
	else if (x < 0 && z >= 0)
	{
		printw("Estimated position (X, Z) = (%.3f, %.3f)\n", x, z);
	}
	else if (x >= 0 && z < 0)
	{
		printw("Estimated position (X, Z) = ( %.3f,%.3f)\n", x, z);
	}
	else if (x < 0 && z < 0)
	{
		printw("Estimated position (X, Z) = (%.3f,%.3f)\n", x, z);
	}

	time_t ltime = time(NULL);
	printw("Execution time = %ld\n", ltime - start_time); // Print the execution time in seconds.

	refresh(); // Send changes to the console.
}

void logPrint ( char * string ) {
    /* Function to print on log file adding time stamps. */

    time_t ltime = time(NULL);
    fprintf( log_file, "%.19s: %s", ctime( &ltime ), string );
    fflush(log_file);
}

/* MAIN */
int main(int argc, char *argv[])
{

	int fd_from_motor_x, fd_from_motor_z, fd_command_pid, fd_stop; //file descriptors
	int ret;													   //select() system call return value

	/*process IDs*/
	pid_t pid_motor_x = atoi(argv[1]);
	pid_t pid_motor_z = atoi(argv[2]);
	pid_t pid_wd = atoi(argv[3]);
	pid_t command_pid;
	//receive command pid
	fd_command_pid = open("command_to_in_pid", O_RDONLY);
	read(fd_command_pid, &command_pid, sizeof(int));

	float est_pos_x, est_pos_z; // estimate hoist X and Z positions
	char alarm;					//char that will contain the 'stop' or 'reset' command
	fd_set rset;				//set of ready file descriptors
	/*the select() system call does not wait for file descriptors to be ready */
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	/*opening pipes*/
	fd_from_motor_x = open("fifo_est_pos_x", O_RDONLY);
	fd_from_motor_z = open("fifo_est_pos_z", O_RDONLY);
	log_file = fopen("Log.txt", "a"); // Open the log file

	start_time = time(NULL);
	time_t ltime = time(NULL);
	fprintf(log_file, "%.19s: inspection: Inspection console started\n", ctime(&ltime));
	fflush(log_file);

	setup_console();

	while (1)
	{

		FD_ZERO(&rset);
		FD_SET(fd_from_motor_x, &rset);
		FD_SET(fd_from_motor_z, &rset);
		FD_SET(0, &rset);

		ret = select(FD_SETSIZE, &rset, NULL, NULL, &tv);

		if (ret == -1)
		{ // An error occours.
			perror("select() on motor x\n");
			return -2;
		}

		if (FD_ISSET(0, &rset) != 0)
		{					   //if the standard input receives any inputs...
			alarm = getchar(); //get keyboard input

			kill(pid_wd, SIGTSTP); //Send a signal to let the watchdog know that an input occurred.

			ltime = time(NULL);
			fprintf(log_file, "%.19s: inspection: Input received = %c\n", ctime(&ltime), alarm);
			fflush(log_file);

			if (alarm == 's')
			{								//STOP command
				kill(command_pid, SIGUSR1); // enable input from keyboard
				kill(pid_motor_x, SIGUSR1); //SIGUSR1 signal has been used for STOP command
				kill(pid_motor_z, SIGUSR1);				alarm = '0';
			}

			if (alarm == 'r')
			{								//RESET command
				kill(pid_motor_x, SIGUSR2); //SIGUSR2 signal has been used for RESET command
				kill(pid_motor_z, SIGUSR2);
				kill(command_pid, SIGUSR2); //alarm the command console that resetting started!
			}
		}

		if (FD_ISSET(fd_from_motor_z, &rset) != 0)
		{
			read(fd_from_motor_z, &est_pos_z, sizeof(float));
		}
		if (FD_ISSET(fd_from_motor_x, &rset) != 0)
		{
			read(fd_from_motor_x, &est_pos_x, sizeof(float));
		}
		if ( (est_pos_x < 0.001) && (est_pos_z < 0.001) && (alarm == 'r') )
		{
			kill(command_pid, SIGUSR1); //alarm the command console that resetting has finished!
			alarm = 's';
		}

		printer(est_pos_x, est_pos_z);

		ltime = time(NULL);
		fprintf(log_file, "%.19s: inspection: est_pos_x = %f, est_pos_z = %f\n", ctime(&ltime), est_pos_x, est_pos_z);
		fflush(log_file);

		usleep(15000); //sleep for 0,5 seconds

	} // End of while.

	//closing pipes
	close(fd_from_motor_x);
	close(fd_from_motor_z);
	close(fd_command_pid);
	close(fd_stop);
	ltime = time(NULL);
	fprintf(log_file, "%.19s: inspection: Inspection console ended.\n", ctime(&ltime));
	fflush(log_file);
	fclose(log_file); // Close log file.

	return 0;
}