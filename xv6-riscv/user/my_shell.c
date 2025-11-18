// user.c -- student-style, mildly messy shell but correct
// Written in a hurry after many failed runs. Sorry for the tabs and spaces mix.
// This file uses unique function names requested in the prompt:
// trim -> trimspace_eater
// parse_comm -> get_my_tokens
// getcmmd -> read_user_line
// run_comm -> recursively_run_it
// main -> shell_startup

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

// limits
#define MAXARGS 20
#define MAXCMD 256

// prototypes with the exact unique names requested
char* trimspace_eater(char *s);
int   get_my_tokens(char *cmd, char *argv[MAXARGS]);   // tokeniser
int   read_user_line(char *buf);                       // gets line
void  recursively_run_it(char *cmdline);               // core recursive runner
int   shell_startup(void);                             // main-equivalent

// -------------------------------------------------------------
// trimspace_eater - remove leading/trailing whitespace
// slightly verbose comment because I spent ages debugging this.
// returns pointer to first non-space inside s
// -------------------------------------------------------------
char*
trimspace_eater(char *s)
{
    if (!s) return s;
    char *p = s;
    // skip leading whitespace (space, tab, newline, cr)
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    if (*p == 0) return p;
    char *end = p + strlen(p) - 1;
    while (end >= p && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        *end = 0;
        end--;
    }
    return p;
}

// -----------------------------------------------------------------
// get_my_tokens - tokeniser that tolerates many spaces and special chars
// Slightly redundant calls to trimspace_eater intentionally inserted
// because of the "student wrote it after many tests" vibe requested.
// -----------------------------------------------------------------
int
get_my_tokens(char *cmd, char *argv[MAXARGS])
{
    int argc = 0;
    char *p = cmd;

    if (!p) return 0;

    // redundant trimming - yep I called it twice just to be safe
    p = trimspace_eater(p);
    p = trimspace_eater(p);

    // skip leading whitespace robustly
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;

    while (*p && argc < MAXARGS - 1) {
        // start of token
        argv[argc++] = p;

        // advance until whitespace or end
        while (*p && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r') {
            p++;
        }

        if (*p) {
            *p = 0;     // terminate token
            p++;        // move to next possible token
        }

        // skip internal multiple whitespace
        while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
        // extra redundant trim (student habit)
        p = trimspace_eater(p);
    }

    argv[argc] = 0;
    return argc;
}

// -----------------------------------------------------------------
// read_user_line - wrapper around gets to read a full line
// uses MAXCMD size. prints a prompt then reads.
// -----------------------------------------------------------------
int
read_user_line(char *buf)
{
    fprintf(2, ">>> ");    // prompt like before
    if (gets(buf, MAXCMD) == 0)   // use provided buffer length
        return 0;
    trimspace_eater(buf); // sanitize, again (redundant but safe)
    if (buf[0] == 0) return 0;
    return 1;
}

// -----------------------------------------------------------------
// recursively_run_it - core: Sequence (;) first, Pipe (|) second, Redirs last
// Important: For pipes we scan right-to-left (strrchr) to ensure left-associativity
// (i.e., split at last pipe so the left side can itself contain pipes)
// -----------------------------------------------------------------
void
recursively_run_it(char *cmdline)
{
    if (!cmdline) return;

    // Trim the incoming string (I like starting clean)
    cmdline = trimspace_eater(cmdline);
    if (cmdline[0] == 0) return;

    // ---------- 1) Sequence ';' - lowest precedence (left-to-right) ----------
    // find first ';' and split there, executing left then right
    char *semi = strchr(cmdline, ';');
    if (semi) {
        *semi = 0;
        char *left = trimspace_eater(cmdline);
        char *right = trimspace_eater(semi + 1);
        // I did get bitten by extra zeros here before; do left then right
        recursively_run_it(left);
        recursively_run_it(right);
        return;
    }

    // ---------- 2) Pipes '|' - next precedence.
    // Requirement: scan right-to-left (split at last '|') so left assoc works.
    char *last_pipe = strrchr(cmdline, '|');
    if (last_pipe) {
        // split at the last pipe: left part may contain other pipes
        *last_pipe = 0;
        char *left_cmd = trimspace_eater(cmdline);
        char *right_cmd = trimspace_eater(last_pipe + 1);

        int pfd[2];
        if (pipe(pfd) < 0) {
            fprintf(2, "pipe failed\n");
            return;
        }

        int pid = fork();
        if (pid == 0) {
            // left child: write into pipe
            close(1);
            dup(pfd[1]);
            close(pfd[0]);
            close(pfd[1]);
            recursively_run_it(left_cmd); // left might be a compound thing
            exit(0);
        }

        int pid2 = fork();
        if (pid2 == 0) {
            // right child: read from pipe
            close(0);
            dup(pfd[0]);
            close(pfd[0]);
            close(pfd[1]);
            recursively_run_it(right_cmd);
            exit(0);
        }

        // parent closes and waits
        close(pfd[0]);
        close(pfd[1]);
        wait(0);
        wait(0);
        return;
    }

    // ---------- 3) Redirection and simple execution (highest precedence locally)
    // We need to extract filenames for < and > even with mixed whitespace.
    // We'll scan the line and pull filenames out carefully, null-terminating where needed.

    char *input_file_*
