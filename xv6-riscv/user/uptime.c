#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]){

    int ticks = uptime(); 

    printf("Uptime: %d ticks\n", ticks);

    exit(0);


exit(0);
}