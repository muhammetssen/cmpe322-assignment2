#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <vector>
#include <sstream>
#include <pthread.h>
#include <algorithm>
#include "analyzer.h"
#include "result.h"
#include <iomanip>

int main(int argc, char const *argv[])
{
    // Read input filename
    if (argc < 3)
    {
        std::cout << "Incorrect use!\n"
                  << std::endl;
        return -1;
    }

    std::string input_filename = argv[1];
    std::string output_filename = argv[2];

    std::ifstream output_file(output_filename, std::ios::out | std::ofstream::trunc);
    output_file.close();
    std::ifstream input_file(input_filename);

    std::string first_line;
    std::string second_line;

    getline(input_file, first_line);
    std::stringstream ss(first_line);
    int number_of_threads, number_of_abstracts, number_of_result;
    ss >> number_of_threads;
    ss >> number_of_abstracts;
    ss >> number_of_result;

    getline(input_file, second_line);

    std::vector<std::string> abstract_filenames;
    for (int i = 0; i < number_of_abstracts; i++)
    {
        std::string t;
        getline(input_file, t);
        abstract_filenames.push_back(t);
    }
    input_file.close();

    std::vector<pthread_t> threads(number_of_threads);
    std::vector<arg_struct *> args_addresses;
    pthread_attr_t attr;
    // setvbuf(stdout, NULL, _IONBF, 0);
    pthread_attr_init(&attr);
    pthread_mutex_t lock;
    pthread_mutex_init(&lock, NULL);
    pthread_mutex_t file_lock;
    pthread_mutex_init(&file_lock, NULL);

    int index = 0;
    for (int i = 0; i < number_of_threads; i++)
    {
        struct arg_struct *args = new struct arg_struct;
        args->filenames = &abstract_filenames;
        args->index = &index;
        args->lock = &lock;
        args->file_lock = &file_lock;
        args->query = second_line;
        args->name = i + 65;
        args->output_filename = argv[2];
        args_addresses.push_back(args);
        pthread_create(&threads.at(i), &attr, *analyze, args);
        std::cout << "thread creating " << i << std::endl;
    }
    for (int i = 0; i < number_of_threads; i++)
    {
        std::cout << "thread joined " << i << std::endl;
        pthread_join(threads.at(i), NULL);
    }
    for (int i = 0; i < number_of_threads; i++)
    {
        if (args_addresses[i] != nullptr)
            delete args_addresses[i];
    }
    pthread_mutex_destroy(&lock);
    pthread_mutex_destroy(&file_lock);

    std::sort(Result::results.begin(), Result::results.end(), std::greater<>());
    std::ofstream outfile(argv[2], std::ios_base::app);
    outfile << "###" << std::endl;
    for (int i = 0; i < number_of_result; i++)
    {
        Result *result = &Result::results.at(i);
        outfile << "Result " << i + 1 << ":" << std::endl;
        outfile << "File: " << result->filename << std::endl;
        outfile << "Score: " << std::fixed << std::setprecision(4) << (float)((result->score * 10000) / 10000) << std::endl;
        outfile << "Summary: " << result->summary << std::endl;
        outfile << "###" << std::endl;
    }
    outfile.close();

    return 0;
}
