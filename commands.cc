
/* ####################################################################################
*                                  COMMANDS.CC
#####################################################################################*/




/* ####################################################################################
*                                  INCLUDES
#####################################################################################*/

#include "signals.h"
#include "commands.h"


#include <string>

/* ####################################################################################
*                                  CONSTS
#####################################################################################*/

typedef enum MODE
{
	BG_EXEC_MODE,
	FG_EXEC_MODE,

}MODE;
/*************************/
typedef enum ERROR
{
	INVALID_JOB,
	KILL_FAILED,
	MV_FAILED,
	WAITPID_FAILED,
	INVALID_PARAM,
	INVALID_PATH,
	NONE,

} ERROR;
/*************************/
typedef enum RETURN_VAL
{
	SUCCESS=0,
	FAILURE=-1,

} RETURN_VAL;

/* ####################################################################################
*                                 HELPING FUNCTIONS
#####################################################################################*/
static bool BgCommand(string cmd);
static bool is_string_number(const std::string& s);
static bool error_handler(ERROR err ,char* cmdString);
static void extractArgs(const char* delimiters, char* lineSize, char *args[MAX_NUM_OF_ARG], int *num_args);
static void execute_command(char* args[MAX_NUM_OF_ARG], MODE exec_mode, bool is_complicated);
static ERROR Fg(char *args[MAX_NUM_OF_ARG], int num_arg);
static ERROR Bg(char *args[MAX_NUM_OF_ARG], int num_arg);
static ERROR Jobs(int num_arg);
static ERROR Kill(char *args[MAX_NUM_OF_ARG], int num_arg);
static ERROR ShowPid(int num_arg);
static ERROR Quit(char *args[MAX_NUM_OF_ARG], int num_arg);
static ERROR Pwd(char *args[MAX_NUM_OF_ARG], int num_arg);
static ERROR Cd(char *args[MAX_NUM_OF_ARG], int num_arg);
static ERROR History(char *args[MAX_NUM_OF_ARG], int num_arg);
static ERROR Mv(char *args[MAX_NUM_OF_ARG], int num_arg);


/* ####################################################################################
*                                 EXTERNAL VARS
#####################################################################################*/




extern smashManager sm;
extern pid_t pid_running_in_fg;




/* ####################################################################################
*                                    MACROS
#####################################################################################*/

#define PRINT_FG_INVALID_JOB(job_id) cout << "Job not found" << endl
#define PRINT_KILL_FAILED(job_id) cout << "kill " << job_id << " - cannot send signal" << endl
#define PRINT_ERROR(cmdString) cout << "smash error: > \"" << cmdString << "\"" << endl
#define PRINT_PATH_NOT_FOUND_ERROR(path) cout << "smash error: > \"" << path << "\" - path not found" << endl
#define PRINT_KILL_INVALID_JOB(job_id) cout << "kill " << job_id << " - job does not exist" << endl



/* ####################################################################################
*                    STATIC FUNCTIONS IMPLIMINTATION (HELPING FUNCTIONS)
#####################################################################################*/
/**
 * error_handler function
 * @param err
 * @param cmdString
 * @return true if no error was found, else false
 */
static bool error_handler(ERROR err ,char* cmdString)
{
	switch (err)
	{
		case INVALID_PARAM:
		{
			PRINT_ERROR(cmdString);
			return false;
		}
		case INVALID_PATH:
		case INVALID_JOB:
		case KILL_FAILED:
		case MV_FAILED:
		case WAITPID_FAILED:
		{
			return false;
		}

		case NONE:
			return true;
	}
	return false;

}


/**
 * BgCommand method
 * @param cmd
 * @return true if command with name cmd (string) is running in bg
 */
static bool BgCommand(string cmd)
{
	if (cmd.length() == 0)
		return false;
	if (cmd.at(cmd.length()-1) == '&')
	{
		return true;
	}
	return false;
}



/**
 * execute_command function
 * @param args
 * @param exec_mode
 * @param is_complicated
 * 				    This function creates a child process and executes the external command in it.
					In the father process, the command is pushed to the job vector.
					When the child process is done, the job is cleaned from the job vector by sig_waitpid.
 */
