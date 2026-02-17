#include <iostream>
#include "FileManager.h"
#include "JobMatcher.h"
#include "Candidate.h"
#include "Job.h"

using namespace std;

int main()
{
    cout << "=========================================\n";
    cout << "        Resume Screening Report\n";
    cout << "=========================================\n";

    FileManager fileManager;

    // 🔹 Load Resume
    Candidate candidate =
        fileManager.parseUnstructuredResume("resumes/my_resume.txt");

    // 🔹 Load Job
    Job job = fileManager.loadJobFromFile("job_profiles/job_ml.txt");

    // 🔹 Get Resume Object
    const Resume &resume = candidate.getResume();

    // 🔹 Print Candidate Info
    cout << "Candidate Name: " << candidate.getName() << endl;
    cout << "Email: " << candidate.getEmail() << endl;
    cout << "Phone: " << resume.getPhoneNumber() << endl;

    cout << "\n-----------------------------------------\n";

    // // 🔹 Display Skills
    // cout << "\nSkills Detected:\n";
    // for (const auto &skill : resume.getSkills())
    // {
    //     cout << "- " << skill << endl;
    // }

    // // 🔹 Display Projects
    // cout << "\nProjects:\n";
    // if (!resume.getProjects().empty())
    // {
    //     for (const auto &project : resume.getProjects())
    //     {
    //         cout << "- " << project << endl;
    //     }
    // }
    // else
    // {
    //     cout << "No Projects Found.\n";
    // }

    // // 🔹 Display Education
    // cout << "\nEducation:\n";
    // if (!resume.getEducation().empty())
    // {
    //     for (const auto &edu : resume.getEducation())
    //     {
    //         cout << "- " << edu << endl;
    //     }
    // }
    // else
    // {
    //     cout << "No Education Data Found.\n";
    // }

    // // 🔹 Display Certifications
    // cout << "\nCertifications:\n";
    // if (!resume.getCertifications().empty())
    // {
    //     for (const auto &cert : resume.getCertifications())
    //     {
    //         cout << "- " << cert << endl;
    //     }
    // }
    // else
    // {
    //     cout << "No Certifications Found.\n";
    // }

    cout << "\n=========================================\n";
    cout << "            Job Matching Result\n";
    cout << "=========================================\n\n";

    cout << "Applied For: " << job.getJobTitle() << endl;

    // 🔹 Evaluate Candidate
    JobMatcher matcher;
    MatchResult result = matcher.evaluateCandidate(candidate, job);

    cout << "\n-----------------------------------------\n";

    // 🔹 Matched Skills
    if (!result.matchedSkills.empty())
    {
        cout << "\nMatched Skills:\n";
        for (const auto &skill : result.matchedSkills)
        {
            cout << "- " << skill << endl;
        }
    }
    else
    {
        cout << "\nNo direct skill match.\n";
    }

    // 🔹 Bonus Breakdown
    cout << "\n--- Bonus Breakdown ---\n";
    cout << "Project Bonus: +" << result.projectBonus << endl;
    cout << "Certification Bonus: +" << result.certificationBonus << endl;
    cout << "Education Bonus: +" << result.educationBonus << endl;

    // 🔹 Final Score
    cout << "\n--- Final Result ---\n";
    cout << "Final Score: "
         << result.rawScore << " / "
         << result.maxScore << endl;

    cout << "Match Percentage: "
         << result.percentage << "%" << endl;

    cout << "Fit Level: "
         << matcher.getFitCategory(result.percentage) << endl;

    cout << "\n=========================================\n";

    return 0;
}
