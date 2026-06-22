"use strict";

/* ---- State ---- */
const state = {
  devices: {},
  packetLog: [],
  rssiHistory: [],
  rateHistory: [],
  rateBuckets: {},
  devCountHistory: [],
  devCountBuckets: {},
  maxLog: 500,
  maxRssi: 300,
  maxRate: 300,
  tab: "devices",
  map: null,
  mapMarkers: {},
  mapTrails: {},
  filterType: "",
  filterRssi: "",
  filterActive: false,
  selectedMac: null,
  notifications: false,
  recording: false,
};

/* ---- Elements ---- */
const els = {
  status: document.getElementById("ws-status"),
  statsDevices: document.getElementById("stats-devices"),
  statsIds: document.getElementById("stats-ids"),
  statsPackets: document.getElementById("stats-packets"),
  statsRecording: document.getElementById("stats-recording"),
  tbody: document.getElementById("device-tbody"),
  packetLog: document.getElementById("packet-log"),
  detailPanel: document.getElementById("detail-panel"),
  detailBody: document.getElementById("detail-body"),
  detailClose: document.getElementById("detail-close"),
  filterInput: document.getElementById("filter-input"),
  filterRssi: document.getElementById("filter-rssi"),
  filterActive: document.getElementById("filter-active"),
  exportCsv: document.getElementById("export-csv"),
  exportKml: document.getElementById("export-kml"),
  exportSession: document.getElementById("export-session"),
  btnRecord: document.getElementById("btn-record"),
  btnReplay: document.getElementById("btn-replay"),
  replayFile: document.getElementById("replay-file"),
  notifToggle: document.getElementById("notif-toggle"),
  totalDevStats: document.getElementById("total-dev-stats"),
  activeDevStats: document.getElementById("active-dev-stats"),
  uniqueIdStats: document.getElementById("unique-id-stats"),
  pktRateStats: document.getElementById("pkt-rate-stats"),
  avgRssiStats: document.getElementById("avg-rssi-stats"),
  deviceCount: document.getElementById("device-count"),
  countChart: document.getElementById("count-chart"),
};

/* ---- WebSocket ---- */
const WS_URL = typeof WS_CONFIG !== "undefined" ? WS_CONFIG : "__WS_URL__";
let ws = null;
let wsReplay = null;

function connectWs() {
  if (ws && ws.readyState === WebSocket.OPEN) return;
  const url = WS_URL.startsWith("__") ? "ws://127.0.0.1:8765" : WS_URL;
  ws = new WebSocket(url);
  ws.onopen = () => {
    els.status.textContent = "Connected";
    els.status.className = "badge badge-on";
    if (state.notifications && "Notification" in window) {
      new Notification("ESP DRONE REMOTEID", { body: "Connected to analyzer" });
    }
  };
  ws.onclose = () => {
    els.status.textContent = "Disconnected";
    els.status.className = "badge badge-off";
    setTimeout(connectWs, 2000);
  };
  ws.onmessage = (e) => {
    try { handleMsg(JSON.parse(e.data)); } catch (_) {}
  };
}

function wsSend(obj) {
  if (ws && ws.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify(obj));
  }
}

function handleMsg(msg) {
  if (msg.type === "reset") {
    state.devices = {};
    state.packetLog = [];
    state.rssiHistory = [];
    state.rateHistory = [];
    state.rateBuckets = {};
    state.devCountHistory = [];
    state.devCountBuckets = {};
    state.selectedMac = null;
    clearMapMarkers();
    clearMapTrails();
    closeDetail();
    render();
    return;
  }
  if (msg.type === "snapshot") {
    for (const d of (msg.devices || [])) {
      restoreDeviceFromSnapshot(d);
    }
    render();
    return;
  }
  if (msg.type === "packet") {
    onPacket(msg.data);
  }
  if (msg.type === "device_detail") {
    showDetailData(msg.data);
  }
  if (msg.type === "csv_data") {
    downloadText(msg.data, "drones.csv", "text/csv");
  }
  if (msg.type === "kml_data") {
    downloadText(msg.data, "drones.kml", "application/vnd.google-earth.kml+xml");
  }
  if (msg.type === "session_data") {
    downloadText(JSON.stringify(msg.data, null, 2), "session.json", "application/json");
  }
  if (msg.type === "session_status") {
    state.recording = msg.data.recording;
    updateRecordBtn();
  }
  if (msg.type === "replay_packet") {
    onPacket(msg.data);
  }
  if (msg.type === "replay_done") {
    addLogLine("--- REPLAY END ---");
  }
}

