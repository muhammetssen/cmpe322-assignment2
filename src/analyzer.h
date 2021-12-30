#ifndef ANALYZER_H
#define ANALYZER_H
#include "result.h"
struct arg_struct {
    std::vector<std::string>* filenames;
    int *index;
    std::string query;
    char name;
    std::string output_filename;
    bool *lock;
    bool *file_lock;

};
void* analyze(void* args);
void tokenize(std::string const &str, const char delim,
              std::vector<std::string> &out);
#endif