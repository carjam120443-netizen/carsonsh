#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <langinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_ARGS 128
#define INPUT_SIZE 4096
#define MAX_ALIASES 64
#define MAX_HISTORY 1000
#define MAX_FRAMEWORK_FILES 32

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
static char loaded_plugins[MAX_FRAMEWORK_FILES][64];
static size_t loaded_plugin_count = 0;
static char active_theme[64] = "carson-green";
static char prompt_style[64] = "carson-green";
static int show_welcome = 1;

/* Framework loaders are used by load_framework_file before their definitions. */
static void load_framework_plugin(const char *name);
static void load_framework_theme(const char *name);

static int use_color(void) {
    return isatty(STDOUT_FILENO);
}

/*
 * A shell cannot inspect the font selected by the terminal emulator.
 * Use locale/terminal hints to avoid emitting glyphs that are unlikely to
 * render, and allow an explicit user override with CARSONSH_ASCII=1.
 */
static int use_unicode_prompt(void) {
    const char *force_ascii = getenv("CARSONSH_ASCII");
    const char *term = getenv("TERM");
    const char *codeset;

    if (force_ascii && *force_ascii && strcmp(force_ascii, "0") != 0)
        return 0;
    if (term && strcmp(term, "dumb") == 0)
        return 0;

    (void)setlocale(LC_CTYPE, "");
    codeset = nl_langinfo(CODESET);
    if (!codeset || (!strstr(codeset, "UTF-8") && !strstr(codeset, "utf8") &&
                     !strstr(codeset, "UTF8")))
        return 0;
    return 1;
}

static size_t prompt_columns(void) {
    const char *value = getenv("COLUMNS");
    char *end = NULL;
    unsigned long columns;

    if (!value || !*value) return 80;
    columns = strtoul(value, &end, 10);
    if (!end || *end != '\0' || columns < 20) return 80;
    return columns > 1000 ? 1000 : (size_t)columns;
}

