#include "crow/app.h"
#include "crow/json.h"
#include "JobMatcher.h"
#include "FileManager.h"
#include <iomanip>
#include <sstream>
#include <cmath>


int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/match")
.methods(crow::HTTPMethod::Post)
([](const crow::request &req) -> crow::response
{
    auto body = crow::json::load(req.body);
    if (!body)
        return crow::response(400);

    std::string resumeFile = body["resume"].s();
    std::string jobFile = body["job"].s();

    Candidate candidate =
        FileManager::parseUnstructuredResume(resumeFile);

    Job job =
        FileManager::loadJobFromFile(jobFile);

    MatchResult result =
    JobMatcher::evaluateCandidate(candidate, job);

const Resume &resume = candidate.getResume();

crow::json::wvalue response;



int i = 0;
for (const auto &skill : result.matchedSkills)
{
    response["matchedSkills"][i++] = skill;
}

response["fit"] = result.fitCategory;

std::ostringstream percentStream;
percentStream << std::fixed << std::setprecision(2)
              << result.percentage;

response["percentage"] = percentStream.str();

response["score"] = result.rawScore;

response["phone"] = resume.getPhoneNumber();

response["email"] = candidate.getEmail();

response["name"] = candidate.getName();


return crow::response{response.dump(4)};
});



    app.port(18080).multithreaded().run();

    return 0;
}