function restoreDeviceFromSnapshot(d) {
  if (!d.mac) return;
  if (!state.devices[d.mac]) {
    state.devices[d.mac] = {
      mac: d.mac,
      firstSeen: d.first_seen || 0,
      lastSeen: d.last_seen || 0,
      rssiSamples: [],
      basicId: d.basic_id || "",
      operatorId: d.operator_id || "",
      selfId: d.self_id || "",
      uaType: d.ua_type || 0,
      lat: null, lon: null,
      packetCount: d.packet_count || 0,
      locationTrail: [],
      messagesSeen: [],
    };
  }
  const dev = state.devices[d.mac];
  if (d.last_location) {
    dev.lat = d.last_location.latitude;
    dev.lon = d.last_location.longitude;
  }
  if (d.location_trail) {
    dev.locationTrail = d.location_trail;
  }
  if (d.self_id) dev.selfId = d.self_id;
}

function onPacket(data) {
  const ts = data.ts || Date.now() / 1000;
  const mac = data.mac || "?";
  const rssi = data.rssi;

  if (!state.devices[mac]) {
    state.devices[mac] = {
      mac, firstSeen: ts, lastSeen: ts, rssiSamples: [],
      basicId: "", operatorId: "", selfId: "",
      uaType: 0, lat: null, lon: null,
      packetCount: 0, locationTrail: [], messagesSeen: [],
    };
    if (state.notifications && document.hidden && "Notification" in window && Notification.permission === "granted") {
      new Notification("New Drone Detected", { body: `${mac} appeared` });
    }
  }
  const dev = state.devices[mac];
  dev.lastSeen = ts;
  dev.packetCount++;
  if (rssi !== null && rssi !== undefined) {
    dev.rssiSamples.push(rssi);
    if (dev.rssiSamples.length > 500) dev.rssiSamples = dev.rssiSamples.slice(-500);
  }

  if (rssi !== null && rssi !== undefined) {
    state.rssiHistory.push({ ts, mac, rssi });
    if (state.rssiHistory.length > state.maxRssi) state.rssiHistory = state.rssiHistory.slice(-state.maxRssi);
  }

  const bucketKey = Math.floor(ts / 5) * 5;
  state.rateBuckets[bucketKey] = (state.rateBuckets[bucketKey] || 0) + 1;
  rebuildRateHistory(ts);

  const countBucket = Math.floor(ts / 10) * 10;
  state.devCountBuckets[countBucket] = Object.keys(state.devices).length;
  rebuildCountHistory(ts);

  const line = `[${fmtTime(ts)}] ${mac} ${rssi !== null ? "RSSI:" + rssi + "dBm" : "       "}  ${data.summary || ""}`;
  state.packetLog.push(line);
  if (state.packetLog.length > state.maxLog) state.packetLog = state.packetLog.slice(-state.maxLog);

  if (data.summary) {
    const idMatch = data.summary.match(/ID:([^(]+)/);
    if (idMatch) dev.basicId = idMatch[1];
    const opMatch = data.summary.match(/Op:([^\|]+)/);
    if (opMatch) dev.operatorId = opMatch[1].trim();
    const gpsMatch = data.summary.match(/GPS:([^,]+),([^\| ]+)/);
    if (gpsMatch) {
      const lat = parseFloat(gpsMatch[1]);
      const lon = parseFloat(gpsMatch[2]);
      if (!isNaN(lat) && !isNaN(lon)) {
        dev.lat = lat;
        dev.lon = lon;
        dev.locationTrail.push({ lat, lon, ts: Math.floor(ts) });
        if (dev.locationTrail.length > 500) dev.locationTrail = dev.locationTrail.slice(-500);
      }
    }
    const uaMatch = data.summary.match(/\(([^)]+)\)/);
    if (uaMatch) dev.uaType = uaMatch[1];
    const selfMatch = data.summary.match(/Self:([^\|]+)/);
    if (selfMatch) dev.selfId = selfMatch[1].trim();
  }

  render();
}

