# unix-shell
implement a command line interpreter (shell) on top of Unix

Besides the most basic function of executing commands, the shell (mysh) provides the following three features: interactive vs. batch mode, output redirection, and aliasing.

## features
1. Modes: Interactive and Batch: Your shell can be run in two modes: interactive and batch.   The mode is determined when your shell is started.  If your shell is started with no arguments (i.e., ./mysh) , it will run in interactive mode; if your shell is given the name of a file (e.g., ./mysh batch-file), it runs in batch mode.
2. Redirection: For example, if a user types /bin/ls -la /tmp > output into your shell, nothing should be printed on the screen.  Instead, the standard output of the ls program should be rerouted to the file output.  Note that standard error output should not be changed; it should continue to print to the screen.  If the output file exists before you run your program, you should simply overwrite it (after truncating it, which sets the file's size to zero bytes).
3. Aliases:
Many shells also contain functionality for aliases.   To see the aliases that are currently active in your Linux shell, you can type alias.    Basically, an alias is just a short-cut so that the user can type in something simple and have something more complex (or more safe) be executed. For example, you could set up: mysh> alias ls /bin/lsso that within this shell session, the user can simply type ls and the executable /bin/ls will be run.
