#include "wsh.h"

job *first_job = NULL;

/* Find the active job with the indicated pgid.  */
job * find_job (pid_t pgid){
  job *j;

  for (j = first_job; j; j = j->next){
    if (j->pgid == pgid){
      return j;
	}
  }
  return NULL;
}

/* Return true if all processes in the job have stopped or completed.  */
int
job_is_stopped (job *j)
{
  process *p;

  for (p = j->first_process; p; p = p->next){
    if (!p->completed && !p->stopped){
      return 0;
	}
  }
  return 1;
}

int mark_process_status (pid_t pid, int status){
  job *j;
  process *p;

  if (pid > 0){
      /* Update the record for the process.  */
      for (j = first_job; j; j = j->next){
        for (p = j->first_process; p; p = p->next){
          if (p->pid == pid){
              p->status = status;
              if (WIFSTOPPED (status)){
                p->stopped = 1;
			  }
              else{
                  p->completed = 1;
				  /*
                  if (WIFSIGNALED (status)){
                    fprintf (stderr, "%d: Terminated by signal %d.\n", (int) pid, WTERMSIG (p->status));
					
                }*/
              return 0;
             }
		  }
		}
	  }
      // fprintf (stderr, "No child process %d.\n", pid);
      return -1;
    }
  else if (pid == 0 || errno == ECHILD){
    /* No processes ready to report.  */
    return -1;
  }
  else {
    /* Other weird errors.  */
    perror ("waitpid");
    return -1;
  }
}

/* Return true if all processes in the job have completed.  */
int
job_is_completed (job *j)
{
  process *p;

  for (p = j->first_process; p; p = p->next){
    if (!p->completed){
      return 0;
	}
  }
  return 1;
}

void wait_for_job (job *j){
  int status;
  pid_t pid;

  do{
    pid = waitpid (WAIT_ANY, &status, WUNTRACED);

  }
  while (!mark_process_status (pid, status) && !job_is_stopped (j) && !job_is_completed (j));
}

void put_job_in_foreground (job *j, int cont){
  /* Put the job into the foreground.  */
  tcsetpgrp (STDIN_FILENO, j->pgid);

  /* Send the job a continue signal, if necessary.  */
  if (cont){
	  j->cur_process->stopped = 0;
      // tcsetattr (shell_terminal, TCSADRAIN, &j->tmodes);
      if (kill (j->cur_process->pid, SIGCONT) < 0){
		handle_error();
	  }
    }
  // wait_for_job(j);
  
  if(tcsetpgrp (STDIN_FILENO, shell_pgid) < 0){
	handle_error();
  }
}

void put_job_in_background (job *j, int cont){
  /* Send the job a continue signal, if necessary.  */
  if (cont && j->cur_process != NULL){
	j->cur_process->stopped = 0;
    if (kill (j->cur_process->pid, SIGCONT) < 0){
		handle_error();
	}
  }
  if(tcsetpgrp (STDIN_FILENO, shell_pgid) < 0){
	handle_error();
  }
}

void update_status (void){
  int status;
  pid_t pid;

  do{
    pid = waitpid (WAIT_ANY, &status, WUNTRACED|WNOHANG);
  }
  while (!mark_process_status (pid, status));
}

void format_job_info (job *j, const char *status){
  fprintf (stderr, "%ld (%s): %s\n", (long)j->pgid, status, j->command);
}

void do_job_notification (void){
  job *j, *jlast, *jnext;
  // process *p;

  /* Update status information for child processes.  */
  update_status ();

  jlast = NULL;
  for (j = first_job; j; j = jnext){
      jnext = j->next;

      /* If all processes have completed, tell the user the job has
         completed and delete it from the list of active jobs.  */
      if (job_is_completed (j)) {
        // format_job_info (j, "completed");
        if (jlast){
          jlast->next = jnext;
		}
        else{
          first_job = jnext;
		}
        // free_job (j);
      }

      /* Notify the user about stopped jobs,
         marking them so that we won't do this more than once.  */
      else if (job_is_stopped (j) && !j->notified) {
        // format_job_info (j, "stopped");
        j->notified = 1;
        jlast = j;
      }

      /* Don't say anything about jobs that are still running.  */
      else{
        jlast = j;
	  }
    }
}

void handle_error(){
    exit(-1);
}

