#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h> 

// colors
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define CYAN    "\033[36m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define GRAY    "\033[90m"

// logo
const char *logo[] = {
    "    |     ",
    "  .'|'.   ",
    " /  |  \\  ",
    " | /|\\ |  ",
    "  \\ | /   ",
    "   \\|/    ",
    "    `     ",
    NULL
};

// get hostname
char* get_hostname(void) {
    static char buf[256];
    if (gethostname(buf, sizeof(buf)) == 0)
        return buf;
    return "unknown";
}

// get username
char* get_username(void) {
    char *user = getenv("USER");
    if (!user) user = getenv("LOGNAME");
    if (!user) return "user";
    return user;
}

// get distro name from /etc/os-release
char* get_distro(void) {
    static char distro[256] = "Leaffy";
    FILE *fp = fopen("/etc/os-release", "r");
    if (!fp) return distro;
    
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "PRETTY_NAME=", 12) == 0) {
            char *start = strchr(line, '"');
            char *end = strrchr(line, '"');
            if (start && end) {
                int len = end - start - 1;
                if (len > 0 && len < 255) {
                    strncpy(distro, start + 1, len);
                    distro[len] = '\0';
                }
                break;
            }
        }
    }
    fclose(fp);
    return distro;
}

// get kernel version
char* get_kernel(void) {
    static struct utsname buf;
    uname(&buf);
    return buf.release;
}

// get system uptime
char* get_uptime(void) {
    static char buf[64];
    FILE *fp = fopen("/proc/uptime", "r");
    if (!fp) return "N/A";
    
    double uptime_sec;
    fscanf(fp, "%lf", &uptime_sec);
    fclose(fp);
    
    int minutes = (int)(uptime_sec / 60);
    int hours = minutes / 60;
    int days = hours / 24;
    
    minutes = minutes % 60;
    hours = hours % 24;
    
    if (days > 0)
        snprintf(buf, sizeof(buf), "%d day%s, %d hour%s, %d minute%s", 
                 days, days > 1 ? "s" : "",
                 hours, hours > 1 ? "s" : "",
                 minutes, minutes > 1 ? "s" : "");
    else if (hours > 0)
        snprintf(buf, sizeof(buf), "%d hour%s, %d minute%s",
                 hours, hours > 1 ? "s" : "",
                 minutes, minutes > 1 ? "s" : "");
    else
        snprintf(buf, sizeof(buf), "%d minute%s",
                 minutes, minutes > 1 ? "s" : "");
    
    return buf;
}

// count pacman packages
int get_packages(void) {
    if (access("/usr/bin/pacman", X_OK) == 0) {
        FILE *fp = popen("pacman -Q 2>/dev/null | wc -l", "r");
        if (fp) {
            int count;
            fscanf(fp, "%d", &count);
            pclose(fp);
            return count;
        }
    }
    return 0;
}

// count lfm packages from database
int get_lfm_packages(void) {
    DIR *dir = opendir("/var/lib/lfm/db");
    if (!dir) {
        return 0;
    }
    
    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        
        char path[512];
        snprintf(path, sizeof(path), "/var/lib/lfm/db/%s", entry->d_name);
        struct stat st;
        if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
            count++;
        }
    }
    closedir(dir);
    
    return count;
}

// get shell name
char* get_shell(void) {
    char *shell = getenv("SHELL");
    if (!shell) return "unknown";
    char *last = strrchr(shell, '/');
    if (last) return last + 1;
    return shell;
}

// get shell version
char* get_shell_version(const char *shell_name) {
    static char buf[32];
    
    if (strcmp(shell_name, "bash") == 0) {
        FILE *fp = popen("bash --version 2>/dev/null | head -1 | grep -oE '[0-9]+\\.[0-9]+\\.[0-9]+'", "r");
        if (fp) {
            if (fgets(buf, sizeof(buf), fp)) {
                buf[strcspn(buf, "\n")] = '\0';
                pclose(fp);
                return buf;
            }
            pclose(fp);
        }
    } else if (strcmp(shell_name, "fish") == 0) {
        FILE *fp = popen("fish --version 2>/dev/null | grep -oE '[0-9]+\\.[0-9]+\\.[0-9]+'", "r");
        if (fp) {
            if (fgets(buf, sizeof(buf), fp)) {
                buf[strcspn(buf, "\n")] = '\0';
                pclose(fp);
                return buf;
            }
            pclose(fp);
        }
    } else if (strcmp(shell_name, "zsh") == 0) {
        FILE *fp = popen("zsh --version 2>/dev/null | grep -oE '[0-9]+\\.[0-9]+\\.[0-9]+'", "r");
        if (fp) {
            if (fgets(buf, sizeof(buf), fp)) {
                buf[strcspn(buf, "\n")] = '\0';
                pclose(fp);
                return buf;
            }
            pclose(fp);
        }
    }
    
    return "";
}

