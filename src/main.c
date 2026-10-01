#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "builtins.h"

#define MAX_ARGS 128
#define INPUT_SIZE 4096

static char *trim(char *s) {
    while (isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) *end-- = '\0';
    return s;
}

static int split_args(char *line, char *argv[], size_t max_args) {
    size_t argc = 0;
    char *p = line;
    while (*p != '\0' && argc + 1 < max_args) {
        while (isspace((unsigned char)*p)) p++;
        if (*p == '\0') break;
        char *start = p;
        if (*p == '\'' || *p == '"') {
            char quote = *p++;
            start = p;
            while (*p != '\0' && *p != quote) p++;
            if (*p == quote) { *p = '\0'; p++; }
        } else {
            while (*p != '\0' && !isspace((unsigned char)*p)) p++;
            if (*p != '\0') { *p = '\0'; p++; }
        }
        argv[argc++] = start;
    }
    argv[argc] = NULL;
    return (int)argc;
}

static void expand_home(const char *path, char *out, size_t out_size) {
    if (path[0] == '~' && (path[1] == '/' || path[1] == '\0')) {
        const char *home = getenv("HOME");
        if (home) { snprintf(out, out_size, "%s%s", home, path + 1); return; }
    }
    snprintf(out, out_size, "%s", path);
}

static int run_external(char *argv[]) {
    pid_t pid = fork();
    if (pid < 0) { perror("carsonsh: fork"); return 1; }
    if (pid == 0) {
        execvp(argv[0], argv);
        fprintf(stderr, "carsonsh: %s: %s\n", argv[0], strerror(errno));
        _exit(127);
    }
    int status;
    while (waitpid(pid, &status, 0) < 0) {
        if (errno == EINTR) continue;
        perror("carsonsh: waitpid");
        return 1;
    }
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
    return 1;
}

int main(void) {
    char input[INPUT_SIZE];
    char *argv[MAX_ARGS];
    int last_status = 0;

    printf("CarsonSH 0.1\nType 'exit' to leave.\n");

    while (1) {
        char cwd[PATH_MAX];
        if (!getcwd(cwd, sizeof(cwd))) snprintf(cwd, sizeof(cwd), "?");
        printf("carsonsh:%s$ ", cwd);
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) { putchar('\n'); break; }
        char *line = trim(input);
        if (*line == '\0') continue;

        int argc = split_args(line, argv, MAX_ARGS);
        if (argc == 0) continue;

        if (strcmp(argv[0], "exit") == 0)
            return argc > 1 ? atoi(argv[1]) : last_status;

        if (strcmp(argv[0], "cd") == 0) {
            char expanded[PATH_MAX];
            const char *target = argc > 1 ? argv[1] : getenv("HOME");
            if (!target) {
                fprintf(stderr, "carsonsh: cd: HOME is not set\n");
                last_status = 1;
                continue;
            }
            expand_home(target, expanded, sizeof(expanded));
            if (chdir(expanded) != 0) {
                fprintf(stderr, "carsonsh: cd: %s: %s\n", expanded, strerror(errno));
                last_status = 1;
            } else last_status = 0;
            continue;
        }

        if (strcmp(argv[0], "pwd") == 0) {
            char cwd[PATH_MAX];
            if (!getcwd(cwd, sizeof(cwd))) { perror("carsonsh: pwd"); last_status = 1; }
            else { puts(cwd); last_status = 0; }
            continue;
        }

        if (strcmp(argv[0], "echo") == 0) {
            for (int i = 1; i < argc; i++) {
                if (i > 1) putchar(' ');
                fputs(argv[i], stdout);
            }
            putchar('\n');
            last_status = 0;
            continue;
        }

        last_status = run_external(argv);
    }
    return last_status;
}
