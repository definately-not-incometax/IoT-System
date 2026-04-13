// alerts.js - Threshold-based alerts

const ALERT_STORAGE_KEY = 'iot_alerts';

class AlertManager {
  constructor() {
    this.alerts = JSON.parse(localStorage.getItem(ALERT_STORAGE_KEY) || '{}');
  }

  saveAlerts() {
    localStorage.setItem(ALERT_STORAGE_KEY, JSON.stringify(this.alerts));
  }

  setAlert(type, value) {
    this.alerts[type] = parseFloat(value) || null;
    this.saveAlerts();
  }

  checkAlerts(data) {
    const temp = data.sensors.environment.temperature;
    const hum = data.sensors.environment.humidity;
    let alerts = [];

    if (this.alerts.tempMax && temp > this.alerts.tempMax) {
      alerts.push(`Temperature high: ${temp.toFixed(1)}°C > ${this.alerts.tempMax}°C`);
    }
    if (this.alerts.tempMin && temp < this.alerts.tempMin) {
      alerts.push(`Temperature low: ${temp.toFixed(1)}°C < ${this.alerts.tempMin}°C`);
    }
    if (this.alerts.humMax && hum > this.alerts.humMax) {
      alerts.push(`Humidity high: ${hum.toFixed(1)}% > ${this.alerts.humMax}%`);
    }
    if (this.alerts.humMin && hum < this.alerts.humMin) {
      alerts.push(`Humidity low: ${hum.toFixed(1)}% < ${this.alerts.humMin}%`);
    }

    updateAlertsUI(alerts);
    
    if (alerts.length > 0 && Notification.permission === 'granted') {
      new Notification('IoT Alert', { body: alerts[0] });
    }

    return alerts.length > 0;
  }
}

const alertMgr = new AlertManager();

function updateAlertsUI(alerts) {
  const alertContainer = document.getElementById('alerts-container');
  if (alerts.length === 0) {
    alertContainer.style.display = 'none';
    return;
  }
  
  alertContainer.innerHTML = alerts.map(a => `<div class="alert">${a}</div>`).join('');
  alertContainer.style.display = 'block';
}

