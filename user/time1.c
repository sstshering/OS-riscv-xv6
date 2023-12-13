#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {

        // for-loop to go through the arguments in the command line
        for (int i = 0; i < argc; i++){

            int pid = fork(); // grabing the pid number

            // This if statement executes if it a child
            if(pid == 0){

                // passing the program from the command line
                exec(argv[1], &argv[1]);

            }
            
            // This if statement executes when the child is done go into the parent
            if(pid > 0){

                // wait time for the child finishing
                wait(&argc);
                int time = uptime();
                printf("elpased time: %d ticks\n", time);

            }
            
            // This if statement occurs neither child or parent execute
            if(pid < 0){
                printf("Something went wrong :(\n");
            }
        }
        
   //exit(0);
    return 0;

}