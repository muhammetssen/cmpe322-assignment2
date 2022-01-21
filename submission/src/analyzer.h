#ifndef ANALYZER_H
#define ANALYZER_H

#include <pthread.h>
#include "result.h"
struct arg_struct
{
    std::vector<std::string> *filenames;
    int *index;
    std::string query;
    char name; // name of the thread
    std::string output_filename;
    pthread_mutex_t *lock;
    pthread_mutex_t *file_lock;
    pthread_mutex_t *vector_lock;
};
void *analyze(void *args);
void tokenize(std::string const &str, const char delim,
              std::vector<std::string> &out);
#endif