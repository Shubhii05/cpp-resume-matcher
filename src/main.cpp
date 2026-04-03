#include "crow.h"
#include "FileManager.h"
#include "JobMatcher.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <filesystem>
#include <algorithm>
#include <chrono>
#include <thread>
#include <atomic>
#include <functional>

namespace fs = std::filesystem;

static std::string makeRequestId()
{
    static std::atomic<unsigned long long> counter{0};
    const auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    const auto tid = std::hash<std::thread::id>{}(std::this_thread::get_id());
    const auto seq = counter.fetch_add(1, std::memory_order_relaxed);
    return std::to_string(now) + "_" + std::to_string(tid) + "_" + std::to_string(seq);
}

int main()
{
    crow::SimpleApp app;

    // ================= ROOT CHECK =================
    CROW_ROUTE(app, "/")([]()
    {
        return "Resume Matcher Server Running";
    });

    // ================= CORS PREFLIGHT =================
    CROW_ROUTE(app, "/upload").methods("OPTIONS"_method)
    ([]()
    {
        crow::response res;
        res.code = 200;
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "POST, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "*");
        return res;
    });

    // ================= FILE UPLOAD =================
    CROW_ROUTE(app, "/upload").methods("POST"_method)
    ([](const crow::request &req)
    {
        try
        {
            crow::multipart::message msg(req);

            if (msg.parts.empty())
            {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = "No file uploaded";
                return crow::response(400, error);
            }

            fs::create_directories("uploads");
            fs::create_directories("extracted");

            const std::string requestId = makeRequestId();
            std::string savedPath = (fs::path("uploads") / ("resume_" + requestId + ".pdf")).string();
            std::string txtPath = (fs::path("extracted") / ("resume_" + requestId + ".txt")).string();

            bool fileSaved = false;

            // Save first file part
            for (auto &part : msg.parts)
            {
                if (!part.body.empty())
                {
                    std::ofstream out(savedPath, std::ios::binary);
                    out.write(part.body.c_str(), part.body.size());
                    out.close();
                    fileSaved = true;
                    break;
                }
            }

            if (!fileSaved)
            {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = "Resume file not found in request";
                return crow::response(400, error);
            }

            // Convert PDF to text
            std::string command =
                "pdftotext -layout \"" + savedPath + "\" \"" + txtPath + "\"";

            if (system(command.c_str()) != 0)
            {
                crow::json::wvalue error;
                error["success"] = false;
                error["message"] = "PDF extraction failed. Install pdftotext and add it to PATH.";
                return crow::response(500, error);
            }

            FileManager fm;
            JobMatcher matcher;

            Candidate candidate = fm.parseUnstructuredResume(txtPath);

            std::vector<std::pair<std::string, MatchResult>> allResults;

            std::string bestRole;
            double bestScore = -1;
            MatchResult bestMatch;

            if (!fs::exists("job_profiles"))
            {
                throw std::runtime_error("job_profiles folder not found");
            }

            for (const auto &entry : fs::directory_iterator("job_profiles"))
            {
                std::string filePath = entry.path().string();

                Job job = fm.loadJobFromFile(filePath);
                MatchResult match = matcher.evaluateCandidate(candidate, job);

                allResults.push_back({job.getJobTitle(), match});

                if (match.percentage > bestScore)
                {
                    bestScore = match.percentage;
                    bestRole  = job.getJobTitle();
                    bestMatch = match;
                }
            }

            std::sort(allResults.begin(), allResults.end(),
                      [](const auto &a, const auto &b)
                      {
                          return a.second.percentage > b.second.percentage;
                      });

            int topLimit = std::min(3, (int)allResults.size());

            crow::json::wvalue response;
            response["success"] = true;

            crow::json::wvalue data;
            data["name"]  = candidate.getName();
            data["email"] = candidate.getEmail();

            for (int i = 0; i < topLimit; ++i)
            {
                data["comparison"][allResults[i].first] =
                    allResults[i].second.percentage;
            }

            data["best_role"]    = bestRole;
            data["percentage"]   = bestMatch.percentage;
            data["fit_category"] = bestMatch.fitCategory;

            int i = 0;
            for (const auto &skill : bestMatch.matchedSkills)
            {
                data["matched_skills"][i++] = skill;
            }

            int j = 0;
            for (const auto &skill : bestMatch.missingSkills)
            {
                data["missing_skills"][j++] = skill.first;
            }

            response["data"] = std::move(data);

            crow::response res(200, response);
            res.add_header("Access-Control-Allow-Origin", "*");
            res.add_header("Access-Control-Allow-Methods", "POST, OPTIONS");
            res.add_header("Access-Control-Allow-Headers", "*");

            return res;
        }
        catch (const std::exception &e)
        {
            crow::json::wvalue error;
            error["success"] = false;
            error["message"] = e.what();

            crow::response res(500, error);
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;
        }
    });

    int port = 18080;
    if (const char *env_p = std::getenv("PORT"))
    {
        port = std::stoi(env_p);
    }

    app.port(port).multithreaded().run();
}
