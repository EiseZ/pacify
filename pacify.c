/*
// Pacify
// A config file reader/writer in unix config format
//
// Eise Zimmerman
//
// Licensed under the MIT license (https://opensource.org/licenses/MIT)
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "pacify.h"

enum PACIFY_VALUE_TYPE
{
    PACIFY_TYPE_NONE,
    PACIFY_TYPE_INT,
    PACIFY_TYPE_DOUBLE,
    PACIFY_TYPE_TEXT,
};

typedef struct
{
    enum PACIFY_VALUE_TYPE type;
    char *name;
    union
    {
        int integer;
        double decimal;
        char *text;
    };
} option;

typedef struct
{
    char *filepath;
    uint optionAmount;
    option options[1];
} pacifyState;
pacifyState *pacifyCurrentState;

/* --- File management --- */
void
pacify_load_options(char *filepath)
{
    FILE *file;
    char *buffer, *optionName, *optionValue;
    uint fileSize, i, j;
    enum PACIFY_VALUE_TYPE optionType = PACIFY_TYPE_NONE;

    pacifyCurrentState = malloc(sizeof(pacifyState) - sizeof(option));
    pacifyCurrentState->optionAmount = 0;

    if ((file = fopen(filepath, "r")) != NULL) {
        fseek(file, 0, SEEK_END);
        fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);
        buffer = malloc(fileSize + 1);
        fread(buffer, sizeof(char), fileSize, file);
        buffer[fileSize] = '\0';
        fclose(file);

        for (i = 0; buffer[i] != '\0'; i++) {
            switch (buffer[i]) {
                case '\n':
                    optionType = PACIFY_TYPE_NONE;
                    break;
                default:
                    if (optionType != PACIFY_TYPE_NONE) {
                        printf
                            ("[PACIFY] Error parsing config file on char %u\n",
                             i);
                        exit(1);
                    }

                    switch (buffer[i]) {
                        case 'i':
                            optionType = PACIFY_TYPE_INT;

                            i++;
                            for (j = i; buffer[j] != ' '; j++);
                            optionName = malloc(sizeof(char) * (j - i + 1));
                            strncpy(optionName, &buffer[i], (j - i));
                            optionName[j - i] = '\0';
                            i = ++j;

                            for (; buffer[j] != '\n' && buffer[j] != '\0'; j++);
                            optionValue = malloc(sizeof(char) * (j - i + 1));
                            strncpy(optionValue, &buffer[i], (j - i));
                            optionValue[j - i] = '\0';
                            i = j - 1;

                            pacify_option_int_set(optionName,
                                                  atoi(optionValue));
                            break;
                        case 'd':
                            optionType = PACIFY_TYPE_DOUBLE;

                            i++;
                            for (j = i; buffer[j] != ' '; j++);
                            optionName = malloc(sizeof(char) * (j - i + 1));
                            strncpy(optionName, &buffer[i], (j - i));
                            optionName[j - i] = '\0';
                            i = ++j;

                            for (; buffer[j] != '\n' && buffer[j] != '\0'; j++);
                            optionValue = malloc(sizeof(char) * (j - i + 1));
                            strncpy(optionValue, &buffer[i], (j - i));
                            optionValue[j - i] = '\0';
                            i = j - 1;

                            pacify_option_double_set(optionName,
                                                     strtod(optionValue, NULL));
                            break;
                        case 't':
                            optionType = PACIFY_TYPE_TEXT;

                            i++;
                            for (j = i; buffer[j] != ' '; j++);
                            optionName = malloc(sizeof(char) * (j - i + 1));
                            strncpy(optionName, &buffer[i], (j - i));
                            optionName[j - i] = '\0';
                            i = ++j;

                            for (; buffer[j] != '\n' && buffer[j] != '\0'; j++);
                            optionValue = malloc(sizeof(char) * (j - i + 1));
                            strncpy(optionValue, &buffer[i], (j - i));
                            optionValue[j - i] = '\0';
                            i = j - 1;

                            pacify_option_text_set(optionName, optionValue);
                            break;
                        default:
                            printf
                                ("[Pacify] Error parsing config file on char %u; Not a valid variable name.\n",
                                 i);
                            exit(1);
                            break;
                    }

                    free(optionName);
                    free(optionValue);
                    break;
            }
        }

        free(buffer);
    }

    pacifyCurrentState->filepath = filepath;
}

