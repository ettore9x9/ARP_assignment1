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
#include <termios.h>
#include <time.h>
#include <stdbool.h>

/* COLORS */
#define RESET "\033[0m"
#define BHRED "\e[1;91m"
#define BHGRN "\e[1;92m"
#define BHYEL "\e[1;93m"
#define BHMAG "\e[1;95m"

/* GLOBAL VARIABLES */
const int cmd_increase_z = 1; // This represents the "increase Z" command.
const int cmd_decrease_z = 2; // This represents the "decrease Z" command.
const int cmd_increase_x = 3; // This represents the "increase X" command.
const int cmd_decrease_x = 4; // This represents the "decrease X" command.
const int cmd_stop_z = 5;     // This represents the "STOP Z" command.
const int cmd_stop_x = 6;     // This represents the "STOP X" command.
int fd_x, fd_z, fd_pid;       // File descriptors.
pid_t command_pid, pid_wd;    // Process IDs.
bool resetting = false;       // Boolean variable for reset the motors.
FILE *log_file;               // Log file.

/* FUNCTIONS HEADERS */
void signal_handler( int sig );
void interpreter();
void setup_terminal ();
void logPrint ( char * string );
void helpPrint ();

/* FUNCTIONS */
void signal_handler( int sig ) {
    /* Function to handle stop and reset signals. */

    if (sig == SIGUSR1) { // SIGUSR1 is the signal to stop the motors.
        resetting = false;
        printf(BHRED "... MOTORS STOPPED ..." RESET "\n");
        fflush(stdout);
    }

    if (sig == SIGUSR2) { // SIGUSR2 is the signal to reset motors.
        resetting = true;
        printf(BHRED "...   RESETTING    ..." RESET "\n");
        fflush(stdout);
    }
}

void interpreter(){
    /* Function to receive input from the keyboard and convert it into commands.
    It also writes on the correct pipe to control motors.*/

    int c, c1, c2;   // Input characters are treated as integers, with ASCII conversion.
    c = getchar();   // Wait for the keyboard input.

    if (c != 120 && c != 122 && c != 27 && c != 104) { // The input is not a command.
        printf(BHMAG " --> Invalid command, press 'h' for help." RESET "\n");

    } else {
        kill(pid_wd, SIGTSTP); // Send a signal to let the watchdog know that an input occurred.

        time_t ltime = time(NULL);
        fprintf(log_file, "%.19s: command   : Input received.\n", ctime(&ltime));
        fflush(log_file);

        if (!resetting) { // When the motors are resetting, this part is not executed.

            /* Arrow command inputs. Arrows are a combination of three different characters */
            if (c == 27) { // The first ASCII numbers for arrows is 27.
                c1 = getchar(); // If c is 27, it reads another character.

                if (c1 == 91) { // The second ASCII numbers for arrows is 27.
                    c2 = getchar(); // Reads another character.

                    if (c2 == 65) { //third ASCII nummber for upwards arrow.
                        printf("\n" BHYEL "Decrease Z" RESET "\n");
                        write(fd_z, &cmd_decrease_z, sizeof(int));
                    }
                    if (c2 == 66) { //third ASCII nummber for downwards arrow.
                        printf("\n" BHYEL "Increase Z" RESET "\n");
                        write(fd_z, &cmd_increase_z, sizeof(int));
                    }
                    if (c2 == 67) { //third ASCII nummber for right arrow.
                        printf("\n" BHMAG "Increase X" RESET "\n");
                        write(fd_x, &cmd_increase_x, sizeof(int));
                    }
                    if (c2 == 68) { //third ASCII nummber for left arrow.
                        printf("\n" BHMAG "Decrease X" RESET "\n");
                        write(fd_x, &cmd_decrease_x, sizeof(int));
                    }
                }
            }
            else
            {
                if (c == 120) { // ASCII number for 'x' keyboard key.
                    printf("\n" BHRED "X stop" RESET "\n");
                    write(fd_x, &cmd_stop_x, sizeof(int));
                }
                if (c == 122) { // ASCII number for 'z' keyboard key.
                    printf("\n" BHRED "Z stop" RESET "\n");
                    write(fd_z, &cmd_stop_z, sizeof(int));
                }
                if ( c == 104) { // ASCII number for 'h' keyboard key.
                    helpPrint();
                }
            }

        } else { // The motors are resetting, the command console can not receive any inputs.

            if ( c == 27 ) {
                getchar();
                getchar();
            }

            printf(BHRED "  Please, wait the end of the resetting." RESET "\n");
            fflush(stdout);
        }
    }
}

void setup_terminal (){
    /* Function to setup the terminal in order to receive immediately input key commands,
    without waiting the ESC key. */

    static struct termios newt;
    /* tcgetattr gets the parameters of the current terminal
    STDIN_FILENO will tell tcgetattr that it should write the settings of stdin to newt. */
    tcgetattr(STDIN_FILENO, &newt);

    /* ICANON normally takes care that one line at a time will be processed
    that means it will return if it sees a "\n" or an EOF or an EOL. */
    newt.c_lflag &= ~(ICANON);

    /* Those new settings will be set to STDIN
    TCSANOW tells tcsetattr to change attributes immediately. */
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void logPrint ( char * string ) {
    /* Function to print on log file adding time stamps. */

    time_t ltime = time(NULL);
    fprintf( log_file, "%.19s: %s", ctime( &ltime ), string );
    fflush(log_file);
}

void helpPrint () {
    /* Function to print the help command message. */

    printf(BHGRN "Please, use the following commands: " RESET "\n");
    printf(BHMAG "Press the arrows to move the hoist." RESET "\n");
    printf(BHYEL "Press 'x' for stopping the horizontal movement." RESET "\n");
    printf(BHYEL "Press 'z' for stopping the vertical movement." RESET "\n");
    printf(BHYEL "Press 'h' to display again this message." RESET "\n");
    fflush(stdout);
}

/*MAIN()*/
int main(int argc, char *argv[]) {

    setup_terminal();

    /* Collects PIDs. */
    pid_wd = atoi(argv[1]);
    command_pid = getpid();

    /* Signals that the process can receive. */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &signal_handler;
    sa.sa_flags = SA_RESTART;

    /* sigaction for SIGUSR1 & SIGUSR2 */
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("Sigaction error, SIGUSR1 command\n");
        return -14;
    }

    if (sigaction(SIGUSR2, &sa, NULL) == -1) {
        perror("Sigaction error, SIGUSR2 command");
        return -15;
    }

    log_file = fopen("Log.txt", "a"); // Opens the log file.

    logPrint("command   : Command console started\n");

    printf(" ### COMMAND CONSOLE ### \n \n");
    fflush(stdout);
    helpPrint();

    /* Open pipes */
    fd_z = open("fifo_command_to_mot_z", O_WRONLY);
    fd_x = open("fifo_command_to_mot_x", O_WRONLY);
    fd_pid = open("command_to_in_pid", O_WRONLY);
    write(fd_pid, &command_pid, sizeof(int));

    while (1)
    {
        interpreter();
    }

    /* Close pipes */
    close(fd_x);
    close(fd_z);
    close(fd_pid);

    logPrint("command   : Command console ended\n");

    fclose(log_file); // Close log file.

    return 0;
}
