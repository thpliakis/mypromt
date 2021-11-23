#include<stdio.h> 
#include <errno.h>
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/wait.h> 
#include<readline/readline.h> 
#include<readline/history.h> 

#include <fcntl.h>

#define clear() printf("\033[H\033[J")
#define MAXCHAR 512

#define true 1
#define false 0

#define BLUE(string) "\x1b[34m" string "\x1b[0m"
#define RED(string) "\x1b[31m" string "\x1b[0m"

int batch_mode = 0; //To know if we are in batch mode or not

typedef struct {
    char **tokens;
    int numberOfElements;
}info_struct;

void removeSpace(char* str){
    int count = 0;

    for (int i = 0; str[i]; i++)
        if (str[i] != ' ')
            str[count++] = str[i]; 
    str[count] = '\0';
}

void initialize_shell(){
    clear();
    printf("Max number of characters allowed: 512\n");
    printf(BLUE("Pliakis_9018> "));
    //sleep(1);
}

void printDir()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("\nDir: %s", cwd);
} 


int spawn_proc (int in, int out, char** parsed)
{
    pid_t pid;
    int status;
    //printf("OUTSIDE FORK \nin = %d\nout = %d\n",in,out);

    if ((pid = fork ()) == 0)
    {
        //printf("INSIDE FORK \nin = %d\nout = %d\n",in,out);
        if (in != 0)
        {
            dup2 (in, 0);
            close (in);
        }

        if (out != 1)
        {
            dup2 (out, 1);
            //printf("fd[1]= %d\n",out);
            close (out);
        }
        //printf("asdfsdparsed[0]=%s",parsed[0]);
        return execvp (parsed[0], parsed);
    }else{
        if (waitpid (pid, &status, 0) != pid){
            //printf("status: %d\n",status);
            status = -1;
        }
        /*
        else {
            if (WIFEXITED(status))
                printf("child exited with status of %d\n", WEXITSTATUS(status));
            else puts("child did not exit successfully");
        }
        */
    }

    return pid;
}


#define LSH_TOK_BUFSIZE 64
//#define LSH_TOK_DELIM " \t\r\n\a"
info_struct parsing_line(char *line,char* delim)
{
    int bufsize = LSH_TOK_BUFSIZE;
    info_struct args;
    args.numberOfElements = 0;
    args.tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!args.tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, delim);
    while (token != NULL) {
        args.tokens[args.numberOfElements] = token;
        args.numberOfElements++;
        // printf("%d\n",args.numberOfElements);
        //     printf("%s\n",token);

        if (args.numberOfElements >= bufsize) {
            bufsize += LSH_TOK_BUFSIZE;
            args.tokens = realloc(args.tokens, bufsize * sizeof(char*));
            if (!args.tokens) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, delim);
    }
    args.tokens[args.numberOfElements] = NULL;

    /*
       printf("%s\n",args.tokens[args.numberOfElements-1]);
       if (execvp(args.tokens[args.numberOfElements-1],&args.tokens[args.numberOfElements-1]) == -1)
       {
       printf("failure to execute because %s\n", strerror(errno));
    //    exit(0);
    }
    printf("%s\n",args.tokens[args.numberOfElements-1]);
    */

    return args;
}

void fork_pipes (info_struct parsed){
    int i,j=0;
    int in, fd [2],x;
    info_struct local_parsed ,local_parsed2;
    local_parsed = parsing_line(parsed.tokens[0]," ");
    local_parsed2 = parsing_line(parsed.tokens[1]," ");
    
    int savestdin = dup(0);
    int savestdout = dup(1);
    in = 0;

    for (i = 0; i < 2 ; ++i)
    {
        if(i==0){
            pipe (fd);
            spawn_proc (in, fd [1], local_parsed.tokens);
        }else{
            spawn_proc (in, fd [1], local_parsed2.tokens);
        }
        close (fd [1]);
        in = fd [0];
    }
    if (in != 0)
        dup2 (in, 0);
    dup2(savestdin,0);
    dup2(savestdout,1);
    /* Execute the last stage with the current process. */
    //return execvp (cmd [i].argv [0], (char * const *)cmd [i].argv);
}

int execArguments(char** parsed) 
{ 
    //    removeSpace(parsed[0]);
    // Forking a child 
    int status,execvp_status;
    pid_t pid = fork();  

    //printf("out :%d\n",out);

    if (pid == -1)
    { 
        printf("\nFailed forking child.."); 
        return 0; 
    } 
    else if (pid == 0)  //Child proccess.
    { 
        if ((execvp_status = execvp(parsed[0], parsed) )< 0)
        { 
            printf("Could not execute command..\n"); 
            //printf("Child ,sleep 2\n"); 
            //sleep(2);
            exit(EXIT_FAILURE); 
        } 
    } 
    else
    { 
        //Parent proccess.
        if (waitpid (pid, &status, 0) != pid){
            //printf("status: %d\n",status);
            status = -1;
        }
        else {
            if (!WEXITSTATUS(status))
                //printf("child exited with status of %d\n", WEXITSTATUS(status));
                return status;
            else{
                //puts("child did not exit successfully");
                exit(1);
            }
        }
        //printf("return status = %d\n",status);
        //return status;
    } 
} 

char *getInput(void)
{
    char *line = NULL;
    size_t bufsize = 512; 
    ssize_t read;
    //getline(&line, &bufsize, stdin);
    while ((read = getline(&line, &bufsize, stdin)) != -1) {
        if(read>511){
            printf("Line with over 512 characters.Try again...\n");
            printf(BLUE("Pliakis_9018> "));
            continue;
        }
        return line;
    }
}



