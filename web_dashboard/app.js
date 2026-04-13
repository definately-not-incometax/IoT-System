const STATE = {
  connected: false,
  data: null,
  lastDataTime: 0,
  reconnectAttempts: 0,
  maxReconnectAttempts: 5
};

let ws = null;
let chartCtx = null;
let tempHistory = [];
let humHistory = [];

const IP_KEY = 'iot_device_ip';

document.addEventListener('DOMContentLoaded', () => {
  const savedIp = localStorage.getItem(IP_KEY);
  if (savedIp) {
    document.getElementById('device-ip').value = savedIp;
  }
  
  document.getElementById('connect-btn').onclick = connect;
  document.getElementById('clear-ip').onclick = () => {
    localStorage.removeItem(IP_KEY);
    document.getElementById('device-ip').value = '';
  };
  
  chartCtx = document.getElementById('chart').getContext('2d');
  initChart();
  
  // Check if already connected
  if (ws && ws.readyState === WebSocket.OPEN) {
    updateStatus(true);
  }
});

function connect() {
  const ip = document.getElementById('device-ip').value.trim();
  if (!ip) {
    alert('Enter device IP');
    return;
  }
  
  localStorage.setItem(IP_KEY, ip);
  STATE.reconnectAttempts = 0;
  
  if (ws) ws.close();
  
  ws = new WebSocket(`ws://${ip}:81/ws`);
  
  ws.onopen = () => {
    STATE.connected = true;
    STATE.reconnectAttempts = 0;
    updateStatus(true, 'Connected');
    log('Connected to ' + ip);
    tempHistory = [];
    humHistory = [];
  };
  
  ws.onmessage = (event) => {
    try {
      const data = JSON.parse(event.data);
      STATE.data = data;
      STATE.lastDataTime = Date.now();
      updateUI(data);
      log('Data update: temp=' + data.sensors.environment.temperature.toFixed(1));
    } catch (e) {
      log('Parse error: ' + e);
    }
  };
  
  ws.onclose = (event) => {
    STATE.connected = false;
    updateStatus(false, 'Disconnected - reconnecting...');
    log('Disconnected (' + event.code + ')');
    scheduleReconnect(ip);
  };
  
  ws.onerror = (e) => {
    log('Connection error');
  };
}

function scheduleReconnect(ip) {
  if (STATE.reconnectAttempts < STATE.maxReconnectAttempts) {
    STATE.reconnectAttempts++;
    setTimeout(() => connectWithIp(ip), 3000 * STATE.reconnectAttempts);
  } else {
    updateStatus(false, 'Failed to reconnect. Check IP.');
  }
}

function connectWithIp(ip) {
  document.getElementById('device-ip').value = ip;
  connect();
}

function disconnect() {
  if (ws) ws.close();
}

function updateStatus(connected, message = '') {
  const statusEl = document.getElementById('status');
  statusEl.textContent = connected ? 'Connected' : 'Disconnected';
  statusEl.className = `status ${connected ? 'connected' : 'disconnected'}`;
  
  document.getElementById('config').style.display = connected ? 'none' : 'block';
  document.getElementById('dashboard').style.display = connected ? 'block' : 'none';
}

function updateUI(data) {
  document.getElementById('temp').textContent = data.sensors.environment.temperature.toFixed(1) + ' °C';
  document.getElementById('humidity').textContent = data.sensors.environment.humidity.toFixed(1) + ' %';
  document.getElementById('uptime').textContent = (data.system.uptime / 1000).toFixed(0) + ' s';
  document.getElementById('rssi').textContent = data.system.rssi + ' dBm';
  document.getElementById('heap').textContent = (data.system.heap / 1024).toFixed(0) + ' KB';
  document.getElementById('timestamp').textContent = new Date(STATE.lastDataTime).toLocaleTimeString();
  
  updateChart(data.sensors.environment.temperature, data.sensors.environment.humidity);
}

function log(msg) {
  console.log(msg);
  // Optional: UI log
}

function initChart() {
  new Chart(chartCtx, {
    type: 'line',
    data: {
      labels: [],
      datasets: [
        {
          label: 'Temperature (°C)',
          data: [],
          borderColor: '#ff6384',
          tension: 0.4
        },
        {
          label: 'Humidity (%)',
          data: [],
          borderColor: '#36a2eb',
          tension: 0.4
        }
      ]
    },
    options: {
      responsive: true,
      scales: {
        y: { beginAtZero: true }
      }
    }
  });
}

function updateChart(temp, hum) {
  const now = new Date().toLocaleTimeString();
  tempHistory.push(temp);
  humHistory.push(hum);
  if (tempHistory.length > 50) {
    tempHistory.shift();
    humHistory.shift();
  }
  
  // Chart instance update (simplified)
  // Note: For full Chart.js, add CDN or lib
}