static void execute_command(char* args[MAX_NUM_OF_ARG], MODE exec_mode, bool is_complicated)
{
	pid_t pID;
	switch(pID = fork())
	{
		case -1:
		{
			perror("Error: fork");
			return;
		}
		case 0 :
		{
			// Child Process
			setpgrp();
			// Execute an external command
			if (execvp(args[0], args))
			{
				perror("external cmd");
			}
			exit(1);
		}

		default:
		{
			int name_index = 0;
			if (is_complicated)
				name_index = 3;
			sm.jobs.push_back(Job(pID, args[name_index], false));
			vector<Job>::iterator j = sm.jobs.end();
			j--;
			// wait if running in fg mode
			if (exec_mode == FG_EXEC_MODE)
			{
				pid_running_in_fg = pID;
				sig_waitpid(j, WUNTRACED);
			}
		}

	}
}

/**
 * Pwd function
 * @param args
 * @param num_arg
 * @return it prints the current working directory and returns errors if there are any
 *  *  NONE- if success
	INVALID_PARAM- if param is NULL or illegal according to the question
	INVALID_PATH- if current path is invalid
	INVALID_JOB- if job is invalid
	KILL_FAILED- kill command did not succeed
	MV_FAILED - mv command did not succeeed
	WAITPID_FAILED- waitpid command did not succeed
 */
static ERROR Pwd(char *args[MAX_NUM_OF_ARG], int num_arg)
{
	if (num_arg != 1)
	{
		return INVALID_PARAM;
	}
	cout << sm.cwd << endl;
	return NONE;
}

/**
 * Cd function
 * @param args
 * @param num_arg
 * @return changes the current working directory and return errors if there are any
 *  *  *  NONE- if success
	INVALID_PARAM- if param is NULL or illegal according to the question
	INVALID_PATH- if current path is invalid
	INVALID_JOB- if job is invalid
	KILL_FAILED- kill command did not succeed
	MV_FAILED - mv command did not succeeed
	WAITPID_FAILED- waitpid command did not succeed
 */
static ERROR Cd(char *args[MAX_NUM_OF_ARG], int num_arg)
{
	int comma = 0;
	if (num_arg != 2)
	{
		return INVALID_PARAM;
	}

	string path = args[1];
	string next_path;
	if (!strcmp(path.c_str(), "-"))
	{
		path = sm.lwd;
		comma = 1;
	}

	if(chdir(path.c_str()))
	{
		PRINT_PATH_NOT_FOUND_ERROR(path);
		return INVALID_PATH;
	}
	if(comma == 1){
		cout << path << endl;
	}
	char current_path[MAX_SIZE];
	getcwd(current_path, MAX_SIZE);
	sm.setLWD();
	sm.setCWD(current_path);

	return NONE;

}
/**
 * History command
 * @param args
 * @param num_arg
 * @return displays the commands history of smash, up to 50 commands back
 *  *  *  NONE- if success
	INVALID_PARAM- if param is NULL or illegal according to the question
	INVALID_PATH- if current path is invalid
	INVALID_JOB- if job is invalid
	KILL_FAILED- kill command did not succeed
	MV_FAILED - mv command did not succeeed
	WAITPID_FAILED- waitpid command did not succeed
 */
static ERROR History(char *args[MAX_NUM_OF_ARG], int num_arg)
{
	if (num_arg != 1)
	{
		return INVALID_PARAM;
	}
	if(sm.history.empty())
		return NONE;
	for(std::vector<string>::iterator it = sm.history.begin(); it != sm.history.end(); ++it)
	{
		cout << *it << endl;
	}

	return NONE;
}


/**
 * Jobs: displays the current job vector, which contains the running in the background and suspended processes
 * @param num_arg
 * @return
 *  *  *  NONE- if success
	INVALID_PARAM- if param is NULL or illegal according to the question
	INVALID_PATH- if current path is invalid
	INVALID_JOB- if job is invalid
	KILL_FAILED- kill command did not succeed
	MV_FAILED - mv command did not succeeed
	WAITPID_FAILED- waitpid command did not succeed
 */
