# Assignment: Smallsh

In this assignment, you'll write your own simple shell known as Small Shell.
You'll write it from scratch.

## Warning: Run only on the OS1 server

Your program will need to fork several child processes as it runs. If you
aren't careful, it's possible to introduce a bug that causes the shell to
fork too many child processes too quickly (e.g., due to a tight infinite loop
that calls `fork()`). This is known as a **fork bomb**.

A fork bomb can starve the entire machine of resources as
the scheduler is forced to context-switch between an extremely large number
of processes /
kernel threads. In some cases, it's possible for a fork bomb to temporarily
slow down an entire server to a crawl. If this happens on an OSU server,
all students can be effectively locked out of the server.

It's **absolutely critical** that you only run your small shell implementation
and the automated unit tests on the OS1 server (or your own local machine).
**Do not** run your small shell implementation or the automated unit tests
on any other OSU servers (e.g., the flip servers). If you accidentally
take down the OS1 server via a fork bomb, you'll get a minor scolding. But if
you accidentally take down any of the other OSU servers, your grade will be
severely penalized.

That said, `main.c` (in this repository) comes with a bit
of *experimental* code that
enables a resource limit for Small Shell and its descendants. This causes
`fork()` calls to fail if the number of living threads associated with your
user account is greater than or equal to N, where N is 30 plus the number
of living threads under your account at the moment Small Shell is started.
The intention of this is to prevent fork bombs. Please don't mess with this
code.

> [!NOTE]
  This limit includes main threads from zombie processes, so make sure to
  clean up your child processes via `waitpid()` as directed in this assignment
  document.

> [!TIP]
  When `fork()` fails due to hitting the thread limit (e.g., because you
  aren't properly cleaning up your child processes), it will return -1
  in the parent process, and no child process will be created. For debugging
  purposes, it'd be a good idea to explicitly check for this using an if
  statement following your `fork()` calls. Otherwise, if `fork()` fails
  silently, and you don't notice, it will be hard to diagnose.

## Assignment

Your shell will need to support several features as outlined in this
section.

### Standard error stream

This program will need to print various messages. In some cases, they must
be printed to the standard error stream (not standard output). Whenever this
is the case, it will be explicitly stated by this document.

You can print a message to the standard error stream like so:

`fprintf(stderr, "my cool error message goes here\n");`

### The command prompt

When the program starts, it should prompt the user for a
command by simply printing out a colon followed by a space (`: `) with
**no newline character sequence** (`\n`) to standard output.
This is the symbol that your
program should use every time it prompts the user for another command.

(Many shells like Bash often use a dollar sign (`$`) for the terminal prompt by
default. Small Shell uses a colon instead.)

The standard output file stream is line-buffered, meaning that if you use
`printf()` to print the terminal prompt, it may not immediately
display in the terminal since it doesn't end in a newline character sequence.
You must fix this by manually flushing the standard output buffer via
`fflush(stdout)`. For example:

```C
printf(": ");
fflush(stdout);
```

The user will then enter a command string in the terminal and press enter. Your
program must read the entire command string (hint: use `getline()` with `stdin`
as the file stream pointer).

You may assume that all command strings entered by the user will adhere to the
following format:

`command [arg1 arg2 ...] [< input_file] [> output_file] [&]`

- In the above syntax, anything written in square brackets is optional,
  meaning the user might supply
  it in their command string, or they might not. `command` is the name or
  path of the command or
  executable that they want to execute (it could be one of
  the three builtins (exit, status, or cd), or
  it could be the name or path of a regular executable file on the
  system). `arg1`, `arg2`, and so on are
  the command-line arguments that they want to supply to the command being
  executed. If the user
  so chooses, they can supply `< input_file`, where `input_file` is
  a path to a file to which they would
  like the command's standard input stream to be redirected. Similarly,
  if the user so chooses, they can
  supply `> output_file`, where `output_file` is a path to a file to
  which they would like the command's
  standard output stream to be redirected. Finally, an optional `&` character
  at the end of the command string indicates
  that the user wants the
  command to be run in the background rather than the
  shell's foreground (more on this requirement
  later).
- The user may choose to redirect standard input, standard
  output, neither, or both. If the user chooses
  to redirect both, the input redirection syntax may
  appear before or after the output redirection syntax
  (e.g., they may supply `> output_file` before `< input_file` in
  their command string, or vice-versa).
