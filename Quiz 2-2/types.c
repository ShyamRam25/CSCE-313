#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


void printStuff(char *filename) {
    struct stat fileStat;

    lstat(filename, &fileStat);

    if (S_ISREG(fileStat.st_mode)) {
        printf("%s: regular file\n", filename);
        return;
    }
    if (S_ISDIR(fileStat.st_mode)) {
        printf("%s: directory\n", filename);return;
    }
     if (S_ISLNK(fileStat.st_mode)) {
        printf("%s: symbolic link\n", filename);
        return;
    }
     if (S_ISCHR(fileStat.st_mode)) {
        printf("%s: character special file\n", filename);
        return;
    }
     if (S_ISBLK(fileStat.st_mode)) {
        printf("%s: block device\n", filename);
        return;
    }
     if (S_ISFIFO(fileStat.st_mode)) {
        printf("%s: FIFO\n", filename);
        return;
    }
    else {
        printf("%s: path error\n", filename);
        return;
    }

}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s path1 path2 ...\n", argv[0]);
        return 1;
    }
    // Write a C program that prints the type of file for each path provided as a command-line argument.

    for (size_t i = 1; i < argc; i++) {
        printStuff(argv[i]);
    }

    return 0;
}