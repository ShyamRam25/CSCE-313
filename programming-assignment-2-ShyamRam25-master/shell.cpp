#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <string>

#include "Tokenizer.h"

// all the basic colours for a shell prompt
#define RED     "\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE	"\033[1;34m"
#define WHITE	"\033[1;37m"
#define NC      "\033[0m"

using namespace std;

int main () {
    //Create copies of stdin/stdout
    int stdin_copy = dup(0);
    int stdout_copy = dup(1);
    int fd[2];
    vector<int> pids;
    vector<char*> directory;
    char* cwd = get_current_dir_name();
    directory.push_back(cwd);

    while (true) {
        time_t rawtime = time(NULL);
        string time = ctime(&rawtime);
        cwd = get_current_dir_name();
        char* username = getlogin();

        cout << GREEN << username << " " << time.substr(0, time.size() - 1) << " " << cwd << NC << endl;
        free(cwd);

        for (long unsigned int i = 0; i < pids.size(); i++) { 
            int status = 0;
            if (waitpid(pids.at(i), &status, WNOHANG) == pids.at(i)) {
                pids.erase(pids.begin() + i);
            }
        }

        cout << YELLOW << "Shell$" << NC << " ";
        
        // get user inputted command
        string input;
        getline(cin, input);
        char * newcwd = nullptr;
        if (input == "exit") {  // print exit message and break out of infinite loop
            //Make sure to free exisitng memory
            if (directory.size() == 0) {
                free(directory.at(0));
            }
            else {
                for (unsigned long int i = 0; i < directory.size(); i++) {
                    free(directory.at(i));
                }
            }
            cout << RED << "Now exiting shell..." << endl << "Goodbye" << NC << endl;
            break;
        }



        // get tokenized commands from user input
        Tokenizer tknr(input);
        if (tknr.hasError()) {  // continue to next prompt if input had an error
            continue;
        }


        //implement cd with chdir()
        //if dir (cd <pir>) is "-" then go to previous working directory
        //variable sorting previous working directory (it needs to be declared outside loop)
        if (tknr.commands[0]->args[0] == "cd") {
            if (tknr.commands[0]->args[1] == "-") {
                if (directory.size() == 1) {
                    cout << "No previous directory" << endl;
                    continue;
                }

                newcwd = get_current_dir_name();
                chdir(directory.back());
                directory.push_back(newcwd);
            }
            else {
                newcwd = get_current_dir_name();
                chdir(tknr.commands[0]->args[1].c_str()); //MIGHT BE FLIPPED
                directory.push_back(newcwd);

            }
            continue;
        }

        // // print out every command token-by-token on individual lines
        // // prints to cerr to avoid influencing autograder
        // for (auto cmd : tknr.commands) {
        //     for (auto str : cmd->args) {
        //         cerr << "|" << str << "| ";
        //     }
        //     if (cmd->hasInput()) {
        //         cerr << "in< " << cmd->in_file << " ";
        //     }
        //     if (cmd->hasOutput()) {
        //         cerr << "out> " << cmd->out_file << " ";
        //     }
        //     cerr << endl;
        // }

        //For piping
        //for cmd : commands
        //          call pipe() to make pipe
        //          fork() - in child, redirect stdout; in par, redirect stdin
        //
        // add checks for first/last command

        for (unsigned long int i = 0; i < tknr.commands.size(); i++) {
            pipe(fd);
            // fork to create child
            pid_t pid = fork();
            if (pid < 0) {  // error check
                perror("fork");
                exit(2);
            }
            
            else if (pid == 0) {  // if child, exec to run command
                // implement multiple arguments - iterate over 
                // char * array
                // char* args[] = {(char*) tknr.commands.at(0)->args.at(0).c_str(), nullptr};
                if (i < tknr.commands.size() - 1) {
                    dup2(fd[1], 1);
                    close(fd[0]);
                }

                char **cmd = new char*[tknr.commands[i]->args.size()+1];

                for (unsigned long int j = 0; j < tknr.commands[i]->args.size(); j++) {
                    cmd[j] = new char[tknr.commands[i]->args[j].size()];
                    memcpy(cmd[j], tknr.commands[i]->args[j].c_str(), tknr.commands[i]->args[j].size());
                }
                cmd[tknr.commands[i]->args.size()] = nullptr;

                if (tknr.commands[i]->hasInput()) {
                    int input_file = open(const_cast<char*>((tknr.commands[i]->in_file).c_str()), O_RDONLY, S_IWUSR | S_IRUSR);
                    dup2(input_file, 0);
                }

                // IO redirection
                if (tknr.commands[i]->hasOutput()) {
                    int output_file = open(const_cast<char*>((tknr.commands[i]->out_file).c_str()), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR, S_IRGRP | S_IROTH);
                    dup2(output_file, 1);
                }
                
                //if current command is redirected, then open file and dup2 std(in/out) that's being redirected
                //implement it safely for both at the same time
                if (execvp(cmd[0], cmd) < 0) {  // error check
                    perror("execvp");
                    exit(2);
                }
            }
            else {  // if parent, wait for child to finish
                //add check for bg process - add pid to vector if bg and dont waitpid() in par
                if (tknr.commands[i]->isBackground()) {
                    pids.push_back(pid);
                    continue;
                }
                
                dup2(fd[0], 0);
                close(fd[1]);
                waitpid(pid, NULL, 0);
                }
            }
            //restore stdin/stdout (variable would be outside the loop)

        }
    dup2(stdin_copy, 0);
    dup2(stdout_copy, 1);
    close(stdin_copy);
    close(stdout_copy);
    }