void func_exit(){
    exit(0);
}

void func_cd(char* arg){
    if(chdir(arg) != 0){
        handle_error();
    }
}

void func_jobs(){
	if(first_job == NULL){
		return;
	}
	
	job *cur_job = first_job;
	while(cur_job != NULL){
    if(strcmp(cur_job->command, "jobs") == 0){
      cur_job = cur_job->next;
      continue;
    }
		if(cur_job->init_bg){
			printf("%d: %s [&]\n", cur_job->id, cur_job->command);
		}
    else{
      printf("%d: %s\n", cur_job->id, cur_job->command);
    }
		cur_job = cur_job->next;
	}
}
void func_fg(char* id_chr){
	int id;
	job *cur_job = first_job;

    if(id_chr != NULL){
        if((id = atoi(id_chr)) == 0){
            handle_error();
        }
		while(cur_job != NULL){
			if(cur_job->id == id){
				break;
			}
			cur_job = cur_job->next;
		}
		if(cur_job == NULL){
			handle_error(); // no job with id found;
		}
    }
	else{
        while(cur_job->next != NULL){
            cur_job = cur_job->next;
        }
    }
	
	cur_job->bg = 0;
	put_job_in_foreground (cur_job, 1);
	wait_for_job(cur_job);
	tcsetpgrp(STDIN_FILENO, shell_pgid);
}

void func_bg(char* id_chr){
	int id;
    job *cur_job = first_job;

    if(id_chr != NULL){
        if((id = atoi(id_chr)) == 0){
            handle_error();
        }
		while(cur_job != NULL){
	        if(cur_job->id == id){
	            break;
            }
            cur_job = cur_job->next;
        }
        if(cur_job == NULL){
            handle_error(); // no job with id found;
        }
    }
    else{
	    while(cur_job->next != NULL){
            cur_job = cur_job->next;
        }
    }	

	cur_job->bg = 1;
	put_job_in_background(cur_job, 1);
}

int check_built_in(job* new_job){
	int pipe = 0;
	if(new_job->first_process->next != NULL){pipe = 1;}
	int argc = new_job->first_process->argc;
    if(strcmp(EXIT, new_job->first_process->argv[0]) == 0){
        if(argc == 1 && !pipe){
            func_exit();
            return 1;
        }
        else{
            handle_error(); // error
        }
    }
    if(strcmp(CD, new_job->first_process->argv[0]) == 0){
        if(argc == 2 && !pipe){
            func_cd(new_job->first_process->argv[1]);
			new_job->first_process->completed = 1;
            return 1;
        }
        else{
            handle_error(); // error
        }
    }
    if(strcmp(JOBS, new_job->first_process->argv[0]) == 0){
        if(argc == 1 && !pipe){
            func_jobs();
			new_job->first_process->completed = 1;
            return 1;
        }
        else{
            handle_error(); // error
        }
    }
    if(strcmp(FG, new_job->first_process->argv[0]) == 0){
        if(argc == 1 && !pipe){
            func_fg(NULL);
			new_job->first_process->completed = 1;
            return 1;
        }
        else if(argc == 2 && !pipe){
            func_fg(new_job->first_process->argv[1]);
			new_job->first_process->completed = 1;
            return 1;
        }
        else{
            handle_error(); // error
        }
    }
    if(strcmp(BG, new_job->first_process->argv[0]) == 0){
        if(argc == 1 && !pipe){
            func_bg(NULL);
			new_job->first_process->completed = 1;
            return 1;
        }
        else if(argc == 2 && !pipe){
            func_bg(new_job->first_process->argv[1]);
			new_job->first_process->completed = 1;
            return 1;
        }
        else{
            handle_error(); // error
        }
    }
    return 0; // not a built-in command
}