/* Keep very long working directories from forcing an awkward prompt wrap. */
static void compact_prompt_path(const char *path, char *out, size_t out_size) {
    size_t limit = prompt_columns() > 24 ? prompt_columns() - 20 : 4;
    size_t length = strlen(path);
    const char *marker = use_unicode_prompt() ? "…" : "...";
    size_t marker_len = strlen(marker);

    if (length <= limit || out_size == 0) {
        snprintf(out, out_size, "%s", path);
        return;
    }
    if (limit <= marker_len + 2) {
        snprintf(out, out_size, "%s", marker);
        return;
    }
    {
        size_t tail = (limit - marker_len) / 2;
        size_t head = limit - marker_len - tail;
        if (head + marker_len + tail + 1 > out_size) {
            snprintf(out, out_size, "%s", path);
            return;
        }
        memcpy(out, path, head);
        memcpy(out + head, marker, marker_len);
        memcpy(out + head + marker_len, path + length - tail, tail);
        out[head + marker_len + tail] = '\0';
    }
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

static void load_framework_file(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) return;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        char *s = trim(line);
        if (*s == '\0' || *s == '#') continue;
        if (strncmp(s, "theme ", 6) == 0) {
            load_framework_theme(trim(s + 6));
        } else if (strncmp(s, "prompt ", 7) == 0) {
            snprintf(prompt_style, sizeof(prompt_style), "%s", trim(s + 7));
        } else if (strncmp(s, "plugin ", 7) == 0) {
            load_framework_plugin(trim(s + 7));
        } else if (strncmp(s, "alias ", 6) == 0) {
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

static void load_framework_plugin(const char *name) {
    const char *home = getenv("HOME");
    if (!home || !name || !*name || loaded_plugin_count >= MAX_FRAMEWORK_FILES) return;
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/.config/carsonsh/plugins/%s.conf", home, name);
    load_framework_file(path);
    snprintf(loaded_plugins[loaded_plugin_count], sizeof(loaded_plugins[0]), "%s", name);
    loaded_plugin_count++;
}

static void load_framework_theme(const char *name) {
    const char *home = getenv("HOME");
    if (!home || !name || !*name) return;
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/.config/carsonsh/themes/%s.conf", home, name);
    load_framework_file(path);
    snprintf(active_theme, sizeof(active_theme), "%s", name);
}

static void list_themes(void) {
    const char *home = getenv("HOME");
    if (!home) {
        puts("carsonsh: HOME is not set; cannot locate themes.");
        return;
    }

    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/.config/carsonsh/themes", home);
    DIR *dir = opendir(path);
    if (!dir) {
        printf("No themes found in %s. Install framework themes first.\\n", path);
        return;
    }

    struct dirent *entry;
    size_t count = 0;
    puts("Available CarsonSH themes:");
    while ((entry = readdir(dir)) != NULL) {
        size_t length = strlen(entry->d_name);
        if (length <= 5 || strcmp(entry->d_name + length - 5, ".conf") != 0)
            continue;

        char name[64];
        size_t name_length = length - 5;
        if (name_length >= sizeof(name)) continue;
        memcpy(name, entry->d_name, name_length);
        name[name_length] = '\\0';

        printf("  %s%s\\n", name,
               strcmp(name, active_theme) == 0 ? " (active)" : "");
        count++;
    }
    closedir(dir);
    if (count == 0) puts("  (no .conf theme files found)");
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
        if (strncmp(s, "theme ", 6) == 0) {
            load_framework_theme(trim(s + 6));
        } else if (strncmp(s, "prompt ", 7) == 0) {
            snprintf(prompt_style, sizeof(prompt_style), "%s", trim(s + 7));
        } else if (strncmp(s, "welcome ", 8) == 0) {
            const char *setting = trim(s + 8);
            if (strcmp(setting, "on") == 0) show_welcome = 1;
            else if (strcmp(setting, "off") == 0) show_welcome = 0;
            else fprintf(stderr, "carsonsh: config: expected 'welcome on' or 'welcome off'\n");
        } else if (strncmp(s, "plugin ", 7) == 0) {
            load_framework_plugin(trim(s + 7));
        } else if (strncmp(s, "alias ", 6) == 0) {
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

static void print_prompt(const char *cwd) {
    if (strcmp(prompt_style, "minimal") == 0) {
        if (use_color()) printf(GREEN);
        printf("carsonsh:%s $ ", cwd);
        if (use_color()) printf(RESET);
    } else if (strcmp(prompt_style, "pure") == 0) {
        if (use_color()) printf(GREEN);
        printf("%s", cwd);
        if (use_color()) printf(RESET);
        print_git_branch();
        printf(" ❯ ");
    } else if (strcmp(prompt_style, "robbyrussell") == 0) {
        if (use_color()) printf(GREEN);
        printf("➜  %s", cwd);
        if (use_color()) printf(RESET);
        print_git_branch();
        printf(" $ ");
    } else if (strcmp(prompt_style, "agnoster") == 0) {
        if (use_color()) printf(GREEN);
        printf("┌─[%s]", cwd);
        if (use_color()) printf(RESET);
        print_git_branch();
        putchar('\n');
        if (use_color()) printf(GREEN);
        printf("└─➤ ");
        if (use_color()) printf(RESET);
    } else if (strcmp(prompt_style, "spaceship") == 0) {
        if (use_color()) printf(CYAN);
        printf("🚀 %s", cwd);
        if (use_color()) printf(RESET);
        print_git_branch();
        printf(" ➜ ");
    } else if (strcmp(prompt_style, "powerlevel10k") == 0) {
        if (use_color()) printf(GREEN);
        printf("╭─%s", cwd);
        if (use_color()) printf(RESET);
        print_git_branch();
        putchar('\n');
        if (use_color()) printf(GREEN);
        printf("╰─❯ ");
        if (use_color()) printf(RESET);
    } else {
        if (use_color()) printf(GREEN);
        printf("┌──(carson㉿carsonsh)-[%s]", cwd);
        if (use_color()) printf(RESET);
        print_git_branch();
        putchar('\n');
        if (use_color()) printf(GREEN);
        printf(use_unicode_prompt() ? "└─$ " : "+-$ ");
        if (use_color()) printf(RESET);
    }
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


static void print_section(const char *title) {
    printf("\n%s\n", title);
    for (size_t i = 0; i < strlen(title); i++) putchar('-');
    putchar('\n');
}

static void cpu_model(char *out, size_t size) {
    FILE *fp = fopen("/proc/cpuinfo", "r");
    char line[512];
    snprintf(out, size, "Unknown CPU");
    if (!fp) return;
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "model name", 10) == 0 ||
            strncmp(line, "Hardware", 8) == 0) {
            char *colon = strchr(line, ':');
            if (colon) {
                snprintf(out, size, "%s", trim(colon + 1));
                out[strcspn(out, "\r\n")] = '\0';
                break;
            }
        }
    }
    fclose(fp);
}

static int read_cpu_ticks(unsigned long long *idle, unsigned long long *total) {
    FILE *fp = fopen("/proc/stat", "r");
    unsigned long long user, nice, system, idle_ticks, iowait = 0;
    unsigned long long irq = 0, softirq = 0, steal = 0;
    if (!fp) return 0;
    int count = fscanf(fp, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
                       &user, &nice, &system, &idle_ticks, &iowait,
                       &irq, &softirq, &steal);
    fclose(fp);
    if (count < 4) return 0;
    *idle = idle_ticks + iowait;
    *total = user + nice + system + idle_ticks + iowait + irq + softirq + steal;
    return 1;
}

static void command_cpufetch(void) {
    char model[256];
    unsigned long long idle1, total1, idle2, total2;
    print_section("CarsonSH CPU Fetch");
#if defined(_WIN32) || defined(__MSYS__) || defined(__CYGWIN__)
    FILE *fp = popen("powershell.exe -NoProfile -Command \"$cpu=Get-CimInstance Win32_Processor | Select-Object -First 1; Write-Output $cpu.Name; Write-Output $cpu.NumberOfLogicalProcessors; try { [math]::Round((Get-Counter '\\Processor(_Total)\\% Processor Time').CounterSamples[0].CookedValue,1) } catch { Write-Output 'N/A' }\"", "r");
    if (fp) {
        char line[512];
        int row = 0;
        while (fgets(line, sizeof(line), fp)) {
            line[strcspn(line, "\r\n")] = '\0';
            if (!*line) continue;
            if (row == 0) printf("CPU: %s\n", line);
            else if (row == 1) printf("Logical CPUs: %s\n", line);
            else if (row == 2) printf("Current usage: %s%s\n", line,
                                      strcmp(line, "N/A") == 0 ? "" : "%");
            row++;
        }
        pclose(fp);
        if (row == 0) puts("CPU information: unavailable");
    } else puts("CPU information: unavailable");
#elif defined(__linux__)
    cpu_model(model, sizeof(model));
    printf("CPU: %s\n", model);
    long cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (cores > 0) printf("Logical CPUs: %ld\n", cores);
    if (read_cpu_ticks(&idle1, &total1)) {
        struct timespec delay = {0, 300000000L};
        (void)nanosleep(&delay, NULL);
        if (read_cpu_ticks(&idle2, &total2) && total2 > total1) {
            double usage = 100.0 * (double)((total2 - total1) - (idle2 - idle1)) /
                           (double)(total2 - total1);
            if (usage < 0.0) usage = 0.0;
            if (usage > 100.0) usage = 100.0;
            printf("Current usage: %.1f%%\n", usage);
        } else puts("Current usage: unavailable");
    } else puts("Current usage: unavailable (no /proc/stat)");
#else
    cpu_model(model, sizeof(model));
    printf("CPU: %s\n", model);
    puts("Current usage: unavailable on this platform");
#endif
}

static void command_gpufetch(void) {
    print_section("CarsonSH GPU Fetch");
#if defined(_WIN32) || defined(__MSYS__) || defined(__CYGWIN__)
    FILE *fp = popen("powershell.exe -NoProfile -Command \"Get-CimInstance Win32_VideoController | ForEach-Object { $_.Name }\"", "r");
    if (fp) {
        char line[512];
        int found = 0;
        while (fgets(line, sizeof(line), fp)) {
            line[strcspn(line, "\r\n")] = '\0';
            if (*line) { printf("GPU: %s\n", line); found = 1; }
        }
        pclose(fp);
        if (!found) puts("GPU: unavailable");
    } else puts("GPU: unavailable");
    puts("GPU utilization: not exposed by this CarsonSH build on Windows");
#else
    FILE *fp = popen("nvidia-smi --query-gpu=name,utilization.gpu,memory.used,memory.total --format=csv,noheader 2>/dev/null", "r");
    if (fp) {
        char line[512];
        int found = 0;
        while (fgets(line, sizeof(line), fp)) {
            line[strcspn(line, "\r\n")] = '\0';
            if (*line) { printf("GPU: %s\n", line); found = 1; }
        }
        int status = pclose(fp);
        if (found && status == 0) return;
    }
    fp = popen("lspci 2>/dev/null | grep -Ei 'VGA compatible controller|3D controller|Display controller'", "r");
    if (fp) {
        char line[512];
        int found = 0;
        while (fgets(line, sizeof(line), fp)) {
            line[strcspn(line, "\r\n")] = '\0';
            if (*line) { printf("GPU: %s\n", line); found = 1; }
        }
        pclose(fp);
        if (!found) puts("GPU: not detected (install/use lspci or nvidia-smi)");
    } else puts("GPU: not detected (install/use lspci or nvidia-smi)");
    puts("GPU utilization: unavailable without a supported vendor tool");
#endif
}

static void command_displayfetch(void) {
    print_section("CarsonSH Display Fetch");
#if defined(_WIN32) || defined(__MSYS__) || defined(__CYGWIN__)
    FILE *fp = popen("powershell.exe -NoProfile -Command \"Get-CimInstance Win32_VideoController | ForEach-Object { if ($_.CurrentHorizontalResolution -gt 0) { '{0}: {1}x{2} @ {3} Hz' -f $_.Name,$_.CurrentHorizontalResolution,$_.CurrentVerticalResolution,$_.CurrentRefreshRate } }\"", "r");
    if (fp) {
        char line[512];
        int found = 0;
        while (fgets(line, sizeof(line), fp)) {
            line[strcspn(line, "\r\n")] = '\0';
            if (*line) { puts(line); found = 1; }
        }
        pclose(fp);
        if (!found) puts("Display mode: unavailable");
    } else puts("Display mode: unavailable");
#else
    FILE *fp = popen("xrandr --current 2>/dev/null | grep ' connected' ", "r");
    if (fp) {
        char line[512];
        int found = 0;
        while (fgets(line, sizeof(line), fp)) {
            line[strcspn(line, "\r\n")] = '\0';
            char *mode = strchr(line, ' ');
            if (mode) {
                while (*mode == ' ') mode++;
                printf("%s\n", mode);
                found = 1;
            }
        }
        pclose(fp);
        if (found) return;
    }
    fp = popen("wlr-randr 2>/dev/null", "r");
    if (fp) {
        char line[512];
        int found = 0;
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, "current") || strstr(line, "Enabled:")) {
                fputs(line, stdout);
                found = 1;
            }
        }
        pclose(fp);
        if (found) return;
    }
    puts("Display mode: unavailable (try xrandr or wlr-randr)");
