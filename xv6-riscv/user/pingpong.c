#include "kernel/types.h"
#include "user/user.h"

int main(){
    int p2c[2];
    int c2p[2];

    pipe(p2c);
    pipe(c2p);

    char our_byte = 'P';

    int pid = fork();
    if (pid > 0){
        write(p2c[1], &our_byte, c2p[1]);
        printf("%d:Recieved ping pong%c", getpid(), our_byte);
        read(c2p[0], &our_byte, 1);
        
        close(p2c[1]);
        close(c2p[0]);
        wait(0);
    }
    else if (pid == 0){
        read(p2c[0], &our_byte, 1);
        char our_byte = 'R';
        printf("%d:Received ping pong%c", getpid(), our_byte);

        close(p2c[0]);
        close(c2p[1]);
        
        write(c2p[1] , &our_byte, 1);

    }
    exit(0);

}