void launch_process(process *p, pid_t pgid, int infile, int outfile, int errfile, int foreground){
  pid_t pid;

  if (shell_is_interactive){
      /* Put the process into the process group and give the process group
         the terminal, if appropriate.
         This has to be done both by the shell and in the individual
         child processes because of potential race conditions.  */
      pid = getpid ();
      if (pgid == 0) {
		//printf("pgid is 0, set to pid: <%d>\n", (int)pid);
		//printf("call info: process pid <%d> pgid <%d>\n", (int)p->pid, (int)pgid);
		pgid = pid;
		handle_error();
	  }
      // setpgid (pid, pgid);
	  // printf("pid %d is set with pgid %d\n", pid, pgid);
	  /*
      if (foreground){
		printf("shell is interactive, grab control\n");
        tcsetpgrp (shell_terminal, pgid);
	  }*/

  }
  // printf("infile: %d (0 means STDIN_FILENO), outfile: %d (1 means STDOUT_FILENO)\n", infile, outfile);
  /* Set the standard input/output channels of the new process.  */
  if (infile != STDIN_FILENO){
      dup2 (infile, STDIN_FILENO);
    }
  if (outfile != STDOUT_FILENO){
      dup2 (outfile, STDOUT_FILENO);
    }
  if (errfile != STDERR_FILENO){
      dup2 (errfile, STDERR_FILENO);
  }

  char full_path[MAX_PATH_LENGTH];
  char usr_path[MAX_PATH_LENGTH];
  snprintf(full_path, sizeof(full_path), "%s/%s", BIN, p->argv[0]);
  snprintf(usr_path, sizeof(usr_path), "%s/%s", USR_BIN, p->argv[0]);
  char **args;
  args = malloc(sizeof(char*) * (MAX_ARG_NUM + 1));
  for (int i = 0; i < MAX_ARG_NUM; i++) {
	if(p->argv[i] == NULL || strlen(p->argv[i]) < 1){
		break;
	}
    args[i] = malloc(sizeof(char* ) * MAX_ARG_SIZE);
    strcpy(args[i], p->argv[i]);
  }
  args[MAX_ARG_NUM] = NULL;  // NULL-terminate the array
  // Use execvp to execute the command
  if(access(full_path, F_OK) != -1){
	  // printf("executing\n");
      execvp(full_path, args);
  }
  else if(access(usr_path, F_OK) != -1){
	  // printf("executing\n");
      execvp(usr_path, args);
  }
  // Exec the new process.
  handle_error();
}

