#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]){

    if (argc < 2){
        printf("Error, no argument ");
        exit(1);
    }
    
    int i = atoi(argv[1]);
    printf("The argument is: %d\n", i);

    pause(i);
    printf("Woke up after sleeping for %d ticks!\n", i);
exit(0);
}