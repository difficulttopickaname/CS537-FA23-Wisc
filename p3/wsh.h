#ifndef WSH_H_
#define WSH_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_PATH_LENGTH 256
#define MAX_ARG_NUM 30
#define MAX_ARG_SIZE 30
#define MAX_JOB_NUM 30

#define PROMPT "wsh> "
#define EXIT "exit"
#define CD "cd"
#define JOBS "jobs"
#define FG "fg"
#define BG "bg"
#define PIPE "|"
#define BIN "/bin"
#define USR_BIN "/usr/bin"

pid_t shell_pgid;
int shell_terminal;
int shell_is_interactive;
int default_out;
int default_in;

/* A process is a single process.  */
typedef struct process
{
  struct process *next;       /* next process in pipeline */
  char argv[MAX_ARG_NUM][MAX_ARG_SIZE]; /* for exec */
  int argc;
  pid_t pid;                  /* process ID */
  char completed;             /* true if process has completed */
  char stopped;               /* true if process has stopped */
  int status;                 /* reported status value */
} process;

/* A job is a pipeline of processes.  */
typedef struct job
{
  struct job *next;
  char command[MAX_ARG_NUM * MAX_ARG_SIZE]; /* command line, used for messages */
  process *first_process;     /* list of processes in this job */
  process *cur_process;
  pid_t pgid;                 /* process group ID */
  int id;
  char notified;              /* true if user told about stopped job */
  int stdin, stdout, stderr;  /* standard i/o channels */
  int bg;
  int init_bg;
} job;

job *first_job;

void handle_error();
void wait_for_job (job *j);
job * find_job (pid_t pgid);
int job_is_stopped (job *j);
int mark_process_status (pid_t pid, int status);
int job_is_completed (job *j);
void put_job_in_foreground (job *j, int cont);
void put_job_in_background (job *j, int cont);
void update_status (void);
void format_job_info (job *j, const char *status);
void do_job_notification (void);
void func_exit();
void func_cd(char* arg);
void func_jobs();
void func_fg(char* id_chr);
void func_bg(char* id_chr);
int check_built_in(job* new_job);
void launch_process(process *p, pid_t pgid, int infile, int outfile, int errfile, int foreground);
void general_func(job* new_job);
void parse_command(char *input);
process* find_foreground_process();
void sigint_handler(int signo);
void sigtstp_handler(int signo);

#endif
