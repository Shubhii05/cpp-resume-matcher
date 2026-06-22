#include "crow.h"
#include "FileManager.h"
#include "JobMatcher.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <filesystem>
#include <algorithm>
#include <chrono>
#include <thread>
#include <atomic>
#include <functional>

namespace fs = std::filesystem;

static std::string trim(const std::string& s)
{
    const auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    const auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static void loadDotEnv(const std::string& path)
{
    std::ifstream file(path);
    if (!file) return;

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        const auto eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = trim(line.substr(0, eq));
        std::string value = trim(line.substr(eq + 1));
        if (value.size() >= 2 &&
            ((value.front() == '"' && value.back() == '"') ||
             (value.front() == '\'' && value.back() == '\''))) {
            value = value.substr(1, value.size() - 2);
        }
        if (key.empty() || std::getenv(key.c_str()) != nullptr) continue;

#ifdef _WIN32
        _putenv_s(key.c_str(), value.c_str());
#else
        setenv(key.c_str(), value.c_str(), 0);
#endif
    }
}

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
    loadDotEnv(".env");
    loadDotEnv("resume-frontend/.env");

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
            data["request_id"] = requestId;
            data["name"]  = candidate.getName();
            data["email"] = candidate.getEmail();

            for (int i = 0; i < topLimit; ++i)
            {
                data["comparison"][allResults[i].first] =
                    allResults[i].second.percentage;

                data["top_roles"][i]["role"] = allResults[i].first;
                data["top_roles"][i]["percentage"] = allResults[i].second.percentage;
                data["top_roles"][i]["fit_category"] = allResults[i].second.fitCategory;

                int mk = 0;
                for (const auto &skill : allResults[i].second.matchedSkills)
                {
                    data["top_roles"][i]["matched_skills"][mk++] = skill;
                }

                int ms = 0;
                for (const auto &skill : allResults[i].second.missingSkills)
                {
                    data["top_roles"][i]["missing_skills"][ms++] = skill.first;
                }
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

    // ================= EVALUATE MATCH =================
    CROW_ROUTE(app, "/evaluate").methods("OPTIONS"_method)
    ([]()
    {
        crow::response res;
        res.code = 200;
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "POST, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "*");
        return res;
    });

    CROW_ROUTE(app, "/evaluate").methods("POST"_method)
    ([](const crow::request &req)
    {
        try {
            auto body = crow::json::load(req.body);
            if (!body) {
                crow::json::wvalue err; err["error"] = "Invalid JSON";
                crow::response res(400, err);
                res.add_header("Access-Control-Allow-Origin", "*");
                return res;
            }

            std::string request_id = body["request_id"].s();
            std::string role = body["role"].s();

            // Load resume text
            std::string txtPath = (fs::path("extracted") / ("resume_" + request_id + ".txt")).string();
            if (!fs::exists(txtPath)) {
                crow::json::wvalue err; err["error"] = "Resume not found for id: " + request_id;
                crow::response res(404, err);
                res.add_header("Access-Control-Allow-Origin", "*");
                return res;
            }
            std::ifstream t(txtPath);
            std::string resume_text((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

            // Load job text (role is display title, e.g. "Full Stack Developer")
            std::string jobPath;
            for (const auto &entry : fs::directory_iterator("job_profiles"))
            {
                Job job = FileManager().loadJobFromFile(entry.path().string());
                if (job.getJobTitle() == role)
                {
                    jobPath = entry.path().string();
                    break;
                }
            }
            if (jobPath.empty())
            {
                jobPath = (fs::path("job_profiles") / (role + ".txt")).string();
                if (!fs::exists(jobPath))
                    jobPath = (fs::path("job_profiles") / (role + ".json")).string();
            }
            std::ifstream jt(jobPath);
            std::string job_text((std::istreambuf_iterator<char>(jt)), std::istreambuf_iterator<char>());

            const char* apiKey = std::getenv("GEMINI_API_KEY");
            if (!apiKey) {
                crow::json::wvalue err; err["error"] = "GEMINI_API_KEY not set on server";
                crow::response res(500, err);
                res.add_header("Access-Control-Allow-Origin", "*");
                return res;
            }

            std::string prompt = "You are an expert technical recruiter evaluating a candidate for a role. Evaluate the best match role against the resume. Return ONLY a valid JSON object with this exact structure: {\"strengths\": [\"string\", \"string\"], \"weaknesses\": [\"string\", \"string\"], \"improvement_tips\": [\"string\", \"string\"]}\n\nCandidate Resume:\n" + resume_text + "\n\nJob Profile:\n" + job_text;
            
            crow::json::wvalue payload;
            payload["contents"][0]["parts"][0]["text"] = prompt;
            
            std::string payloadPath = (fs::path("extracted") / ("gemini_payload_" + request_id + ".json")).string();
            std::ofstream out(payloadPath);
            out << payload.dump();
            out.close();

            std::string respPath = (fs::path("extracted") / ("gemini_response_" + request_id + ".json")).string();

            const char* modelEnv = std::getenv("GEMINI_MODEL");
            std::string model = modelEnv ? modelEnv : "gemini-2.5-flash";

            std::string cmd = "curl -s -X POST \"https://generativelanguage.googleapis.com/v1beta/models/";
            cmd += model;
            cmd += ":generateContent\" -H \"x-goog-api-key: ";
            cmd += apiKey;
            cmd += "\" -H \"Content-Type: application/json\" -d @\"" + payloadPath + "\" > \"" + respPath + "\"";

            system(cmd.c_str());

            std::ifstream rt(respPath);
            std::string resp_text((std::istreambuf_iterator<char>(rt)), std::istreambuf_iterator<char>());

            if (resp_text.empty()) {
                crow::json::wvalue fallback;
                fallback["strengths"][0] = "Strong alignment with the matched technical skills for this role.";
                fallback["strengths"][1] = "Resume shows relevant project and technology experience for the selected profile.";
                fallback["weaknesses"][0] = "Some role-specific tools or keywords may need stronger emphasis.";
                fallback["weaknesses"][1] = "Add more measurable outcomes to make the resume easier for recruiters to evaluate.";
                fallback["improvement_tips"][0] = "Add 2-3 bullet points with quantified impact for your strongest projects.";
                fallback["improvement_tips"][1] = "Tailor the summary and skills section to match the selected job profile keywords.";
                crow::response res(200, fallback);
                res.add_header("Access-Control-Allow-Origin", "*");
                return res;
            }

            auto resp_json = crow::json::load(resp_text);
            if (!resp_json) {
                crow::json::wvalue err; err["error"] = "Invalid JSON from LLM: " + resp_text.substr(0, 200);
                crow::response res(500, err);
                res.add_header("Access-Control-Allow-Origin", "*");
                return res;
            }

            // Check if Gemini returned an error
            if (resp_json.has("error")) {
                std::string gemini_err = resp_json["error"]["message"].s();
                crow::json::wvalue err; err["error"] = "Gemini API error: " + gemini_err;
                crow::response res(500, err);
                res.add_header("Access-Control-Allow-Origin", "*");
                return res;
            }

            if (!resp_json.has("candidates")) {
                crow::json::wvalue err; err["error"] = "Unexpected API response: " + resp_text.substr(0, 200);
                crow::response res(500, err);
                res.add_header("Access-Control-Allow-Origin", "*");
                return res;
            }
            
            std::string llm_text = resp_json["candidates"][0]["content"]["parts"][0]["text"].s();
            
            // Clean markdown if present
            if (llm_text.rfind("```json", 0) == 0) {
                llm_text = llm_text.substr(7);
                auto pos = llm_text.rfind("```");
                if (pos != std::string::npos) llm_text = llm_text.substr(0, pos);
            } else if (llm_text.rfind("```", 0) == 0) {
                llm_text = llm_text.substr(3);
                auto pos = llm_text.rfind("```");
                if (pos != std::string::npos) llm_text = llm_text.substr(0, pos);
            }

            // trim whitespace
            auto trim_ws = [](std::string s) {
                s.erase(0, s.find_first_not_of(" \t\r\n"));
                s.erase(s.find_last_not_of(" \t\r\n") + 1);
                return s;
            };
            llm_text = trim_ws(llm_text);

            auto llm_parsed = crow::json::load(llm_text);

            if (!llm_parsed) {
                crow::json::wvalue err; err["error"] = "Could not parse LLM output: " + llm_text.substr(0, 200);
                crow::response res(500, err);
                res.add_header("Access-Control-Allow-Origin", "*");
                return res;
            }

            // Convert rvalue -> wvalue for response
            crow::json::wvalue llm_wval = crow::json::load(llm_text);
            crow::response res(200, llm_wval);
            res.add_header("Access-Control-Allow-Origin", "*");
            return res;

        } catch (const std::exception& e) {
             crow::json::wvalue err; err["error"] = e.what();
             crow::response res(500, err);
             res.add_header("Access-Control-Allow-Origin", "*");
             return res;
        }
    });

    try {
        int port = 18080;
        if (const char *env_p = std::getenv("PORT"))
        {
            port = std::stoi(env_p);
        }

        app.port(port).multithreaded().run();
    } catch (const std::exception& e) {
        std::cerr << "Exception in main: " << e.what() << "\n";
        return 1;
    }
}
