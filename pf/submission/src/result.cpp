#include <string>
#include "result.h"

std::vector<Result> Result::results = std::vector<Result>();
Result::Result(std::string summary, float score, std::string filename)
{
    this->summary = summary;
    this->score = score;
    this->filename = filename;
}

// Since I am comparing Result objects according to their scores
bool operator<(const Result &c1, const Result &c2)
{
    return c1.score < c2.score;
}

bool operator>(const Result &c1, const Result &c2)
{
    return c1.score > c2.score;
}
