#ifndef JOBMATCHER_H                             //problem
#define JOBMATCHER_H

#include "Candidate.h"
#include "Job.h"

struct MatchResult{
    double percentage;
    int rawScore;
    int maxScore;
    int projectBonus;
    int certificationBonus;
    int educationBonus;
    std::vector<std::string> matchedSkills;
    std::string fitCategory;
};

class JobMatcher {
public:
    static std::string getFitCategory(double percentage);
    static MatchResult evaluateCandidate(const Candidate& candidate,const Job& job);
};

#endif
