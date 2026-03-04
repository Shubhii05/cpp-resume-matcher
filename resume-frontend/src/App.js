import React, { useState, useEffect, useRef } from "react";
import "./App.css";

/* ================= COMPARISON BAR ================= */
function ComparisonBar({ role, score, index }) {
  const [width, setWidth] = useState(0);

  useEffect(() => {
    const t = setTimeout(() => setWidth(score), 150 + index * 130);
    return () => clearTimeout(t);
  }, [score, index]);

  return (
    <div className="comparison-item">
      <div className="comparison-meta">
        <span className="comparison-role">{role}</span>
        <span className="comparison-pct">{score}%</span>
      </div>
      <div className="bar-track">
        <div className="bar-fill" style={{ width: `${width}%` }} />
      </div>
    </div>
  );
}

/* ================= BEST MATCH CARD ================= */
function BestMatchCard({ role, percent, fitCategory }) {
  const [animPct, setAnimPct] = useState(0);

  useEffect(() => {
    setAnimPct(0);
    const end = parseFloat(percent);
    const inc = end / 100;
    let cur = 0;

    const t = setInterval(() => {
      cur += inc;
      if (cur >= end) { cur = end; clearInterval(t); }
      setAnimPct(cur.toFixed(2));
    }, 10);

    return () => clearInterval(t);
  }, [percent]);

  return (
    <div className="best-match-card fade-in">
      <div>
        <div className="bm-eyebrow">Best Match</div>
        <div className="bm-role">{role}</div>
      </div>
      <div className="bm-bottom">
        <div className="bm-percent">{animPct}<sup>%</sup></div>
        <div className="fit-badge">{fitCategory}</div>
      </div>
    </div>
  );
}

/* ================= ANIMATED LOADING STATE ================= */
function LoadingState() {
  const steps = [
    "Reading your resume…",
    "Extracting skills…",
    "Matching against roles…",
    "Computing fit scores…",
    "Almost done…",
  ];
  const [step, setStep] = useState(0);
  const [dots, setDots] = useState("");

  useEffect(() => {
    // Cycle through steps every 2s
    const stepTimer = setInterval(() => {
      setStep(s => (s + 1) % steps.length);
    }, 2000);

    // Animate dots
    const dotTimer = setInterval(() => {
      setDots(d => d.length >= 3 ? "" : d + ".");
    }, 400);

    return () => { clearInterval(stepTimer); clearInterval(dotTimer); };
  }, []);

  return (
    <div className="result-panel panel-card">
      <div className="result-topbar">
        <div className="topbar-dot" />
        <span className="topbar-label">Analysis Results</span>
        <span className="topbar-slash">/</span>
        <span className="topbar-sub">Resume Matcher</span>
      </div>

      <div className="loading-state">
        {/* Animated rings */}
        <div className="loading-rings">
          <div className="ring ring-1" />
          <div className="ring ring-2" />
          <div className="ring ring-3" />
          <div className="loading-icon">⚡</div>
        </div>

        {/* Animated text */}
        <div className="loading-title">Analyzing{dots}</div>
        <div className="loading-step fade-step" key={step}>{steps[step]}</div>

        {/* Progress bar */}
        <div className="loading-bar-track">
          <div className="loading-bar-fill" />
        </div>
      </div>
    </div>
  );
}