function rebuildRateHistory(now) {
  const keys = Object.keys(state.rateBuckets).map(Number).sort();
  const cutoff = now - state.maxRate;
  state.rateHistory = [];
  for (const k of keys) {
    if (k < cutoff) { delete state.rateBuckets[k]; continue; }
    state.rateHistory.push({ ts: k, count: state.rateBuckets[k] });
  }
}

function rebuildCountHistory(now) {
  const keys = Object.keys(state.devCountBuckets).map(Number).sort();
  const cutoff = now - 600;
  state.devCountHistory = [];
  for (const k of keys) {
    if (k < cutoff) { delete state.devCountBuckets[k]; continue; }
    state.devCountHistory.push({ ts: k, count: state.devCountBuckets[k] });
  }
}

/* ---- Render ---- */

function fmtTime(ts) {
  const d = new Date(ts * 1000);
  return d.toLocaleTimeString("en-US", { hour12: false });
}

function fmtTimeAgo(ts) {
  const sec = Math.floor(Date.now() / 1000 - ts);
  if (sec < 5) return "now";
  if (sec < 60) return sec + "s ago";
  if (sec < 3600) return Math.floor(sec / 60) + "m ago";
  return Math.floor(sec / 3600) + "h ago";
}

function avg(arr) {
  if (!arr.length) return null;
  return arr.reduce((a, b) => a + b, 0) / arr.length;
}

function rssiClass(val) {
  if (val === null || val === undefined) return "";
  if (val >= -60) return "rssi-ok";
  if (val >= -75) return "rssi-warn";
  return "rssi-bad";
}

function uaTypeLabel(val) {
  const labels = { 0: "Not defined", 1: "Aeroplane", 2: "Helicopter", 3: "Gyroplane", 4: "Hybrid Lift", 5: "Ornithopter", 6: "Fixed Wing", 7: "Rotorcraft", 8: "VTOL", 9: "Other" };
  return labels[val] || val || "";
}

function esc(s) {
  if (!s) return "";
  return s.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;").replace(/"/g, "&quot;");
}

function filterDevices(list) {
  const fType = (els.filterInput?.value || "").toLowerCase();
  const fRssi = els.filterRssi?.value || "";
  const fActive = els.filterActive?.checked || false;
  const now = Date.now() / 1000;

  return list.filter(d => {
    if (fActive && now - d.lastSeen >= 30) return false;
    if (fType) {
      const search = `${d.mac} ${d.basicId} ${d.operatorId} ${d.selfId} ${d.uaType}`.toLowerCase();
      if (!search.includes(fType)) return false;
    }
    if (fRssi) {
      const rssiVal = d.rssiSamples.length > 0 ? d.rssiSamples[d.rssiSamples.length - 1] : null;
      if (rssiVal === null) return false;
      if (fRssi === "good" && rssiVal < -60) return false;
      if (fRssi === "warn" && (rssiVal < -75 || rssiVal >= -60)) return false;
      if (fRssi === "bad" && rssiVal >= -75) return false;
    }
    return true;
  });
}

