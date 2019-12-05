#include <stdlib.h>
#include <string.h>

#include "request.h"

static int request_count_arguments_non_optimised(char* argument, char delimiter)
{
    int words;
    int inWord;

    words = 0;
    inWord = 0;
    while (1) {
        if (*argument == delimiter || !*argument) {
            if (inWord) {
                inWord = 1;
                words++;
            }
            if (!*argument)
                break;
        } else
            inWord = 1;
        argument++;
    }
    return (words);
}

char** request_parse_arguments(char* arg)
{
    char** args;
    int wordCount;
    int i;
    char* s;

    if (strlen(arg) >= strlen(REQUEST_END)
        && strcmp(arg + strlen(arg) - strlen(REQUEST_END), REQUEST_END) == 0)
        arg[strlen(arg) - strlen(REQUEST_END)] = 0;
    if (strlen(arg) > 0)
        arg[strlen(arg) - 1]
            = arg[strlen(arg) - 1] == '\n' ? 0 : arg[strlen(arg) - 1];
    wordCount = request_count_arguments_non_optimised(arg, REQUEST_DELIMITER);
    if (!(args = malloc(sizeof(char*) * (wordCount + 1))))
        return (NULL);
    i = 0;
    while (i < wordCount) {
        s = strtok(i == 0 ? arg : NULL, REQUEST_DELIMITER_PTR);
        if (!s || !(args[i] = strdup(s)))
            return (NULL);
        i++;
    }
    args[i] = NULL;
    return (args);
}

char** request_parse_arguments_delimiter(char* arg, char delimiter)
{
    char** args;
    int wordCount;
    int i;
    char* s;

    if (strlen(arg) >= strlen(REQUEST_END)
        && strcmp(arg + strlen(arg) - strlen(REQUEST_END), REQUEST_END) == 0)
        arg[strlen(arg) - strlen(REQUEST_END)] = 0;
    wordCount = request_count_arguments_non_optimised(arg, delimiter);
    if (!(args = malloc(sizeof(char*) * (wordCount + 1))))
        return (NULL);
    i = 0;
    while (i < wordCount) {
        s = strtok(i == 0 ? arg : NULL, &delimiter);
        if (!s)
            return (NULL);
        if (!(args[i] = strdup(s)))
            return (NULL);
        i++;
    }
    args[i] = NULL;
    return (args);
}

void request_free_arguments(char** arguments)
{
    char** arguments_ptr;

    arguments_ptr = arguments;
    if (!arguments)
        return;
    while (*arguments_ptr) {
        free(*arguments_ptr);
        arguments_ptr++;
    }
    free(arguments);
}

int request_count_arguments(char** arguments)
{
    int i;

    i = 0;
    while (*arguments) {
        arguments++;
        i++;
    }
    return (i);
}