/* ================= RESULT PANEL ================= */
function ResultPanel({ result, loading }) {
  const EmptyShell = ({ icon, title, sub }) => (
    <div className="result-panel panel-card">
      <div className="result-topbar">
        <div className="topbar-dot" />
        <span className="topbar-label">Analysis Results</span>
        <span className="topbar-slash">/</span>
        <span className="topbar-sub">Resume Matcher</span>
      </div>
      <div className="empty-state">
        <div className="empty-illustration">{icon}</div>
        <div className="empty-title">{title}</div>
        <div className="empty-sub">{sub}</div>
      </div>
    </div>
  );

  if (loading) return <LoadingState />;

  if (!result)
    return <EmptyShell icon="📋" title="No analysis yet" sub="Upload your resume and click Analyze." />;

  if (!result.success)
    return <EmptyShell icon="⚠️" title="Something went wrong" sub={result.message} />;

  const { name, email, best_role, percentage, fit_category, comparison, matched_skills, missing_skills } = result.data;

  return (
    <div className="result-panel panel-card">
      <div className="result-topbar">
        <div className="topbar-dot" />
        <span className="topbar-label">Analysis Results</span>
        <span className="topbar-slash">/</span>
        <span className="topbar-sub">Resume Matcher</span>
      </div>

      <div className="name-block">
        <span className="display-name">{name}</span>
        <span className="display-email">{email}</span>
      </div>

      <div className="top-row">
        <BestMatchCard role={best_role} percent={percentage} fitCategory={fit_category} />
        <div className="stat-card fade-in">
          <div className="stat-card-label">Role Comparison</div>
          <div className="comparison-list">
            {Object.entries(comparison).map(([role, score], i) => (
              <ComparisonBar key={role} role={role} score={parseFloat(score)} index={i} />
            ))}
          </div>
        </div>
      </div>

      <div className="bottom-row">
        <div className="stat-card fade-in">
          <div className="stat-card-label">Matched Skills</div>
          {matched_skills?.length > 0 ? (
            <div className="skills-wrap">
              {matched_skills.map((s, i) => (
                <span key={i} className="skill-tag matched" style={{ animationDelay: `${i * 0.04}s` }}>
                  ✓ {s}
                </span>
              ))}
            </div>
          ) : <p className="no-skills">No matched skills found</p>}
        </div>

        <div className="stat-card fade-in">
          <div className="stat-card-label">Skills to Acquire</div>
          {missing_skills?.length > 0 ? (
            <div className="skills-wrap">
              {missing_skills.map((s, i) => (
                <span key={i} className="skill-tag missing" style={{ animationDelay: `${i * 0.04}s` }}>
                  + {s}
                </span>
              ))}
            </div>
          ) : <p className="no-skills" style={{ color: "#065f46" }}>No missing skills.</p>}
        </div>
      </div>
    </div>
  );
}

/* ================= UPLOAD PANEL ================= */
function UploadPanel({ onResult, onLoading, loading }) {
  const [file, setFile] = useState(null);
  const [dragActive, setDragActive] = useState(false);
  const [error, setError] = useState(null);
  const inputRef = useRef();

  const handleUpload = async () => {
    if (!file) { setError("Please select a file first."); return; }
    setError(null);
    onLoading(true);
    onResult(null);

    const formData = new FormData();
    formData.append("resume", file);

    try {
      const response = await fetch("https://cpp-resume-matcher.onrender.com/upload", {
        method: "POST",
        body: formData,
      });
      const data = await response.json();
      onResult(data);
    } catch (err) {
      console.error(err);
      setError("Unable to reach the server. Please try again.");
      onResult(null);
    }

    onLoading(false);
  };

  const handleDrop = (e) => {
    e.preventDefault();
    setDragActive(false);
    const f = e.dataTransfer.files[0];
    if (f) setFile(f);
  };

  return (
    <div className="upload-panel panel-card">
      <div className="brand">
        <div className="brand-icon">✦</div>
        <div className="brand-name">Resume <span>Matcher</span></div>
      </div>

      <h2 className="upload-heading">Find your <span>perfect</span> role match</h2>

      <p className="upload-subtext">
        Upload your resume and instantly see how well you match top roles,
        what skills you have, and what to learn next.
      </p>

      <div
        className={`drop-zone ${dragActive ? "active" : ""} ${file ? "has-file" : ""}`}
        onDragOver={(e) => { e.preventDefault(); setDragActive(true); }}
        onDragLeave={() => setDragActive(false)}
        onDrop={handleDrop}
        onClick={() => inputRef.current.click()}
      >
        <input
          ref={inputRef}
          type="file"
          accept="application/pdf"
          hidden
          onChange={(e) => { if (e.target.files[0]) setFile(e.target.files[0]); }}
        />
        <span className="drop-icon">{file ? "📄" : "☁️"}</span>
        {file ? (
          <div className="drop-file-info">
            <span className="drop-filename">{file.name}</span>
            <span className="drop-sub">Click to change file</span>
          </div>
        ) : (
          <>
            <span>Click or drag &amp; drop your resume</span>
            <span className="drop-sub">PDF supported</span>
          </>
        )}
      </div>

      <button className="btn-analyze" onClick={handleUpload} disabled={loading}>
        {loading ? (
          <span className="btn-loading">
            <span className="btn-spinner" />
            Analyzing…
          </span>
        ) : "Analyze Resume →"}
      </button>

      {error && <p className="error-msg">⚠️ {error}</p>}

      <div className="tip-card">
        <p className="tip-text">
          <strong>Tip: </strong>
          Make sure your resume clearly lists your skills and technologies for the most accurate match.
        </p>
      </div>
    </div>
  );
}

/* ================= MAIN APP ================= */
export default function App() {
  const [result, setResult] = useState(null);
  const [loading, setLoading] = useState(false);

  return (
    <div className="app">
      <UploadPanel onResult={setResult} onLoading={setLoading} loading={loading} />
      <ResultPanel result={result} loading={loading} />
    </div>
  );
}