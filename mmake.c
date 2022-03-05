/* 
 * Description:  This is a program that is used to open and parse a given or defult makefile. This program can take 3 flags:
 *               -B this flag force the program to update.
 *               -s this flag silcene the stdout.
 *               -f this flag makes the program open and parse another makefile that the defult one.
 * Author:       Mohammad Mshaleh
 * CS username:  dv19mmh
 * Date:         2021,08,22
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <getopt.h>
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>
#include <assert.h>
#include "parser.h"


bool get_time_difference(const char* file_1,const char* file_2);
void execute(const char* target_file_name,makefile *makefile_file, int s_flag, int b_flag);




int main(int argc, char *argv[]){

    FILE *target_file;
    const char *target_file_name;
    int temp_flag, f_flag = 0, s_flag = 0, b_flag = 0;
    
    //Parsing the program given arguments.
    while((temp_flag = getopt(argc, argv , "B | f:%s | s")) != -1){ 
        switch (temp_flag){
            case 'B':
                b_flag = 1;
                break;
            case 's':
                s_flag = 1;
                break;
            case 'f':
                f_flag = 1;
                target_file_name = optarg;
                break;
            case '?':
                fprintf(stderr, "Unknown flag -%c try again",optopt);
                break;
        }
    }
    
    //Opening the given target file, if non is given the defult mmakefile will be opened instead.
    if(f_flag == 1){
        if ((target_file = fopen(target_file_name,"r")) == NULL){
            perror("fopen");
            exit(1);
        }
    }else{
        if((target_file = fopen("mmakefile","r")) == NULL){
            perror("fopen");
            exit(1);
        }
    }

    //Parsing the makefile.
    makefile *makefile_file;
    if ((makefile_file = parse_makefile(target_file)) == NULL){
        perror("Could not parse the makefile! \n");
        exit(1);
    }

    // Excuting the makefile.
    if(optind == argc){
        target_file_name = makefile_default_target(makefile_file);
        execute(target_file_name,makefile_file,s_flag,b_flag);
    }else{
        for (int i = optind; i < argc; i++){
            execute(argv[i],makefile_file,s_flag,b_flag);
        }
    }

    makefile_del(makefile_file);

    return 0;
}


/**
* @brief          This is a recusivly function that execute the commands in the given makefile. 
*
* @param 1        The name of the targeted file.
* @param 2        The makefile.
* @param 3        The -s flag.
* @param 4        The -B flag.
*
* @return         noting.
*/
void execute(const char* target_file_name,makefile *makefile_file, int s_flag, int b_flag){
    
    rule *target_file_rule = makefile_rule(makefile_file,target_file_name);
    if(target_file_rule == NULL){
        if (!(access(target_file_name, F_OK) == 0)){
            fprintf(stderr,"No rule to make target '%s'\n",target_file_name);
            exit(1); 
        }
        return;
    }else{
        
        int temp;
        char **command;
        bool need_update = false;
        const char **target_rule = rule_prereq(target_file_rule);

        for (temp = 0; target_rule[temp] != NULL; temp++){
           execute(target_rule[temp],makefile_file,s_flag,b_flag);
        }

        for (int i = 0; i < temp; i++){
            if(!(access(target_file_name, F_OK) == 0) || get_time_difference(target_file_name,target_rule[i]) || b_flag == 1){
                need_update = true;
                break;
            }
        }

        if(need_update){
            pid_t pid;
            int status;
            if ((pid = fork()) < 0){
                perror("Error: fork");
                exit(1);
            }else if(pid == 0){
                command = rule_cmd(target_file_rule);
                if (s_flag == 0) {
                    fprintf(stdout, "%s", command[0]); 
                    for (int i = 1; command[i] != NULL ; i++){
                        fprintf(stdout, " %s", command[i]); 
                    }
                    printf("\n" );
                }
                fflush(stdout);
                execvp(command[0], command);
                exit(0); 
            }
            if ((pid = wait(&status)) == -1){
                perror("Error while witing");
            }else if (status != 0){
                exit(1);
            }
        }

    }
}

/**
* @brief          This function get the times of to diffrent files and then retuns the time diffrent 
*                 between them and then return true or false if the time diffrent is < 0.         
*
* @param 1        The name of the first file.
* @param 2        The name of the second file.
*
* @return         True/False
*/
bool get_time_difference(const char* file_1,const char* file_2){
    struct stat struct_1, struct_2;
    if(lstat(file_1,&struct_1) < 0){
        perror("lstat");
    }
    if(lstat(file_2,&struct_2) < 0){
        perror("lstat");
    }
    return difftime(struct_1.st_mtime, struct_2.st_mtime) < 0;
}