#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

void printstuff(const char *filename, const struct stat * fileStat) {
    printf("File name: %s\n", filename);
    printf("inode number: %lu\n", (unsigned long)fileStat->st_ino);
    printf("number of links: %lu\n", (unsigned long)fileStat->st_nlink);

    printf("User ID of owner: %u\n", fileStat->st_uid);

    printf("Group ID of owner: %u\n", fileStat->st_gid);//WHAT TO PUT HERE

    printf("Size in bytes: %ld bytes\n", (long)fileStat->st_size);
    printf("Last access: %s", ctime(&fileStat->st_atime));
    printf("Last modification: %s", ctime(&fileStat->st_mtime));
    printf("Last status change: %s", ctime(&fileStat->st_ctime));
    printf("Number of disk blocks allocated: %lu\n", (unsigned long)fileStat->st_blocks);
    printf("Access mode in octal: %o\n", fileStat->st_mode);
    printf("Access mode flags: ");
    //How do to access mode flags
    printf((fileStat->st_mode & S_IRUSR) ? "r" : "-");
    printf((fileStat->st_mode & S_IWUSR) ? "w" : "-");
    printf((fileStat->st_mode & S_IXUSR) ? "x" : "-");
    printf((fileStat->st_mode & S_IRGRP) ? "r" : "-");
    printf((fileStat->st_mode & S_IWGRP) ? "w" : "-");
    printf((fileStat->st_mode & S_IXGRP) ? "x" : "-");
    printf((fileStat->st_mode & S_IROTH) ? "r" : "-");
    printf((fileStat->st_mode & S_IWOTH) ? "w" : "-");
    printf((fileStat->st_mode & S_IXOTH) ? "x" : "-");
    printf("\n");
}

int main(int argc, char *argv[])
{
    // todo: Write a C program that prints the type of file for each path provided as a command-line argument

    if (argc != 2) {
        fprintf(stderr, "Usage: %s directory name\n", argv[0]);
        return 1;
    }

    const char *directory_name = argv[1];
    DIR *dir = opendir(directory_name);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {

        char filename[1024];

        snprintf(filename, sizeof(filename), "%s/%s", directory_name, entry->d_name);
        struct stat fileStat;

        stat(filename, &fileStat);
        printstuff(filename, &fileStat);
    }
    

    return 0;

}