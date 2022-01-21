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
    struct arg_struct *args = (struct arg_struct *)args_pointer; // Typecast void* to struct arg_struct*

    std::vector<std::string> *names = (std::vector<std::string> *)args->filenames; // save the pointer of filenames since we will use it frequently
    int number_of_abstracts = (int)names->size();                                  // this does not change through the program so we can save it in a separate variable

    // Split keywords in the query
    std::vector<std::string> keywords;
    tokenize(args->query, ' ', keywords);
    // convert to set and sort, we will need this in intersection and union
    std::set<std::string> unique_keywords(keywords.begin(), keywords.end());
    std::sort(keywords.begin(), keywords.end());

    std::string current_filename; // If this is lucky enough, it can get the lock and an abstract

    while (true)
    {
        // we will check index and close the thread if no more abstract file to read.
        // Since we will use index variable, we should go into critical region
        pthread_mutex_lock(args->lock);
        int temp = *args->index; // store the current index in a temporary variable
        *(args->index) += 1;     // Increment the index since this thread got one

        if (*args->index > number_of_abstracts) // If the index thread has is greater than number_of_abstracts, all the files are processed. We can close this thread.
        {
            pthread_mutex_unlock(args->lock); // release the lock before exitting
            pthread_exit(0);                  // close the thread with 0
        }
        pthread_mutex_unlock(args->lock); // release the lock since we are done with index variable

        current_filename = names->at(temp); // get the filename

        float score;
        std::string summary;

        std::ifstream abstract_file("../abstracts/" + current_filename);
        // read all of the abstract
        std::string content((std::istreambuf_iterator<char>(abstract_file)),
                            (std::istreambuf_iterator<char>()));
        abstract_file.close();

        // we will open the output file which is used by almost all the threads. We need to have the lock to do that
        pthread_mutex_lock(args->file_lock);

        std::ofstream outfile(args->output_filename, std::ios_base::app);                          // Open the output file in append mode
        outfile << "Thread " << args->name << " is calculating " << current_filename << std::endl; // Print the log "Thread X is calculating abstract_X.txt"
        outfile.close();

        pthread_mutex_unlock(args->file_lock); // Release the lock, we are done with the file

        content.erase(std::remove(content.begin(), content.end(), '\n'), content.end()); // remove all new lines since we are separating according to dots/

        // Split the abstract to sentences
        std::vector<std::string> sentences;
        tokenize(content, '.', sentences);

        std::set<std::string> intersect_all; // intesection of the whole abstract
        std::set<std::string> unions_all;    // union of the whole absract

        for (std::string &sentence : sentences)
        {
            // Split the sentence to words
            std::vector<std::string> words_splitted;
            tokenize(sentence, ' ', words_splitted);

            std::sort(words_splitted.begin(), words_splitted.end()); 

            // Get the intersection of keywords and words in sentence
            std::set<std::string> intersect;
            std::set_intersection(words_splitted.begin(), words_splitted.end(), keywords.begin(), keywords.end(), // get the intersection of words and keywords. Then, insert those to intersect set
                                  std::inserter(intersect, intersect.begin()));
            if (intersect.size() > 0) // Add the sentence to summary if at least one keyword is in the sentence
            {
                if (sentence.front() == ' ') // If sentence starts with a whitespace, remove it since we are adding 2 lines below
                    sentence = sentence.substr(1);
                summary += sentence + ". ";
            }
            std::set<std::string> unions;
            std::set_union(words_splitted.begin(), words_splitted.end(), keywords.begin(), keywords.end(), // get the union of words and keywords. Then, insert those to union set
                           std::inserter(unions, unions.begin()));
            intersect_all.insert(intersect.begin(), intersect.end());
            unions_all.insert(unions.begin(), unions.end());
        }

        score = (float)intersect_all.size() / (float)unions_all.size();

        pthread_mutex_lock(args->vector_lock); // only one thread can push to the result vector

        Result::results.push_back(Result(summary, score, current_filename)); // save the result to Result::results vector
        pthread_mutex_unlock(args->vector_lock);
    }
}

// Very similar version of https://stackoverflow.com/q/14265581/12277831

// Tokenize a given string according to given delimeter
void tokenize(std::string const &str, const char delim,
              std::vector<std::string> &out)
{
    // construct a stream from the string
    std::stringstream ss(str);
    std::string s;

    while (std::getline(ss, s, delim)) // Search for delimeter
        out.push_back(s);              // insert the found part
}