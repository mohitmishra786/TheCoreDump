---
# The default layout is 'page'
icon: fas fa-info-circle
order: 4
---

# About TheCoreDump

**Raw tech insights, unfiltered.**

TheCoreDump is an independent technical publication written and maintained by Mohit Mishra. It focuses on low-level systems programming, operating system internals, database engines, distributed systems resilience, and architectural forensics of real-world production outages.

The objective is simple: cut through marketing buzzwords and superficial overviews to examine how computing platforms actually operate under the hood—from CPU cache lines and virtual memory paging to global BGP route convergence and database storage engines.

---

## About the Author

I am **Mohit Mishra**, a software engineer specializing in database performance, operating system internals, and distributed systems reliability. My engineering work and technical writing focus on understanding systems from the hardware boundary up to distributed application layers:

- **Database Engines & Storage Internals:** PostgreSQL internals (MVCC, buffer pools, WAL, B-Tree indexing, write-back caching, lock contention), storage engine optimization, and transactional consistency models.
- **Operating Systems & Low-Level Architecture:** Linux kernel mechanisms, virtual memory management, multi-level page tables, ELF binary parsing, process scheduling, dynamic linking, and concurrency primitives.
- **Distributed Systems & Cloud Outages:** Conducting technical autopsies of real-world service disruptions (AWS, Cloudflare, Azure, Oracle Cloud), investigating cascading retries, BGP route withdrawals, DNS partition failures, and edge network routing.

---

## Technical Focus Areas

The writing on TheCoreDump is organized around three recurring disciplines:

### Production Incident Autopsies
Forensic breakdowns of major cloud and infrastructure incidents. Each analysis examines root causes, architectural contributing factors, failure cascades, and systemic lessons learned from real incidents at scale—including DNS resolution failures, BGP route withdrawals, and automated script bugs.

### Database Systems & Storage Engines
Deep dives into the internal mechanics of database engines, with a strong focus on PostgreSQL. Topics include multiversion concurrency control (MVCC), transaction isolation levels (RCSI vs. Read Committed), asynchronous I/O pipelines, SSD page alignment, and lock hierarchies.

### Systems & Kernel Engineering
Explorations of low-level software design and systems programming in C/C++. Areas covered include custom memory allocators, asynchronous event multiplexing (`epoll`/`select`), Unix domain and network sockets, dynamic linker mechanics, and processor memory architectures.

---

## Publication Telemetry

{% assign stats = site.data.dashboard %}

{% assign total_views = stats.totalViews %}
{% if total_views == nil or total_views == 0 or total_views == "" %}
  {% assign total_views = 1158 %}
{% endif %}

{% assign gh_followers = stats.githubFollowers %}
{% if gh_followers == nil or gh_followers == 0 or gh_followers == "" %}
  {% assign gh_followers = 824 %}
{% endif %}

{% assign v_chessman = stats.siteViews.chessman %}
{% if v_chessman == nil or v_chessman == 0 %}{% assign v_chessman = 776 %}{% endif %}

{% assign v_exploringos = stats.siteViews.exploringos %}
{% if v_exploringos == nil or v_exploringos == 0 %}{% assign v_exploringos = 148 %}{% endif %}

{% assign v_learningresource = stats.siteViews.learningresource %}
{% if v_learningresource == nil or v_learningresource == 0 %}{% assign v_learningresource = 102 %}{% endif %}

{% assign v_legacy = stats.siteViews.legacy %}
{% if v_legacy == nil or v_legacy == 0 %}{% assign v_legacy = 70 %}{% endif %}

{% assign v_reversingbits = stats.siteViews.reversingbits %}
{% if v_reversingbits == nil or v_reversingbits == 0 %}{% assign v_reversingbits = 25 %}{% endif %}

{% assign v_executables = stats.siteViews.executables %}
{% if v_executables == nil or v_executables == 0 %}{% assign v_executables = 23 %}{% endif %}

{% assign v_osjourney = stats.siteViews.osjourney %}
{% if v_osjourney == nil or v_osjourney == 0 %}{% assign v_osjourney = 14 %}{% endif %}

