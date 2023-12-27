# Assignment 3: System Information Fetching Kernel Module

- Click [here](https://hackmd.io/@a3020008/r1Txj5ES6) for the assignment description.

- OS: Ubuntu 22.04 AMD64 / ARM64

- You can test the program by first compiling it:

    `cc kfetch.c -o kfetch`

    and then run it with the option -h as root

    `sudo ./kfetch -h`

    It will show the program usage:
    
    ```
    Usage:
        ./kfetch [options]
    Options:
        -a  Show all information
        -c  Show CPU model name
        -m  Show memory information
        -n  Show the number of CPU cores
        -p  Show the number of processes
        -r  Show the kernel release information
        -u  Show how long the system has been running
    ```