static ERROR Jobs(int num_arg)
{
	if (num_arg != 1)
	{
		return INVALID_PARAM;
	}

	for (vector<Job>::iterator j = sm.jobs.begin(); j != sm.jobs.end(); ++j)
	{
		j->printJob();
	}
	return NONE;
}

/**
 * Kill function
 * @param args
 * @param num_arg
 * @return It handles the kill command (send signal to a different process).
 */
static ERROR Kill(char *args[MAX_NUM_OF_ARG], int num_arg)
{
	if (num_arg != 3)
	{
		return INVALID_PARAM;
	}
	int job_id = atoi(args[2]);
	vector<Job>::iterator job = sm.getJobById(job_id);
	if (job== sm.jobs.end())
	{
		PRINT_KILL_INVALID_JOB(job_id);
		return INVALID_JOB;
	}
	int signum = atoi(args[1]+1);
	if (!sig_kill(job->pid, signum))
	{
		PRINT_KILL_FAILED(job_id);
		return KILL_FAILED;
	}
	return NONE;
}

/**
 * ShowPid func: It handles the showpid command (show the pid of smash).
 * @param num_arg
 * @return
 * NONE- if success
	INVALID_PARAM- if param is NULL or illegal according to the question
	INVALID_PATH- if current path is invalid
	INVALID_JOB- if job is invalid
	KILL_FAILED- kill command did not succeed
	MV_FAILED - mv command did not succeeed
	WAITPID_FAILED- waitpid command did not succeed
 */
static ERROR ShowPid(int num_arg)
{
	if (num_arg != 1)
	{
		return INVALID_PARAM;
	}
	cout << "smash pid is " << sm.id << endl;
	return NONE;
}

/*  Fg
 *  It handles the fg command (move a job running in the background or suspended to the foreground).
	If no arguments are given, the fg command moves the newest job to the foreground
 *  @param args
 *  @param num_arg
 *  @return
 *  NONE- if success
	INVALID_PARAM- if param is NULL or illegal according to the question
	INVALID_PATH- if current path is invalid
	INVALID_JOB- if job is invalid
	KILL_FAILED- kill command did not succeed
	MV_FAILED - mv command did not succeeed
	WAITPID_FAILED- waitpid command did not succeed
 */
static ERROR Fg(char *args[MAX_NUM_OF_ARG], int num_arg)
{
	if ((num_arg != 1) && (num_arg != 2))
	{
		return INVALID_PARAM;
	}

	if (num_arg == 2 && (!is_string_number(args[1])))
		return INVALID_PARAM;

	if (sm.jobs.empty())
		return NONE;

	vector<Job>::iterator job_to_fg;
	if (num_arg == 1)
		job_to_fg = sm.findLatestJob();
	else
	{

		int id_to_fg = atoi(args[1]);
		job_to_fg = sm.getJobById(id_to_fg);
		if (job_to_fg == sm.jobs.end())
		{
			PRINT_FG_INVALID_JOB(id_to_fg);
			return INVALID_JOB;
		}
	}
	cout << job_to_fg->name << endl;

	pid_running_in_fg = job_to_fg->pid;
	if (job_to_fg->is_delayed)
	{
		job_to_fg->is_delayed = false;
		if (!sig_kill(job_to_fg->pid, SIGCONT))
		{
			return KILL_FAILED;
		}
	}
	sig_waitpid(job_to_fg, WUNTRACED);
	return NONE;

}

/**
 * Bg func: It handles the bg command (move a job running in the foreground to the background).
 * @param args
 * @param num_arg
 * @return
 * NONE- if success
	INVALID_PARAM- if param is NULL or illegal according to the question
	INVALID_PATH- if current path is invalid
	INVALID_JOB- if job is invalid
	KILL_FAILED- kill command did not succeed
	MV_FAILED - mv command did not succeeed
	WAITPID_FAILED- waitpid command did not succeed
 */