function renderDevices() {
  const now = Date.now() / 1000;
  const list = Object.values(state.devices).sort((a, b) => b.lastSeen - a.lastSeen);
  const filtered = filterDevices(list);

  if (filtered.length === 0) {
    els.tbody.innerHTML = '<tr><td colspan="10" class="empty">No devices match filters</td></tr>';
    return;
  }

  let html = "";
  for (const d of filtered) {
    const active = now - d.lastSeen < 30;
    const rssiLast = d.rssiSamples.length > 0 ? d.rssiSamples[d.rssiSamples.length - 1] : null;
    const latStr = d.lat !== null ? d.lat.toFixed(5) : "";
    const lonStr = d.lon !== null ? d.lon.toFixed(5) : "";
    const selected = state.selectedMac === d.mac ? " row-selected" : "";
    html += `<tr class="${selected}" onclick="selectDevice('${esc(d.mac)}')" style="opacity:${active ? 1 : 0.4}">
      <td class="mac">${esc(d.mac)}</td>
      <td>${esc(d.basicId || "")}</td>
      <td>${uaTypeLabel(d.uaType)}</td>
      <td>${esc(d.operatorId || "")}</td>
      <td>${latStr}</td>
      <td>${lonStr}</td>
      <td class="rssi ${rssiClass(rssiLast)}">${rssiLast !== null ? rssiLast + " dBm" : ""}</td>
      <td>${d.packetCount}</td>
      <td>${d.selfId ? esc(d.selfId.substring(0, 20)) : ""}</td>
      <td>${fmtTimeAgo(d.lastSeen)}</td>
    </tr>`;
  }
  els.tbody.innerHTML = html;
}

function renderLog() {
  if (state.packetLog.length === 0) {
    els.packetLog.innerHTML = '<div class="placeholder">No packets yet.</div>';
    return;
  }
  els.packetLog.innerHTML = state.packetLog.map(l => `<div class="log-entry">${esc(l)}</div>`).join("");
  els.packetLog.scrollTop = els.packetLog.scrollHeight;
}

function addLogLine(text) {
  state.packetLog.push(text);
  if (state.packetLog.length > state.maxLog) state.packetLog = state.packetLog.slice(-state.maxLog);
  renderLog();
}

function renderStats() {
  const now = Date.now() / 1000;
  const total = Object.keys(state.devices).length;
  const active = Object.values(state.devices).filter(d => now - d.lastSeen < 30).length;
  const uniqueIds = new Set(Object.values(state.devices).map(d => d.basicId).filter(Boolean)).size;
  const recent = state.packetLog.length;
  const allRssi = Object.values(state.devices).flatMap(d => d.rssiSamples);
  const avgRssi = allRssi.length ? (allRssi.reduce((a, b) => a + b, 0) / allRssi.length).toFixed(1) : "-";

  els.statsDevices.textContent = `${active}/${total} devices`;
  els.statsIds.textContent = `${uniqueIds} IDs`;
  els.statsPackets.textContent = `${recent} pkt`;

  if (els.totalDevStats) els.totalDevStats.textContent = total;
  if (els.activeDevStats) els.activeDevStats.textContent = active;
  if (els.uniqueIdStats) els.uniqueIdStats.textContent = uniqueIds;
  if (els.pktRateStats) {
    const rate = state.rateHistory.length > 0 ? state.rateHistory[state.rateHistory.length - 1].count : 0;
    els.pktRateStats.textContent = rate + "/5s";
  }
  if (els.avgRssiStats) els.avgRssiStats.textContent = avgRssi + " dBm";
  if (els.deviceCount) els.deviceCount.textContent = total;
}

/* ---- Detail Panel ---- */

function selectDevice(mac) {
  if (state.selectedMac === mac) {
    closeDetail();
    return;
  }
  state.selectedMac = mac;
  wsSend({ command: "get_device_detail", mac });
}

