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
    <h3>Total Blog Views</h3>
    <span id="total-views">Loading...</span>
  </div>
  <!-- Per-Site Views -->
  <div class="dashboard-card">
    <h3>Site Views</h3>
    <ul>
      <li>chessman: <span id="chessman-views">Loading...</span></li>
      <li>executables: <span id="executables-views">Loading...</span></li>
      <li>exploringos: <span id="exploringos-views">Loading...</span></li>
      <li>learningresource: <span id="learningresource-views">Loading...</span></li>
      <li>legacy: <span id="legacy-views">Loading...</span></li>
      <li>osjourney: <span id="osjourney-views">Loading...</span></li>
      <li>reversingbits: <span id="reversingbits-views">Loading...</span></li>
    </ul>
  </div>
  <!-- 7-Day Graph -->
  <div class="dashboard-card" style="flex: 1 1 100%; min-width: 350px;">
    <h3>7-Day Views (All Sites)</h3>
    <canvas id="views-graph" width="600" height="250"></canvas>
  </div>
  <!-- Social Metrics -->
  <div class="dashboard-card">
    <h3>Followers</h3>
    <ul>
      <li>GitHub: <span id="github-followers">Loading...</span></li>
      <li>LinkedIn: <span id="linkedin-followers">Loading...</span></li>
      <li>Twitter: <span id="twitter-followers">Loading...</span></li>
    </ul>
  </div>
  <!-- GitHub Profile Views -->
  <div class="dashboard-card">
    <h3>GitHub Profile Views</h3>
    <span id="github-profile-views">Loading...</span>
  </div>
  <!-- Social Links -->
  <div class="dashboard-card">
    <h3>Links</h3>
    <ul>
      <li><a href="https://github.com/mohitmishra786" target="_blank">GitHub</a></li>
      <li><a href="https://linkedin.com/in/mohitmishraml/" target="_blank">LinkedIn</a></li>
      <li><a href="https://x.com/chessMan786" target="_blank">Twitter</a></li>
      <li><a href="https://medium.com/@mohitmishra786687" target="_blank">Medium</a></li>
      <li><a href="https://chessman7.substack.com" target="_blank">Substack</a></li>
    </ul>
  </div>
</div>

<!-- Chart.js CDN -->
<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
<script>
(async function() {
  // GoatCounter site list
  const goatSites = [
    {name: 'chessman', url: 'https://chessman.goatcounter.com'},
    {name: 'executables', url: 'https://executables.goatcounter.com'},
    {name: 'exploringos', url: 'https://exploringos.goatcounter.com'},
    {name: 'learningresource', url: 'https://learningresource.goatcounter.com'},
    {name: 'legacy', url: 'https://legacy.goatcounter.com'},
    {name: 'osjourney', url: 'https://osjourney.goatcounter.com'},
    {name: 'reversingbits', url: 'https://reversingbits.goatcounter.com'}
  ];

  // Helper: fetch JSON with CORS fallback
  async function fetchJSON(url) {
    try {
      const resp = await fetch(url);
      if (!resp.ok) throw new Error('HTTP ' + resp.status);
      return await resp.json();
    } catch (e) {
      return null;
    }
  }

  // 1. Fetch total views and per-site views
  let totalViews = 0;
  let perSiteViews = {};
  let allDays = {};
  for (const site of goatSites) {
    // Total views
    const total = await fetchJSON(site.url + '/api/v0/stats/total');
    if (total && total.totals && typeof total.totals.views === 'number') {
      perSiteViews[site.name] = total.totals.views;
      totalViews += total.totals.views;
      document.getElementById(site.name + '-views').textContent = total.totals.views.toLocaleString();
    } else {
      document.getElementById(site.name + '-views').textContent = 'N/A';
    }
    // 7-day views
    const byDay = await fetchJSON(site.url + '/api/v0/stats/by-day?days=7');
    if (byDay && Array.isArray(byDay.days)) {
      byDay.days.forEach(day => {
        if (!allDays[day.date]) allDays[day.date] = 0;
        allDays[day.date] += day.views;
      });
    }
  }
  document.getElementById('total-views').textContent = totalViews.toLocaleString();

  // 2. Render 7-day graph
  const labels = Object.keys(allDays).sort();
  const data = labels.map(date => allDays[date]);
  new Chart(document.getElementById('views-graph').getContext('2d'), {
    type: 'line',
    data: {
      labels,
      datasets: [{
        label: 'Total Views',
        data,
        borderColor: '#36a2eb',
        backgroundColor: 'rgba(54,162,235,0.1)',
        fill: true,
        tension: 0.3
      }]
    },
    options: {
      responsive: true,
      plugins: { legend: { display: false } },
      scales: { y: { beginAtZero: true } }
    }
  });

  // 3. GitHub followers
  fetchJSON('https://api.github.com/users/mohitmishra786').then(data => {
    document.getElementById('github-followers').textContent = data && data.followers ? data.followers : 'N/A';
  });

  // 4. GitHub profile views (from shields.io badge in README)
  fetch('https://raw.githubusercontent.com/mohitmishra786/mohitmishra786/main/README.md')
    .then(r => r.text())
    .then(md => {
      const match = md.match(/profile-views-(\d+)-blue/);
      document.getElementById('github-profile-views').textContent = match ? match[1] : 'N/A';
    });

  // 5. Twitter followers (fallback: Nitter, else manual)
  fetch('https://nitter.net/chessMan786').then(r => r.text()).then(html => {
    const match = html.match(/Followers<\/span>\\s*<span[^>]*>([\d,]+)/);
    document.getElementById('twitter-followers').textContent = match ? match[1] : 'N/A';
  }).catch(() => {
    document.getElementById('twitter-followers').textContent = 'N/A';
  });

  // 6. LinkedIn followers (manual update, or use a placeholder)
  document.getElementById('linkedin-followers').textContent = '8,429'; // Update as needed
})();
</script>

<style>
.dashboard-card {
  background: #181c24;
  color: #eaf6fb;
  border-radius: 12px;
  box-shadow: 0 2px 8px rgba(0,0,0,0.08);
  padding: 1.5rem;
  min-width: 220px;
  flex: 1 1 250px;
  margin-bottom: 1rem;
}
.dashboard-card h3 {
  margin-top: 0;
  font-size: 1.2rem;
  color: #7fd6ff;
}
#dashboard ul {
  list-style: none;
  padding: 0;
  margin: 0;
}
#dashboard li {
  margin-bottom: 0.5rem;
}
</style>