static ERROR Bg(char *args[MAX_NUM_OF_ARG], int num_arg)
{
	if ((num_arg != 1) && (num_arg != 2))
	{
		return INVALID_PARAM;
	}

	if (num_arg == 2 && (!is_string_number(args[1])))
		return INVALID_PARAM;

	vector<Job>::iterator job_to_bg;
	if (num_arg == 1)
	{
		job_to_bg = sm.get_latest_delayed_job();
		if (job_to_bg == sm.jobs.end())
			return NONE;
	}
	else
	{
		int id_to_bg = atoi(args[1]);
		job_to_bg = sm.getJobById(id_to_bg);
		if (job_to_bg == sm.jobs.end())
		{
			PRINT_FG_INVALID_JOB(id_to_bg);
			return INVALID_JOB;
		}
	}
	cout << job_to_bg->name << endl;
	if (!sig_kill(job_to_bg->pid, SIGCONT))
	{
		return KILL_FAILED;
	}

	return NONE;
}

/**
 * Mv renames a file from its old name to a new name, given as arguments
 * @param args
 * @param num_arg
 * @return
 * NONE- if success
	INVALID_PARAM- if param is NULL or illegal according to the question
	INVALID_PATH- if current path is invalid
	INVALID_JOB- if job is invalid
	KILL_FAILED- kill command did not succeed
	MV_FAILED - mv command did not succeeed
	WAITPID_FAILED- waitpid command did not succeed
 */
static ERROR Mv(char *args[MAX_NUM_OF_ARG], int num_arg){
	if (num_arg != 3){
		return INVALID_PARAM;
	}
	string oldFileName = args[1];
	string newFileName = args[2];
	if(rename(oldFileName.c_str(),newFileName.c_str()) == -1){
		perror("mv");
		return MV_FAILED;
	}
	cout << oldFileName << " has been renamed to " << newFileName << endl;
	return NONE;

}

/**
 * 		Quit func:
		It handles the quit and quit kill commands
		If quit is called, the command exits smash (calling the smashManager destructor to erase the job vector memory).
		If quit kill is called, the command sends SIGTERM to all processes running in the background, waits 5 seconds,
		and if they don't respond it sends SIGKILL.
 * @param args
 * @param num_arg
 * @return
 * NONE- if success
	INVALID_PARAM- if param is NULL or illegal according to the question
	INVALID_PATH- if current path is invalid
	INVALID_JOB- if job is invalid
	KILL_FAILED- kill command did not succeed
	MV_FAILED - mv command did not succeeed
	WAITPID_FAILED- waitpid command did not succeed
 */
static ERROR Quit(char *args[MAX_NUM_OF_ARG], int num_arg)
{
	if ((num_arg != 1) && (num_arg != 2))
	{
		return INVALID_PARAM;
	}


	if (num_arg == 1)
	{
		exit(0);
	}
	// illegal command
	if (strcmp(args[1], "kill"))
	{
		return INVALID_PARAM;
	}
	signal(SIGCHLD, SIG_DFL);

	vector<Job>::iterator j;
	for(j = sm.jobs.begin(); j != sm.jobs.end(); ++j)
	{
		cout << "Sending SIGTERM... ";
		if (kill(j->pid, SIGTERM))
		{
			perror("kill failed");
			return KILL_FAILED;
		}

		sleep(5);
		int stat;
		pid_t returned_pid = waitpid(j->pid, &stat, WNOHANG | WUNTRACED);
		if (returned_pid == -1)
		{
			perror("waitpid");
			return WAITPID_FAILED;
		}
		else if (returned_pid == 0)
		{
			cout << "(5 seconds passed) ";

			cout << "Sending SIGKILL... ";
			if (kill(j->pid, SIGKILL))
			{
				perror("kill failed");
				return KILL_FAILED;
			}

		}
		cout << "Done." << endl;
		j = sm.jobs.erase(j);
		j--;
	}
	exit(0);

}
/**
 * is_string_number method:
 * determines if a string is a number or not
 * @param s
 * @return
 */
static bool is_string_number(const std::string& s)
{
	string::const_iterator it = s.begin();
	while (it != s.end() && std::isdigit(*it))
	{
		++it;
	}
	return !s.empty() && it == s.end();
}
/* ####################################################################################
*                                 HEADER FUNCTIONS
#####################################################################################*/

/**
 * interprets and executes built-in commands
 * @param lineSize
 * @param cmdString
 * @return
 */
