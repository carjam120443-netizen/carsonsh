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
#define MAX_ALIASES 64
#define MAX_HISTORY 1000

#define GREEN "\033[1;32m"
#define CYAN "\033[1;36m"
#define RESET "\033[0m"

struct alias_entry {
    char name[64];
    char value[256];
};

static struct alias_entry aliases[MAX_ALIASES];
static size_t alias_count = 0;
static char history_entries[MAX_HISTORY][INPUT_SIZE];
static size_t history_count = 0;

static int use_color(void) {
    return isatty(STDOUT_FILENO);
}

static char *trim(char *s) {
    while (isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) *end-- = '\0';
    return s;
}

static int set_alias(const char *name, const char *value) {
    for (size_t i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].name, name) == 0) {
            snprintf(aliases[i].value, sizeof(aliases[i].value), "%s", value);
            return 0;
        }
    }
    if (alias_count >= MAX_ALIASES) return 1;
    snprintf(aliases[alias_count].name, sizeof(aliases[alias_count].name), "%s", name);
    snprintf(aliases[alias_count].value, sizeof(aliases[alias_count].value), "%s", value);
    alias_count++;
    return 0;
}

static const char *find_alias(const char *name) {
    for (size_t i = 0; i < alias_count; i++)
        if (strcmp(aliases[i].name, name) == 0) return aliases[i].value;
    return NULL;
}

static void load_config(void) {
    const char *home = getenv("HOME");
    if (!home) return;

    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/.config/carsonsh/config", home);
    FILE *fp = fopen(path, "r");
    if (!fp) return;

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        char *s = trim(line);
        if (*s == '\0' || *s == '#') continue;
        if (strncmp(s, "alias ", 6) == 0) {
            s += 6;
            char *eq = strchr(s, '=');
            if (!eq) continue;
            *eq = '\0';
            char *name = trim(s);
            char *value = trim(eq + 1);
            if (*name && *value) set_alias(name, value);
        }
    }
    fclose(fp);
}

static void load_history(void) {
    const char *home = getenv("HOME");
    if (!home) return;

    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/.carsonsh_history", home);
    FILE *fp = fopen(path, "r");
    if (!fp) return;

    char line[INPUT_SIZE];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (*line == '\0') continue;

        if (history_count >= MAX_HISTORY) {
            memmove(history_entries, history_entries + 1,
                    (MAX_HISTORY - 1) * sizeof(history_entries[0]));
            history_count = MAX_HISTORY - 1;
        }
        snprintf(history_entries[history_count], sizeof(history_entries[0]), "%s", line);
        history_count++;
    }
    fclose(fp);
}

static void save_history_entry(const char *line) {
    const char *home = getenv("HOME");
    if (!home || !line || !*line) return;

    /* Match the useful dotfile behavior of ignoring consecutive duplicates. */
    if (history_count > 0 && strcmp(history_entries[history_count - 1], line) == 0)
        return;

    if (history_count >= MAX_HISTORY) {
        memmove(history_entries, history_entries + 1,
                (MAX_HISTORY - 1) * sizeof(history_entries[0]));
        history_count = MAX_HISTORY - 1;
    }
    snprintf(history_entries[history_count], sizeof(history_entries[0]), "%s", line);
    history_count++;

    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/.carsonsh_history", home);
    FILE *fp = fopen(path, "a");
    if (!fp) return;
    fprintf(fp, "%s\n", line);
    fclose(fp);
}

static void print_history(size_t limit) {
    size_t start = history_count > limit ? history_count - limit : 0;
    for (size_t i = start; i < history_count; i++)
        printf("%zu  %s\n", i + 1, history_entries[i]);
}

static void print_banner(void) {
    if (use_color()) printf(GREEN);
    printf("  ____                           ____  _   _\n");
    printf(" / ___|__ _ _ __ ___  ___  ___ / ___|| | | |\n");
    printf("| |   / _' | '__/ __|/ _ \\/ __|\\___ \\| |_| |\n");
    printf("| |__| (_| | |  \\__ \\  __/ (__  ___) | |_| |\n");
    printf(" \\____\\__,_|_|  |___/\\___|\\___||____/ \\___/\n");
    printf("CarsonSH 0.1 - Oh-My-Shell inspired mode\n");
    printf("Type 'help' for CarsonSH commands.\n");
    if (use_color()) printf(RESET);
}

