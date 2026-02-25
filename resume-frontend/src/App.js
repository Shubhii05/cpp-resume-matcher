import React, { useState, useEffect } from "react";
import "./App.css";

function App() {
  const [file, setFile] = useState(null);
  const [result, setResult] = useState(null);
  const [loading, setLoading] = useState(false);
  const [animatedPercent, setAnimatedPercent] = useState(0);
  const [dragActive, setDragActive] = useState(false);

  const handleUpload = async () => {
    if (!file) {
      alert("Please select a file");
      return;
    }

    setLoading(true);
    setResult(null);

    const formData = new FormData();
    formData.append("file", file);

    try {
      const response = await fetch("http://localhost:18080/upload", {
        method: "POST",
        body: formData,
      });

      const data = await response.json();
      setResult(data);
    } catch (error) {
      alert("Unable to reach the server. Please try again.");
    }

    setLoading(false);
  };

  // Animate percentage
  useEffect(() => {
    if (result && result.success) {
      let start = 0;
      const end = parseFloat(result.percentage);
      const duration = 1000;
      const increment = end / (duration / 10);

      const timer = setInterval(() => {
        start += increment;
        if (start >= end) {
          start = end;
          clearInterval(timer);
        }
        setAnimatedPercent(start.toFixed(2));
      }, 10);
    }
  }, [result]);

  // Drag & Drop
  const handleDragOver = (e) => {
    e.preventDefault();
    setDragActive(true);
  };

  const handleDragLeave = () => {
    setDragActive(false);
  };

  const handleDrop = (e) => {
    e.preventDefault();
    setDragActive(false);
    const droppedFile = e.dataTransfer.files[0];
    if (droppedFile) {
      setFile(droppedFile);
    }
  };

  return (
    <div className="container">
      <div className="card">
        <h1>Resume Matcher</h1>

        {/* Small content under heading
        <p className="subtext">
          Upload your resume and instantly see your compatibility score.
        </p> */}

        <div
          className={`upload-box ${file ? "uploaded" : ""} ${dragActive ? "dragging" : ""}`}
          onDragOver={handleDragOver}
          onDragLeave={handleDragLeave}
          onDrop={handleDrop}
          onClick={() => document.getElementById("fileInput").click()}
        >
          <input
            id="fileInput"
            type="file"
            hidden
            onChange={(e) => setFile(e.target.files[0])}
          />
          {file ? file.name : "Click or Drag & Drop Resume Here"}
        </div>

        <button onClick={handleUpload}>
          {loading ? "Analyzing..." : "Submit"}
        </button>
        
          

        {result && result.success && (
          <div className="result">
            <h3>{result.data.name}</h3>
            <p>
              <strong>Evaluated For:</strong>{" "}
              {result?.data?.job_type?.toUpperCase()}
            </p>
            <p>
              <strong>Email:</strong> {result.data.email}
            </p>

            <p>
              <strong>Match:</strong> {animatedPercent}%
            </p>
            <p>
              <strong>Fit:</strong> {result.data.fit_category}
            </p>

            <h4>Matched Skills:</h4>

            {result.data?.matched_skills?.length > 0 ? (
              <ul>
                {result.data.matched_skills.map((skill, index) => (
                  <li key={index}>{skill}</li>
                ))}
              </ul>
            ) : (
              <p style={{ color: "red", fontWeight: "bold" }}>
                {result.data?.no_skills_message}
              </p>
            )}
          </div>
        )}

        {result && !result.success && (
          <p style={{ color: "red" }}>{result.message}</p>
        )}
      </div>
    </div>
  );
}

export default App;
