#include <iostream>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>

/*
To synchronize the child processes in C++, 
one should sort the PIDs of the child processes in ascending order, 
print from the parent process, then iterate in reverse order sending a signal to each child. 
Finally, the parent process should print its exiting message.
*/


int main(int argc, char** argv) {
    int n = 1, opt;
    while ((opt = getopt(argc, argv, "n:")) != -1) {
        switch (opt) {
        case 'n':
            n = atoi(optarg);
            break;
        }
    }

    /*
    1. TODO: fork n child processes and run p1-helper on each using execvp
        > note: we need to synchronize the child processes to print in the desired order
        > note: sleep(..) does not guarantee desired synchronization
        > note: check "raise" system call
        > note: check "WUNTRACED" flag for "waitpid"
    */
    std::vector<pid_t> child_pids;
    for (int i = 0; i < n; i ++) { //MAYBE NOT n - 1
        pid_t pid = fork();
        if (pid == 0) {
            char* args[] = {nullptr};
            raise(SIGSTOP);
            execvp("./p1-helper", args);
        }
        else if (pid > 0) {
            // parent
            child_pids.push_back(pid);
            //HAVE SOME WAIT HERE??
        }
    }
    
    /* 
    2. TODO: print children pids 
    */

   for (long unsigned int i = 0; i < child_pids.size(); i ++) {
       if (i == child_pids.size() - 1) {
           printf("%d\n", child_pids[i]);
       }
       else {
           printf("%d ", child_pids[i]);
       }
   }

   

    

    fflush(stdout);             // DO NOT REMOVE: ensures the first line prints all pids
    std::sort(child_pids.begin(), child_pids.end(), std::greater<pid_t>());
    /* 
    3. TODO: signal children with the reverse order of pids 
        > note: take a look at "kill" system call 
    */

    for (auto child : child_pids) {
        kill(child, SIGCONT);
        waitpid(child, nullptr, 0);
    }


    printf("Parent: exiting\n");

    return 0;
}