- However, all other ordering in the command string is strict:
  the command string must start with the
  command, followed by its command-line arguments (if any), followed
  by the optional input and / or
  output redirection syntax, followed by the optional `&`
  character to start the process in the background.
  When the user enters a command string, you
  may assume that it will obey this syntax.
- You may assume that no `&` symbols will appear anywhere in the command
  string, other than at the
  end of the command string if the user is trying to run a
  command in the background. For example,
  the name of the command itself will never have a `&` character in it.
- You may assume that no `<` or `>` characters will appear anywhere
  in the command string, other than
  where allowed to specify input and output redirection files.
  For example, the name of the command
  itself will never have a `<` character in it.
- Your shell does not need to support any quoting, so you may
  assume that each argument supplied by
  the user will be exactly one word (i.e., it will not contain any whitespace).
- Although you need to support input and output redirection to and
  from files, you do **not** need to
  support redirecting inputs and outputs from one process to another
  (i.e., you do not need to support
  bash's pipe operator, '|'). However, see the extra credit section.
  
- Your shell should support command strings with a maximum of 2047
  characters of text (including the
  line feed from when the user presses the enter key, but not
  including any null terminators, so you'll
  need an array of at least 2048 characters to store the command string).
- Your shell is required to support command strings with anywhere between
  0 and 20 arguments. For example:

  `some_command arg1 arg2 ... arg20`

  If it supports more than 20 arguments, that's fine too, but it's not required.

### Comments and blank lines

- Any command string supplied by the user that begins with the `#`
  character is a **comment**.
- Similarly, any command string that contains no characters (besides the
  newline character sequence from when the user presses enter) is a
  **blank line**.

Whenever the user enters a comment or a blank line, Small Shell should
"ignore" it. That is, it should just immediately print out another colon and
space (`: `), prompting the user for their next command.

### Expansion of shell variable `$$`

In every command string entered by the user, your shell must search for any
and all occurrences of the character sequence `$$` left-to-right. Upon
finding such a character sequence, it must expand it to (i.e.,
replace it with) the PID of Small Shell (the parent process) itself.
This is the only shell variable expansion that your program needs
to support.

For example, suppose the PID of Small Shell is 179. Then the string
`foo$$$$` should be expanded to `foo179179`, and the string
`foo$$$` should be expanded to `foo179$`.

> [!TIP]
  Use `getpid()` to get the PID of the calling process. Use string
  functions like `sprintf()` and / or `strcat()` to do the actual
  expansion of `$$` instances to the PID. Note that the rigorously
  correct C99+ way to inject a `pid_t` value into a format string is to
  cast the `pid_t` value to type `intmax_t` (provided by `<stdint.h>`),
  then use a `%jd` format specifier.

> [!TIP]
  For computing the size (number of characters) necessary to store a
  string that will be constructed via `sprintf()` or `snprintf()`, you can
  first call `snprintf()` with `NULL` as the first argument and `0`
  as the second argument. This will write nothing, but it returns the
  size of the string that it would have produced if given a sufficiently
  large size and legitimate destination buffer. Refer to the
  [man pages](https://man7.org/linux/man-pages/man3/snprintf.3.html)
  for more information.

### Built-in commands

Your shell must support three built-in commands: exit, cd, and
status. These three built-in commands are the only ones that
your shell will handle itself. For all other commands, your shell
will fork a child process that will call an exec()-family function
to run the command.

Here are some high-level notes about these built-in commands:

- You do not have to support input/output redirection for these
  built-in commands
- These commands should **not** update Small Shell's running exit status
  tracker (see the `status` built-in command below).
- If the user tries to run one of these built-in commands in the
  background with the `&` option, ignore that option and run the command
  in the foreground anyway, regardless of whether Small Shell is currently
  operating in foreground-only mode (don't display an error; just run the
  command in the foreground).
- When the user runs these commands, you may assume that they will run
  them with the proper syntax (e.g., they won't supply more than one
  argument to the `cd` command, and they won't supply any arguments to
  `status` or `exit`).
- You may assume that the user will use these built-in commands
  correctly. For example, you may assume that the user will not
  run `exit` with a non-integer argument, or `cd` with an invalid
  path argument. That is, no error handling is required for the arguments
  of these commands.

Here are the details on the three individual built-ins:

1. The `exit` built-in command can accept a single argument, which must be a
   whole number. It should cause Small Shell to exit (terminate) entirely,
   with the supplied whole
   number specifying the Small Shell process's exit status. (i.e., Small Shell
   should return this whole number from its `main()` function). For example,
   if the user runs `exit 5`, then Small Shell should terminate with an
   exit status of `5`.
   
   However, just before exiting, Small Shell (the parent process) must
   **kill all of its remaining background child processes**. It should do
   this by sending `SIGKILL` to each of its children, one at a time.
   Use the `kill()` function to do this.
   
   The `exit` built-in command may also be given no arguments, which is
   equivalent to `exit 0`.

2. The `cd` built-in command should change the current working directory of
   Small Shell (the parent process). If given no arguments, it should
   change the working directory to the path stored in the `HOME` environment
   variable (use
   [`getenv()`](https://man7.org/linux/man-pages/man3/getenv.3.html)
   to get its value). If given an argument, it
   should change the working directory to the path given by that argument.
   You may assume that the user will never supply a path to an invalid
   directory (e.g., one that doesn't exist, or one for which the user doesn't
   have traversible permissions).

   Recall: You can use
   [`chdir()`](https://man7.org/linux/man-pages/man2/chdir.2.html)
   to change a process's working directory.

3. The `status` built-in command takes no arguments. When run by the user,
   the program should print out the termination information of the most
   recently terminated <ins>foreground</ins> command. In particular, if the
   child process that executed the most recent foreground command terminated
   by exiting, then the `status` built-in command should print `exit status S`,
   followed by a newline character sequence, where `S` is the
   integer exit status of the child process (as retrieved via `WEXITSTATUS()`.
   In contrast, if the child process that executed the most
   recent foreground command was killed by a signal,
   then the `status` built-in command should print `terminated by signal S`,
   followed by a newline character sequence, where `S` is the signal number
   of the signal that killed the child process (as retrieved via `WTERMSIG()`).
   See [the example output](#example-output) for what this should look like.

   If the `status` command is executed before any foreground non-builtin
   commands have been executed, it should simply print `exit status 0`.

   > **TIP:**
     In the parent process, use one or more variables to keep track of the
     termination information of the most recently terminated foreground command.
     Whenever Small Shell (the parent process) finishes cleaning up a foreground
     command process via `waitpid()`, parse its termination information
     using `WIFEXITED()`, `WIFSIGNALED()`, `WEXITSTATUS()`, and `WTERMSIG()`,
     then update those variables accordingly.

   > **NOTE:**
     Remember: The built-in commands themselves (`exit`, `cd`, and `status`)
     should not affect the outputs of subsequent `status`
     commands. For example, suppose a child process runs the foreground
     command `ls fjdsafjdsaj`, which fails and exits with status
     `2`. Suppose the user then
     immediately invokes the `cd` built-in command successfully to navigate
     back to their home directory. If the user then runs the `status` command,
     it should print `2` (the exit status of the `ls` command, <ins>not</ins>
     the termination information about the more recent `cd` built-in).

> [!NOTE]
  The built-ins operate on Small Shell (the parent process) itself. Indeed,
  there is no need to fork any child processes when executing built-in commands.

### Executing other commands

When the user attempts to run any other (non-built-in) command, Small Shell
should fork a child process dedicated to executing the specified command.
This child process will need to use `execvp()` (or some other exec-family
function), passing in the name of the executable and an array of string
command-line arguments as parsed from the user's command string.

Your program must be capable of executing programs installed in the system
path (e.g., as specified by the `PATH` environment variable). Recall that
`execvp()` is capable of doing this without any extra configuration (some other
exec-family functions do not support this capability).

If the exec-family function call in the child process fails (e.g., because the
executable specified by the user does not exist), the child process must
immediately print `command not found` to its <ins>standard error stream</ins>,
and then call `exit(1)` so as to exit with status 1. Note that this will
cause Valgrind to report reachable resources in use at exit for the child
process (e.g., memory leaks). That's okay;
[see the technical requirements](#technical-requirements).

More information about what the parent and child processes should do,
depending on whether the command is a foreground vs background command,
is provided in [a later section](#foreground-vs-background-commands).

### Input and output redirection

If a command string specifying a non-built-in command contains `>` and / or `<`
operators, the child process dedicated to executing the command must
use `dup2()` or `dup3()` to redirect its standard
output and / or standard input,
respectively, to the file(s) at the specified path(s). For example, the command
string `echo Hello > out.txt` should write `Hello` to `out.txt` instead of
printing it to the terminal.

If standard output is redirected to a file (via `> some_file`), then the
file should be opened for writing only. If it doesn't already exist, it
should be created with permissions `0644` (minus the process's umask). If it
does already exist, its contents should be truncated. If the child process
fails to open the file for writing, it should print the error message
`failed to open output file` to its <ins>standard error stream</ins> and
immediately exit with exit status `1`.

> [!TIP]
  Use `open()` with `O_WRONLY | O_CREAT | O_TRUNC` for the second argument
  and `0644` for the third argument.

If standard input is redirected to a file (via `< some_file`), then the file
should be opened for reading only (use `open()` with `O_RDONLY`). If the
child process fails to open the file for reading, it should print the error
message `failed to open input file` to its <ins>standard error stream</ins>
and immediately exit with exit status `1`.

Recall that a command may have both a standard output redirection operator
<ins>and</ins> a standard input redirection operator (or neither, or just
one of the two). If a command contains both redirection operators, they
may appear in either order (e.g., `my_command > out_file < in_file`, or
`my_command < in_file > out_file`).

If both the input file *and* the output file fail to open (e.g., due to
permissions errors), <ins>at least one</ins> of the two corresponding error
messages should be printed (it doesn't matter which).

Finally, if a <ins>background command</ins> 
(explained in [the next section](#foreground-vs-background-commands)) does
not have an input redirection operator (`<`) in its command string,
its standard input should be redirected to the file `/dev/null` (opened for
reading just like any other file). For example, the following two commands
should be treated equivalently:

```
some_cool_program &
some_cool_program < /dev/null &
```

(This is important so that background commands do not consume the user's
input from the terminal, which could prevent Small Shell from receiving those
user inputs).

### Foreground vs background commands

This section only applies to non-built-in commands. Built-in commands should
always run in the foreground (they should not be handled by background
child processes).

A command <ins>without</ins> an ampersand (`&`) at
the end must be executed in the
foreground. In other words, Small Shell (the parent process) must not present
another terminal prompt (`: `) until the command has finished executing.
Small Shell should wait for the designated
child process to terminate via `waitpid()` (*without* `WNOHANG`). Importantly,
if the child process is killed by a signal, Small Shell should
immediately print `terminated by signal S`, followed by a newline character
sequence, where `S` is the signal number of the terminating signal (if the
child process terminated by exiting, Small Shell should print nothing).
Small Shell should then move on and print another terminal prompt (`: `) to
receive the user's next command.

If Small Shell is currently operating in foreground-only mode, all commands
should be treated as foreground commands, even if they have an ampersand at
the end. Foreground-only mode and regular mode are discussed in
[the section on signals](#signals).

However, if Small Shell is currently operating in regular mode (not
foreground-only mode), then any command string <ins>with</ins>
an ampersand (`&`) at
the end is a background command. When Small Shell
forks a child process to run a background
command, Small Shell should <ins>not</ins> immediately use `waitpid()` to
wait for the child process to terminate. Rather, it should immediately
print `background pid is N`, followed by a newline character sequence, where N
is the PID of the newly forked child process. From there, Small Shell
should simply run concurrently
with the child process, printing another terminal prompt to
prompt the user for their next command.

Child processes running background commands must be cleaned up
via `waitpid()` *eventually*. In particular, <ins>every time Small Shell
is just about to print a terminal prompt</ins> (`: `), it should first clean up
all of its background child processes that have terminated but haven't been
cleaned up yet (i.e., all of its existing zombie children). It should
<ins>not</ins> block / hang / wait for them
to terminate; it should simply clean them up *if* they have already terminated
by that point. Recall that you can do this by using `waitpid()` with the
`WNOHANG` option.

Whenever a background child process is cleaned up as explained above,
Small Shell must print a message to the terminal. In particular, if the
child process terminated via exiting, Small Shell should print
`background pid N is done: exit status S`, where `N` is the PID of the
child process that was cleaned up, and `S` is its exit status. However, if
the child process was terminated by a signal, Small Shell should instead
print `background pid N is done: terminated by signal S`, where `N` is the
PID of the child process that was cleaned up, and `S` is the signal number of
its terminating signal. See
[the example output](#example-output) for what this should look like.

> [!NOTE]
  You can use `-1` for the first argument to `waitpid()`, telling
  Small Shell to clean up any single one of its terminated child processes.
  If any have terminated, it will clean up <ins>one</ins> of them and return
  its PID. Else, it will return either 0 or -1, depending on the context and
  whether errors occurred. Consider: because `waitpid()` only cleans up at
  most one child process for each call, it may need to be called several times
  in a loop, in case there are many zombie children waiting to be cleaned up.

> [!NOTE]
  In addition, remember that Small Shell needs to constantly keep track of all
  its remaining background child processes for the purpose of sending them
  all `SIGKILL` when the `exit` built-in command is run by the user. Hence,
  you'll need some sort of data structure (e.g., a dynamic array) to keep track
  of their PIDs. Whenever a background child process is successfully cleaned
  up via `waitpid()`, you should remove their PID from this data structure,
  so that the `exit` command later on doesn't accidentally send `SIGKILL` to
  nonexistent child processes.

As mentioned earlier, if a background command does not explicitly include
an input redirection operator, its standard input should be redirected to
`/dev/null`.

### Signals

When the user presses CTRL+C in the terminal, in general, it will send `SIGINT`
to Small Shell (the parent process) and all of its descendants. This is
an automatic feature of terminals in Linux; you do not need to do anything
special to make this happen. Similarly, when the user presses CTRL+Z, it
will send `SIGTSTP` to Small Shell and all of its descendants.

Your program must handle these signals in very particular ways:

- Small Shell (the parent process) must ignore `SIGINT`.
- All child processses running <ins>background commands</ins> must ignore
  `SIGINT`
- Any child process running a <ins>foreground command</ins> must terminate
  when it receives `SIGINT`. Recall that this is the default action for
  `SIGINT`. Recall that Small Shell's `waitpid()` logic should detect that
  the child was terminated by a signal and print the signal number of the
  terminating signal (in this case, it'd be `2` since that's the signal number
  of `SIGINT`).
- All child processes (including those running foreground commands and
  background commands) must ignore `SIGTSTP`.
- However, when Small Shell itself receives `SIGTSTP`, it must execute
  a custom signal handler that immediately 1) toggles Small Shell between
  **regular mode** and **foreground-only mode**, and 2) writes a message to
  standard output (when toggling to foreground-only mode, it should write the
  message `entering foreground-only mode`, and when toggling to regular mode,
  it should write the message `exiting foreground-only mode`).

  When the program first starts, Small Shell should initially be in regular
  mode. So, upon the first receipt of `SIGTSTP`, it should switch to
  foreground-only mode; upon the second receipt of `SIGTSTP`, it should
  switch back to regular mode; upon the third receipt of `SIGTSTP`, it should
  switch back to foreground-only mode; and so on.

  As explained earlier, when Small Shell is in foreground-only mode, it should
  ignore ampersands (`&`) at the end of all command strings. That is, even
  when the user attempts to run a command in the background, it should
  instead be run in the foreground, treated just like any other foreground
  command.

  > **TIP:**
    Use `write()` instead of `printf()` to write the message to the terminal
    since `printf()` is non-reentrant. When using `write()`, <ins>do not</ins>
    include the null terminator byte (e.g., just use `strlen(message)` to
    compute the length of the message to be written, rather than
    `strlen(message) + 1`). `printf()` and similar utilities automatically
    detect null terminators and avoid writing them to output streams, but
    `write()` does not.

  > **TIP:**
    Track Small Shell's operating mode (regular vs foreground-only) via a
    global variable of type `volatile sig_atomic_t`, toggled within the
    `SIGTSTP` handler. Make sure that your main
    application only interacts with this variable in atomic ways (i.e., it
    just reads it with a single if / else statement---<ins>not</ins> if
    / *else if*) so that the `SIGTSTP`
    handler can't interrupt the operations. Of course, the handler itself
    will need to interact with this global variable in non-atomic ways, so it's
    important that the handler can't interrupt itself in turn---but remember
    that, by
    default, unless `SA_NODEFER` is set in `sa_flags`, a signal will always be
    temporarily blocked whenever its signal handler is executing (you can
    add `SIGTSTP` to its own `sa_mask` field if you want, but that's redundant).

  > **TIP:**
    If `SIGTSTP` is received while the main application is blocking (waiting)
    on user input (e.g., via a `getline()` call), by default, immediately after
    the `SIGTSTP` handler finishes executing, `getline()` will
    terminate early and return `-1` without reading any characters; it
    will set `errno` to `EINTR`; and it will set the standard input stream's
    internal error indicator to 1. Your program
    should be equipped to handle that. A simple, recommended solution is to
    include the `SA_RESTART` flag in the `sa_flags` field of your `sigaction`
    structure when configuring the `SIGTSTP` disposition. This will cause
    interrupted `getline()` calls to automatically restart after the signal
    handler finishes. An alternative, more complicated
    solution is to explicitly check `getline()`'s return value; if it's `-1`,
    use `clearerr(stdin)` to clear the standard input stream's internal
    error indicator, then call `getline()` again in a loop.

### Technical requirements

Your C program must successfully build on the OS1 server via
the shell command `gcc -std=gnu99 -o smallsh *.c`.

You may use any and all functions, types, etc from the C standard
library, POSIX C API, and standard Linux C API. Basically, anything that
can be included off the shelf via `#include` on the OS1 server is fair
game. However, you may **not** download and use third-party libraries.

Your parent process (the one running Small Shell itself) must never encounter
any memory-related errors as reported
by Valgrind (e.g., invalid reads / writes / frees, conditional jumps / moves
depending on uninitialized values, etc).

In **all cases where Small Shell is explicitly required to exit as**
**described in this document**, your
parent process (the one running Small Shell itself) **must** exit with no
dynamic memory in use.
That is, Valgrind should report 0 bytes in 0 blocks in use at exit. This
includes scenarios where the program is required to exit early due to
anticipated errors. (If your program exits due to an *unanticipated*
error not mentioned in this document, such as an out-of-memory error,
then it's okay if there's still some dynamic memory in use at exit).

Note: When a process being traced by Valgrind calls `fork()`, Valgrind
duplicates its internal structures and begins simultaneously tracing both
the parent process and the child process. The only official requirement
here is that the **parent process** (Small Shell itself) does not have
memory-related errors or memory leaks. However, to the extent that it's
within your control, you *should* of course take
care not to introduce any memory-related errors or leaks in the child
processes as well. This simply isn't *tested* because the child processes
will often run arbitrary system-installed programs, and it's not necessarily
your fault if those programs happen to leak memory or encounter memory-related
errors.

> [!TIP]
  To diagnose leaks, Use Valgrind with `--leak-check=full`

### Example output

Below is an example run of Small Shell. A few notes:

- `^C` indicates that the user pressed CTRL+C, and `^Z` indicates that the
  user pressed CTRL+Z.
- Note that CTRL+C has no effect towards the bottom of the example when it's
  used while sitting at the command prompt. At that point, there is no
  foreground child process, and Small Shell (along with background child
  processes) should ignore `SIGINT`.
- The standalone dollar signs (`$`) are Bash's terminal prompts, hence why
  they're only seen at the very beginning and end of the example output
  (everything in between is Small Shell's outputs, which uses colons instead
  of dollar signs for terminal prompts).
  The double-dollar-sign (`$$`), on the other hand, is an intentional reference
  to the PID shell variable, which Small Shell must expand to its own PID
  accordingly (4867, in the below example output).
- Your program's outputs might not look *exactly* like this, but it should
  be very similar, and the phrasing of all the printed messages should match
  verbatim.

```
$ smallsh
: /bin/timedatectl
Local time: Sun 2021-10-17 15:26:30 PDT
Universal time: Sun 2021-10-17 22:26:30 UTC
RTC time: Sun 2021-10-17 22:26:31
Time zone: America/Los_Angeles (PDT, -0700)
System clock synchronized: yes
NTP service: active
RTC in local TZ: no
: ls > junk
: status
exit status 0
: cat junk
junk
smallsh
smallsh.c
: wc < junk > junk2
: wc < junk
3 3 23
: test -f badfile
: status
exit status 1
: wc < badfile
failed to open input file
: status
exit status 1
: badfile
command not found
: sleep 5
^Cterminated by signal 2
: status &
terminated by signal 2
: sleep 15 &
background pid is 4923
: ps
PID TTY TIME CMD
4923 pts/0 00:00:00 sleep
4564 pts/0 00:00:03 bash
4867 pts/0 00:01:32 smallsh
4927 pts/0 00:00:00 ps
:
: # that was a blank command line, this is a comment line
:
background pid 4923 is done: exit status 0
: # the background sleep finally finished
: sleep 30 &
background pid is 4941
: kill -15 4941
background pid 4941 is done: terminated by signal 15
: pwd
/nfs/stak/users/chaudhrn/CS344/prog3
: cd
: pwd
/nfs/stak/users/chaudhrn
: cd CS344
: pwd
/nfs/stak/users/chaudhrn/CS344
: echo hello
hello
: echo $$
4867
: ^C^Z
entering foreground-only mode
: date
Mon Jan 2 11:24:33 PST 2017
: sleep 5 &
: date
Mon Jan 2 11:24:38 PST 2017
: ^Z
exiting foreground-only mode
: date
Mon Jan 2 11:24:39 PST 2017
: sleep 5 &
background pid is 4963
: date
Mon Jan 2 11:24:39 PST 2017
: exit
$
```

## Testing

The starter code ships with a test script to check whether your
program works as specified in this document. You can run this test
script by executing the following Linux shell command from your root
project directory:

`./tests/run.sh`

A very similar (or possibly identical) test script is used to by the
TAs to assist with grading
your submission(s). If you're unsure how to interpret the
outputs of the test scripts, don't hesitate to contact the instructor
or a TA.

## Rubric

This assignment is graded via a grading script very similar to the test script.
Run the grading script to see its corresponding rubric / grade breakdown.
The names of the unit tests reflect the features that they test. The presence
of one or more memory leaks reported by Valgrind (for the parent process)
results in a -5pt penalty. The presence of one or more memory-related errors
(for the parent process) results in a -5pt penalty.

## Extra credit

For 10 points of extra credit, optionally include support for the pipe
operator (`|`). To keep things simple, you may assume that a user's command
string will never include
both a pipe operator *and* standard redirection operators (`>` or `<`).
Moreover, if a user's command string includes a pipe operator, it's guaranteed
that it will <ins>not</ins> contain an ampersand.

If a user's command string contains a pipe operator, then their full command
string will be of the following format:

`command1 [arg1_1 arg1_2 ...] | command2 [arg2_1 arg2_2 ...]`

In such a case, Small Shell should fork <ins>two</ins> child processes.
The first child process should execute the first command (`command1`) with
the specified command-line arguments (`arg1_1`, `arg1_2`, ...), redirecting
its standard output stream to the writing end of a pipe. The second child
process should execute the second command (`command2`) with the specified
command-line arguments (`arg2_1`, `arg2_2`, ...), redirecting its standard
input stream to the reading end of the same pipe. Hence, the outputs of
the first command are piped into the inputs of the second command.

Small Shell should call `waitpid()` twice---once for each of these two
children. You may assume that these child processes will not be killed by
signals.

All un-needed pipe file descriptors should be closed as early as possible.
For example, the first child process does not need the reading file
descriptor of the pipe, so it should close it, and the second child process
does not need the writing file descriptor of the pipe, so it should close it.
Small Shell itself does not need either file descriptors of the pipe,
so it should close both of them (but only *after* forking the two child
processes, else they won't inherit open file descriptors for the pipe).

If the user runs a command with a pipe operator as above, and then they
immediately run the `status` built-in command, the exit status of the
child process that ran the <ins>second command</ins> (`command2`) should
be printed---<ins>not</ins> the exit status of the chlid process that
ran the first command (`command1`).

You may assume that the user will only use the pipe operator with non-built-in
commands (i.e., `command1` and `command2` in the above format will
<ins>not</ins> be `exit`, `cd`, or `status`).

## Submitting

To submit your work:

1. Stage, commit, and push all your changes back to this GitHub repository.
2. Run `git status` just to verify there are no remaining modified files
(uncommited changes).
3. Upload your repository to Gradescope. Wait for the autograder to give you
   a grade.
   - If there are rubric components requiring manual grading, a TA will
     grade them later.