// get cpu model name
char* get_cpu(void) {
    static char cpu[256] = "Unknown";
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (!fp) return cpu;
    
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "model name", 10) == 0) {
            char *colon = strchr(line, ':');
            if (colon) {
                char *start = colon + 2;
                char *newline = strchr(start, '\n');
                if (newline) *newline = '\0';
                
                char *p = start;
                char *dst = cpu;
                while (*p) {
                    if (*p == '(') {
                        while (*p && *p != ')') p++;
                        if (*p) p++;
                        continue;
                    }
                    *dst++ = *p++;
                }
                *dst = '\0';
                
                char *slow = cpu, *fast = cpu;
                while (*fast) {
                    if (*fast == ' ' && *(fast+1) == ' ') {
                        fast++;
                        continue;
                    }
                    *slow++ = *fast++;
                }
                *slow = '\0';
                break;
            }
        }
    }
    fclose(fp);
    return cpu;
}

// get number of cpu cores
int get_cpu_cores(void) {
    int cores = 0;
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (!fp) return 0;
    
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "processor", 9) == 0)
            cores++;
    }
    fclose(fp);
    return cores;
}

// get cpu frequency
char* get_cpu_freq(void) {
    static char buf[32];
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (!fp) return "";
    
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "cpu MHz", 7) == 0) {
            char *colon = strchr(line, ':');
            if (colon) {
                double mhz;
                sscanf(colon + 1, "%lf", &mhz);
                snprintf(buf, sizeof(buf), "@ %.2f GHz", mhz / 1000.0);
                fclose(fp);
                return buf;
            }
        }
    }
    fclose(fp);
    return "";
}

// get memory usage
char* get_memory(void) {
    static char buf[64];
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) return "N/A";
    
    long total = 0, available = 0;
    char line[256];
    
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "MemTotal:", 9) == 0)
            sscanf(line, "MemTotal: %ld", &total);
        else if (strncmp(line, "MemAvailable:", 13) == 0)
            sscanf(line, "MemAvailable: %ld", &available);
    }
    fclose(fp);
    
    long used = total - available;
    double used_gb = used / 1048576.0;
    double total_gb = total / 1048576.0;
    int percent = (int)((double)used / total * 100);
    
    snprintf(buf, sizeof(buf), "%.2f GiB / %.2f GiB (%d%%)",
             used_gb, total_gb, percent);
    
    return buf;
}

// print help message
void print_help(void) {
    printf("lfetch - Leaffy system info fetch\n");
    printf("Usage: lfetch [OPTIONS]\n");
    printf("Options:\n");
    printf("  -h, --help     Show this help\n");
    printf("  -v, --version  Show version\n");
}

int main(int argc, char **argv)
{
    // parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help();
            return 0;
        }
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("lfetch version 0.0.2 (lfm)\n");
            return 0;
        }
    }
    
    // collect all info
    char *user = get_username();
    char *host = get_hostname();
    char *distro = get_distro();
    char *kernel = get_kernel();
    char *uptime = get_uptime();
    int packages = get_packages();
    int lfm_packages = get_lfm_packages();
    char *shell_name = get_shell();
    char *shell_ver = get_shell_version(shell_name);
    char *cpu = get_cpu();
    int cores = get_cpu_cores();
    char *freq = get_cpu_freq();
    char *memory = get_memory();
    
    // print user@host line
    printf("\n%s%s%s@%s%s\n", BOLD, GREEN, user, host, RESET);
    printf("\n");
    
    // print logo and info side by side
    int logo_line = 0;
    int info_lines = 7;
    
    for (int i = 0; i < info_lines; i++) {
        // print logo line
        if (logo[logo_line] != NULL) {
            printf("%s%s%s", GREEN, logo[logo_line], RESET);
            logo_line++;
        } else {
            printf("         ");
        }
        
        // print info line
        switch(i) {
            case 0:
                printf("  %sOS%s %s\n", BOLD, RESET, distro);
                break;
            case 1:
                printf("  %sKernel%s %s\n", BOLD, RESET, kernel);
                break;
            case 2:
                printf("  %sUptime%s %s\n", BOLD, RESET, uptime);
                break;
            case 3:
                printf("  %sPackages%s %d (pacman), %d (lfm) = %d total\n", BOLD, RESET, packages, lfm_packages, packages + lfm_packages);
                break;
            case 4: {
                char shell_info[64];
                if (strlen(shell_ver) > 0)
                    snprintf(shell_info, sizeof(shell_info), "%s %s", shell_name, shell_ver);
                else
                    snprintf(shell_info, sizeof(shell_info), "%s", shell_name);
                printf("  %sShell%s %s\n", BOLD, RESET, shell_info);
                break;
            }
            case 5: {
                char cpu_info[256];
                snprintf(cpu_info, sizeof(cpu_info), "%s (%d) %s", cpu, cores, freq);
                printf("  %sCPU%s %s\n", BOLD, RESET, cpu_info);
                break;
            }
            case 6:
                printf("  %sMemory%s %s\n", BOLD, RESET, memory);
                break;
        }
    }
    
    printf("\n");
    return 0;
}
