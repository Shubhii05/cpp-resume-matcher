#include "crow.h"
#include "FileManager.h"
#include "JobMatcher.h"
#include "cmath"
#include <fstream>
#include <sstream>           //can identify different data types
#include <iomanip>
#include <cstdlib>           //imports system

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

        system("mkdir uploads >nul 2>nul");                 //makes the folders
        system("mkdir extracted >nul 2>nul");

        std::string savedPath = "uploads/resume.pdf";       //saves the path
        std::string txtPath = "extracted/resume.txt";

        for (auto &part : msg.parts)
        {                                                       //opens file in binary and writes pdf data 
            std::ofstream out(savedPath, std::ios::binary);
            out.write(part.body.c_str(), part.body.size());     //saves as resume.pdf
            out.close();
        }

        std::string command =
            "pdftotext -layout \"" + savedPath + "\" \"" + txtPath + "\"";      //converts pdf to text
            
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

        string jobType = "frontend"; // change here: frontend, backend, ml
        Job job = fm.loadJobFromFile("job_profiles/job_" + jobType + ".txt");      //loads the job profile


        MatchResult match = matcher.evaluateCandidate(candidate, job);             //calculates percentaage,score,skills,fitcategory
        std::cout << "RAW SCORE: " << match.rawScore << std::endl;                 //DEBUG CODE
        std::cout << "PERCENTAGE: " << match.percentage << std::endl;


        crow::json::wvalue response;                                               //Creates a json object.response is the name if the variable
        response["success"] = true;

        crow::json::wvalue data;
        
        data["name"] = candidate.getName();
        data["email"] = candidate.getEmail();
        data["raw_score"] = match.rawScore;
        data["fit_category"] = match.fitCategory;

        std::ostringstream percentStream;
        percentStream << std::fixed << std::setprecision(2)
                    << match.percentage;

        response["percentage"] = percentStream.str();

        int i = 0;
        for (const auto &skill : match.matchedSkills)
            data["matched_skills"][i++] = skill;

        data["job_type"] = jobType;
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
            }});

            app.port(18080).multithreaded().run();                              //gives port
        }