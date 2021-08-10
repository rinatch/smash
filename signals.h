#ifndef _SIGS_H
#define _SIGS_H

/* ####################################################################################
 *                                  INCLUDES
#####################################################################################*/

/* ####################################################################################
 *                                 SIGNALS.H
#####################################################################################*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include "commands.h"


using namespace std;

/* ####################################################################################
 *                                 HEADER FUNCTIONS
#####################################################################################*/


void catch_int(int sig_num);
void catch_tstp(int sig_num);
void sig_child_handler(int sig_num);
bool sig_kill(pid_t pid, int signum);
int  setSignalHandlers();
void sig_waitpid(vector<Job>::iterator j, int options);


#endif