int ExeCmd(char* lineSize, char* cmdString)
{

	char* args[MAX_NUM_OF_ARG];
	string dels = " \t\n";
	const char* delimiters = dels.c_str();
	int num_arg;
	bool is_history = false;
	ERROR result = NONE;
	char* cmd_str = strtok(lineSize, delimiters);
		if (cmd_str == NULL)
			return FAILURE;
	args[0] = cmd_str;
	num_arg = 1;
	for (int i=1; i<MAX_NUM_OF_ARG; i++)
	{
		args[i] = strtok(NULL, delimiters);
		if (args[i] != NULL)
		{
			num_arg++;
		}

	}

	/*************************************************/
	/*						pwd						 */
	/*************************************************/
	if (!strcmp(cmd_str, "pwd"))
	{
		result = Pwd(args, num_arg);
	}

	/*************************************************/
	/*						cd						 */
	/*************************************************/
	else if (!strcmp(cmd_str, "cd") )
	{
		result = Cd(args, num_arg);
	}
	/*************************************************/
	/*						history					 */
	/*************************************************/
	else if (!strcmp(cmd_str, "history")) //history commands will not be listed
	{
		result = History(args, num_arg);
		is_history = true;
	}
	/*************************************************/
	/*						jobs					 */
	/*************************************************/
	else if (!strcmp(cmd_str, "jobs"))
	{
		result = Jobs(num_arg);
	}
	/*************************************************/
	/*						kill					 */
	/*************************************************/
	else if (!strcmp(cmd_str, "kill"))
	{
		result = Kill(args, num_arg);
	}

	/*************************************************/
	/*						showpid					 */
	/*************************************************/
	else if (!strcmp(cmd_str, "showpid"))
	{
		result = ShowPid(num_arg);
	}
	/*************************************************/
	/*						fg						 */
	/*************************************************/
	else if (!strcmp(cmd_str, "fg"))
	{
		result = Fg(args, num_arg);

	}
	/*************************************************/
	/*						bg						 */
	/*************************************************/
	else if (!strcmp(cmd_str, "bg"))
	{
		result = Bg(args, num_arg);
	}
	/*************************************************/
	/*						quit					 */
	/*************************************************/
	else if (!strcmp(cmd_str, "quit"))
	{
		result = Quit(args, num_arg);
	}
	/*************************************************/
	/*						mv						 */
	/*************************************************/
	else if (!strcmp(cmd_str, "mv"))
	{
		result = Mv(args, num_arg);
	}
	/*************************************************/
	else // external command
	{
		ExeExternal(args, cmdString);
		result = NONE;
	}
	if (!is_history) // do not add history command to history vector
		sm.addToHistory(cmdString);

	if (!error_handler(result, cmdString))
		return FAILURE;

	return SUCCESS;
}
/**
 * executes external command
 * @param args
 * @param cmdString
 */
void ExeExternal(char *args[MAX_NUM_OF_ARG], string cmdString)
{
	execute_command(args, FG_EXEC_MODE, false);
}

/**
 * if command is in background, insert the command to jobs
 * @param lineSize
 * @param cmdString
 * @return
 */
int BgCmd(char* lineSize, char* cmdString)
{
	string cmd = lineSize;

	if (BgCommand(cmd))
	{
		lineSize[cmd.length()-1] = '\0';
		string dels = " \t\n";
		const char* delimiters = dels.c_str();
		char *args[MAX_NUM_OF_ARG];
		int num_arg;
		extractArgs(delimiters, lineSize, args, &num_arg);
		execute_command(args, BG_EXEC_MODE, false);
		sm.addToHistory(cmdString);
		return SUCCESS;

	}
	return FAILURE;
}

/**
 * Extracts the arguments and number of arguments from the command string given delimiters to separate
 * @param delimiters
 * @param lineSize
 * @param args
 * @param num_args
 */
static void extractArgs(const char* delimiters, char* lineSize, char *args[MAX_NUM_OF_ARG], int *num_args)
{
	char *cmd = strtok(lineSize, delimiters);
	args[0] = cmd;
	int i;
	for (i = 1; i < MAX_NUM_OF_ARG; ++i)
	{
		args[i] = strtok(NULL, delimiters);

		if (args[i] == NULL)
			break;
	}
	*num_args = i;
}


