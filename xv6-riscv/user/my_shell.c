#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAXARGS 20
#define MAXCMD 256

int getcmmd(char *buff, int n){
    fprintf(2, ">>>");
    if(gets(buff, n) == 0)
        return 0;

    // if the command is empty, return 0
    trim(buff);
    if(buff[0] == 0) 
        return 0;
    return 1;
}
//
char* trim(char* st){
    char *end;

    while(*st == ' ' || *st == '\t' || *st == '\r' || *st == '\n')
        st++;

    if (*st == 0) 
        return st;

    end = st;
    while(end[1]) end++;

    while(end > st && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n')){
        *end = 0;
        end--;
    }

    return st;
}
//


void run_comm(char* cmd){
    char* argv[MAXARGS];
    int argc = 0;
    char* p = cmd;

    if (cmd == 0)
        return;

    cmd = trim(cmd);

    p = cmd;
    while (*p && argc < MAXARGS - 1){
        // skip separators
        while(*p && (*p == ' ' || *p == '\t' || *p == '\n'))
            *p++ = 0;

        if (*p == 0)
            break;

        argv[argc++] = trim(p);

        while(*p && !(*p == ' ' || *p == '\t' || *p == '\n'))
            p++;
    }

    argv[argc] = 0;

    if (argc == 0)
        return;



    if (strcmp(argv[0], "cd") == 0) {
        if (argc < 2){
            fprintf(2, "cd: missing argument\n");
        } 
        char * dir = trim(argv[1]);
        if (chdir(trim(argv[1])) < 0){
            fprintf(2, "cd: no such file or directory: %s\n", dir);
        }
        return;
    }

    //Part 2
    for (int i = 0; cmd[i]; i++){
        if(cmd[i] == '>'){
             cmd[i] = 0;
            int fd = open(trim(&cmd[i+1]), O_CREATE | O_WRONLY);
            if(fd < 0){fprintf(2, "open file error\n"); trim(&cmd[i+1]); return; }
            close(1); // close stdout
            dup(fd);  // dup fd to stdout
            close(fd);
            break;
        }
        else if(cmd[i] == '<'){
            cmd[i] = 0;
            char* filename =trim(&cmd[i+1]);
            int fd = open(filename, O_RDONLY);
            if(fd < 0){fprintf(2, "open file error\n"); filename; return; }
            close(0); // close stdin
            dup(fd);  // dup fd to stdin
            close(fd);
            break;
        }
           
    }

    //part 3
    for (int m = 0; cmd[m]; m++){
        if(cmd[m] == '|'){
            cmd[m] = 0;
            int pfd[2];
            pipe(pfd);

            int pid = fork();
            if (pid == 0){
                close(1); // close stdout
                dup(pfd[1]); // dup write end to stdout
                close(pfd[0]);
                close(pfd[1]);
                run_comm(trim(cmd));
                exit(0);
            } 

            int pid2 = fork();
            if(pid2 == 0){
                close(0); // close stdin
                dup(pfd[0]); // dup read end to stdin
                close(pfd[0]);
                close(pfd[1]);
                run_comm(trim(&cmd[m+1]));
                exit(0);
            }

            close(pfd[0]);
            close(pfd[1]);
            wait(0);
            wait(0);
            return;
        }
    }

    for (int h = 0; cmd[h]; h++){
        if(cmd[h] == ';'){
            cmd[h] = 0;
            run_comm(trim(cmd));
            run_comm(trim(&cmd[h+1]));
            return;
        }
    }

    int pid = fork();
    if (pid < 0){
        fprintf(2, "fork error\n");
        return;
    }

    if(pid == 0){
        exec(argv[0], argv);
        fprintf(2, "exec %s failed\n", argv[0]);
        exit(1);
    } 

    wait(0);



}

int main(void){
    static char buff[MAXCMD];
    while(1){
        if (getcmmd(buff, sizeof(buff)) == 0){
            exit(0);
        }
        run_comm(buff);
    }
}






