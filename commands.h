#ifndef _COMMANDS_H
#define _COMMANDS_H

/* ####################################################################################
*                                  INCLUDES
#####################################################################################*/
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>


using namespace std;


/* ####################################################################################
*                                  DEFINES
#####################################################################################*/



#define MAX_SIZE 80
#define MAX_NUM_OF_ARG 20
#define HISTORY_SIZE 50




/* ####################################################################################
*                                 GLOBALS
#####################################################################################*/


extern int Job_Num;


/* ####################################################################################
*                   FUNCTIONS THAT DEAL WITH USER'S COMMANDS
#####################################################################################*/



int BgCmd(char* lineSize, char* cmdString);
int ExeCmd(char* lineSize, char* cmdString);
void ExeExternal(char *args[MAX_NUM_OF_ARG], string cmdString);



/* ####################################################################################
*                                  CLASSES
#####################################################################################*/
/**
 * Job params and methods
 */
class Job
{
	public:
        string name;
        pid_t pid;
		int id;
		struct timeval time;
		struct timeval suspension_time;
		bool is_delayed;


		//constructor
		Job(int my_pid, string command, bool suspended)
        {
            id = Job_Num;
            Job_Num ++;
            pid = my_pid;
            name = command;
            gettimeofday(&time, NULL);
            is_delayed = suspended;
        }

		// defaults dtor is enough

		//methods:
		void printJob()
        {
            struct timeval curr;
            gettimeofday(&curr, NULL);

            cout << "[" << this->id << "] " << this->name << " : " << this->pid << " " << (curr.tv_sec - this->time.tv_sec) << " secs";
            if (is_delayed)
            {
                cout << " Stopped " << endl;
            }
            cout << endl;
        }
};


/* ####################################################################################
*                                  smashManager CLASS
#####################################################################################*/

class smashManager {
public:
	int id;
    vector<string> history;
	vector<Job> jobs;
	string lwd;
	string cwd;

	//constructor
    smashManager()
    {
		id = getpid();
		jobs.clear();
		history.clear();
		char workDir[MAX_SIZE];
		getcwd(workDir,MAX_SIZE);
		cwd = workDir;
		lwd = cwd;
	}

	//no need for destructor, vector is freed automatically

	//******************* METHODS************************/

	void addToHistory(string cmd)
    {
		if(history.size() == HISTORY_SIZE)
			history.erase(history.begin());
		history.push_back(cmd);
	}
	/*****************************************************/
    void setCWD(string path)
    {
        cwd = path;
    }
    /*****************************************************/
    void setLWD()
    {
        lwd = cwd;
    }
    /*****************************************************/
	vector<Job>::iterator getJobBbPID(pid_t pid)
	{
		for (vector<Job>::iterator j = jobs.begin(); j != jobs.end(); ++j)
		{
			if (j->pid == pid)
			{
				return j;
			}
		}
		return jobs.end();
	}
    /*****************************************************/
    vector<Job>::iterator findLatestJob(void)
    {
        time_t min_time = jobs.begin()->time.tv_sec;
        vector<Job>::iterator job = jobs.begin();
        for (vector<Job>::iterator j = jobs.begin(); j != jobs.end(); ++j)
        {
            if (j->time.tv_sec < min_time)
            {
                min_time = j->time.tv_sec;
                job = j;
            }
        }
        return job;
    }
    /*****************************************************/
    vector<Job>::iterator getJobById(int id)
    {
        for (vector<Job>::iterator j = jobs.begin(); j != jobs.end(); ++j)
        {
            if (j->id == id)
            {
                return j;
            }
        }
        return jobs.end();
    }
    /*****************************************************/
	vector<Job>::iterator get_latest_delayed_job()
	{
		vector<Job>::iterator job = jobs.begin();
        time_t max_delay;

		for (vector<Job>::iterator j = jobs.begin(); j != jobs.end(); ++j)
		{
			if(!j->is_delayed)
				continue;

			if ((j->suspension_time.tv_sec) > max_delay)
			{
				max_delay = j->time.tv_sec;
				job = j;
			}
		}
		return job;
	}

};

#endif

