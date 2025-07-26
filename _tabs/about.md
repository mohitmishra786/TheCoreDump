---
# the default layout is 'page'
icon: fas fa-info-circle
order: 4
---

# 👋 Welcome to TheCoreDump

**Raw tech insights, unfiltered.**  
We cut through the hype to deliver actionable technical knowledge.

## 🔧 What I Do
- Deep dives into system architecture
- Performance optimization guides
- Hard-earned lessons from production environments
- No-nonsense technology analysis

## 🧑‍💻 Who We Are
```yaml
name: Mohit Mishra
background: 
  - Software Engineer
  - Database Performance Specialist
  - Interested in Low-Level System/ Machine Learning/ Statistics/ Compilers
philosophy: "Dissatisfaction gives birth to greater goals", "If you can't measure it, you can't improve it"
```

## 🌐 Connect
- Twitter: [@chessMan786](https://x.com/chessMan786)
- GitHub: [mohitmishra786](https://github.com/mohitmishra786)
- LinkedIn: [Mohit Mishra](https://www.linkedin.com/in/mohitmishraml/)

## 📬 Get in Touch
For consulting inquiries or technical questions:  
📧 [dukechessman@gmail.com](mailto:dukechessman@gmail.com)

"The most profound technologies are those that disappear."
— Mark Weiser

## 📊 My Personal Dashboard

<div id="dashboard" style="display: flex; flex-wrap: wrap; gap: 1.5rem;">
  <!-- Total Views Card -->
  <div class="dashboard-card">
    <h3>📈 Total Blog Views</h3>
    <div class="stat-number" id="total-views">Loading...</div>
    <div class="stat-subtitle">All Sites Combined</div>
  </div>
  
  <!-- Per-Site Views -->
  <div class="dashboard-card">
    <h3>🌐 Site Views</h3>
    <ul class="stats-list">
      <li>chessman: <span id="chessman-views">Loading...</span></li>
      <li>executables: <span id="executables-views">Loading...</span></li>
      <li>exploringos: <span id="exploringos-views">Loading...</span></li>
      <li>learningresource: <span id="learningresource-views">Loading...</span></li>
      <li>legacy: <span id="legacy-views">Loading...</span></li>
      <li>osjourney: <span id="osjourney-views">Loading...</span></li>
      <li>reversingbits: <span id="reversingbits-views">Loading...</span></li>
    </ul>
  </div>
  
  <!-- Publication Views -->
  <div class="dashboard-card">
    <h3>📝 Publication Views</h3>
    <ul class="stats-list">
      <li>Medium: <span id="medium-views">Loading...</span></li>
      <li>Substack: <span id="substack-views">Loading...</span></li>
    </ul>
  </div>
  
  <!-- 7-Day Chart -->
  <div class="dashboard-card chart-card">
    <h3>📊 7-Day Views Trend</h3>
    <canvas id="views-graph" width="600" height="250"></canvas>
    <div class="chart-subtitle">All sites combined</div>
  </div>
  
  <!-- Social Metrics -->
  <div class="dashboard-card">
    <h3>👥 Followers</h3>
    <ul class="stats-list">
      <li>
        <i class="fab fa-github"></i>
        GitHub: <span id="github-followers">Loading...</span>
      </li>
      <li>
        <i class="fab fa-linkedin"></i>
        LinkedIn: <span id="linkedin-followers">Loading...</span>
      </li>
      <li>
        <i class="fab fa-twitter"></i>
        Twitter: <span id="twitter-followers">Loading...</span>
      </li>
    </ul>
  </div>
  
  <!-- GitHub Profile Views -->
  <div class="dashboard-card">
    <h3>👀 GitHub Profile Views</h3>
    <div class="stat-number" id="github-profile-views">Loading...</div>
    <div class="stat-subtitle">Total visits</div>
  </div>
  
  <!-- Last Updated -->
  <div class="dashboard-card">
    <h3>🕒 Last Updated</h3>
    <div class="stat-text" id="last-updated">Loading...</div>
    <div class="stat-subtitle">Data freshness</div>
  </div>
  
  <!-- Social Links -->
  <div class="dashboard-card">
    <h3>🔗 Connect With Me</h3>
    <div class="social-links">
      <a href="https://github.com/mohitmishra786" target="_blank" class="social-link">
        <i class="fab fa-github"></i> GitHub
      </a>
      <a href="https://linkedin.com/in/mohitmishraml/" target="_blank" class="social-link">
        <i class="fab fa-linkedin"></i> LinkedIn
      </a>
      <a href="https://x.com/chessMan786" target="_blank" class="social-link">
        <i class="fab fa-twitter"></i> Twitter
      </a>
      <a href="https://medium.com/@mohitmishra786687" target="_blank" class="social-link">
        <i class="fab fa-medium"></i> Medium
      </a>
      <a href="https://chessman7.substack.com" target="_blank" class="social-link">
        <i class="fas fa-newspaper"></i> Substack
      </a>
    </div>
  </div>
</div>

<!-- Status Indicator -->
<div id="status-indicator" class="status-indicator">
  <span id="status-text">Connecting...</span>
  <div class="status-dot" id="status-dot"></div>
</div>

<!-- Chart.js CDN -->
<script src="https://cdn.jsdelivr.net/npm/chart.js@4.4.0/dist/chart.umd.js"></script>

<script>
(async function() {
  console.log('🚀 Dashboard script starting...');
  
  // Configuration for fallback/manual data
  const FALLBACK_CONFIG = {
    manual: {
      linkedin: 5301,
      twitter: 29001,
      medium: 437,
      substack: 1355
    }
  };

  let chartInstance = null;

  // Update status indicator
  function updateStatus(message, type = 'loading') {
    console.log(`Status: ${message} (${type})`);
    const statusText = document.getElementById('status-text');
    const statusDot = document.getElementById('status-dot');
    
    if (statusText) statusText.textContent = message;
    if (statusDot) {
      statusDot.className = `status-dot ${type}`;
    }
  }

  // Format numbers with commas
  function formatNumber(num) {
    if (typeof num === 'number') {
      return num.toLocaleString();
    }
    return num;
  }

  // Format relative time
  function formatRelativeTime(dateString) {
    try {
      const date = new Date(dateString);
      const now = new Date();
      const diffMs = now - date;
      const diffHours = Math.floor(diffMs / (1000 * 60 * 60));
      const diffDays = Math.floor(diffHours / 24);
      
      if (diffHours < 1) return 'Just updated';
      if (diffHours < 24) return `${diffHours} hour${diffHours !== 1 ? 's' : ''} ago`;
      if (diffDays < 7) return `${diffDays} day${diffDays !== 1 ? 's' : ''} ago`;
      return date.toLocaleDateString();
    } catch (e) {
      return 'Unknown';
    }
  }

  // Render chart
  function renderChart(chartData = {}) {
    const canvas = document.getElementById('views-graph');
    if (!canvas) {
      console.warn('Chart canvas not found');
      return;
    }

    const ctx = canvas.getContext('2d');
    
    // Destroy existing chart
    if (chartInstance) {
      chartInstance.destroy();
    }

    const labels = Object.keys(chartData).sort();
    const data = labels.map(date => chartData[date] || 0);
    
    // If no data, show placeholder
    if (labels.length === 0) {
      const days = ['6 days ago', '5 days ago', '4 days ago', '3 days ago', '2 days ago', 'Yesterday', 'Today'];
      labels.push(...days);
      data.push(...[0, 0, 0, 0, 0, 0, 0]);
    }

    chartInstance = new Chart(ctx, {
      type: 'line',
      data: {
        labels: labels.map(label => {
          // Format date labels
          try {
            return new Date(label).toLocaleDateString('en-US', { month: 'short', day: 'numeric' });
          } catch (e) {
            return label;
          }
        }),
        datasets: [{
          label: 'Daily Views',
          data,
          borderColor: '#7fd6ff',
          backgroundColor: 'rgba(127, 214, 255, 0.1)',
          fill: true,
          tension: 0.4,
          pointBackgroundColor: '#7fd6ff',
          pointBorderColor: '#ffffff',
          pointBorderWidth: 2,
          pointRadius: 4,
          pointHoverRadius: 6
        }]
      },
      options: {
        responsive: true,
        maintainAspectRatio: false,
        plugins: { 
          legend: { display: false },
          tooltip: {
            backgroundColor: 'rgba(24, 28, 36, 0.9)',
            titleColor: '#7fd6ff',
            bodyColor: '#eaf6fb',
            borderColor: '#7fd6ff',
            borderWidth: 1
          }
        },
        scales: { 
          y: { 
            beginAtZero: true,
            ticks: {
              color: '#8892b0'
            },
            grid: {
              color: 'rgba(136, 146, 176, 0.1)'
            }
          },
          x: {
            ticks: {
              color: '#8892b0'
            },
            grid: {
              color: 'rgba(136, 146, 176, 0.1)'
            }
          }
        }
      }
    });
  }

  // Update UI with stats
  function updateDashboard(stats) {
    console.log('Updating dashboard with stats:', stats);
    
    // Total views
    const totalEl = document.getElementById('total-views');
    if (totalEl && stats.totalViews !== undefined) {
      totalEl.textContent = formatNumber(stats.totalViews);
      console.log('Updated total views:', stats.totalViews);
    } else {
      console.warn('Total views element not found or stats.totalViews undefined');
    }

    // Site views
    if (stats.siteViews) {
      Object.entries(stats.siteViews).forEach(([site, views]) => {
        const el = document.getElementById(site + '-views');
        if (el) {
          el.textContent = formatNumber(views);
          console.log(`Updated ${site} views:`, views);
        } else {
          console.warn(`Element not found for ${site}-views`);
        }
      });
    }

    // GitHub followers
    const githubEl = document.getElementById('github-followers');
    if (githubEl && stats.githubFollowers !== undefined) {
      githubEl.textContent = formatNumber(stats.githubFollowers);
      console.log('Updated GitHub followers:', stats.githubFollowers);
    }

    // GitHub profile views
    const profileViewsEl = document.getElementById('github-profile-views');
    if (profileViewsEl && stats.githubProfileViews !== undefined) {
      profileViewsEl.textContent = formatNumber(stats.githubProfileViews);
      console.log('Updated GitHub profile views:', stats.githubProfileViews);
    }

    // Publication views
    const mediumEl = document.getElementById('medium-views');
    const substackEl = document.getElementById('substack-views');
    if (mediumEl) {
      mediumEl.textContent = formatNumber(stats.mediumViews || FALLBACK_CONFIG.manual.medium);
      console.log('Updated Medium views:', stats.mediumViews || FALLBACK_CONFIG.manual.medium);
    }
    if (substackEl) {
      substackEl.textContent = formatNumber(stats.substackViews || FALLBACK_CONFIG.manual.substack);
      console.log('Updated Substack views:', stats.substackViews || FALLBACK_CONFIG.manual.substack);
    }

    // Social followers (use manual data as fallback)
    const linkedinEl = document.getElementById('linkedin-followers');
    const twitterEl = document.getElementById('twitter-followers');
    if (linkedinEl) {
      linkedinEl.textContent = formatNumber(stats.linkedinFollowers || FALLBACK_CONFIG.manual.linkedin);
      console.log('Updated LinkedIn followers:', stats.linkedinFollowers || FALLBACK_CONFIG.manual.linkedin);
    }
    if (twitterEl) {
      twitterEl.textContent = formatNumber(stats.twitterFollowers || FALLBACK_CONFIG.manual.twitter);
      console.log('Updated Twitter followers:', stats.twitterFollowers || FALLBACK_CONFIG.manual.twitter);
    }

    // Last updated
    const lastUpdatedEl = document.getElementById('last-updated');
    if (lastUpdatedEl && stats.lastUpdated) {
      lastUpdatedEl.textContent = formatRelativeTime(stats.lastUpdated);
      console.log('Updated last updated:', stats.lastUpdated);
    }

    // Render chart
    renderChart(stats.chartData || {});
  }

  // Main data loading function
  async function loadDashboardData() {
    updateStatus('Loading dashboard data...', 'loading');

    try {
      // Try to fetch from dashboard.json file
      console.log('Attempting to fetch dashboard.json...');
      const response = await fetch('./assets/data/dashboard.json');
      console.log('Response status:', response.status, response.ok);
      
      if (response.ok) {
        const stats = await response.json();
        console.log('Loaded dashboard data:', stats);
        updateDashboard(stats);
        updateStatus('Data loaded successfully', 'success');
        console.log('Dashboard loaded from dashboard.json');
        return;
      } else {
        console.warn('Failed to load dashboard.json, status:', response.status);
      }
    } catch (e) {
      console.warn('Failed to load from dashboard.json:', e);
    }

    // Fallback: Use manual/placeholder data
    console.log('Using fallback data...');
    updateStatus('Using cached data', 'warning');
    const fallbackStats = {
      totalViews: 3797,
      siteViews: {
        chessman: 1319,
        executables: 28,
        exploringos: 1263,
        learningresource: 437,
        legacy: 494,
        osjourney: 189,
        reversingbits: 67
      },
      githubFollowers: 0,
      githubProfileViews: 0,
      lastUpdated: new Date().toISOString(),
      chartData: {}
    };
    console.log('Fallback stats:', fallbackStats);

    // Try to fetch GitHub data directly (no auth needed for public info)
    try {
      const githubResponse = await fetch('https://api.github.com/users/mohitmishra786');
      if (githubResponse.ok) {
        const githubData = await githubResponse.json();
        fallbackStats.githubFollowers = githubData.followers;
      }
    } catch (e) {
      console.warn('Failed to fetch GitHub data:', e);
    }

    // Try to fetch GitHub profile views from README
    try {
      const readmeResponse = await fetch('https://raw.githubusercontent.com/mohitmishra786/mohitmishra786/main/README.md');
      if (readmeResponse.ok) {
        const readmeText = await readmeResponse.text();
        const viewsMatch = readmeText.match(/profile-views-(\d+)-blue|visitors-(\d+)-blue/);
        if (viewsMatch) {
          fallbackStats.githubProfileViews = parseInt(viewsMatch[1] || viewsMatch[2]);
        }
      }
    } catch (e) {
      console.warn('Failed to fetch GitHub profile views:', e);
    }

    console.log('Calling updateDashboard with fallback stats...');
    updateDashboard(fallbackStats);
    console.log('Dashboard loaded with fallback data');
  }

  // Initialize dashboard
  console.log('Starting dashboard initialization...');
  await loadDashboardData();

  // Auto-refresh every 5 minutes
  setInterval(loadDashboardData, 5 * 60 * 1000);

  console.log('🚀 Dashboard initialization complete!');
})();
</script>

<style>
#dashboard {
  margin: 2rem 0;
}

.dashboard-card {
  background: linear-gradient(135deg, #181c24 0%, #1e2329 100%);
  color: #eaf6fb;
  border-radius: 16px;
  box-shadow: 0 4px 16px rgba(0,0,0,0.1), 0 1px 3px rgba(0,0,0,0.1);
  padding: 1.5rem;
  min-width: 220px;
  flex: 1 1 250px;
  margin-bottom: 1rem;
  border: 1px solid rgba(127, 214, 255, 0.1);
  transition: transform 0.2s ease, box-shadow 0.2s ease;
}

.dashboard-card:hover {
  transform: translateY(-2px);
  box-shadow: 0 8px 24px rgba(0,0,0,0.15), 0 2px 6px rgba(0,0,0,0.1);
  border-color: rgba(127, 214, 255, 0.2);
}

.dashboard-card h3 {
  margin-top: 0;
  margin-bottom: 1rem;
  font-size: 1.1rem;
  color: #7fd6ff;
  font-weight: 600;
  display: flex;
  align-items: center;
  gap: 0.5rem;
}

.stat-number {
  font-size: 2rem;
  font-weight: 700;
  color: #ffffff;
  margin: 0.5rem 0;
}

.stat-text {
  font-size: 1.2rem;
  font-weight: 600;
  color: #ffffff;
  margin: 0.5rem 0;
}

.stat-subtitle {
  font-size: 0.9rem;
  color: #8892b0;
  margin-top: 0.25rem;
}

.stats-list {
  list-style: none;
  padding: 0;
  margin: 0;
}

.stats-list li {
  margin-bottom: 0.75rem;
  padding: 0.5rem 0;
  border-bottom: 1px solid rgba(136, 146, 176, 0.1);
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.stats-list li:last-child {
  border-bottom: none;
  margin-bottom: 0;
}

.stats-list li i {
  margin-right: 0.5rem;
  width: 16px;
  color: #7fd6ff;
}

.stats-list span {
  font-weight: 600;
  color: #ffffff;
}

.chart-card {
  flex: 1 1 100%;
  min-width: 350px;
  min-height: 320px;
}

.chart-subtitle {
  text-align: center;
  font-size: 0.9rem;
  color: #8892b0;
  margin-top: 1rem;
}

.social-links {
  display: flex;
  flex-direction: column;
  gap: 0.75rem;
}

.social-link {
  color: #7fd6ff;
  text-decoration: none;
  padding: 0.5rem;
  border-radius: 8px;
  transition: all 0.2s ease;
  display: flex;
  align-items: center;
  gap: 0.5rem;
}

.social-link:hover {
  background: rgba(127, 214, 255, 0.1);
  color: #9fe7ff;
  transform: translateX(4px);
}

.social-link i {
  width: 16px;
  text-align: center;
}

.status-indicator {
  position: fixed;
  bottom: 2rem;
  right: 2rem;
  background: rgba(24, 28, 36, 0.95);
  color: #eaf6fb;
  padding: 0.75rem 1rem;
  border-radius: 8px;
  font-size: 0.9rem;
  display: flex;
  align-items: center;
  gap: 0.5rem;
  box-shadow: 0 4px 12px rgba(0,0,0,0.15);
  border: 1px solid rgba(127, 214, 255, 0.2);
  backdrop-filter: blur(10px);
  z-index: 1000;
}

.status-dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  animation: pulse 2s infinite;
}

.status-dot.loading {
  background: #ffa500;
}

.status-dot.success {
  background: #00ff00;
}

.status-dot.warning {
  background: #ffff00;
}

.status-dot.error {
  background: #ff0000;
}

@keyframes pulse {
  0%, 100% { opacity: 1; }
  50% { opacity: 0.5; }
}

/* Responsive adjustments */
@media (max-width: 768px) {
  #dashboard {
    gap: 1rem;
  }
  
  .dashboard-card {
    min-width: 100%;
    flex: 1 1 100%;
  }
  
  .stat-number {
    font-size: 1.5rem;
  }
  
  .social-links {
    flex-direction: row;
    flex-wrap: wrap;
  }
  
  .status-indicator {
    bottom: 1rem;
    right: 1rem;
    font-size: 0.8rem;
  }
}

/* Dark mode enhancements */
@media (prefers-color-scheme: dark) {
  .dashboard-card {
    background: linear-gradient(135deg, #0f1419 0%, #151920 100%);
    border-color: rgba(127, 214, 255, 0.15);
  }
}

/* Print styles */
@media print {
  .status-indicator {
    display: none;
  }
  
  .dashboard-card {
    break-inside: avoid;
    box-shadow: none;
    border: 1px solid #ccc;
  }
}
</style>