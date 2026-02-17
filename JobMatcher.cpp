#include "JobMatcher.h"
#include <algorithm>
#include <iostream>

using namespace std;

// tolowercopy is a function to convert string in lower case
string toLowerCopy(string str)
{
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

// Fit Category
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

// ==========================
// Main Matching Function
// ==========================
MatchResult JobMatcher::evaluateCandidate(const Candidate &candidate, const Job &job)
{
    MatchResult result;
    result.rawScore = 0;
    result.projectBonus = 0;
    result.certificationBonus = 0;
    result.educationBonus = 0;
    result.matchedSkills.clear();

    const Resume &resume = candidate.getResume();
    const vector<string> &candidateSkills = resume.getSkills();
    const set<string> &mandatorySkills = job.getMandatorySkills();
    const map<string, int> &jobSkills = job.getRequiredSkills();

    cout << "\nDEBUG Candidate Skills:\n";
    for (auto &s : candidateSkills)
        cout << "[" << s << "]\n";

    cout << "\nDEBUG Mandatory Skills:\n";
    for (auto &m : mandatorySkills)
        cout << "[" << m << "]\n";

    // -------------------------
    // STEP 1: Mandatory Check
    // -------------------------
    for (const auto &mandatory : mandatorySkills)
    {
        if (find(candidateSkills.begin(), candidateSkills.end(), mandatory) == candidateSkills.end())
        {

            result.percentage = 0;
            result.maxScore = 0;
            return result; // Immediate rejection
        }
    }

    // -------------------------
    // STEP 2: Weighted Skill Matching
    // -------------------------
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

    // -------------------------
    // STEP 3: Project Bonus
    // -------------------------
   

            // STEP 3: Project Bonus
            if (!resume.getProjects().empty() && !result.matchedSkills.empty())
            {
                result.projectBonus = 5;
                result.rawScore += 5;
            }
        
    

   // STEP 4: Certification Bonus
bool certMatched = false;

for (const auto &cert : resume.getCertifications())
{
    string lowerCert = toLowerCopy(cert);

    for (const auto &required : jobSkills)
    {
        if (lowerCert.find(required.first) != string::npos)
        {
            certMatched = true;
            break;
        }
    }

    if (certMatched)
        break;
}

if (certMatched)
{
    result.certificationBonus = 10;
    result.rawScore += 10;
}


    // -------------------------
    // STEP 5: Education Bonus
    // -------------------------
    for (const auto &edu : resume.getEducation())
    {

        string lowerEdu = toLowerCopy(edu);

        if (lowerEdu.find("computer") != string::npos ||
            lowerEdu.find("engineering") != string::npos ||
            lowerEdu.find("artificial") != string::npos)
        {

            result.educationBonus += 15;
            result.rawScore += 15;
            break;
        }
    }

    // -------------------------
    // STEP 6: Calculate Maximum Score
    // -------------------------
    result.maxScore = 0;

    for (const auto &skill : jobSkills)
    {
        result.maxScore += skill.second;
    }

    // Add possible bonuses
    result.maxScore += 15; // education
    result.maxScore += 10; // certification
    result.maxScore += 5;  // project

    // -------------------------
    // STEP 7: Percentage
    // -------------------------
    if (result.maxScore > 0)
        result.percentage =
            (double(result.rawScore) / result.maxScore) * 100.0;
    else
        result.percentage = 0;

    return result;
}
