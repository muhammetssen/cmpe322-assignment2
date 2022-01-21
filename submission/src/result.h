#ifndef RESULT_H
#include <string>
#include <vector>
#define RESULT_H

class Result
{
public:
    Result(std::string summary, float score, std::string filename);
    std::string summary;
    std::string filename;
    float score;


    friend bool operator< (const Result& c1, const Result& c2);
    friend bool operator> (const Result& c1, const Result& c2);
    static std::vector<Result> results; // Will store all the results in this vector
};

#endif
