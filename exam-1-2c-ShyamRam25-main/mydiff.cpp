#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

/* Process 1 must use pipe1, Process 2 must use pipe2 */
const char* PIPES[] = {"./pipe1", "./pipe2"};

vector<string> parse_command(string command) {
    /*
    This function parses a string command and returns the list of arguments
    DO NOT modify this function
    */
	vector<string> args;
	string arg;
	for (char c : command) {
		if (c == ' ') {
			if (arg.length() > 0) args.push_back(arg);
			arg = "";
		} else {
			arg += c;
		}
	}
	if (arg.length() > 0) args.push_back(arg);
	return args;
}

void redirect_stdout(int pipeid) {
	/* TODO: Open the correct pipe, and redirect stdout to the pipe */
    int new_pipe = open(PIPES[pipeid], O_WRONLY);
    dup2(new_pipe, 1);
    close(new_pipe);
    /* ******************************************* */
}

int main(int argc, char* argv[]) {
	// We can only accept two commands
    if (argc != 3) {
        cerr << "Usage: ./mydiff <command1> <command2>" << endl;
        exit(EXIT_FAILURE);
    }
	// extract the commands to be executed
    vector<string> commands[2] = {parse_command(argv[1]), parse_command(argv[2])};

    pid_t pids[2];
    std::ifstream fstreams[2];


    for(int i = 0; i < 2; ++i) {
        /* TODO: Create a named pipe here with the right permissions */
        mkfifo(PIPES[i], 0666);

        /* ********************************************************* */

        /* TODO: Create a new process */
        pid_t pid = fork();
        
        /* ************************** */
        if (pid == 0/* Execute this block only for the child process */) {
            redirect_stdout(i);
            /* TODO: Execute the command commands[i] here. */
            char ** args = new char*[commands[i].size() + 1];
            for (long unsigned int j = 0; j < commands[i].size(); j ++) {
                args[j] = const_cast<char*>(commands[i][j].c_str());
            }
            args[commands[i].size()] = nullptr;

            execvp(args[0], args);

            /***********************************************/
        }
        else {
            // parent process
            pids[i] = pid;
            // create streams for reading from pipes (no need to modify)
            fstreams[i] = std::ifstream(PIPES[i], std::ios::in | std::ios::binary);
        }
    }

	/* compare lines and print differences
    DO NOT modify this section.
    */
	string line1, line2;
	while (fstreams[0] && fstreams[1]) {
        getline(fstreams[0], line1);
        getline(fstreams[1], line2);
		if (line1 != line2) {
			cout << "<<<<<<<<<<<<<<<<<<<<<<" << endl;
			cout << line1 << endl;
			cout << "======================" << endl;
			cout << line2 << endl;
			cout << ">>>>>>>>>>>>>>>>>>>>>>" << endl;
		}
	}
    while(fstreams[0]) {
        getline(fstreams[0], line1);
        cout << "<<<<<<<<<<<<<<<<<<<<<<" << endl;
        cout << line1 << endl;
        cout << "======================" << endl;
        cout << ">>>>>>>>>>>>>>>>>>>>>>" << endl;
    }
    while(fstreams[1]) {
        getline(fstreams[1], line2);
        cout << "<<<<<<<<<<<<<<<<<<<<<<" << endl;
        cout << "======================" << endl;
        cout << line2 << endl;
        cout << ">>>>>>>>>>>>>>>>>>>>>>" << endl;
    }

    
    for(int i = 0; i < 2; ++i) {
        /* TODO: Cleanup: close any open pipes and wait for any child processes */
        //CLOSE something
        
        waitpid(pids[i], NULL, 0);
        /* *********************************************************** */
        unlink(PIPES[i]);
    }
}
