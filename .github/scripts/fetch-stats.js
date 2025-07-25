const fs = require('fs');
const https = require('https');

const goatSites = [
  {
    name: 'chessman',
    url: 'https://chessman.goatcounter.com',
    token: process.env.CHESSMAN_TOKEN,
  },
  {
    name: 'executables',
    url: 'https://executables.goatcounter.com',
    token: process.env.EXECUTABLES_TOKEN,
  },
  {
    name: 'exploringos',
    url: 'https://exploringos.goatcounter.com',
    token: process.env.EXPLORINGOS_TOKEN,
  },
  {
    name: 'learningresource',
    url: 'https://learningresource.goatcounter.com',
    token: process.env.LEARNINGRESOURCE_TOKEN,
  },
  {
    name: 'legacy',
    url: 'https://legacy.goatcounter.com',
    token: process.env.LEGACY_TOKEN,
  },
  {
    name: 'osjourney',
    url: 'https://osjourney.goatcounter.com',
    token: process.env.OSJOURNEY_TOKEN,
  },
  {
    name: 'reversingbits',
    url: 'https://reversingbits.goatcounter.com',
    token: process.env.REVERSINGBITS_TOKEN,
  },
];

async function fetchJSON(url, headers = {}) {
  return new Promise((resolve, reject) => {
    const req = https.get(url, { headers }, (res) => {
      let data = '';
      res.on('data', (chunk) => (data += chunk));
      res.on('end', () => {
        try {
          resolve(JSON.parse(data));
        } catch (e) {
          resolve(null);
        }
      });
    });
    req.on('error', () => resolve(null));
    req.setTimeout(10000, () => {
      req.destroy();
      resolve(null);
    });
  });
}

async function main() {
  const stats = {
    totalViews: 0,
    siteViews: {},
    lastUpdated: new Date().toISOString(),
    chartData: {},
  };

  // Fetch GoatCounter data
  for (const site of goatSites) {
    if (!site.token) continue;

    const headers = {
      Authorization: `Bearer ${site.token}`,
      'Content-Type': 'application/json',
    };

    // Get total views
    const total = await fetchJSON(`${site.url}/api/v0/stats/total`, headers);
    if (total && total.count) {
      const views = parseInt(total.count.replace(/,/g, ''));
      stats.siteViews[site.name] = views;
      stats.totalViews += views;
    }

    // Get 7-day data
    const byDay = await fetchJSON(
      `${site.url}/api/v0/stats/hits?days=7`,
      headers
    );
    if (byDay && Array.isArray(byDay.stats)) {
      byDay.stats.forEach((day) => {
        if (!stats.chartData[day.day]) stats.chartData[day.day] = 0;
        stats.chartData[day.day] += day.count || 0;
      });
    }
  }

  // Fetch GitHub data
  const github = await fetchJSON('https://api.github.com/users/mohitmishra786');
  if (github) {
    stats.githubFollowers = github.followers;
  }

  // Ensure _data directory exists
  if (!fs.existsSync('_data')) {
    fs.mkdirSync('_data');
  }

  // Write to Jekyll data file
  fs.writeFileSync('_data/dashboard.json', JSON.stringify(stats, null, 2));
  console.log('Dashboard stats updated successfully!');
}

main().catch(console.error);
