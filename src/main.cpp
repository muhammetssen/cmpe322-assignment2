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

    std::ifstream input_file("input.txt"); 

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

    std::vector<std::vector<std::string>> abstract_filenames_seperated(number_of_threads);
    for (int i = 0; i < number_of_threads; i++)
    {
        abstract_filenames_seperated[i] = std::vector<std::string>();
    }

    for (int i = 0; i < number_of_abstracts / number_of_threads; i++)
    {
        for (int j = 0; j < number_of_threads; j++)
        {
            abstract_filenames_seperated[j].push_back(abstract_filenames.at(i * number_of_threads + j));
            // std::cout << "adding " <<abstract_filenames.at(i * number_of_threads + j) << " to thread " << j <<  std::endl;
        }
    }


    std::vector<pthread_t> threads(number_of_threads);
    std::vector<arg_struct*> args_addresses(number_of_threads);
    pthread_attr_t attr;
    setvbuf(stdout, NULL, _IONBF, 0);
    pthread_attr_init(&attr);

    std::string t = "input.txt";
    for (int i = 0; i < number_of_threads; i++)
    {
    struct arg_struct* args = new struct arg_struct;
    args->filenames =  &abstract_filenames_seperated[i];
    args->query = second_line;
    args->name = i + 65;
    args->output_filename = argv[2];
    args_addresses.push_back(args);
    pthread_create(&threads.at(i), &attr, *analyze, args);
        
    }
    for (int i = 0; i < number_of_threads; i++)
    {
    pthread_join(threads.at(i),NULL);    
    delete args_addresses[i];
    }
    std::sort(Result::results.begin(),Result::results.end(),std::greater<>());
    std::ofstream outfile(argv[2] ,std::ios_base::app);
    outfile << "###" << std::endl;
    for (int i = 0; i < number_of_result; i++)
    {
        Result* result = &Result::results.at(i);
        outfile << "Result " << i+1 << ":" << std::endl;
        outfile << "File: " << result->filename << std::endl;
        outfile << "Score: " << std::fixed << std::setprecision(4) <<(float) ((result->score * 10000) / 10000) << std::endl;
        outfile << "Summary: " << result->summary <<  std::endl;
        outfile << "###" << std::endl;
        

    }
    outfile.close();

    return 0;
}