<div class="metrics-container">
  <div class="metrics-grid">
    <div class="metric-card metric-hero">
      <div class="metric-label">Total Publication Views</div>
      <div class="metric-value" id="stat-total-views">{{ total_views }}</div>
      <div class="metric-subtext">Cumulative readership across technical guides, OS series, and architecture deep dives</div>
    </div>

    <div class="metric-card">
      <div class="metric-label">GitHub Community</div>
      <div class="metric-value" id="stat-github-followers">{{ gh_followers }}</div>
      <div class="metric-subtext">Developers following open-source implementations and engineering notes</div>
    </div>

    <div class="metric-card metric-full">
      <div class="metric-label">Publication Channels</div>
      <ul class="channel-list">
        <li class="channel-item">
          <span class="channel-name"><i class="fas fa-terminal fa-fw"></i> TheCoreDump Core</span>
          <span class="channel-count" id="stat-site-chessman">{{ v_chessman }}</span>
        </li>
        <li class="channel-item">
          <span class="channel-name"><i class="fas fa-microchip fa-fw"></i> Exploring OS</span>
          <span class="channel-count" id="stat-site-exploringos">{{ v_exploringos }}</span>
        </li>
        <li class="channel-item">
          <span class="channel-name"><i class="fas fa-book fa-fw"></i> Learning Resources</span>
          <span class="channel-count" id="stat-site-learningresource">{{ v_learningresource }}</span>
        </li>
        <li class="channel-item">
          <span class="channel-name"><i class="fas fa-server fa-fw"></i> Architecture & Legacy</span>
          <span class="channel-count" id="stat-site-legacy">{{ v_legacy }}</span>
        </li>
        <li class="channel-item">
          <span class="channel-name"><i class="fas fa-memory fa-fw"></i> Reversing Bits</span>
          <span class="channel-count" id="stat-site-reversingbits">{{ v_reversingbits }}</span>
        </li>
        <li class="channel-item">
          <span class="channel-name"><i class="fas fa-code fa-fw"></i> Executables & Tooling</span>
          <span class="channel-count" id="stat-site-executables">{{ v_executables }}</span>
        </li>
        <li class="channel-item">
          <span class="channel-name"><i class="fas fa-route fa-fw"></i> OS Journey</span>
          <span class="channel-count" id="stat-site-osjourney">{{ v_osjourney }}</span>
        </li>
      </ul>
    </div>
  </div>

  <div class="metrics-meta">
    <i class="fas fa-shield-alt fa-fw"></i>
    <span>Privacy-respecting telemetry tracked via GoatCounter and compiled automatically via GitHub Actions.</span>
  </div>
</div>

---

## Publications & Writing Elsewhere

In addition to TheCoreDump, select articles and newsletters are published across external platforms:

