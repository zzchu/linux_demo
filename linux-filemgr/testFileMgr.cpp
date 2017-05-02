#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#define MAX_LEN 100


unsigned long get_file_size(const char *path)
{
    unsigned long filesize = -1;
    struct stat statbuff;
    if(stat(path, &statbuff) < 0){
        return filesize;
    }else{
        filesize = statbuff.st_size;
    }
    return filesize;
}

int main(void)
{
    
    char* filename = "./makefile";
    DIR *dir;
    struct dirent *ptr;
    char *flow[MAX_LEN];
    int num = 0, i = 0;
    
    printf("file size : %d\n", get_file_size(filename));
    
    char *home;
    home = "./";
    printf("the home path is %s\n", home);
    
    if ((dir=opendir(home)) == NULL)
    {
        perror("Open dir error...");
        exit(1);
    }
    // readdir() return next enter point of directory dir
    while ((ptr=readdir(dir)) != NULL)
    {
        flow[num++] = ptr->d_name;
        printf ("file => %s\n", ptr->d_name);
//        printf("%s\n", flow[num - 1]);
        char* retStr = strstr(ptr->d_name, "core.");
        
        if ((retStr != NULL) && strcmp(retStr, ptr->d_name) == 0) {
            printf("Find the core dump file! find string: %s\n", retStr);
        }
    }
    
    for(i = 0; i < num; i++)
    {
        printf("%s\n", flow[i]);
    }
    
    closedir(dir);
}
