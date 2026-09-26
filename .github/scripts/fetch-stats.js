const fs = require("fs");
const https = require("https");

const goatSites = [
  {
    name: "chessman",
    url: "https://chessman.goatcounter.com",
    token: process.env.CHESSMAN_TOKEN,
  },
  {
    name: "executables",
    url: "https://executables.goatcounter.com",
    token: process.env.EXECUTABLES_TOKEN,
  },
  {
    name: "exploringos",
    url: "https://exploringos.goatcounter.com",
    token: process.env.EXPLORINGOS_TOKEN,
  },
  {
    name: "learningresource",
    url: "https://learningresource.goatcounter.com",
    token: process.env.LEARNINGRESOURCE_TOKEN,
  },
  {
    name: "legacy",
    url: "https://legacy.goatcounter.com",
    token: process.env.LEGACY_TOKEN,
  },
  {
    name: "osjourney",
    url: "https://osjourney.goatcounter.com",
    token: process.env.OSJOURNEY_TOKEN,
  },
  {
    name: "reversingbits",
    url: "https://reversingbits.goatcounter.com",
    token: process.env.REVERSINGBITS_TOKEN,
  },
];

async function fetchJSON(url, headers = {}) {
  const mergedHeaders = Object.assign(
    { "User-Agent": "TheCoreDump-Stats-Pipeline" },
    headers
  );
  return new Promise((resolve) => {
    const req = https.get(url, { headers: mergedHeaders }, (res) => {
      let data = "";
      res.on("data", (chunk) => (data += chunk));
      res.on("end", () => {
        try {
          resolve(JSON.parse(data));
        } catch (e) {
          resolve(null);
        }
      });
    });
    req.on("error", () => resolve(null));
    req.setTimeout(10000, () => {
      req.destroy();
      resolve(null);
    });
  });
}

async function main() {
  let stats = {
    totalViews: 1158,
    siteViews: {
      chessman: 776,
      executables: 23,
      exploringos: 148,
      learningresource: 102,
      legacy: 70,
      osjourney: 14,
      reversingbits: 25,
    },
    githubFollowers: 824,
    lastUpdated: new Date().toISOString(),
    chartData: {},
  };

  // Load existing stats if available so we never overwrite with empty data
  try {
    if (fs.existsSync("_data/dashboard.json")) {
      const fileContent = fs.readFileSync("_data/dashboard.json", "utf8");
      const parsed = JSON.parse(fileContent);
      if (parsed && typeof parsed === "object") {
        if (parsed.totalViews && parsed.totalViews > 0) stats.totalViews = parsed.totalViews;
        if (parsed.siteViews && Object.keys(parsed.siteViews).length > 0) stats.siteViews = parsed.siteViews;
        if (parsed.githubFollowers) stats.githubFollowers = parsed.githubFollowers;
        if (parsed.chartData) stats.chartData = parsed.chartData;
      }
    }
  } catch (e) {
    console.warn("Could not read previous _data/dashboard.json:", e.message);
  }

  let fetchedAnySite = false;

  // Fetch GoatCounter data
  for (const site of goatSites) {
    if (!site.token) continue;

    const headers = {
      Authorization: `Bearer ${site.token}`,
      "Content-Type": "application/json",
    };

    // Get total views
    const total = await fetchJSON(`${site.url}/api/v0/stats/total`, headers);
    if (total && typeof total.total === "number" && total.total > 0) {
      stats.siteViews[site.name] = total.total;
      fetchedAnySite = true;
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

  // Recalculate total views if any site views were fetched or exist
  if (stats.siteViews && Object.keys(stats.siteViews).length > 0) {
    const sum = Object.values(stats.siteViews).reduce((acc, v) => acc + (typeof v === "number" ? v : 0), 0);
    if (sum > 0) {
      stats.totalViews = sum;
    }
  }

  // Fetch GitHub data
  const github = await fetchJSON("https://api.github.com/users/mohitmishra786");
  if (github && typeof github.followers === "number") {
    stats.githubFollowers = github.followers;
  }

  stats.lastUpdated = new Date().toISOString();

  // Ensure directories exist
  if (!fs.existsSync("_data")) {
    fs.mkdirSync("_data");
  }
  if (!fs.existsSync("assets/data")) {
    fs.mkdirSync("assets/data", { recursive: true });
  }

  // Write to Jekyll data files
  fs.writeFileSync("_data/dashboard.json", JSON.stringify(stats, null, 2));
  fs.writeFileSync("assets/data/dashboard.json", JSON.stringify(stats, null, 2));
  console.log("Dashboard stats updated successfully! Total views:", stats.totalViews, "GitHub followers:", stats.githubFollowers);
}

main().catch(console.error);
