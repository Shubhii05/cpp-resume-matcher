#include "JobMatcher.h"
#include <algorithm>
#include <iostream>
#include <cmath>

using namespace std;

string toLowerCopy(string str)
{
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

string JobMatcher::getFitCategory(double percentage)
{
    if (percentage >= 75)
        return "Excellent Fit";
    if (percentage >= 50)
        return "Strong Fit";
    if (percentage >= 35)
        return "Moderate Fit";
    return "Weak Fit";
}

MatchResult JobMatcher::evaluateCandidate(const Candidate &candidate, const Job &job)
{
    MatchResult result{};
    result.rawScore = 0;
    result.projectBonus = 0;
    result.certificationBonus = 0;
    result.educationBonus = 0;

    const Resume &resume = candidate.getResume();
    const vector<string> &candidateSkills = resume.getSkills();
    const set<string> &mandatorySkills = job.getMandatorySkills();
    cout << "Mandatory Skills From Job:" << endl;
for (const auto &m : mandatorySkills)
{
    cout << "[" << m << "]" << endl;
}

    const map<string, int> &jobSkills = job.getRequiredSkills();

    cout << "Detected Skills Count: " << candidateSkills.size() << endl;
    cout << "Job Skills Count: " << jobSkills.size() << endl;
    for (const auto &s : candidateSkills)
{
    cout << "Candidate Skill: [" << s << "]" << endl;
}


    // Mandatory Check
    for (const auto &mandatory : mandatorySkills)
    {
        if (find(candidateSkills.begin(),
                 candidateSkills.end(),
                 mandatory) == candidateSkills.end())
        {
            cout << "Candidate is missing mandatory Skills." << endl;
            return result;
        }
    }

    // Weighted Skill Matching
    for (const auto &skill : candidateSkills)
    {
        auto it = jobSkills.find(skill);
        if (it != jobSkills.end())
        {
            result.rawScore += it->second;
            result.matchedSkills.push_back(
                skill + " (+" + to_string(it->second) + ")");
        }
    }

// Calculate max score
for (const auto &skill : jobSkills)
    result.maxScore += skill.second;

result.maxScore += 30; // bonus allowance

if (result.maxScore > 0)
{
    double rawPercent =
        (double(result.rawScore) / result.maxScore) * 100.0;

    result.percentage = round(rawPercent * 100.0) / 100.0;
}
else
{
    result.percentage = 0;
}

result.fitCategory = getFitCategory(result.percentage);

return result;

}