function showDetailData(data) {
  if (!data) return;
  const d = data;
  const trailLen = d.location_trail ? d.location_trail.length : 0;
  const msgs = d.messages_seen ? d.messages_seen.join(", ") : "";
  const firstSeen = new Date(d.first_seen * 1000).toLocaleString();
  const lastSeen = new Date(d.last_seen * 1000).toLocaleString();

  els.detailBody.innerHTML = `
    <div class="detail-grid">
      <div class="detail-section">
        <h4>Identity</h4>
        <dl>
          <dt>MAC</dt><dd class="mac">${esc(d.mac)}</dd>
          <dt>Basic ID</dt><dd>${esc(d.basic_id || "—")}</dd>
          <dt>UA Type</dt><dd>${uaTypeLabel(d.ua_type)}</dd>
          <dt>Operator ID</dt><dd>${esc(d.operator_id || "—")}</dd>
          <dt>Self ID</dt><dd>${esc(d.self_id || "—")}</dd>
        </dl>
      </div>
      <div class="detail-section">
        <h4>Signal</h4>
        <dl>
          <dt>Avg RSSI</dt><dd class="rssi ${rssiClass(d.rssi_avg)}">${d.rssi_avg !== null ? d.rssi_avg + " dBm" : "—"}</dd>
          <dt>Last RSSI</dt><dd class="rssi ${rssiClass(d.rssi_last)}">${d.rssi_last !== null ? d.rssi_last + " dBm" : "—"}</dd>
          <dt>Min RSSI</dt><dd>${d.rssi_min !== null ? d.rssi_min + " dBm" : "—"}</dd>
          <dt>Max RSSI</dt><dd>${d.rssi_max !== null ? d.rssi_max + " dBm" : "—"}</dd>
          <dt>Samples</dt><dd>${d.rssi_samples ? d.rssi_samples.length : 0}</dd>
        </dl>
      </div>
      <div class="detail-section">
        <h4>Movement</h4>
        <dl>
          <dt>Trail Points</dt><dd>${trailLen}</dd>
          <dt>First Seen</dt><dd>${firstSeen}</dd>
          <dt>Last Seen</dt><dd>${lastSeen}</dd>
          <dt>Packets</dt><dd>${d.packet_count}</dd>
          <dt>Messages</dt><dd>${msgs || "—"}</dd>
        </dl>
      </div>
    </div>
    ${d.last_location ? `
    <div class="detail-section">
      <h4>Last Location</h4>
      <dl class="detail-inline">
        <dt>Latitude</dt><dd>${d.last_location.latitude}</dd>
        <dt>Longitude</dt><dd>${d.last_location.longitude}</dd>
        <dt>Altitude (Press)</dt><dd>${d.last_location.altitude_pressure !== undefined ? d.last_location.altitude_pressure + " m" : "—"}</dd>
        <dt>Altitude (Geo)</dt><dd>${d.last_location.altitude_geodetic !== undefined ? d.last_location.altitude_geodetic + " m" : "—"}</dd>
        <dt>Speed H</dt><dd>${d.last_location.speed_horizontal !== undefined ? d.last_location.speed_horizontal + " m/s" : "—"}</dd>
        <dt>Speed V</dt><dd>${d.last_location.speed_vertical !== undefined ? d.last_location.speed_vertical + " m/s" : "—"}</dd>
        <dt>Direction</dt><dd>${d.last_location.direction !== undefined ? d.last_location.direction + "°" : "—"}</dd>
      </dl>
    </div>` : ""}
    ${trailLen > 0 ? `<div class="detail-section"><h4>Trail</h4><pre class="trail-preview">${d.location_trail.map(p => `[${fmtTime(p.ts)}] ${p.lat.toFixed(5)},${p.lon.toFixed(5)}`).join("\n")}</pre></div>` : ""}
  `;
  els.detailPanel.classList.add("open");

  // Center map on this device
  if (d.last_location && state.map) {
    state.map.setView([d.last_location.latitude, d.last_location.longitude], 16);
  }
}

function closeDetail() {
  state.selectedMac = null;
  els.detailPanel.classList.remove("open");
}

/* ---- Charts ---- */
let chartRssi = null;
let chartRate = null;
let chartCount = null;
const chartColors = ["#58a6ff", "#3fb950", "#f0883e", "#f85149", "#bc8cff", "#ff7b72", "#79c0ff", "#a5d6ff", "#ffa657", "#7ee787"];
let rssiDsMap = {};

