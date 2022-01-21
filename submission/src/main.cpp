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
    // Check the length of arguments. We need input and output files
    if (argc < 3)
    {
        std::cout << "Incorrect use!" << std::endl;
        return -1;
    }

    // Get filenames from arguments
    std::string input_filename = argv[1];
    std::string output_filename = argv[2];

    // Open the input file
    std::ifstream input_file(input_filename);

    std::string first_line;
    std::string second_line;

    getline(input_file, first_line);  // Read the first line
    std::stringstream ss(first_line); // I am using stringstream to convert "1 2 3" to three integer variables
    int number_of_threads, number_of_abstracts, number_of_result;
    ss >> number_of_threads;
    ss >> number_of_abstracts;
    ss >> number_of_result;

    getline(input_file, second_line); // Read the second line

    std::vector<std::string> abstract_filenames; // I am using vector of strings to store abstract filenames
    for (int i = 0; i < number_of_abstracts; i++)
    {
        std::string t;
        getline(input_file, t);
        abstract_filenames.push_back(t);
    }
    input_file.close(); // I am done reading input file

    std::vector<pthread_t> threads(number_of_threads); //to store threads

    // I have to be sure that argument structs always will be available by the threads. I am creating argument structs before creating threads and deleting them after all the threads exit.
    std::vector<arg_struct *> args_addresses; //

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // I am usıng mutex locks to make sure only one thread is in critical region
    pthread_mutex_t lock;
    pthread_mutex_init(&lock, NULL);
    pthread_mutex_t file_lock;
    pthread_mutex_init(&file_lock, NULL);
    pthread_mutex_t vector_lock;
    pthread_mutex_init(&vector_lock, NULL);

    int index = 0;
    for (int i = 0; i < number_of_threads; i++)
    {
        struct arg_struct *args = new struct arg_struct;       // Initialize argument struct for each thread
        args->filenames = &abstract_filenames;                 // Vector of abstract filenames since we will read the files in threads
        args->index = &index;                                  // Index of the last used filename
        args->lock = &lock;                                    // Mutex lock for index checks
        args->file_lock = &file_lock;                          // Mutex lock for opening output file
        args->vector_lock = &vector_lock;                      // Mutex lock for pushing back to vector
        args->query = second_line;                             // Given query
        args->name = i + 65;                                   // Name of the threads, starting from A
        args->output_filename = argv[2];                       // Output filename
        args_addresses.push_back(args);                        // Add the pointer to args_addresses vector
        pthread_create(&threads.at(i), &attr, *analyze, args); // Create the thread
    }
    // I have created all the threads. We should wait all of them to end.
    for (int i = 0; i < number_of_threads; i++)
    {
        pthread_join(threads.at(i), NULL);
        if (args_addresses[i] != nullptr) // Delete the arguments of exited threads
            delete args_addresses[i];
    }

    // We are done with threads so we can destroy mutex locks
    pthread_mutex_destroy(&lock);
    pthread_mutex_destroy(&file_lock);
    pthread_mutex_destroy(&vector_lock);

    // Store the results in descending order, according to their scores
    std::sort(Result::results.begin(), Result::results.end(), std::greater<>());
    std::ofstream outfile(output_filename, std::ios_base::app); // Open the output file in append mode!
    outfile << "###" << std::endl;
    for (int i = 0; i < number_of_result; i++)
    {
        Result *result = &Result::results.at(i);
        outfile << "Result " << i + 1 << ":" << std::endl;
        outfile << "File: " << result->filename << std::endl;
        outfile << "Score: " << std::fixed << std::setprecision(4) << (float)((result->score * 10000) / 10000) << std::endl; // Round the score to 4 precision
        outfile << "Summary: " << result->summary << std::endl;
        outfile << "###" << std::endl;
    }
    outfile.close(); // Close the output file, we are done!

    return 0;
}
