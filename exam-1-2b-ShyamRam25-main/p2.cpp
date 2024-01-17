#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#define MAX_MESSAGE 256

long long unsigned int hash(int seed, char* buf, int nbytes) {
    long long unsigned int H = seed; 
    for (int i = 0; i < nbytes; ++i) 
        H = H * 2 * buf[i] + 1;
    return H;
}

int main(int argc, char** argv) {

    if (argc < 2){
        return 1;
    }
	
    int fd1[2];
    pipe(fd1);

    int pid = fork();
    if (pid == 0) {
        char buf[256];
        
        int bytes = read(fd1[0],buf,sizeof(buf));
        close(fd1[0]);

        long long unsigned int h = hash(getpid(), buf, bytes);


        write(fd1[1],&h,sizeof(&h)); 

        close(fd1[1]);
    }
    else {

        write(fd1[1],argv[1],strlen(argv[1]));
        close(fd1[1]);

        waitpid(pid,nullptr,0);
        
        long long unsigned int hrecv;
        read(fd1[0],&hrecv,sizeof(&hrecv));

        long long unsigned int h = hash(pid, argv[1], strlen(argv[1]));
		
		// print hashes; DO NOT change
        printf("%llX\n", h);
        printf("%llX\n", hrecv);
    }

    return 0;
}