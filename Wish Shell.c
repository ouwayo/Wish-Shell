#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pthread.h>
#include <process.h>
#include <sys/types.h>


#define NTHREADS 10
#define size 10

void errorMessage(); 
char **split_input(char *s,int* i);  
void prompt(); 
int wish_exit(); 
int wish_path(char **args); 
int wish_cd(char **location); 
char *concat(char *s1,char *s2);
int execute_command(char **arg); 
// whenever user types exit.     
int wish_exit(){
    exit(0);
    }
// This is the array of builtin commands, specifically required for this project.    
char *builtin_comm[] = {
        "cd",
        "path",
        "exit"
    };

//concat() function is meant to concatenate 2 strings(i.e path and filename) into 1 string.
char *concat(char *s1,char *s2){
   
        /* Define a temporary variable */
        char *var = (char *) malloc (sizeof (char));

        /* Copy the first string into the variable */
        strcpy(var, s1);

        /* Concatenate the following two strings to the end of the first one */
        strcat(var, s2);
        return var; 
    }

// This is an array of function pointers. 
//It takes the strings of my builtin commands. 
int (*builtin_func[]) (char **) = {
        &wish_cd,
        &wish_path,
        &wish_exit
    };

/*
The  setenv()  function adds the variable name to the environment with the value value,
 if name does not already exist.
 ref: man setenv()
*/
int wish_path(char **args)
    {    
        char *name=args[2]; // Not used here yet. 
        if (args[1] != NULL){
            setenv("$PATH", args[1], 0);
            return 1;
        } else {
            errorMessage(); 
        }
   }

//wish_cd() changes the currennt directory of the wish bash shell
// for example: cd ..  will change the directory back
int wish_cd(char **location){
    if(strcmp(location[0], "cd") == 0) 
    {
        if(location[1] != NULL && location[2] == NULL)
        {
            chdir(location[1]); 
            return 1;
        }
        if(location[2] != NULL || location[1] == NULL)
        {   
            errorMessage();            
        }
        // R_OK, W_OK, and X_OK test whether the input exists and grants
      //  read, write, and execute permissions, respectively.
        else if(access(location[1], R_OK)!=0)
        {   
            errorMessage();             
        }
    }
    return 1;
	}
	
	
//strremove() takes out all of the newline characters from the inputs given by the user
char *strremove(char *str) {
        char* new= str;  
        char* sub = str;

        while (*new)
        {
            if (*new != ' ' && *new != '\n')
                *sub++ = *new;
            new++;
        }
        *sub = 0;
        return str;
	}



//execute_file() is used to read the commands provided by the user.

char *execute_file(){
        char *line = NULL;
        ssize_t rmode = 0; 
        if (getline(&line, &rmode, stdin) == -1) {
            if (feof(stdin)) {
            exit(EXIT_SUCCESS);  
            } else  {
            errorMessage(); 
            exit(EXIT_FAILURE);
            }
        }
        return line;
	}

//the execute_command() function is responsble for executing...
//the command and the parameters passed into the function.
//And this is where the process will be started with the help of execv() sys call. 
int execute_command(char **arg){
	
        pid_t pid;
        int status;
        char **path  = malloc(sizeof(char*));
        path[0] = strdup("/bin");  //default path as required. 
        path[1] = NULL;
        char* path_loc = NULL;
        int i;
        pid = fork();
        if (pid == 0) {
            for(i = 0; path[i] != NULL; i++){
                path_loc = malloc(sizeof(path[i])*size);
                strcpy(path_loc, path[i]);
                strcat(path_loc,"/");
                strcat(path_loc, arg[0]);
                if (access(path_loc, X_OK) == 0){ 
                    execv(path_loc,arg);   //apply execv, where it takes path and arg
                    errorMessage();
                        }
                else{
                    errorMessage();
                }
            }
        } else if (pid < 0){
            errorMessage();
        } 
        else {
            // For the parent process 
            do {
            waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }

     return 1;
	}
    

//check_file() function checks to see if a file can be found in any of the directories and if it is executable
int check_file(char *c){
        
        FILE* filename; 
        // In case user passes a batch file, does it exist?
        if(c!= NULL){

            if(access(c, R_OK) == 0){
                filename = fopen(c, "r");
            } else {
                errorMessage(); 
                exit(0);
            }
        
        }
    }    

//parallel_check_file() is responsible for checking whether a file provided through parallel commands is executable.
int parallel_check_file(char *argv[]){
    FILE *input; 
    if(argv[1] != NULL){

            if(access(argv[1], R_OK) == 0){
                input = fopen(argv[1], "r");
            } else {
                errorMessage(); //error if path invalid
                exit(0);
            }

        } else{
            input = stdin;
            prompt();
        }
    }

//split_input function splits the input provided by the user into tokens and places them in an array
char **split_input(char *s,int* i){
    char* del=" \t\r\n\a"; 
    char **array_comm = malloc(sizeof(char*));
    char *token = strtok(s, del); 
    int index = 0;
   // Checking all tokens 
    while(token != NULL){
        array_comm[index] = token;
        index++;
        array_comm = realloc(array_comm,sizeof(char*));
        token = strtok(NULL, del);
    }
    
    array_comm[index] = NULL;
    if (i!= NULL)
        *i = index;
    return array_comm;
	
	}
// wish prompt. 
void prompt()  {
        printf("\033[1;33m");  // set a color
        printf("wish> ");
        printf("\033[0m");  // clear the color
        
   }
// This is the error message that comes up whenever an error occurs.    
void errorMessage()
{   
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
   }

  // The following function calls runs the builtin commands and used by the process execution.  
int wish_execute(char **args)
{
        if (args[0] == NULL) {
            return 1;
        }
        for (int i = 0; i <3; i++) { // working with 3 builtin's 
            if (strcmp(args[0],builtin_comm[i]) == 0) {
            return (*builtin_func[i])(args);
            }
        }
        return execute_command(args);
   }
void wish_loop(void)
{
        char *line;
        char **args;
        int status;
        int k;

        do {
            prompt(); 
            line = execute_file();
            args = split_input(line,&k);
            status = wish_execute(args);

            free(line);
            free(args);
        } while (status);
   }

	
//the main function of the wish command is going to simulate a shell program.
int main(int argc,char *argv[]){
        FILE* filNam; 
       int m;
        size_t len=0; 
        // check if the file provided exist. 
        if(argv[1] != NULL){

            if(access(argv[1], R_OK) == 0){
                filNam = fopen(argv[1], "r");
                char *line; 
                while (getline(&line, &len,filNam) != -1) {
                   char **args = split_input(line,&m);
                    wish_execute(args);
                    
                        
                    }

            }
            else {
                errorMessage(); 
                wish_loop();
            }

        }
        else{
            filNam = stdin;
            wish_loop(); 
        }
}



