#include "test_results.h"

#include <stdarg.h>
#include <stdio.h>

#define MAX_TESTS 128
#define MAX_OUT   64
#define STRN      512

typedef struct {
    char name[STRN];
    char input[STRN];
    int  failed;
    int  out_count;
    char out[MAX_OUT][STRN];
} entry_t;

static char    g_component[128];
static entry_t g_entries[MAX_TESTS];
static int     g_count;
static char    g_pending_input[STRN];

void results_begin(const char *component) {
    snprintf(g_component, sizeof(g_component), "%s", component ? component : "");
    g_count = 0;
    g_pending_input[0] = '\0';
}

void results_set_input(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(g_pending_input, sizeof(g_pending_input), fmt, ap);
    va_end(ap);
}

void results_record(const char *name, int failed) {
    if (g_count >= MAX_TESTS) {
        return;
    }
    entry_t *e = &g_entries[g_count++];
    snprintf(e->name, sizeof(e->name), "%s", name ? name : "");
    snprintf(e->input, sizeof(e->input), "%s", g_pending_input);
    e->failed = failed;
    e->out_count = 0;
    g_pending_input[0] = '\0';
}

void results_add_output(const char *line) {
    if (g_count == 0) {
        return;
    }
    entry_t *e = &g_entries[g_count - 1];
    if (e->out_count >= MAX_OUT) {
        return;
    }
    snprintf(e->out[e->out_count++], STRN, "%s", line ? line : "");
}

static void json_str(FILE *f, const char *s) {
    fputc('"', f);
    for (const unsigned char *p = (const unsigned char *)s; *p; p++) {
        switch (*p) {
            case '"':  fputs("\\\"", f); break;
            case '\\': fputs("\\\\", f); break;
            case '\n': fputs("\\n", f);  break;
            case '\r': fputs("\\r", f);  break;
            case '\t': fputs("\\t", f);  break;
            default:
                if (*p < 0x20) {
                    fprintf(f, "\\u%04x", *p);
                } else {
                    fputc(*p, f);
                }
        }
    }
    fputc('"', f);
}

int results_write(const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) {
        return -1;
    }

    int pass = 0, fail = 0;
    for (int i = 0; i < g_count; i++) {
        if (g_entries[i].failed) {
            fail++;
        } else {
            pass++;
        }
    }

    fputs("{\n", f);
    fputs("  \"componente\": ", f);
    json_str(f, g_component);
    fputs(",\n", f);
    fprintf(f, "  \"resumo\": { \"total\": %d, \"pass\": %d, \"fail\": %d },\n",
            g_count, pass, fail);
    fputs("  \"testes\": [\n", f);

    for (int i = 0; i < g_count; i++) {
        entry_t *e = &g_entries[i];
        fputs("    {\n", f);
        fputs("      \"nome\": ", f);
        json_str(f, e->name);
        fputs(",\n", f);
        fputs("      \"entrada\": ", f);
        json_str(f, e->input);
        fputs(",\n", f);
        fprintf(f, "      \"resultado\": \"%s\",\n", e->failed ? "FAIL" : "PASS");
        fputs("      \"saida_obtida\": [", f);
        for (int j = 0; j < e->out_count; j++) {
            if (j) {
                fputs(", ", f);
            }
            json_str(f, e->out[j]);
        }
        fputs("]\n", f);
        fprintf(f, "    }%s\n", (i + 1 < g_count) ? "," : "");
    }

    fputs("  ]\n", f);
    fputs("}\n", f);
    fclose(f);
    return 0;
}