void
pacify_log_options(void)
{
    uint i;

    printf("[Pacify] Logging current options: ");
    for (i = 0; i < pacifyCurrentState->optionAmount; i++) {
        switch (pacifyCurrentState->options[i].type) {
            case PACIFY_TYPE_INT:
                printf("[Pacify] Int: ");
                printf("%s %d\n", pacifyCurrentState->options[i].name,
                       pacifyCurrentState->options[i].integer);
                break;
            case PACIFY_TYPE_DOUBLE:
                printf("[Pacify] Double: ");
                printf("%s %f\n", pacifyCurrentState->options[i].name,
                       pacifyCurrentState->options[i].decimal);
                break;
            case PACIFY_TYPE_TEXT:
                printf("[Pacify] Text: ");
                printf("%s %s\n", pacifyCurrentState->options[i].name,
                       pacifyCurrentState->options[i].text);
                break;
            case PACIFY_TYPE_NONE:
                printf("[Pacify] Error: trying to log option without type");
                exit(1);
                break;
        }
    }
}

void
pacify_save_options(void)
{
    uint i;
    FILE *file;

    if ((file = fopen(pacifyCurrentState->filepath, "w")) == NULL) {
        printf("[Pacify] Error opening file for saving settings on path %s\n",
               pacifyCurrentState->filepath);
        exit(1);
    }
    for (i = 0; i < pacifyCurrentState->optionAmount; i++) {
        switch (pacifyCurrentState->options[i].type) {
            case PACIFY_TYPE_INT:
                fprintf(file, "i");
                fprintf(file, "%s %d\n", pacifyCurrentState->options[i].name,
                        pacifyCurrentState->options[i].integer);
                break;
            case PACIFY_TYPE_DOUBLE:
                fprintf(file, "d");
                fprintf(file, "%s %f\n", pacifyCurrentState->options[i].name,
                        pacifyCurrentState->options[i].decimal);
                break;
            case PACIFY_TYPE_TEXT:
                fprintf(file, "t");
                fprintf(file, "%s %s\n", pacifyCurrentState->options[i].name,
                        pacifyCurrentState->options[i].text);
                free(pacifyCurrentState->options[i].text);
                break;
            case PACIFY_TYPE_NONE:
                printf("[Pacify] Error: trying to save option without type");
                exit(1);
                break;
        }
        free(pacifyCurrentState->options[i].name);
    }

    fclose(file);
    free(pacifyCurrentState);
}

/* --- Setting management --- */
int
pacify_option_int_get(char *optionName, int default_value)
{
    uint i;

    for (i = 0; i < pacifyCurrentState->optionAmount; i++) {
        if (pacifyCurrentState->options[i].type == PACIFY_TYPE_INT) {
            if (strcmp(pacifyCurrentState->options[i].name, optionName) == 0) {
                return pacifyCurrentState->options[i].integer;
            }
        }
    }

    pacify_option_int_set(optionName, default_value);
    return default_value;
}

double
pacify_option_double_get(char *optionName, double default_value)
{
    uint i;

    for (i = 0; i < pacifyCurrentState->optionAmount; i++) {
        if (pacifyCurrentState->options[i].type == PACIFY_TYPE_DOUBLE) {
            if (strcmp(pacifyCurrentState->options[i].name, optionName) == 0) {
                return pacifyCurrentState->options[i].decimal;
            }
        }
    }

    pacify_option_double_set(optionName, default_value);
    return default_value;
}

