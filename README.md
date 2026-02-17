# C++ Resume Matcher

A console-based C++ application that matches candidate resumes with job profiles using a skill-based scoring system.

## Features
- Reads job profiles and resumes from text files
- Compares skills and mandatory requirements
- Calculates:
  - Matched Skills
  - Project Bonus
  - Education Bonus
  - Final Score & Match Percentage
- Displays Fit Level (Strong / Moderate / Weak)

## Tech Stack
- C++
- Object-Oriented Programming (OOP)
- File Handling
- STL (Vectors, Strings, Algorithms)

## How to Run
```bash
g++ main.cpp Candidate.cpp Resume.cpp Job.cpp JobMatcher.cpp FileManager.cpp -o ResumeMatcher
./ResumeMatcher
```

## Purpose
Simulates a basic Applicant Tracking System (ATS) to evaluate candidate-job compatibility.
