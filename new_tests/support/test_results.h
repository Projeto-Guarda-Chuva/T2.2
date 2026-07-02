#pragma once

void results_begin(const char *component);
void results_set_input(const char *fmt, ...);
void results_record(const char *name, int failed);
void results_add_output(const char *line);
int  results_write(const char *path);
