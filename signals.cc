

 /* ####################################################################################
 *                                  SIGNALS.CC
#####################################################################################*/
#include <sys/wait.h>
#include "commands.h"
#include "signals.h"
#include "signal.h"


 /* ####################################################################################
 *                                    EXTERNS
#####################################################################################*/


extern smashManager sm;
extern pid_t pid_running_in_fg;



 /* ####################################################################################
 *                                  HELPING FUNCTIONS
#####################################################################################*/



 static string signal_num_to_string(int signum);
 static bool check_if_removable(vector<Job>::iterator j, int options);
 static bool stat_handler(int stat_val, pid_t pid);



 /* ####################################################################################
 *                           IMPLEMINTING HELPING FUNCTIONS
#####################################################################################*/

/**
 * stat_handler function
 * @param stat_val
 * @param pid
 * @return true if if the child process terminated normally with exit or _exit.
 *  If the child process for which status was returned by the wait
 *  or waitpid function exited because it raised a signal that caused it to exit,
 *  the WIFSIGNALED macro evaluates to TRUE
 *  and the WTERMSIG macro can be used to determine which signal was raised by the child process.
 *  Otherwise, the WIFSIGNALED macro evaluates to FALSE.
 *  else it returns false. (if stopper for example)
 */
 static bool stat_handler(int stat_val, pid_t pid)
 {
	 vector<Job>::iterator j;
	 if (WIFEXITED(stat_val) || WIFSIGNALED(stat_val))
		 return true;
	 else if (WIFSTOPPED(stat_val))
	 {
		 vector<Job>::iterator j;
		 j = sm.getJobBbPID(pid);
		 j->is_delayed = true;
		 if (time(&j->suspension_time.tv_sec) == -1)
		 {
			 perror("failed");
		 }
	 }
	 else if (WIFCONTINUED(stat_val))
	 {
		 vector<Job>::iterator j;
		 j = sm.getJobBbPID(pid);
		 j->is_delayed = false;
	 }
	 return false;

 }
/****************************************************************************************/
/**
 * check_if_removable function
 * @param j
 * @param options
 * @return job that has pid finished-> true: we can remove from job vector
 * else: false
 */
 static bool check_if_removable(vector<Job>::iterator j, int options)
 {
	 int stat_val;
	 bool result = false;
	 pid_t returned_pid = waitpid(j->pid, &stat_val, options);
	 if (returned_pid == -1)
	 {
		 perror("waitpid");
	 }
	 else if (returned_pid == j->pid)
	 {
		 result = stat_handler(stat_val, j->pid);
	 }
	 return result;
 }
/********************************************************************************************/
 /**
  * signal_num_to_string function
  * @param signum
  * @return return "" for unknown signals else return string for signum
  */
 static string signal_num_to_string(int signum)
 {
	 switch (signum)
	 {
		 case SIGILL:
			 return "SIGILL";
		 case SIGHUP:
			 return "SIGHUP";
		 case SIGSYS:
			 return "SIGSYS";
		 case SIGINT:
			 return "SIGINT";
		 case SIGQUIT:
			 return "SIGQUIT";
		 case SIGTRAP:
			 return "SIGTRAP";
		 case SIGABRT:
			 return "SIGABRT";
		 case SIGFPE:
			 return "SIGFPE";
		 case SIGKILL:
			 return "SIGKILL";
		 case SIGUSR1:
			 return "SIGUSR1";
		 case SIGSEGV:
			 return "SIGSEGV";
		 case SIGUSR2:
			 return "SIGUSR2";
		 case SIGPIPE:
			 return "SIGPIPE";
		 case SIGALRM:
			 return "SIGALRM";
		 case SIGTERM:
			 return "SIGTERM";
		 case SIGCONT:
			 return "SIGCONT";
		 case SIGSTOP:
			 return "SIGSTOP";
		 case SIGTSTP:
			 return "SIGTSTP";
		 case SIGTTIN:
			 return "SIGTTIN";
		 case SIGTTOU:
			 return "SIGTTOU";
		 case SIGURG:
			 return "SIGURG";
		 case SIGXCPU:
			 return "SIGXCPU";
		 case SIGXFSZ:
			 return "SIGXFSZ";
		 case SIGVTALRM:
			 return "SIGVTALRM";
		 case SIGPROF:
			 return "SIGPROF";
		 case SIGWINCH:
			 return "SIGWINCH";
		 case SIGIO:
			 return "SIGIO";
		 case SIGPWR:
			 return "SIGPWR";
		 case SIGSTKFLT:
			 return "SIGSTKFLT";


		 default:
			 return "";
	 }
	 //we will never get here
	 return "";
 }


 /* ####################################################################################
*                                DONE WITH HELPING FUNCTIONS
#####################################################################################*/





 /* ####################################################################################
*                               HEADER FUNCTIONS IMPLIMINTATION
#####################################################################################*/



 /**
  * setSignalHandlers function
  * @return Set the signal handlers by methods for SIGCHLD, SIGINT and SIGTSTP
  */