int almost_main(char* inputString){

    char fileName[40];
    info_struct parsedArgs,parsedArgs1,parsedArgs2;
    int exec_status;
    int  between_semicolons = 0,between_ampersands =0;
    int count = 0;
    int a = false;
    int b = false;
    int file;
    int savestdout;
    int  quit_search=0;

    count = 0;
    a = false;
    b = false;
    parsedArgs = parsing_line(inputString,";\t\r\n\a");
    between_semicolons = parsedArgs.numberOfElements;
    for(int i = 0;i < between_semicolons;i++)
    {
        //printf("here : %s\n",parsedArgs.tokens[i]);
        parsedArgs1 = parsing_line(parsedArgs.tokens[i],"&\t\r\n\a");
        between_ampersands = parsedArgs1.numberOfElements;
        for(int j = 0;j < between_ampersands;j++)
        {
            //printf("parsedArgs1.tokens[%d]: %s\n",j,parsedArgs1.tokens[j]);
            if(batch_mode)
                printf("%s\n",parsedArgs1.tokens[j]);
            for (int k = 0; parsedArgs1.tokens[j][k]; k++){
                if (parsedArgs1.tokens[j][k] == '>')
                {
                    a=true;  //True if there is redirection.
                    parsedArgs1.tokens[j][k] = '\0';
                    strcpy(fileName,&parsedArgs1.tokens[j][k+1]);
                    //printf("fileName: %s\n",fileName);
                }
            }
            for (int k = 0; parsedArgs1.tokens[j][k]; k++){
                if (parsedArgs1.tokens[j][k] == '<')
                {
                    parsedArgs1.tokens[j][k] = ' ';
                }
            }
            for (int k = 0; parsedArgs1.tokens[j][k]; k++){
                //printf("%d\n",parsedArgs1.tokens[j][k]);
                if(parsedArgs1.tokens[j][k] == '|'){
                    //parsedArgs1.tokens[j][k] = ' ';
                    parsedArgs2 = parsing_line(parsedArgs1.tokens[j],"|");
                    //printf("parsedArgs2.numberOfElements= %d\n",parsedArgs2.numberOfElements);
                    fork_pipes(parsedArgs2);
                    b=true;
                    break;
                }
            }
            if(b)
                continue;
            //printf("a = %d\n",a);
            parsedArgs2 = parsing_line(parsedArgs1.tokens[j]," ");
            quit_search = parsedArgs2.numberOfElements;
            for(int x = 0;x < quit_search;x++)
                if(strcmp(parsedArgs2.tokens[x],"quit") ==0){
                    printf("\n");
                    exit(0);
                }
            removeSpace(fileName);
            if (a){
                savestdout = dup(1);
                file = open(fileName,O_WRONLY | O_RDONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
                dup2(file, 1);
                //printf("file = %d\n",file);
            }
            exec_status = execArguments(parsedArgs2.tokens);
            if (a){
                if(close(file)!=0)
                    printf("error");
                dup2(savestdout,1);
            }
            //printf("exec_status: %d\n",exec_status);
            if (exec_status != 0){
                //printf("exec_status = %d\n",exec_status);
                break;
            }
        }
    }
    printf(RED("Pliakis_9018> "));
    return 1;
}

void check_quit(char* filename){
    char* inputString = NULL;
    char c[512];
    FILE *fptr;
    char * line = NULL;
    size_t len = 512;
    ssize_t read;
    int b = false;
    info_struct parsedArg;

    if ((fptr = fopen(filename, "r")) == NULL)
    {
        printf("Error! opening file");
        // Program exits if file pointer returns NULL.
        exit(1);
    }
    while ((read = getline(&inputString, &len, fptr)) != -1) {
        parsedArg = parsing_line(inputString,";& \t\r\n\a");
        for(int i=0;i<parsedArg.numberOfElements;i++){
            //printf("parsedArg.tokens[%d] = %s\n",i,parsedArg.tokens[i]);
            if(strcmp(parsedArg.tokens[i],"quit") == 0){
                b = true;
            }
        }
    }
    if(!b){
        printf("No 'quit' found in the file.Exiting...\n" );
        exit(1);
    }
}

int main(int argc, char *argv[]){
    char* inputString = NULL;
    FILE *fptr;
    char * line = NULL;
    size_t len = 512;
    ssize_t read;

    initialize_shell();

    if (argc ==1){
        while(1){
            inputString = getInput();
            almost_main(inputString);
        }
    }
    else if(argc == 2){
        batch_mode = 1;
        if ((fptr = fopen(argv[1], "r")) == NULL)
        {
            printf("Error! opening file");
            // Program exits if file pointer returns NULL.
            exit(1);
        }
        check_quit(argv[1]);
        while ((read = getline(&inputString, &len, fptr)) != -1) {
            if(read ==1){
                printf("\n");
                almost_main(inputString);
                //printf("Line with 0 characters...\n");
                //printf(BLUE("Pliakis_9018> "));
                continue;
            }                
            if(read>511){
                printf("Line with over 512 characters...\n");
                printf(BLUE("Pliakis_9018> "));
                continue;
            }
            almost_main(inputString);
        }
        fclose(fptr);
        if (line)
            free(line);
    }else{
        printf("Too many inputs.Exiting...");
        exit(1);
    }
}
