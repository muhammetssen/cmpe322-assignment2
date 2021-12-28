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

    std::vector<std::string> keywords;
    tokenize(args->query, ' ', keywords);
    std::set<std::string> unique_keywords(keywords.begin(), keywords.end());
    std::sort(keywords.begin(), keywords.end());

    // std::cout << args->query << std::endl;

    for (std::string &filename : *names)
    {
        float score;
        std::string summary;
        std::ifstream abstract_file("../abstracts/"+filename);
        // std::stringstream buffer;
        // buffer << input_file.rdbuf();
        std::string content((std::istreambuf_iterator<char>(abstract_file)),
                            (std::istreambuf_iterator<char>()));

        abstract_file.close();
        // std::cout << << std::endl;
        std::ofstream outfile(args->output_filename, std::ios_base::app);
        outfile << "Thread " << args->name << " is calculating " << filename << std::endl;
        outfile.close();

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
        Result::results.push_back(Result(summary, score, filename));
    }

    pthread_exit(0);
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