void general_func(job* new_job){
  process *p;
  pid_t pid;
  int mypipe[2], infile, outfile;
  infile = new_job->stdin;
  // go through every process
  for (p = new_job->first_process; p; p = p->next){
      // Set up pipes
      if (p->next){
        if(pipe(mypipe) < 0){handle_error();}
        outfile = mypipe[1];
      }
      else{
        outfile = new_job->stdout;
	  }

      /* Fork the child processes.  */
      pid = fork ();
      if (pid == 0){
		signal(SIGTSTP, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        // This is the child process
		int foreground = 0;
		if(!new_job->bg){foreground = 1;}
		p->pid = getpid();
		if(!new_job->pgid){
			new_job->pgid = p->pid;
			// printf("set pgid to %d in child process\n",(int)p->pid);
		}
		setpgid(p->pid, new_job->pgid);
		new_job->cur_process = p;
		launch_process (p, new_job->pgid, infile, outfile, new_job->stderr, foreground);
	  }
      else if (pid < 0){handle_error();} // The fork failed.
      else{
          /* This is the parent process.  */
          p->pid = pid;
		  if(shell_is_interactive){
			if(!new_job->pgid){
				new_job->pgid = pid;
				// printf("set pgid to %d in parent process\n",(int)pid);
			}
			setpgid(pid, new_job->pgid);
		  }
		  if(!new_job->bg){
			wait_for_job(new_job);
		  }
		  tcsetpgrp(STDIN_FILENO, shell_pgid);
      }

      /* Clean up after pipes.  */
      if (infile != new_job->stdin)
        close (infile);
      if (outfile != new_job->stdout)
        close (outfile);
      infile = mypipe[0];
    }
  
  if(!shell_is_interactive){
	wait_for_job(new_job);
  }
  if (!new_job->bg){
    put_job_in_foreground (new_job, 0);
  }
  else{
    put_job_in_background (new_job, 0);
  }
}

void parse_command(char *input){
	size_t length = strlen(input);
    if (length > 0 && input[length - 1] == '\n') {
        input[length - 1] = '\0';
    }
	char *token;
    char *rest = input;
	int end_sig_checker = 0; // check if & is at the end
	
	job *new_job = malloc(sizeof(job));
	strcpy(new_job->command, input);
	process *start_process = malloc(sizeof(process));
	new_job->stdout = default_out;
	new_job->stdin = default_in;
    process *cur_process = start_process;
	new_job->first_process = start_process;
	new_job->cur_process = start_process;
	// assign the job id 
	if (!new_job->id){
    int id_set = 0;
	    if(first_job == NULL){
        new_job->id = 1;
        first_job = new_job;
        id_set = 1;
	    }
        else{
			      int apgid = 1;
            job *cur_job = first_job;
            job *last_job;
            while(cur_job != NULL){
              if(cur_job->id == apgid){
				        last_job = cur_job;
                cur_job = cur_job->next;
                apgid ++;
              }
              // job id not match, a gap
              else{
                // first job's id is greater that 1
                if(last_job == NULL){
                  first_job = new_job;
                  new_job->next = cur_job;
                  new_job->id = 1;
                }
                else{
                  new_job->id = apgid;
                  new_job->next = cur_job;
                  last_job->next = new_job;
                }
                id_set = 1;
                break;
              }
            }
            // all id are in correct order
            if(!id_set){
              new_job->id = apgid;
              new_job->next = cur_job;
              last_job->next = new_job;
            }
        }
	}

	
	while ((token = strsep(&rest, " ")) != NULL) {
		if(end_sig_checker){handle_error();} // check if & is not at the end
		// piping
        if(strcmp(PIPE, token) == 0){
          cur_process->next = malloc(sizeof(process));
          cur_process = cur_process->next;
          continue;
        }
		// background 
		if((strcmp("&", token) == 0)){
			new_job->bg = 1;
      new_job->init_bg = 1;
			end_sig_checker = 1;
			new_job->command[strlen(new_job->command) - 2] = '\0';
			continue;
		}

		strcpy(cur_process->argv[cur_process->argc], token);  // store the argument to argv
		cur_process->argc++;
    }

	if(!check_built_in(new_job)){
		general_func(new_job);
	}
}

process* find_foreground_process(){
	job *j;

    for (j = first_job; j; j = j->next){
      if (!j->bg){
        process *p;
        for (p = j->first_process; p; p = p->next){
          if(!p->completed && !p->stopped){
            return p;
          }
        }
      }
    }
	return NULL;
}

void sigint_handler(int signo) {
	process *fg_process;

	if((fg_process = find_foreground_process()) != NULL){
		if(kill(fg_process->pid, SIGINT) < 0){
			handle_error();
		}
	}
	if(tcsetpgrp (STDIN_FILENO, shell_pgid) < 0){
		handle_error();
	}
}

void sigtstp_handler(int signo) {
	if(tcsetpgrp (STDIN_FILENO, shell_pgid) < 0){handle_error();}
	process *fg_process;

		if((fg_process = find_foreground_process()) != NULL){
			if (kill(fg_process->pid, SIGTSTP) == -1){
				handle_error();
			}
		  fg_process->stopped = 1;
		}
	
}

int main(int argc, char **argv){
	if(argc > 2){
        exit(-1);
    }
    // start wsh
	default_out = STDOUT_FILENO;
	default_in = STDIN_FILENO;
	shell_terminal = STDIN_FILENO;
	shell_pgid = getpid();
	setpgid(shell_pgid,shell_pgid);
	shell_is_interactive = isatty(shell_terminal);
		
	if(signal(SIGINT, sigint_handler) == SIG_ERR){
		handle_error();
	}
	
	if(signal(SIGTSTP, sigtstp_handler) == SIG_ERR){
		handle_error();
	}
		
	signal(SIGTTOU, SIG_IGN);
	if (argc == 1){
        while(1){
			    do_job_notification(); // clear completed jobs;
			    // print_all_jobs();
          char *command;
			    size_t command_len = 50;
			
            printf("%s", PROMPT);
            getline(&command, &command_len, stdin);
            if(feof(stdin)){
                printf("\n");
                exit(0);
            }
            parse_command(command);
        }
    }
	else{
		shell_is_interactive = 0;

		FILE *file = fopen(argv[1], "r");
	  if (file == NULL)
      {handle_error();}

    size_t command_len = 50;
	  char input[command_len];

		while (fgets(input, sizeof(input), file) != NULL)
		{
			parse_command(input);
		}

		fclose(file);
	}
}
