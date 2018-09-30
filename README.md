# Fish Shell

A custom version of a Linux command line shell. 

* Able to run built-in commands as well as commands in both the current directory and those in the PATH environment variable.

* Tracks how long each process runs and makes this information available in the history.

* Supports file output redirection and pipe redirection.

* Handle commands piped in from stdin.

* The shell prompt includes:
    * Command number (starting from 0)
    * User name and host name: (username)@(hostname) followed by:
        * The current working directory
        * A $ sign followed by a space
        * *Note: If the current working directory is the user’s home directory, then the entire path is replaced with* ~

Example shell prompt:
```c
[0|apanisales@apanisales-pi:~]$

```

Example usage:
```c
# Normal usage:
$ ./fish

# Commands piped in from stdin:
$ cat <<EOM | ./fish
ls /
echo "This command is from stdin"
exit
EOM

# Assuming commands.txt contains shell commands:
$ ./fish < commands.txt
```

## Built-In Commands
* **cd**: changes the current working directory. cd without arguments returns to the user’s home directory.
* **#** (comments): strings prefixed with # will be ignored by the shell
* **history**: prints the last 100 commands entered with their command numbers
* **!** (history execution): entering !39 will re-run command 39, for instance. !ls will re-run the most recent command prefixed with ls and !! re-runs the last command that was entered.
* **exit**: exits the shell
