#include "crow.h"
#include "FileManager.h"
#include "JobMatcher.h"
#include "cmath"
#include <fstream>
#include <sstream>           //can identify different data types
#include <iomanip>
#include <cstdlib>           //imports system
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

int main()
{
    crow::SimpleApp app; // creats web server object

    CROW_ROUTE(app, "/")([]()
                         { return "Resume Matcher Server Running"; });   //checks live server

    CROW_ROUTE(app, "/upload").methods("POST"_method)([](const crow::request &req)
                                                      {

        try{
        crow::multipart::message msg(req);          //Extracts file. multipart used for file uploads and complex form submission

        if (msg.parts.empty())
        {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "No file uploaded";
            return crow::response(400, error);
        }

        fs::create_directories("uploads");
        fs::create_directories("extracted");

        std::string savedPath = "uploads/resume.pdf";       //saves the path
        std::string txtPath = "extracted/resume.txt";

        for (auto &part : msg.parts)
        {                                                       //opens file in binary and writes pdf data
            std::ofstream out(savedPath, std::ios::binary);
            out.write(part.body.c_str(), part.body.size());     //saves as resume.pdf
            out.close();
        }

        std::string command =
    "/usr/bin/pdftotext -layout \"" + savedPath + "\" \"" + txtPath + "\"";     //converts pdf to text

        if (system(command.c_str()) != 0)                                        //executes it
        {
            crow::json::wvalue error;
            error["success"] = false;                                            //error if failed
            error["message"] = "PDF extraction failed";
            return crow::response(500, error);
        }

        FileManager fm;
        JobMatcher matcher;

        Candidate candidate = fm.parseUnstructuredResume(txtPath);                //reads name, email and skills

        // Store all role results
        std::vector<std::pair<std::string, MatchResult>> allResults;

        std::string bestRole;
        double bestScore = -1;
        MatchResult bestMatch;

        // Iterate over all job files dynamically
        for (const auto& entry : fs::directory_iterator("../job_profiles"))
        {
            std::string filePath = entry.path().string();

            Job job = fm.loadJobFromFile(filePath);
            MatchResult match = matcher.evaluateCandidate(candidate, job);

            allResults.push_back({job.getJobTitle(), match});

            if (match.percentage > bestScore)
            {
                bestScore = match.percentage;
                bestRole = job.getJobTitle();
                bestMatch = match;
            }
        }

        // Sort roles by percentage descending
        std::sort(allResults.begin(), allResults.end(),
                  [](const auto& a, const auto& b)
                  {
                      return a.second.percentage > b.second.percentage;
                  });

        // Keep only top 3
        int topLimit = std::min(3, (int)allResults.size());

        crow::json::wvalue response;                                               //Creates a json object.response is the name if the variable
        response["success"] = true;

        crow::json::wvalue data;

        data["name"] = candidate.getName();
        data["email"] = candidate.getEmail();

        // Send top 3 comparison results
        for (int i = 0; i < topLimit; ++i)
        {
            data["comparison"][allResults[i].first] =
                allResults[i].second.percentage;
        }

        data["best_role"] = bestRole;
        data["percentage"] = bestMatch.percentage;
        data["fit_category"] = bestMatch.fitCategory;

        // Matched Skills (Best Role Only)
        int i = 0;
        for (const auto &skill : bestMatch.matchedSkills)
        {
            data["matched_skills"][i++] = skill;
        }

        // Missing Skills (Best Role Only - already sorted by weight)
        int j = 0;
        for (const auto &skill : bestMatch.missingSkills)
        {
            data["missing_skills"][j++] = skill.first;   // only skill name
        }

        response["data"] = std::move(data);

        auto res = crow::response(200, response);
        res.add_header("Access-Control-Allow-Origin", "*");                     //Allows React frontend to call backend.
        return res;
        }

        catch (...)
        {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = "Internal server error";

            return crow::response(500, error);
        } });

int port = 18080;
if (const char* env_p = std::getenv("PORT"))
{
    port = std::stoi(env_p);
}

app.port(port).multithreaded().run();
}