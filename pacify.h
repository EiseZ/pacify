/*
// Pacify
// A config file reader/writer in unix config format
//
// Eise Zimmerman
//
// Licensed under the MIT license (https://opensource.org/licenses/MIT)
*/

#ifndef PACIFY_H
#define PACIFY_H

#include <stddef.h>
#include <solidify.h>

/* --- Debugging --- */
extern void pacify_log_options(void);

/* --- File management --- */
extern void pacify_load_options(char *filepath);
extern void pacify_save_options(void);

/* --- Setting management --- */
extern int pacify_option_int_get(char *name, int default_value);
extern double pacify_option_double_get(char *name, double default_value);
extern char *pacify_option_text_get(char *name, char *default_value);
extern void pacify_option_int_set(char *name, int value);
extern void pacify_option_double_set(char *name, double value);
extern void pacify_option_text_set(char *name, char *value);

#endif /* PACIFY_H */
