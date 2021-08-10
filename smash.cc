
/* ####################################################################################
 *                                  INCLUDES
#####################################################################################*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "commands.h"
#include "signals.h"

/* ####################################################################################
 *                                  CONSTANTS
#####################################################################################*/

#define MAX_SIZE 80
#define SUCCESS 0


/* ####################################################################################
 *                                  GLOBALS
#####################################################################################*/

smashManager sm;
char lineSize[MAX_SIZE];
pid_t pid_running_in_fg;
int Job_Num;

/* ####################################################################################
 *                                 MAIN FUNCTION
#####################################################################################*/

/**
 * main function
 * @param argc
 * @param argv
 * @return SUCCESS (0) in case of success. It gets command from user and calls appropriate methods
 */
int main(int argc, char *argv[])
{
    char cmdString[MAX_SIZE];

	//here we deal with signal declerations
	if (setSignalHandlers() == -1)
	{
		cout << "signal handlers" << endl;
		exit(1);
	}

	//globals
	pid_running_in_fg = -1;
	Job_Num = 1;

    while (1)
    {
	 	cout << "smash > ";
	 	cin.getline(lineSize, MAX_SIZE, '\n');
	 	strcpy(cmdString, lineSize);
	 	if(!BgCmd(lineSize, cmdString)) continue;
		ExeCmd(lineSize, cmdString);
	}

    return SUCCESS;
}

