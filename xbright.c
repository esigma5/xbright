/*
 *  xbright.c
 *  author: Alex Kozadaev (2011-2017)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "build_host.h"

#define NAME            "xbright"

#define ERROR(msg)      { fprintf(stderr, "ERROR: %s - %s\n", __func__, \
                            (msg)); exit(EXIT_FAILURE); } while(0)
#define MIN(a, b)       ((a) < (b) ? (a) : (b))
#define MAX(a, b)       ((a) > (b) ? (a) : (b))

static void bright_up(const char*);
static void bright_down(const char*);
static void bright_set(const char*);
static int get_current(void);
static void commit_change(int);
static void usage(void);

int
main(int argc, char **argv)
{
    if (geteuid()) {
        usage();
        ERROR("Must be root");
        return 1;
    }

    if (argc > 1) {
        switch(*argv[1]) {
            case '+': bright_up(argv[1]); break;
            case '-': bright_down(argv[1]); break;
            case '=': bright_set(argv[1]); break;
            default: usage();
        }
    } else {
        usage();
    }
    return EXIT_SUCCESS;
}

void
bright_up(const char * value)
{
    int offset_value = atoi(++value);
    int cur_value = get_current();
    int new_value = MIN(cur_value + offset_value, MAXVALUE);

    if (new_value != cur_value) {
        commit_change(new_value);
    }
}

void
bright_down(const char * value)
{
    int offset_value = atoi(++value);
    int cur_value = get_current();
    int new_value = MAX(cur_value - offset_value, 0);

    if (new_value != cur_value) {
        commit_change(new_value);
    }
}

void
bright_set(const char *value)
{
    int set_value = atoi(++value);
    if ((set_value < 0) || (set_value > MAXVALUE)) {
        ERROR("Invalid value");
    }
    commit_change(set_value);
}

int
get_current(void)
{
    int value = 0;
    FILE* input = fopen(BRIGHTNESSFILE, "r");
    if (input == NULL) {
        ERROR("Cannot open kernel pipe");
    }
    fscanf(input, "%d", &value);

#ifdef DEBUG
    printf("Current %d ", value);
#endif

    return value;
}

void
commit_change(int value)
{
    FILE* output = fopen(BRIGHTNESSFILE, "w");
    if (output == NULL) {
        ERROR("Cannot open kernel pipe");
    }

#ifdef DEBUG
    fprintf(stderr, "new brightness: %d\n", value);
#endif

    fprintf(output, "%d", value);
    fclose(output);
}

void
usage(void)
{
    printf("%s v%s (works only with 2.6+ kernels).\n", NAME, VERSION);
    printf("usage: %s [+-=[0-%d]]\n", NAME, MAXVALUE);
}
