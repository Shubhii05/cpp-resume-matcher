#include "JobMatcher.h"
#include <algorithm>
#include <cmath>

using namespace std;

string JobMatcher::getFitCategory(double percentage) // checks per and give fit
{
    if (percentage >= 80)
        return "Excellent Fit";
    if (percentage >= 70)
        return "Strong Fit";
    if (percentage >= 30)
        return "Moderate Fit";
    return "Weak Fit";
}

MatchResult JobMatcher::evaluateCandidate( // struct
    const Candidate &candidate,
    const Job &job)
{
    MatchResult result{};
    result.rawScore = 0;
    result.maxScore = 0;

    const Resume &resume = candidate.getResume();
    const vector<string> &candidateSkills = resume.getSkills();
    const set<string> &mandatorySkills = job.getMandatorySkills();
    const map<string, int> &jobSkills = job.getRequiredSkills();

    // ====================================================
    // 1️ Mandatory Skill Check (UNCHANGED)
    // ====================================================
    for (const auto &mandatory : mandatorySkills)
    {
        if (find(candidateSkills.begin(),
                 candidateSkills.end(),
                 mandatory) == candidateSkills.end())
        {
            result.percentage = 0;
            result.fitCategory = "Rejected";
            return result;
        }
    }

    // ==============================
    //  Missing Skills Detection
    // ==============================

    for (const auto &jobSkill : jobSkills)
    {
        if (find(candidateSkills.begin(),
                 candidateSkills.end(),
                 jobSkill.first) == candidateSkills.end())
        {
            result.missingSkills.push_back(
                {jobSkill.first, jobSkill.second});
        }
    }
    sort(result.missingSkills.begin(),
         result.missingSkills.end(),
         [](const pair<string, int> &a,
            const pair<string, int> &b)
         {
             return a.second > b.second; // descending weight
         });

    // ====================================================
    // 2️ Weighted Skill Scoring (UNCHANGED)
    // ====================================================
    for (const auto &skill : candidateSkills)
    {
        auto it = jobSkills.find(skill);
        if (it != jobSkills.end())
        {
            result.rawScore += it->second;
            result.matchedSkills.push_back(skill);
        }
    }

    for (const auto &skill : jobSkills)
        result.maxScore += skill.second;

    // ====================================================
    // 3️ Certification Bonus
    // ====================================================
    int certCount = resume.getCertifications().size();
    int certBonus = certCount * 3; // 3 points per certification

    result.rawScore += certBonus;

    // Assume max 5 certifications contribute
    result.maxScore += 15;

    // ====================================================
    // 4️ Education Bonus
    // ====================================================
    if (!resume.getEducation().empty())
        result.rawScore += 5;

    result.maxScore += 5;

    // ====================================================
    // 5️ Project Relevance Bonus
    // ====================================================
    int projectBonus = 0;

    for (const string &project : resume.getProjects())
    {
        string lowerProject = project;
        transform(lowerProject.begin(), lowerProject.end(),
                  lowerProject.begin(), ::tolower);

        for (const auto &jobSkill : jobSkills)
        {
            if (lowerProject.find(jobSkill.first) != string::npos)
            {
                projectBonus += 4; // 4 points if project mentions required skill
                break;
            }
        }
    }

    result.rawScore += projectBonus;

    // Max possible project contribution
    result.maxScore += jobSkills.size() * 4;

    // ====================================================
    // 6️ Final Percentage Calculation
    // ====================================================
    if (result.maxScore > 0)
        result.percentage = round((result.rawScore * 100.0 / result.maxScore) * 100) / 100;

    result.fitCategory = getFitCategory(result.percentage);

    return result;
}