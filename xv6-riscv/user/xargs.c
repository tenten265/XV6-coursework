#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"
void execute_line(char *line, char *args[], int skip, int max_append) {
    // Parse all words from line into words[]
    char *words[MAXARG];
    int nw = 0;
    char *s = line;
    int ws = 1;
    while (*s) {
        if (*s == ' ' || *s == '\t') {
            *s = '\0';
            ws = 1;
        } else {
            if (ws) {
                if (nw < MAXARG - 1)
                    words[nw++] = s;
                ws = 0;
            }
        }
        s++;
    }
    words[nw] = 0;

    int idx = 0;
    while (idx < nw) {
        int start = idx;
        // determine how many to use in this invocation
        int can_use = nw - start;
        if (max_append > 0 && can_use > max_append)
            can_use = max_append;

        int f = fork();
        if (f < 0) {
            fprintf(2, "xargs: fork failed\n");
            exit(1);
        } else if (f == 0) { // child
            char **a = &args[skip];
            int used = 0;
            for (int j = start; j < start + can_use; j++) {
                if ((a - args) < (MAXARG - 1)) {
                    *a = words[j];
                    a++;
                    used++;
                } else {
                    break;
                }
            }
            *a = 0;
            if (args[0] == 0) // nothing to exec
                exit(0);
            exec(args[0], args);
            // if exec returns, it's an error
            fprintf(2, "xargs: exec %s failed\n", args[0]);
            exit(1);
        } else { // parent
            wait((int *)0);
            // advance idx by the number used (same logic as child)
            idx += can_use;
        }
    }
}

int main(int argc, char *argv[]) {
    char *args[MAXARG] = { 0 };
    char buf[512], *b;

    // Simple option parsing: support "-n N"
    int max_append = -1; // -1 == unlimited
    int ai = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            // parse decimal N (basic)
            int val = 0;
            char *p = argv[i+1];
            while (*p >= '0' && *p <= '9') {
                val = val * 10 + (*p - '0');
                p++;
            }
            if (val > 0) max_append = val;
            i++; // skip the number token
        } else {
            // treat as initial arg for the command
            if (ai < MAXARG - 1) {
                args[ai++] = argv[i];
            }
        }
    }
    b = buf;

    // Read each line of stdin into the buffer and execute
    // the command given as args. This routine replaces
    // newlines with nulls to terminate the string.
    while (read(0, b, 1) > 0) {
        if (*b == '\n') {
            *b = '\0';
            execute_line(buf, args, ai, max_append);
            b = buf;
        } else {
            b++;
        }
    }
    exit(0);
}