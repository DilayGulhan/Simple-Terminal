#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

#define SIZE 1024
#define MAX_HISTORY_SIZE 10

int run_by_type(char *command, int history_count, char **command_history);

volatile sig_atomic_t background_pid = 0;

void handle_signal(int sig)
{
    if (background_pid != 0 && (sig == SIGINT || sig == SIGQUIT))
    {
        
        kill(background_pid, sig);
    }
    else
    {
        
        signal(sig, SIG_DFL);
        raise(sig);
    }
}
void change_directory(const char *path)
{
    if (path == NULL)
    {
        chdir(getenv("HOME"));
    }
    else
    {
        if (chdir(path) != 0)
        {
            fprintf(stderr, "chdir error: %s\n", strerror(errno));
        }
    }
}

void print_working_directory()
{
    char cwd[SIZE];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        printf(" %s\n", cwd);
    }
    else
    {
        perror("getcwd");
    }
}

void add_to_history(const char *new_command, char **command_array, int *count)
{
    if (*count < MAX_HISTORY_SIZE)
    {
        command_array[*count] = strdup(new_command);
        (*count)++;
    }
    else
    {
        free(command_array[0]);
        for (int i = 0; i < MAX_HISTORY_SIZE - 1; i++)
        {
            command_array[i] = command_array[i + 1];
        }
        command_array[MAX_HISTORY_SIZE - 1] = strdup(new_command);
    }
}

int other_commands(char **command)
{
    pid_t child_pid;
    child_pid = fork();
    if (child_pid == 0)
    {
        
      
        if (execvp(command[0], command  ) ==-1 )
        {    perror("execvp failed");
            return 0 ; 
           
            exit(EXIT_FAILURE);
            
            
        }
        else {return 1;}
        
    
    }
    else if (child_pid < 0)
    {
        perror("Couldn't fork ");
    }
    else
    {
        wait(NULL);
        return 1 ; 
        
    }
}

void string_to_command_array(const char *str, char **command_array)
{
    char *token;
    int i = 0;
    token = strtok((char *)str, " ");

    while (token != NULL)
    {
        command_array[i] = token;
        token = strtok(NULL, " ");
        i++;
    }

    command_array[i] = NULL;
}

void parse_if_pipe(char *command, char *tokens[2])
{
    char *token;
    char delimiters[] = "|";

    token = strtok(command, delimiters);
    if (token != NULL)
    {
        tokens[0] = strdup(token);
        token = strtok(NULL, delimiters);
        if (token != NULL)
        {
            tokens[1] = strdup(token);
        }
    }
}

void exec_as_pipe(char *tokens[], int history_count, char **command_history)
{
    int fd[2];
    if (pipe(fd) == -1)
    {
        perror("Pipe Failed");
        exit(EXIT_FAILURE);
    }

    pid_t p1 = fork();
    if (p1 == -1)
    {
        perror("Fork Failed");
        exit(EXIT_FAILURE);
    }
    else if (p1 == 0)
    {

        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);

        run_by_type(tokens[0], history_count, command_history);

        exit(EXIT_SUCCESS);
    }

    pid_t p2 = fork();
    if (p2 == -1)
    {
        perror("Fork Failed");
        exit(EXIT_FAILURE);
    }
    else if (p2 == 0)
    {

        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);

        run_by_type(tokens[1], history_count, command_history);

        exit(EXIT_SUCCESS);
    }

    close(fd[0]);
    close(fd[1]);

    waitpid(p1, NULL, 0);
    waitpid(p2, NULL, 0);
}
int run_by_type(char *command, int history_count, char **command_history)
{
    char *cmd3[10];
    if (strstr(command, "cd") != NULL)
    {
        char *path = strchr(command, ' ');
        if (path != NULL)
        {
            change_directory(path + 1);
        }
        else
        {
            change_directory(NULL);
        }
        return 1;
    }
    else if (strstr(command, "pwd") != NULL)
    {
        print_working_directory();
        return 1;
    }
    else if (strstr(command, "history") != NULL)
    {
        for (int i = 0; i < history_count; i++)
        {
            printf("%s\n", command_history[i]);
        }
        return 1;
    }
    else if (strstr(command, "exit") != NULL)
    {
        printf("Process terminated\n");
        exit(EXIT_SUCCESS);
    }
    else
    {
        int id ; 
        string_to_command_array(command, cmd3);


        if (other_commands(cmd3))
        {
            add_to_history(command, command_history, &history_count);
            return 1;
        }
        else
        {   
            return 0;
        }
    }
}

void parse_and_execute(char *command, int history_count, char **command_history)
{
    char *cmd1 = strtok(command, "&&");
    char *cmd2 = strtok(NULL, "&&");
    
    int cmd1_succeeded;
    if (cmd1 != NULL && cmd2 != NULL)
    {

        cmd1_succeeded = run_by_type(cmd1, history_count, command_history);
        
        if (cmd1_succeeded == 0)
        {
            printf("Command '%s' failed\n", cmd1);
           
        }
        else
        {
        
           run_by_type(cmd2, history_count, command_history);
           
        }
    }
}

int main()
{
    char *command_history[MAX_HISTORY_SIZE] = {NULL};
    int history_count = 0;
    

    char *tokening[2] = {NULL};
    char command[SIZE];

    signal(SIGINT, handle_signal);
    signal(SIGQUIT, handle_signal);

    while (1)
    {
        printf("> My Terminal ");
        fflush(stdout);

        if (fgets(command, SIZE, stdin) == NULL)
        {
            perror("Input error");
            break;
        }

        command[strcspn(command, "\n")] = '\0'; 
        if (strlen(command) == 0)
            continue;

        
        int background = (command[strlen(command) - 1] == '&');
        if (background)
        {
            command[strlen(command) - 1] = '\0'; 
        }

        add_to_history(command, command_history, &history_count);

        if (strcmp(command, "exit") == 0)
        {
            printf("Process terminated\n");
            break;
        }

        if (strstr(command, "|") != NULL)
        {
            parse_if_pipe(command, tokening);
            exec_as_pipe(tokening, history_count, command_history);
        }
        else if (strstr(command, "&&") != NULL)
        {
            parse_and_execute(command, history_count, command_history);
        }
        else
        {
            if (background)
            {
                pid_t pid = fork();
                if (pid == 0)
                {
                    
                    run_by_type(command, history_count, command_history);
                    exit(EXIT_SUCCESS);
                }
                else if (pid > 0)
                {
                    
                    printf("Background process %d\n", pid);
                    background_pid = pid;
                }
                else
                {
                    perror("Fork failed");
                }
            }
            else
            {
                run_by_type(command, history_count, command_history);
            }
        }
    }

    for (int i = 0; i < history_count; i++)
    {
        free(command_history[i]);
    }
    return 0;
}