char *
pacify_option_text_get(char *optionName, char *default_value)
{
    uint i;

    for (i = 0; i < pacifyCurrentState->optionAmount; i++) {
        if (pacifyCurrentState->options[i].type == PACIFY_TYPE_TEXT) {
            if (strcmp(pacifyCurrentState->options[i].name, optionName) == 0) {
                return pacifyCurrentState->options[i].text;
            }
        }
    }

    pacify_option_text_set(optionName, default_value);
    return default_value;
}

void
pacify_option_int_set(char *optionName, int optionValue)
{
    uint i;

    for (i = 0; i < pacifyCurrentState->optionAmount; i++) {
        if (pacifyCurrentState->options[i].type == PACIFY_TYPE_INT) {
            if (strcmp(pacifyCurrentState->options[i].name, optionName) == 0) {
                pacifyCurrentState->options[i].integer = optionValue;
                return;
            }
        }
    }

    pacifyCurrentState =
        realloc(pacifyCurrentState,
                sizeof(pacifyState) +
                sizeof(option) * pacifyCurrentState->optionAmount++);
    pacifyCurrentState->options[pacifyCurrentState->optionAmount - 1].type =
        PACIFY_TYPE_INT;
    pacifyCurrentState->options[pacifyCurrentState->optionAmount - 1].name =
        malloc(sizeof(char) * (strlen(optionName) + 1));
    strcpy(pacifyCurrentState->options[pacifyCurrentState->optionAmount - 1].
           name, optionName);
    pacifyCurrentState->options[pacifyCurrentState->optionAmount - 1].integer =
        optionValue;
}

void
pacify_option_double_set(char *optionName, double optionValue)
{
    uint i;

    for (i = 0; i < pacifyCurrentState->optionAmount; i++) {
        if (pacifyCurrentState->options[i].type == PACIFY_TYPE_DOUBLE) {
            if (strcmp(pacifyCurrentState->options[i].name, optionName) == 0) {
                pacifyCurrentState->options[i].decimal = optionValue;
                return;
            }
        }
    }

    pacifyCurrentState =
        realloc(pacifyCurrentState,
                sizeof(pacifyState) +
                sizeof(option) * pacifyCurrentState->optionAmount++);
    pacifyCurrentState->options[pacifyCurrentState->optionAmount - 1].type =
        PACIFY_TYPE_DOUBLE;
    pacifyCurrentState->options[pacifyCurrentState->optionAmount - 1].name =
        malloc(sizeof(char) * (strlen(optionName) + 1));
    strcpy(pacifyCurrentState->options[pacifyCurrentState->optionAmount - 1].
           name, optionName);
    pacifyCurrentState->options[pacifyCurrentState->optionAmount - 1].decimal =
        optionValue;
}

void
pacify_option_text_set(char *optionName, char *optionValue)
{
    uint i;

    for (i = 0; i < pacifyCurrentState->optionAmount; i++) {
        if (pacifyCurrentState->options[i].type == PACIFY_TYPE_TEXT) {
            if (strcmp(pacifyCurrentState->options[i].name, optionName) == 0) {
                strcpy(pacifyCurrentState->options[i].text, optionValue);
                return;
            }
        }
    }

    pacifyCurrentState =
        realloc(pacifyCurrentState,
                sizeof(pacifyState) +
                sizeof(option) * pacifyCurrentState->optionAmount++);
    pacifyCurrentState->options[pacifyCurrentState->optionAmount - 1].type =
        PACIFY_TYPE_TEXT;
    pacifyCurrentState->options[pacifyCurrentState->optionAmount - 1].name =
        malloc(sizeof(char) * (strlen(optionName) + 1));
    strcpy(pacifyCurrentState->options[pacifyCurrentState->optionAmount - 1].
           name, optionName);
    pacifyCurrentState->options[pacifyCurrentState->optionAmount - 1].text =
        malloc(sizeof(char) * (strlen(optionValue) + 1));
    strcpy(pacifyCurrentState->options[pacifyCurrentState->optionAmount - 1].
           text, optionValue);
}
