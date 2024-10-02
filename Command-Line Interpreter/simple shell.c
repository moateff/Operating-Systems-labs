#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<sys/resource.h>

#define MAX_SIZE 100
int count = 0, shell_bultin_flag = 0;
int last_token;

char input[MAX_SIZE];
char *tokens[MAX_SIZE];

void read_input(void){
    char arr[MAX_SIZE];
    char *current_dir = getcwd(arr, MAX_SIZE);
    if(!strcmp(current_dir, "/home/ubuntu"))
        printf("~$ ");
    else
        printf("%s$ ",current_dir);
    scanf("%[^\n]%*c", input);
}

void parse_input(void){
    char *token = strtok(input, " ");
    if(!strcmp(token , "export")){
        while (token != NULL && count < MAX_SIZE)
        {
            tokens[count++] = token;
            token = strtok(NULL, "=");
        }  
        tokens[count] = '\0';
        count = 0;
        shell_bultin_flag = 1;
    }
    else
    {
        if((!strcmp(token, "cd"))|(!strcmp(token, "echo"))|(!strcmp(token, "pwd"))|(!strcmp(token, "exit")))
            shell_bultin_flag = 1;
        while (token != NULL && count < MAX_SIZE)
        {
            tokens[count++] = token;
            token = strtok(NULL, " ");
        }  
        tokens[count] = '\0';
        last_token = count - 1;
        count = 0;
    }
}

void excute_cd(void)
{
    if(!(strcmp(tokens[1],"~")))
    {
        chdir("/home/ubuntu/");
    }
    else
    {
        int flag = 0;
        flag = chdir(tokens[1]); 
        if(flag != 0)
            printf("Error, the directory is not found\n");
    }
}

void excute_export(void)
{
    char* data = tokens[2];
    if(data[0] == '"')
    {
        data++;
        data[strlen(data)-1] = '\0';
        setenv(tokens[1], data, 1);
    }
    else
        setenv(tokens[1], tokens[2], 1);
}

void excute_echo(void)
{
    for(int i = 1; i <= last_token; i++)
    {
        char* str = tokens[i];
        if(str[0] == '"')
            str++;
        if(str[strlen(str)-1] == '"')
            str[strlen(str)- 1] = '\0';
        if(str[0] == '$')
        {
            str++;
            printf("%s",getenv(str));
        }
        else
            printf("%s",str);
        printf(" ");
    }
    printf("\n");
}

void excute_pwd(void)
{
    printf("%s\n",getcwd(NULL,0));
}

void execute_shell_bultin(void)
{
    if(!strcmp(tokens[0], "exit")) exit(0);
    else if(!strcmp(tokens[0], "cd")) excute_cd();
    else if(!strcmp(tokens[0], "export")) excute_export();
    else if(!strcmp(tokens[0], "echo")) excute_echo();
    else if(!strcmp(tokens[0], "pwd")) excute_pwd();
    shell_bultin_flag = 0;
}

void execute_command (void)
{
    int error_flag = 1;
    int pid = fork();
    if(pid < 0)
    {
        printf("System Error!\n");
        exit(0);
    }
    else if (!pid)
    {
        for(int i = 0, j = 0; i <= last_token; i++)
        {
            char* env = tokens[i];
            if(env[0] == '$')
            {
                char* ptr_env;
                env++;
                ptr_env = getenv(env);
                char* token = strtok(ptr_env, " ");
                while(token != NULL)
                {
                    tokens[j++] = token;    
                    token = strtok(NULL, " ");
                }
            }
            else
                j++;
        }
        error_flag = execvp(tokens[0], tokens);
        if(error_flag)
            printf("Error! unknown command\n");
    }
    else
    {
        if(!strcmp(tokens[last_token], "&"))
            return;
        else
            waitpid(pid, NULL, 0);
    }
}

void shell(){
    do
    {
        read_input();
        parse_input();
        if(shell_bultin_flag)
            execute_shell_bultin();
        else
            execute_command();
    } while(1);
}

void write_to_log_file(void){
    FILE *file = fopen("log.text", "append");
    if(file == NULL)
    {
        printf("Error in file\n");
        exit(0);
    }
    else
    {
        fprintf(file, "%s", "Child process terminated\n");
        fclose(file);
    }
}

void reap_child_zombie(void){
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
        printf("Child process %d terminated\n", pid);
    if (pid == -1)
        return;
}

void on_child_exit(void){
    reap_child_zombie();
    write_to_log_file();
}

void setup_environment(void){
    chdir("/home/ubuntu/");
}

void register_child_signal(void){
    signal(SIGCHLD, on_child_exit);
}

int main(){
   register_child_signal();
   setup_environment();
   shell();
   return 0;
}
