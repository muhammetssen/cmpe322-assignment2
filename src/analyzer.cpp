#include <string>
#include "analyzer.h"
#include "result.h"
#include "iostream"
#include <pthread.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <set>
#include <algorithm>

void *analyze(void *args_pointer)
{
    struct arg_struct *args = (struct arg_struct *)args_pointer;

    std::vector<std::string> *names = (std::vector<std::string> *)args->filenames;
    int number_of_abstracts = (int)names->size();

    std::vector<std::string> keywords;
    tokenize(args->query, ' ', keywords);
    std::set<std::string> unique_keywords(keywords.begin(), keywords.end());
    std::sort(keywords.begin(), keywords.end());
    std::string current_filename;

    std::cout << "starting to task" << std::endl;
    while (true)
    {
        // we will check index and close the thread if no more abstract file to read.
        // Since we will use index variable, we should go into critical region
        // while (*args->lock)
        // {
        //     // wait until lock is released
        // }
        // *args->lock = true; // this thread has the right to go into critical region
        pthread_mutex_lock(args->lock);

        // std::cout << "thread has the lock " << (int)(args->name - 65) << std::endl;

        int temp = *args->index;
        // std::cout << args->name << " with index" << temp << std::endl;
        *(args->index) += 1;

        if (*args->index > number_of_abstracts)
        { // No more abstract to process. we can close this thread

            // std::cout << "thread closing " << (int)(args->name - 65) << std::endl;
            pthread_mutex_unlock(args->lock);

            pthread_exit(0);
        }
        pthread_mutex_unlock(args->lock);

        // there is still more abstracts files to process, get one and increment index
        current_filename = names->at(temp);
        // std::cout << "Starting with index " << temp << std::endl;

        float score;
        std::string summary;
        std::ifstream abstract_file("../abstracts/" + current_filename);
        // std::stringstream buffer;
        // buffer << input_file.rdbuf();
        std::string content((std::istreambuf_iterator<char>(abstract_file)),
                            (std::istreambuf_iterator<char>()));

        abstract_file.close();
        // std::cout << << std::endl;
        // while (*args->file_lock) // We will write to a file; hence we need critical region
        // {
        // }

        // *args->file_lock = true;
        pthread_mutex_lock(args->file_lock);

        std::ofstream outfile(args->output_filename, std::ios_base::app);
        outfile << "Thread " << args->name << " is calculating " << current_filename << std::endl;
        outfile.close();
        // *args->file_lock = false;
        pthread_mutex_unlock(args->file_lock);

        content.erase(std::remove(content.begin(), content.end(), '\n'), content.end());

        std::vector<std::string> sentences;
        tokenize(content, '.', sentences);
        for (std::string &sentence : sentences)
        {
            std::vector<std::string> words_splitted;
            tokenize(sentence, ' ', words_splitted);
            std::set<std::string> unique_words(words_splitted.begin(), words_splitted.end());
            std::sort(words_splitted.begin(), words_splitted.end());
            std::set<std::string> intersect;
            std::set_intersection(words_splitted.begin(), words_splitted.end(), keywords.begin(), keywords.end(),
                                  std::inserter(intersect, intersect.begin()));
            if (intersect.size() > 0)
            {
                if (sentence.front() == ' ')
                {
                    sentence = sentence.substr(1);
                }
                summary += sentence + ". ";
            }
        }

        std::vector<std::string> words_splitted;
        tokenize(content, ' ', words_splitted);
        std::set<std::string> unique_words(words_splitted.begin(), words_splitted.end());
        std::sort(words_splitted.begin(), words_splitted.end());
        std::set<std::string> intersect;
        std::set_intersection(words_splitted.begin(), words_splitted.end(), keywords.begin(), keywords.end(),
                              std::inserter(intersect, intersect.begin()));

        std::set<std::string> unions;
        std::set_union(words_splitted.begin(), words_splitted.end(), keywords.begin(), keywords.end(),
                       std::inserter(unions, unions.begin()));

        score = (float)intersect.size() / (float)unions.size();
        // std::cout << "Number of intersec with " << filename << "is " << intersect.size() << " with score " << score << summary<< std::endl;

        Result::results.push_back(Result(summary, score, current_filename));
    }
}

void tokenize(std::string const &str, const char delim,
              std::vector<std::string> &out)
{
    // construct a stream from the string
    std::stringstream ss(str);
    std::string s;

    while (std::getline(ss, s, delim))
    {
        out.push_back(s);
    }
}