int main() {
	// TODO Complete the program
}










// YOU SHOULD NOT MODIFY ANYTHING BELOW THIS LINE. The below code automatically
// enables a thread limit on your user account for Small Shell and its
// descendants. This limit intentionally causes fork() calls to fail if your
// user account has too many simultaneous living threads. The intention is
// to prevent accidental fork bombs.

#ifndef DISABLE_NPROC_LIMIT

#include <errno.h>
#include <sys/resource.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void set_thread_limit(int bound) {
	int pipe_fds[2];
	int pipe_result = pipe(pipe_fds);
	if (pipe_result) {
		fprintf(stderr, "Error on pipe() in set_thread_limit()\n");
		exit(1);
	}
	
	pid_t fork_result = fork();
	if (fork_result == 0) {
		close(pipe_fds[0]);
		char* user = getenv("USER");
		char command[256];
		sprintf(command, "ps --no-headers auxwwwm | awk '$2 == \"-\" { "
			"print $1 }' | grep %s | wc -l", user);
		dup2(pipe_fds[1], STDOUT_FILENO);
		close(pipe_fds[1]);
		system(command);
		exit(0);
	} else if (fork_result > 0) {
		// Retrieve result of pgrep -u USER -c from the pipe, written by the
		// child process
		close(pipe_fds[1]);
		char captured[256] = {0};
		int read_result;
		size_t total_bytes_read = 0;
		while ((read_result = read(
					pipe_fds[0],
					captured + total_bytes_read,
					255 - total_bytes_read)
				) > 0) {
			total_bytes_read += read_result;
		}
		if (read_result != 0) {
			fprintf(stderr, "Error on read() in set_thread_limit()\n");
			exit(1);
		}
		close(pipe_fds[0]);
		int exit_info;
		waitpid(fork_result, &exit_info, 0);
		errno = 0;
		// subtract 5 to account for all the child processes
		int n_cur_procs = strtol(captured, NULL, 10) - 5;
		if (errno) {
			fprintf(stderr, "Error on strtol() in set_thread_limit()\n");
			exit(1);
		}

		// Set RLIMIT_NPROC
		int soft_proc_limit = n_cur_procs + bound;
		struct rlimit proc_limit = {0};
		int result = getrlimit(RLIMIT_NPROC, &proc_limit);
		if (result) {
			fprintf(stderr, "Error on getrlimit() in set_thread_limit()\n");
			exit(1);
		}
		if (soft_proc_limit < proc_limit.rlim_cur) {
			proc_limit.rlim_cur = soft_proc_limit;
		}
		result = setrlimit(RLIMIT_NPROC, &proc_limit);
		if (result) {
			fprintf(stderr, "Error on setrlimit() in set_thread_limit()\n");
			exit(1);
		}
	} else {
		fprintf(stderr, "Error on fork() in set_thread_limit()\n");
		exit(1);
	}
}

void __attribute__((constructor)) __set_thread_limit(void) {
	const int thread_limit = 30;
	printf(
		"Setting thread limit to %d to mitigate fork bombs. "
			"This will prevent your "
			"user account from having more than N living threads "
			"simultaneously, where N is %d plus the number "
			"of threads that you have at this moment (at Small "
			"Shell startup). This limit includes main threads "
			"from zombie processes, so don't forget to use "
			"waitpid() to clean up all your processes as "
			"explained in the assignment document.\n",
		thread_limit,
		thread_limit
	);
	set_thread_limit(thread_limit);
}

#endif