function initCharts() {
  const ctx1 = document.getElementById("chart-rssi");
  const ctx2 = document.getElementById("chart-rate");
  const ctx3 = document.getElementById("chart-count");
  if (!ctx1 || !ctx2) return;

  const gridCol = "#21262d";
  const tickCol = "#8b949e";

  chartRssi = new Chart(ctx1, {
    type: "scatter",
    data: { datasets: [] },
    options: {
      animation: false,
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        x: { type: "linear", min: Date.now() / 1000 - 120, max: Date.now() / 1000, grid: { color: gridCol }, ticks: { color: tickCol, maxTicksLimit: 8 } },
        y: { min: -100, max: -20, title: { display: true, text: "RSSI (dBm)", color: tickCol }, grid: { color: gridCol }, ticks: { color: tickCol } },
      },
      plugins: { legend: { display: false } },
    },
  });

  chartRate = new Chart(ctx2, {
    type: "bar",
    data: {
      datasets: [{
        label: "Packets/5s",
        data: [],
        backgroundColor: "#58a6ff",
        borderRadius: 2,
      }],
    },
    options: {
      animation: false,
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        x: { grid: { display: false }, ticks: { color: tickCol, maxTicksLimit: 10 } },
        y: { beginAtZero: true, title: { display: true, text: "Count", color: tickCol }, grid: { color: gridCol }, ticks: { color: tickCol } },
      },
      plugins: { legend: { display: false } },
    },
  });

  if (ctx3) {
    chartCount = new Chart(ctx3, {
      type: "line",
      data: {
        datasets: [{
          label: "Devices",
          data: [],
          borderColor: "#58a6ff",
          backgroundColor: "rgba(88, 166, 255, 0.1)",
          fill: true,
          tension: 0.4,
          pointRadius: 1,
          pointHoverRadius: 4,
          borderWidth: 2,
        }],
      },
      options: {
        animation: false,
        responsive: true,
        maintainAspectRatio: false,
        scales: {
          x: { grid: { display: false }, ticks: { color: tickCol, maxTicksLimit: 8 } },
          y: { beginAtZero: true, title: { display: true, text: "Count", color: tickCol }, grid: { color: gridCol }, ticks: { color: tickCol, stepSize: 1 } },
        },
        plugins: { legend: { display: false } },
      },
    });
  }
}

function updateCharts() {
  if (!chartRssi || !chartRate) return;

  const now = Date.now() / 1000;
  const history = state.rssiHistory;
  const macs = [...new Set(history.map(h => h.mac))].slice(0, 8);

  for (const mac of Object.keys(rssiDsMap)) {
    if (!macs.includes(mac)) {
      const idx = chartRssi.data.datasets.indexOf(rssiDsMap[mac]);
      if (idx !== -1) chartRssi.data.datasets.splice(idx, 1);
      delete rssiDsMap[mac];
    }
  }

  let ci = 0;
  for (const mac of macs) {
    if (!rssiDsMap[mac]) {
      const color = chartColors[ci++ % chartColors.length];
      const ds = { label: mac.length > 12 ? mac.slice(-12) : mac, data: [], backgroundColor: color, borderColor: color, pointRadius: 2, pointHoverRadius: 5 };
      chartRssi.data.datasets.push(ds);
      rssiDsMap[mac] = ds;
    }
    const ds = rssiDsMap[mac];
    const pts = history.filter(h => h.mac === mac).map(h => ({ x: h.ts, y: h.rssi }));
    ds.data.splice(0, ds.data.length, ...pts);
  }

  chartRssi.options.scales.x.min = now - 120;
  chartRssi.options.scales.x.max = now;
  chartRssi.update("none");

  const rate = state.rateHistory;
  chartRate.data.labels = rate.map(r => fmtTime(r.ts));
  chartRate.data.datasets[0].data = rate.map(r => r.count);
  chartRate.update("none");

  if (chartCount) {
    const cnt = state.devCountHistory;
    chartCount.data.labels = cnt.map(r => fmtTime(r.ts));
    chartCount.data.datasets[0].data = cnt.map(r => r.count);
    chartCount.update("none");
  }
}

