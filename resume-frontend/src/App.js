import React, { useState, useEffect, useRef } from "react";
import "./App.css";

const API_BASE_URL = (process.env.REACT_APP_API_URL || "http://localhost:18080").replace(/\/$/, "");

/* ================= COMPARISON BAR ================= */
function ComparisonBar({ role, score, index, onClick, isSelected }) {
  const [width, setWidth] = useState(0);

  useEffect(() => {
    const t = setTimeout(() => setWidth(score), 150 + index * 130);
    return () => clearTimeout(t);
  }, [score, index]);

  return (
    <div
      className={`comparison-item ${isSelected ? "selected" : ""}`}
      onClick={onClick}
      role="button"
      tabIndex={0}
      onKeyDown={(e) => {
        if (e.key === "Enter" || e.key === " ") {
          onClick?.();
        }
      }}
    >
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

/* ================= PANEL FOOTER ================= */
function PanelFooter() {
  return (
    <div className="panel-footer">
      <p>
        &copy; 2026 Resume Matcher |{" "}
        <a href="https://digitalheroesco.com" target="_blank" rel="noopener noreferrer">
          Built for Digital Heroes
        </a>
      </p>
      <p>
        Developed by Shubhi Gupta &middot;{" "}
        <a href="mailto:shubhigupta775@gmail.com">shubhigupta775@gmail.com</a>
      </p>
    </div>
  );
}


/* ================= ANIMATED LOADING STATE ================= */
const LOADING_STEPS = [
  "Reading your resume…",
  "Extracting skills…",
  "Matching against roles…",
  "Computing fit scores…",
  "Almost done…",
];

function LoadingState() {
  const [step, setStep] = useState(0);
  const [dots, setDots] = useState("");

  useEffect(() => {
    const stepTimer = setInterval(() => {
      setStep(s => (s + 1) % LOADING_STEPS.length);
    }, 2000);

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
          <div className="loading-icon">{"\u26A1"}</div>
        </div>

        {/* Animated text */}
        <div className="loading-title">Analyzing{dots}</div>
        <div className="loading-step fade-step" key={step}>{LOADING_STEPS[step]}</div>

        {/* Progress bar */}
        <div className="loading-bar-track">
          <div className="loading-bar-fill" />
        </div>
      </div>
      <PanelFooter />
    </div>
  );
}

/* ================= AI FEEDBACK CARD ================= */
function AIFeedbackCard({ requestId, role }) {
  const [feedback, setFeedback] = useState(null);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState(null);
  const [isExpanded, setIsExpanded] = useState(false);

  const fetchFeedback = async () => {
    setLoading(true);
    setError(null);
    try {
      const response = await fetch(`${API_BASE_URL}/evaluate`, {
        method: "POST",
        headers: { "Content-Type": "text/plain" },
        body: JSON.stringify({ request_id: requestId, role: role })
      });
      const text = await response.text();
      let data;
      try { data = JSON.parse(text); } catch(e) { throw new Error("Bad response: " + text.substring(0, 100)); }
      if (data.error) {
        setError(data.error);
      } else {
        setFeedback(data);
      }
    } catch (err) {
      setError(err.message || "Unable to reach AI service.");
    }
    setLoading(false);
  };

  useEffect(() => {
    // Reset feedback when resume changes (new upload)
    setFeedback(null);
    setError(null);
    setIsExpanded(false);
  }, [requestId]);

  const glassStyle = {
    background: 'rgba(255, 255, 255, 0.4)',
    backdropFilter: 'blur(12px)',
    WebkitBackdropFilter: 'blur(12px)',
    border: '1px solid rgba(255, 255, 255, 0.8)',
    borderRadius: '12px',
    padding: '16px',
    boxShadow: '0 4px 16px rgba(0, 0, 0, 0.03)'
  };

  const headerStyle = {
    color: 'var(--text-1)',
    display: 'flex',
    alignItems: 'center',
    gap: '8px',
    marginBottom: '10px',
    fontSize: '15px',
    fontWeight: '600',
    fontFamily: "'Playfair Display', serif"
  };

  const listStyle = {
    margin: 0,
    paddingLeft: '24px',
    color: 'var(--text-2)',
    fontSize: '13.5px',
    lineHeight: '1.6',
    fontWeight: '400'
  };

  return (
    <div className="stat-card fade-in ai-feedback-card" style={{ marginTop: '16px', borderTop: '4px solid #B8D8F8' }}>
      <div className="stat-card-label" style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '12px' }}>
        <span>✨ AI Recruiter Analysis</span>
        {feedback && (
          <button 
            onClick={() => setIsExpanded(!isExpanded)}
            style={{
              background: 'none', border: 'none', cursor: 'pointer',
              color: 'var(--sky-deep)', fontSize: '12px', fontWeight: '600',
              padding: '4px 8px', borderRadius: '4px', display: 'flex', alignItems: 'center', gap: '4px'
            }}
          >
            {isExpanded ? 'Hide Details ▲' : 'View Full Report ▼'}
          </button>
        )}
      </div>

      {!feedback && !loading && !error && (
        <div style={{ ...glassStyle, textAlign: 'center', padding: '20px' }}>
          <p style={{ color: 'var(--text-3)', fontSize: '13px', marginBottom: '12px' }}>
            Get a personalized AI recruiter review of your resume for the <strong style={{ color: 'var(--sky-deep)' }}>{role}</strong> role.
          </p>
          <button className="btn-ai" onClick={fetchFeedback} style={{ margin: '0 auto' }}>
            ✨ Generate AI Analysis
          </button>
        </div>
      )}

      {loading && (
        <div style={{ ...glassStyle, textAlign: 'center', padding: '24px' }}>
          <div className="btn-spinner" style={{ margin: '0 auto', width: '24px', height: '24px', borderColor: 'rgba(30,111,171,0.3)', borderTopColor: 'var(--sky-deep)' }} />
          <p style={{ color: 'var(--sky-deep)', fontSize: '13px', fontWeight: '500', marginTop: '10px' }}>Generating Insights...</p>
        </div>
      )}

      {error && (
        <div style={{ ...glassStyle, textAlign: 'center' }}>
          <p className="error-msg">{error}</p>
          <button className="btn-ai" onClick={() => fetchFeedback()} style={{ marginTop: '12px', padding: '8px 16px', fontSize: '13px' }}>
            🔄 Retry Analysis
          </button>
        </div>
      )}

      {feedback && !isExpanded && (
        <div style={{ ...glassStyle, cursor: 'pointer' }} onClick={() => setIsExpanded(true)}>
          <div style={{ display: 'flex', alignItems: 'flex-start', gap: '12px' }}>
            <span style={{ fontSize: '18px', flexShrink: 0 }}>💡</span>
            <div>
              <h4 style={{ margin: '0 0 4px 0', fontSize: '14px', color: 'var(--text-1)' }}>Key Strength</h4>
              <p style={{ margin: 0, fontSize: '13px', color: 'var(--text-2)', lineHeight: '1.5' }}>
                {feedback.strengths?.[0] || "Strong match for this role based on core skills."}
              </p>
              <p style={{ margin: '8px 0 0 0', fontSize: '12px', color: 'var(--sky-deep)', fontWeight: '500' }}>
                Click to view full analysis →
              </p>
            </div>
          </div>
        </div>
      )}

      {feedback && isExpanded && (
        <div className="ai-feedback-content" style={{ display: 'flex', flexDirection: 'column', gap: '16px' }}>
          
          <div className="feedback-section fade-in" style={glassStyle}>
            <h4 style={headerStyle}>
              <span style={{ opacity: 0.8 }}>🚀</span> Strengths
            </h4>
            <ul style={listStyle}>
              {feedback.strengths?.map((s, i) => <li key={i} style={{marginBottom: '6px'}}>{s}</li>)}
            </ul>
          </div>

          <div className="feedback-section fade-in" style={glassStyle}>
            <h4 style={headerStyle}>
              <span style={{ opacity: 0.8 }}>🎯</span> Areas to Improve
            </h4>
            <ul style={listStyle}>
              {feedback.weaknesses?.map((w, i) => <li key={i} style={{marginBottom: '6px'}}>{w}</li>)}
            </ul>
          </div>

          <div className="feedback-section fade-in" style={glassStyle}>
            <h4 style={headerStyle}>
              <span style={{ opacity: 0.8 }}>💡</span> Actionable Tips
            </h4>
            <ul style={listStyle}>
              {feedback.improvement_tips?.map((t, i) => <li key={i} style={{marginBottom: '6px'}}>{t}</li>)}
            </ul>
          </div>

        </div>
      )}
    </div>
  );
}

/* ================= RESULT PANEL ================= */
function ResultPanel({ result, loading }) {
  const [selectedRole, setSelectedRole] = useState("");

  useEffect(() => {
    if (!result?.success) {
      setSelectedRole("");
      return;
    }

    const { best_role, top_roles, comparison } = result.data;
    const firstRole = Array.isArray(top_roles)
      ? top_roles[0]?.role
      : Object.keys(comparison || {})[0];
    setSelectedRole(best_role || firstRole || "");
  }, [result]);

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
      <PanelFooter />
    </div>
  );

  if (loading) return <LoadingState />;

  if (!result)
    return <EmptyShell icon={"\uD83D\uDCCB"} title="No analysis yet" sub="Upload your resume and click Analyze." />;

  if (!result.success)
    return <EmptyShell icon={"\u26A0\uFE0F"} title="Something went wrong" sub={result.message} />;

  const { request_id, name, email, best_role, percentage, fit_category, comparison, matched_skills, missing_skills } = result.data;

  const topRoles = Array.isArray(result.data.top_roles)
    ? result.data.top_roles
    : Object.entries(comparison || {}).map(([role, score]) => ({
        role,
        percentage: score,
        matched_skills: matched_skills || [],
        missing_skills: missing_skills || [],
      }));


  const selectedRoleData =
    topRoles.find((item) => item.role === selectedRole) || {
      matched_skills: matched_skills || [],
      missing_skills: missing_skills || [],
    };

  return (
    <div className="result-panel panel-card">
      <div className="result-topbar">
        <div className="topbar-dot" />
        <span className="topbar-label">Analysis Results</span>
        <span className="topbar-slash">/</span>
        <span className="topbar-sub">Resume Matcher</span>
      </div>

      <div className="result-content-scroll" style={{ overflowY: 'auto', flex: 1, paddingBottom: '20px' }}>
        <div className="name-block">
          <span className="display-name">{name}</span>
          <span className="display-email">{email}</span>
        </div>

        <div className="top-row" style={{ height: 'auto', minHeight: '180px', paddingBottom: '16px' }}>
          <BestMatchCard role={best_role} percent={percentage} fitCategory={fit_category} />
          <div className="stat-card fade-in">
            <div className="stat-card-label">Role Comparison</div>
            <div className="comparison-list">
              {Object.entries(comparison || {}).map(([role, score], i) => (
                <ComparisonBar
                  key={role}
                  role={role}
                  score={parseFloat(score)}
                  index={i}
                  onClick={() => setSelectedRole(role)}
                  isSelected={selectedRole === role}
                />
              ))}
            </div>
          </div>
        </div>

        <div className="bottom-row" style={{ height: 'auto', overflow: 'visible', paddingBottom: '0' }}>
          <div className="stat-card fade-in">
            <div className="stat-card-label">Matched Skills</div>
            {selectedRoleData?.matched_skills?.length > 0 ? (
              <div className="skills-wrap">
                {selectedRoleData.matched_skills.map((s, i) => (
                  <span key={i} className="skill-tag matched" style={{ animationDelay: `${i * 0.04}s` }}>
                    {"\u2713"} {s}
                  </span>
                ))}
              </div>
            ) : <p className="no-skills">No matched skills found</p>}
          </div>

          <div className="stat-card fade-in">
            <div className="stat-card-label">Skills to Acquire</div>
            {selectedRoleData?.missing_skills?.length > 0 ? (
              <div className="skills-wrap">
                {selectedRoleData.missing_skills.map((s, i) => (
                  <span key={i} className="skill-tag missing" style={{ animationDelay: `${i * 0.04}s` }}>
                    + {s}
                  </span>
                ))}
              </div>
            ) : <p className="no-skills" style={{ color: "#065f46" }}>No missing skills.</p>}
          </div>
        </div>

        {request_id && best_role && (
          <div style={{ padding: '0 16px' }}>
            <AIFeedbackCard requestId={request_id} role={selectedRole} />
          </div>
        )}
      </div>
      <PanelFooter />
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
      const response = await fetch(`${API_BASE_URL}/upload`, {
        method: "POST",
        body: formData,
        mode: "cors"

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
        <div className="brand-icon">{"\u2726"}</div>
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
        <span className="drop-icon">{file ? "\uD83D\uDCC4" : "\u2601\uFE0F"}</span>
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
        ) : "Analyze Resume \u2192"}
      </button>

      {error && <p className="error-msg">{"\u26A0\uFE0F"} {error}</p>}

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