static void print_git_branch(void) {
    FILE *fp = popen("git rev-parse --abbrev-ref HEAD 2>/dev/null", "r");
    if (!fp) return;
    char branch[128];
    if (fgets(branch, sizeof(branch), fp)) {
        branch[strcspn(branch, "\r\n")] = '\0';
        if (*branch) {
            if (use_color()) printf(CYAN);
            printf(" (%s)", branch);
            if (use_color()) printf(RESET);
        }
    }
    pclose(fp);
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

static void print_help(void) {
    puts("CarsonSH built-ins:");
    puts("  cd [DIR]          Change directory");
    puts("  pwd               Print current directory");
    puts("  echo [TEXT]       Print text");
    puts("  alias [N=VALUE]   List or define an alias");
    puts("  unalias NAME      Remove an alias");
    puts("  history [N]       Show recent command history");
    puts("  reload            Reload CarsonSH configuration");
    puts("  help              Show this help");
    puts("  exit [STATUS]     Exit CarsonSH");
}

int main(void) {
    char input[INPUT_SIZE];
    char expanded_input[INPUT_SIZE];
    char *argv[MAX_ARGS];
    int last_status = 0;

    load_config();
    load_history();
    print_banner();

    while (1) {
        char cwd[PATH_MAX];
        if (!getcwd(cwd, sizeof(cwd))) snprintf(cwd, sizeof(cwd), "?");

        if (use_color()) printf(GREEN);
        printf("┌──(carson㉿carsonsh)-[%s]", cwd);
        if (use_color()) printf(RESET);
        print_git_branch();
        putchar('\n');
        if (use_color()) printf(GREEN);
        printf("└─$ ");
        if (use_color()) printf(RESET);
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) { putchar('\n'); break; }
        char *line = trim(input);
        if (*line == '\0') continue;

        save_history_entry(line);

        int argc = split_args(line, argv, MAX_ARGS);
        if (argc == 0) continue;

        const char *alias = find_alias(argv[0]);
        if (alias) {
            snprintf(expanded_input, sizeof(expanded_input), "%s", alias);
            for (int i = 1; i < argc; i++) {
                size_t used = strlen(expanded_input);
                if (used + 1 + strlen(argv[i]) + 1 >= sizeof(expanded_input)) break;
                snprintf(expanded_input + used, sizeof(expanded_input) - used, " %s", argv[i]);
            }
            argc = split_args(expanded_input, argv, MAX_ARGS);
        }

        if (strcmp(argv[0], "exit") == 0)
            return argc > 1 ? atoi(argv[1]) : last_status;

        if (strcmp(argv[0], "help") == 0) {
            print_help();
            last_status = 0;
            continue;
        }

        if (strcmp(argv[0], "reload") == 0) {
            alias_count = 0;
            load_config();
            puts("CarsonSH configuration reloaded.");
            last_status = 0;
            continue;
        }

        if (strcmp(argv[0], "history") == 0) {
            size_t limit = 20;
            if (argc > 1) {
                char *end = NULL;
                unsigned long requested = strtoul(argv[1], &end, 10);
                if (!end || *end != '\0' || requested == 0) {
                    fprintf(stderr, "carsonsh: history: usage: history [N]\n");
                    last_status = 1;
                    continue;
                }
                limit = requested > MAX_HISTORY ? MAX_HISTORY : (size_t)requested;
            }
            print_history(limit);
            last_status = 0;
            continue;
        }

        if (strcmp(argv[0], "alias") == 0) {
            if (argc == 1) {
                for (size_t i = 0; i < alias_count; i++)
                    printf("alias %s='%s'\n", aliases[i].name, aliases[i].value);
                last_status = 0;
                continue;
            }
            char *eq = strchr(argv[1], '=');
            if (!eq) {
                fprintf(stderr, "carsonsh: alias: use NAME=VALUE\n");
                last_status = 1;
                continue;
            }
            *eq = '\0';
            last_status = set_alias(argv[1], eq + 1);
            continue;
        }

        if (strcmp(argv[0], "unalias") == 0) {
            if (argc != 2) {
                fprintf(stderr, "carsonsh: unalias: usage: unalias NAME\n");
                last_status = 1;
                continue;
            }
            int found = 0;
            for (size_t i = 0; i < alias_count; i++) {
                if (strcmp(aliases[i].name, argv[1]) == 0) {
                    aliases[i] = aliases[--alias_count];
                    found = 1;
                    break;
                }
            }
            last_status = found ? 0 : 1;
            continue;
        }

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