/* ---- Map ---- */

function initMap() {
  const el = document.getElementById("map");
  if (!el) return;
  state.map = L.map("map", { attributionControl: false }).setView([45, 10], 3);
  L.tileLayer("https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png", {
    maxZoom: 18, opacity: 0.7,
  }).addTo(state.map);
}

function updateMap() {
  if (!state.map) return;
  const now = Date.now() / 1000;
  const newMacs = new Set();

  for (const [mac, dev] of Object.entries(state.devices)) {
    if (dev.lat === null || dev.lon === null) continue;
    if (now - dev.lastSeen > 60) continue;
    newMacs.add(mac);

    if (!state.mapMarkers[mac]) {
      const icon = L.circleMarker([dev.lat, dev.lon], {
        radius: 8, color: "#58a6ff", fillColor: "#58a6ff", fillOpacity: 0.5, weight: 2,
      });
      const label = `<b>${esc(dev.basicId || mac)}</b><br>${mac}<br>${dev.lat.toFixed(5)}, ${dev.lon.toFixed(5)}`;
      icon.bindTooltip(label);
      icon.on("click", () => selectDevice(mac));
      icon.addTo(state.map);
      state.mapMarkers[mac] = icon;
    } else {
      state.mapMarkers[mac].setLatLng([dev.lat, dev.lon]);
      const label = `<b>${esc(dev.basicId || mac)}</b><br>${mac}<br>${dev.lat.toFixed(5)}, ${dev.lon.toFixed(5)}`;
      state.mapMarkers[mac].setTooltipContent(label);
    }

    // Trail polyline
    if (dev.locationTrail.length >= 2) {
      const trailCoords = dev.locationTrail.map(p => [p.lat, p.lon]);
      if (state.mapTrails[mac]) {
        state.mapTrails[mac].setLatLngs(trailCoords);
      } else {
        const trail = L.polyline(trailCoords, {
          color: "#58a6ff", weight: 2, opacity: 0.5, dashArray: "5, 5",
        });
        trail.addTo(state.map);
        state.mapTrails[mac] = trail;
      }
    }
  }

  for (const mac of Object.keys(state.mapMarkers)) {
    if (!newMacs.has(mac)) {
      state.map.removeLayer(state.mapMarkers[mac]);
      delete state.mapMarkers[mac];
    }
  }

  // Keep trails for stale devices (but fade)
  for (const mac of Object.keys(state.mapTrails)) {
    if (!newMacs.has(mac)) {
      const dev = state.devices[mac];
      if (dev && now - dev.lastSeen > 120) {
        state.map.removeLayer(state.mapTrails[mac]);
        delete state.mapTrails[mac];
      }
    }
  }

  if (Object.keys(state.mapMarkers).length > 0) {
    const group = L.featureGroup(Object.values(state.mapMarkers));
    state.map.fitBounds(group.getBounds().pad(0.1), { maxZoom: 15 });
  }
}

function clearMapMarkers() {
  for (const mac of Object.keys(state.mapMarkers)) {
    state.map.removeLayer(state.mapMarkers[mac]);
  }
  state.mapMarkers = {};
}

function clearMapTrails() {
  for (const mac of Object.keys(state.mapTrails)) {
    state.map.removeLayer(state.mapTrails[mac]);
  }
  state.mapTrails = {};
}

/* ---- Recording ---- */

function toggleRecording() {
  if (state.recording) {
    wsSend({ command: "session_stop" });
  } else {
    wsSend({ command: "session_start" });
  }
}

function updateRecordBtn() {
  if (!els.btnRecord) return;
  els.btnRecord.textContent = state.recording ? "⏹ Stop" : "⏺ Record";
  els.btnRecord.className = state.recording ? "btn btn-danger" : "btn";
  if (els.statsRecording) {
    els.statsRecording.style.display = state.recording ? "inline" : "none";
  }
}

/* ---- Exports ---- */

