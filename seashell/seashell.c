#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define MAX_ARGS 131072
#define MAX_DIR 255


void welcome(){
    printf("\n");
    printf("           _.-''|''-._           \n");
    printf("        .-'     |     `-.        \n");
    printf("      .'\\       |       /`.         _____                 __         ____\n");
    printf("    .'   \\      |      /   `.      / ___/___  ____ ______/ /_  ___  / / /\n");
    printf("    \\     \\     |     /     /      \\__ \\/ _ \\/ __ `/ ___/ __ \\/ _ \\/ / / \n");
    printf("     `\\    \\    |    /    /'      ___/ /  __/ /_/ (__  ) / / /  __/ / /\n");
    printf("       `\\   \\   |   /   /'       /____/\\___/\\__,_/____/_/ /_/\\___/_/_/ \n");
    printf("         `\\  \\  |  /  /'         \n");
    printf("        _.-`\\ \\ | / /'-._        \n");
    printf("       {_____`\\\\|//'_____}       \n");
    printf("               `-'                 \n");
    printf("\n");
    printf("\n");

}

void clear(){
    #if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
        system("clear");
    #endif

    #if defined(_WIN32) || defined(_WIN64)
        system("cls");
    #endif
}


void token_args(char* in_buff, char** out_buff, char* delim){
    int i = 0;
    char* token = strtok(in_buff, delim);
    
    while( token != NULL){
      out_buff[i] = token;
      i++;
      token = strtok(NULL, delim);
      
      assert(i < MAX_ARGS);
    }
    
    out_buff[i] = NULL;
}


int execute_prog(char** args){
    int ret = 0;
    int pipe[2];
    int pid = fork();
    
    if(pid == -1){                          // fork failed
        ret = 0;
        
    }else if( pid == 0){                    // child process
        
        if ( execvp(args[0], args) == -1 ){
            perror("Failed to execute:");
        }
        
        exit(EXIT_FAILURE);
        
    }else{                                  // parent process
        int status = 0;
        
        do {
            waitpid(pid, &status, 0);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        
        ret = 1;
    }


    return ret;
}





int main(int argc, char* argv[]){
    clear();
    welcome();
    
    
    char curdir[MAX_DIR];
    char buff[MAX_ARGS];
    char* args[MAX_ARGS];
    char* delim = " \t\r\n\v\f";

    
     
    
    while(1){
        getcwd(curdir, MAX_DIR);    
        
        printf("Seashell: %s-$", curdir);
        fgets(buff, MAX_ARGS, stdin);
        token_args(buff, args, delim);
        
        //branches Optimize by using switch and hashes?
        
        
        if(args[0]){
            if(!strcmp(args[0], "quit") || !strcmp(args[0], "exit")){
                return 1;
            }    
            else if(!strcmp(args[0], "clear")){
                clear();
            }
            else if(!strcmp(args[0], "cd")){
                
                if(chdir(args[1]) == -1){
                    perror("Error");
                }
            }
            else{

                execute_prog(args);
            }
        }
        
        
        
    }  

    return 1;
} 