- **Substack:** [chessman7.substack.com](https://chessman7.substack.com) — Architecture notes and essay releases.
- **Medium:** [@mohitmishra786687](https://medium.com/@mohitmishra786687) — System design and technical tutorials.

---

## Connect & Contact

- **GitHub:** [mohitmishra786](https://github.com/mohitmishra786)
- **Twitter / X:** [@chessMan786](https://x.com/chessMan786)
- **LinkedIn:** [Mohit Mishra](https://www.linkedin.com/in/mohitmishraml/)
- **Email:** [dukechessman@gmail.com](mailto:dukechessman@gmail.com)

For consulting inquiries, technical discussions, system architecture reviews, or errata, feel free to reach out via email.

<blockquote class="site-quote">
  <p>"The most profound technologies are those that disappear. They weave themselves into the fabric of everyday life until they are indistinguishable from it."</p>
  <footer>— Mark Weiser</footer>
</blockquote>

<style>
.metrics-container {
  margin: 1.75rem 0 2rem;
}

.metrics-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
  gap: 1.25rem;
  margin-bottom: 0.75rem;
}

.metric-card {
  background: var(--card-bg, rgba(255, 255, 255, 0.04));
  border: 1px solid var(--card-border-color, rgba(127, 127, 127, 0.18));
  border-radius: 12px;
  padding: 1.5rem;
  box-shadow: var(--card-box-shadow, 0 2px 8px rgba(0, 0, 0, 0.05));
  transition: border-color 0.2s ease, transform 0.15s ease;
}

.metric-card:hover {
  border-color: var(--link-color, #0d6efd);
  transform: translateY(-2px);
}

.metric-hero {
  display: flex;
  flex-direction: column;
  justify-content: center;
}

.metric-label {
  font-size: 0.8rem;
  font-weight: 600;
  text-transform: uppercase;
  letter-spacing: 0.06em;
  color: var(--text-muted-color, #888);
  margin-bottom: 0.5rem;
}

.metric-value {
  font-size: 2.75rem;
  font-weight: 700;
  color: var(--heading-color, #fff);
  line-height: 1.1;
  font-variant-numeric: tabular-nums;
}

.metric-subtext {
  font-size: 0.85rem;
  color: var(--text-muted-color, #888);
  margin-top: 0.6rem;
  line-height: 1.45;
}

.channel-list {
  list-style: none;
  padding: 0;
  margin: 0;
}

.channel-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 0.55rem 0;
  border-bottom: 1px solid var(--border-color, rgba(127, 127, 127, 0.12));
  font-size: 0.9rem;
}

.channel-item:last-child {
  border-bottom: none;
  padding-bottom: 0;
}

.channel-name {
  color: var(--text-color, #ddd);
  display: flex;
  align-items: center;
  gap: 0.6rem;
}

.metric-full {
  grid-column: 1 / -1;
}

.channel-name i {
  color: var(--link-color, #0d6efd);
  font-size: 0.85rem;
}

.channel-count {
  font-weight: 600;
  font-variant-numeric: tabular-nums;
  color: var(--heading-color, #fff);
}

.metrics-meta {
  font-size: 0.8rem;
  color: var(--text-muted-color, #888);
  display: flex;
  align-items: center;
  gap: 0.5rem;
  padding: 0.25rem 0.25rem 0;
}

.metrics-meta i {
  font-size: 0.85rem;
  opacity: 0.8;
}

.site-quote {
  margin-top: 2rem;
  padding: 1rem 1.25rem;
  border-left: 3px solid var(--link-color, #0d6efd);
  background: var(--card-bg, rgba(255, 255, 255, 0.02));
  border-radius: 0 8px 8px 0;
}

.site-quote p {
  margin-bottom: 0.35rem;
  font-style: italic;
  color: var(--text-color, #ddd);
}

.site-quote footer {
  font-size: 0.85rem;
  color: var(--text-muted-color, #888);
}
</style>

<script>
(function() {
  /* Progressive enhancement: safely refresh stats if newer dashboard.json is available */
  var dataUrl = '{{ "/assets/data/dashboard.json" | relative_url }}';
  fetch(dataUrl)
    .then(function(res) {
      if (res.ok) {
        return res.json();
      }
      throw new Error('Metrics unreachable');
    })
    .then(function(data) {
      if (!data) return;
      var fmt = new Intl.NumberFormat('en-US');

      /* Total views: only update if positive number */
      if (typeof data.totalViews === 'number' && data.totalViews > 0) {
        var totalEl = document.getElementById('stat-total-views');
        if (totalEl) totalEl.textContent = fmt.format(data.totalViews);
      }

      /* GitHub followers: only update if positive number */
      if (typeof data.githubFollowers === 'number' && data.githubFollowers > 0) {
        var ghEl = document.getElementById('stat-github-followers');
        if (ghEl) ghEl.textContent = fmt.format(data.githubFollowers);
      }

      /* Channel views: only update if object with positive numbers */
      if (data.siteViews && typeof data.siteViews === 'object') {
        for (var key in data.siteViews) {
          if (Object.prototype.hasOwnProperty.call(data.siteViews, key)) {
            var count = data.siteViews[key];
            if (typeof count === 'number' && count > 0) {
              var siteEl = document.getElementById('stat-site-' + key);
              if (siteEl) siteEl.textContent = fmt.format(count);
            }
          }
        }
      }
    })
    .catch(function(err) {
      /* Graceful fallback: statically pre-rendered Liquid values remain intact */
    });
})();
</script>