function downloadText(text, filename, mime) {
  const blob = new Blob([text], { type: mime });
  const url = URL.createObjectURL(blob);
  const a = document.createElement("a");
  a.href = url;
  a.download = filename;
  a.click();
  URL.revokeObjectURL(url);
}

function exportCSV() {
  wsSend({ command: "get_csv" });
}

function exportKML() {
  wsSend({ command: "get_kml" });
}

function exportSession() {
  wsSend({ command: "get_session" });
}

/* ---- Replay ---- */

function handleReplayFile(file) {
  if (!file) return;
  const reader = new FileReader();
  reader.onload = (e) => {
    try {
      const session = JSON.parse(e.target.result);
      addLogLine(`--- REPLAY START (${session.length} packets) ---`);
      let idx = 0;
      function playNext() {
        if (idx >= session.length) return;
        onPacket(session[idx]);
        idx++;
        setTimeout(playNext, 100);
      }
      playNext();
    } catch (err) {
      addLogLine("--- REPLAY ERROR: " + err.message + " ---");
    }
  };
  reader.readAsText(file);
}

/* ---- Notifications ---- */

function toggleNotifications() {
  if (!("Notification" in window)) return;
  if (Notification.permission === "granted") {
    state.notifications = !state.notifications;
  } else if (Notification.permission === "default") {
    Notification.requestPermission().then(perm => {
      state.notifications = perm === "granted";
    });
  }
  updateNotifBtn();
}

function updateNotifBtn() {
  if (!els.notifToggle) return;
  els.notifToggle.textContent = state.notifications ? "🔔 On" : "🔕 Off";
}

/* ---- Tabs ---- */
document.querySelectorAll(".tab").forEach(tab => {
  tab.addEventListener("click", () => {
    document.querySelectorAll(".tab").forEach(t => t.classList.remove("active"));
    document.querySelectorAll(".tab-content").forEach(t => t.classList.remove("active"));
    tab.classList.add("active");
    const target = document.getElementById("tab-" + tab.dataset.tab);
    if (target) target.classList.add("active");
    state.tab = tab.dataset.tab;
    if (state.tab === "map") {
      setTimeout(() => state.map && state.map.invalidateSize(), 50);
      updateMap();
    }
  });
});

/* ---- Main loop ---- */
function tick() {
  renderDevices();
  renderLog();
  renderStats();
  updateCharts();
  if (state.tab === "map") updateMap();
}

/* ---- Init ---- */
function init() {
  connectWs();
  initCharts();
  initMap();

  // Event listeners
  if (els.detailClose) els.detailClose.addEventListener("click", closeDetail);
  if (els.filterInput) els.filterInput.addEventListener("input", render);
  if (els.filterRssi) els.filterRssi.addEventListener("change", render);
  if (els.filterActive) els.filterActive.addEventListener("change", render);
  if (els.exportCsv) els.exportCsv.addEventListener("click", exportCSV);
  if (els.exportKml) els.exportKml.addEventListener("click", exportKML);
  if (els.exportSession) els.exportSession.addEventListener("click", exportSession);
  if (els.btnRecord) els.btnRecord.addEventListener("click", toggleRecording);
  if (els.btnReplay && els.replayFile) {
    els.btnReplay.addEventListener("click", () => els.replayFile.click());
    els.replayFile.addEventListener("change", (e) => handleReplayFile(e.target.files[0]));
  }
  if (els.notifToggle) els.notifToggle.addEventListener("click", toggleNotifications);

  setInterval(tick, 1000);
}

/* ---- Dark mode ---- */
function toggleDark() {
  const html = document.documentElement;
  const isDark = html.getAttribute("data-theme") === "dark";
  html.setAttribute("data-theme", isDark ? "light" : "dark");
  localStorage.setItem("rid-theme", isDark ? "light" : "dark");
}

(function initTheme() {
  const saved = localStorage.getItem("rid-theme");
  if (saved === "dark" || (!saved && window.matchMedia("(prefers-color-scheme: dark)").matches)) {
    document.documentElement.setAttribute("data-theme", "dark");
  }
})();

document.addEventListener("DOMContentLoaded", init);
