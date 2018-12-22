/* ----------------------------------------------------------------------------
 * File: shell.c
 * 
 * Name: Gregory Norton
 * 
 * Description:  This program is a simple command line shell. This shell
 * assumes that the user will input a command that is less than 80 chars long.
 * Additionally, it is assumed that the user enters no more than 10 args.
 * To exit the shell, enter "exit". To purposely cause a segmentation fault,
 * enter "segfault".
 * After 30 seconds of inactivity, the shell will receive the SIGALRM signal
 * and the current session will expire, eciting with code 3.
 * When a SIGSEGV (segfault) signal is detected, the program will exit with
 * error code 2.
 * When a SIGINT (interruption) signal is detected, the program will not exit,
 * instead it will continue execution.
 * Syntax: ./shell
 
 *///--------------------------------------------------------------------------

/*References:
 * N/A
 *///--------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h> //used for strcmp())
#include <signal.h> //usef for signal handling

#define INPUT_SIZE 80
#define SOURCE_ARG 1
#define DEST_ARG 2
#define IDENTICAL 0
#define MAX_ARGS 10
#define EXACT_MATCH 0
#define EXIT_ON_ALARM 3
#define EXIT_ON_SEGFAULT 2
#define ALARM_SET_TIME 30
#define ALLOWED_ARGS 1

void sig_handler(int signal); //prototype for the signal handler

int main(int argc, char *argv[])
{
        char input[INPUT_SIZE]; //user can enter a string of up to 80 chars
        int continue_prompting = 1;
        int len;
        char *arguments[MAX_ARGS]; //create an array of strings to hold the max number of allowed args
        int pid;
        int status;
        struct sigaction act; //used to handle signals
        
        
        if (argc > ALLOWED_ARGS) {
                fprintf(stderr, "no arguments are allowed. Use ./shell\n");
                exit(EXIT_FAILURE);
        }
        
        //set an alarm signal to be scheduled for 30 seconds
        alarm(ALARM_SET_TIME);
        while(continue_prompting == 1) {
                
                //alarm handling - will handle
                //set sa_handler to our sig_handler function defined below main
                act.sa_handler = sig_handler;

                //prepare sa_mask
                sigemptyset(&act.sa_mask);

                //since we can handle 3 different signals, restart
                //if interrupted by handler
                act.sa_flags = SA_RESTART;
                
                errno = 0;
               
                //SIGALRM allows us to receive alarm signals
                sigaction(SIGALRM, &act, NULL);
                
                //SIGSEGV allows us to receive segfault detection signals
                sigaction(SIGSEGV, &act, NULL);
                
                //SIGINT allows us to receive interrupt signals
                sigaction(SIGINT, &act, NULL);
                
                if (errno){
                        perror("Unexpected error in sigaction");
                }
                
                fprintf(stdout, "prompt>");
                
                //get the command from the user
                fgets(input, INPUT_SIZE, stdin);
                
                //let's get the input length
                len = strlen(input); 
                
                //let's place a null terminator after the
                //end of the user input, so we don't get
                //the carriage return 
                input[len-1] = '\0';
                
                //we will tokenize the user's input, using a space as the 
                //delimiter
                arguments[0] = strtok(input, " ");
                                
                //there are two special cases where we will not call execvp
                //if the user enters exit or segfault
                
                //we will only check if there was a command given
                if (arguments[0] != NULL){
                        //we'll reset the alarm if given a command
                        alarm(ALARM_SET_TIME); 
                        
                        //detect exit command
                        if (strcmp(arguments[0], "exit") == EXACT_MATCH){
                                //printf("%s", "exit detected");
                                exit(EXIT_SUCCESS);
                        }

                        //detect segfault command
                        if (strcmp(arguments[0], "segfault") == EXACT_MATCH){
                                int *bomb = NULL;
                                *bomb = 42;
                        }
                }
                
                //we will go through the the max number of possible args
                //and tokenize the input, assigning it to the corresponding
                //slot in the arguments array. Once we detect NULL, we are 
                //done processing the arguments list, and we can break.                
                for (int i = 1; i < MAX_ARGS; i++){
                        
                        arguments[i] = strtok(NULL, " ");
                        if (arguments[i] == NULL) {
                                break;
                        }
                }
                arguments[MAX_ARGS] = '\0';
                
                //here we will fork a new process, in order to execute
                //the program specified by the user.
                if (arguments[0] != NULL){                     
                        pid = fork();
                        if (pid == 0){
                                //the child, which will call exec on the user's command
                                //and arguments if any
                                errno = 0;
                                status = execvp(arguments[0], arguments);
                                if (errno){
                                        perror("exec failed");
                                        exit(EXIT_FAILURE);
                                } else {
                                        exit(EXIT_SUCCESS);
                                }
                                

                        } else if (pid > 0){
                                //we will continue to wait for the child
                                //until the child exits
                                 pid = waitpid(pid, &status, 0);
                                /*if (WIFEXITED(status)) {
                                        printf("Parent: the child has terminated. "
                                       "Result was %d\n",
                                       WEXITSTATUS(status));
                                }*/
                        } else if (pid < 0){
                                //we will exit with an error, because
                                //fork was unable to create a child process
                                fprintf(stderr, "Unable to fork.\n"
                                        "Exiting...\n");
                                exit(EXIT_FAILURE);
                        }
                        
                }
                
        }
        
        
        return EXIT_SUCCESS;
}

void sig_handler(int signal) {
        
        if (signal == SIGALRM ) {
                fprintf(stdout, "\nThe session has expired.\n"
                        "Exiting...\n");
                 exit(EXIT_ON_ALARM);
                 
        } else if (signal == SIGSEGV){
                fprintf(stderr, "\nA segmentation fault has been detected.\n"
                        "Exiting... \n");

                exit(EXIT_ON_SEGFAULT);
        
        } else if (signal == SIGINT) {
                fprintf(stdout, "\nGood try...I don’t die that easily.\n"
                        "Enter 'exit' at the prompt to terminate this shell.\n"
                        "prompt>");
                fflush(stdout); //make sure we show the prompt
                return;
        
        } else {
                fprintf(stderr, "Invalid signal passed by sigaction\n"
                        "Exiting...\n");
                exit(EXIT_FAILURE);
        }
        
        
        
        
        
       
        
}