#endif
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
    puts("  theme [NAME]      Show or select a CarsonSH framework theme");
    puts("  theme list        List themes installed in your user config");
    puts("  plugin [NAME]     Show or load a CarsonSH framework plugin");
    puts("  cpufetch          Show CPU model, logical CPUs, and current usage");
    puts("  gpufetch          Show detected GPU and available utilization data");
    puts("  displayfetch      Show connected display modes and refresh rates");
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
    if (getenv("CARSONSH_BANNER")) print_banner();
    if (show_welcome) puts("welcome to carsonsh");

    while (1) {
        char cwd[PATH_MAX];
        if (!getcwd(cwd, sizeof(cwd))) snprintf(cwd, sizeof(cwd), "?");

        char display_cwd[PATH_MAX];
        compact_prompt_path(cwd, display_cwd, sizeof(display_cwd));
        print_prompt(display_cwd);
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
            loaded_plugin_count = 0;
            snprintf(active_theme, sizeof(active_theme), "%s", "carson-green");
            snprintf(prompt_style, sizeof(prompt_style), "%s", "carson-green");
            show_welcome = 1;
            load_config();
            puts("CarsonSH configuration reloaded.");
            last_status = 0;
            continue;
        }

        if (strcmp(argv[0], "theme") == 0) {
            if (argc == 1) {
                printf("Active CarsonSH theme: %s\\n", active_theme);
                last_status = 0;
                continue;
            }
            if (argc == 2 && strcmp(argv[1], "list") == 0) {
                list_themes();
                last_status = 0;
                continue;
            }
            if (argc != 2) {
                fprintf(stderr, "carsonsh: theme: usage: theme [NAME|list]\\n");
                last_status = 1;
                continue;
            }
            load_framework_theme(argv[1]);
            printf("CarsonSH theme selected: %s\\n", active_theme);
            last_status = 0;
            continue;
        }

        if (strcmp(argv[0], "plugin") == 0) {
            if (argc == 1) {
                if (loaded_plugin_count == 0) puts("No CarsonSH framework plugins loaded.");
                else {
                    puts("Loaded CarsonSH framework plugins:");
                    for (size_t i = 0; i < loaded_plugin_count; i++)
                        printf("  %s\n", loaded_plugins[i]);
                }
                last_status = 0;
                continue;
            }
            if (argc != 2) {
                fprintf(stderr, "carsonsh: plugin: usage: plugin [NAME]\n");
                last_status = 1;
                continue;
            }
            load_framework_plugin(argv[1]);
            printf("CarsonSH plugin loaded: %s\n", argv[1]);
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

        if (strcmp(argv[0], "cpufetch") == 0) {
            command_cpufetch();
            last_status = 0;
            continue;
        }

        if (strcmp(argv[0], "gpufetch") == 0) {
            command_gpufetch();
            last_status = 0;
            continue;
        }

        if (strcmp(argv[0], "displayfetch") == 0) {
            command_displayfetch();
            last_status = 0;
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