int setSignalHandlers()
{
	//SIGCHLD
	if (signal(SIGCHLD, sig_child_handler) == SIG_ERR)
	{
		perror("signal");
		return -1;
	}

	//SIGINT - CTRL+C
	if (signal(SIGINT,&catch_int) == SIG_ERR)
	{
		perror("signal");
		return -1;
	}

	//SIGTSTP - CTRL+Z
	if (signal(SIGTSTP,&catch_tstp) == SIG_ERR)
	{
		perror("signal");
		return -1;
	}
	return 0;
}
/*################################################################################################*/
/**
 * catch_tstp function
 * handle CTRL+Z
 * @param sig_num
 */
void catch_tstp(int sig_num)
 {
	if(pid_running_in_fg == -1)
	{
		return;
	}
	vector<Job>::iterator fg_job = sm.getJobBbPID(pid_running_in_fg);
	if(!sig_kill(pid_running_in_fg,SIGTSTP)){
		return;
	}
	fg_job->is_delayed = true;
	return;
}
/*################################################################################################*/
/**
 * handle CTRL+C
 * @param sig_num
 */
void catch_int(int sig_num)
 {
	if(pid_running_in_fg == -1)
	{
		return;
	}
	//send SIGINT
	vector<Job>::iterator fg_job = sm.getJobBbPID(pid_running_in_fg);
	sig_kill(pid_running_in_fg,SIGINT);
}
/*################################################################################################*/

/**
 * sig_kill: wrapper func for kill function that sends sig to jpb with pid
 * @param pid
 * @param signum
 * @return true if success and false if error happens, we do not deal with special cases like 0 and -1
 */
bool sig_kill(pid_t pid, int signum)
{

	if ((pid == -1) || (pid == 0))
	{
		return false;
	}
	cout << "signal " << (signal_num_to_string(signum)) << " was sent to pid " << pid << endl;

	if (kill(pid, signum) == -1)
	{
		perror("kill");
		return false;
	}
	return true;
}
/*################################################################################################*/

/**
 * wrapper function for waitpid
 * @param j
 * @param options
 */
 void sig_waitpid(vector<Job>::iterator j, int options)
 {
	 if (check_if_removable(j, options))
	 {
		 j = sm.jobs.erase(j);
	 }
	 pid_running_in_fg = -1;
 }
/*################################################################################################*/
/**
 * 	A handler to the SIGCHLD signal
 * @param sig_num
 */
void sig_child_handler(int sig_num)
{
	for (vector<Job>::iterator j = sm.jobs.begin(); j != sm.jobs.end() && !sm.jobs.empty(); ++j)
	{
		if (j->pid != pid_running_in_fg)
		{
			if (check_if_removable(j,WCONTINUED|WUNTRACED|WNOHANG))
			{
				j = sm.jobs.erase(j);
				j--;
			}
		}
	}
}





