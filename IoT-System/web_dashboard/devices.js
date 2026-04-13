// devices.js - Multi-device management

const DEVICE_STORAGE_KEY = 'iot_devices';
const MAX_DEVICES = 10;

class DeviceManager {
  constructor() {
    this.devices = this.loadDevices();
    this.activeDeviceId = localStorage.getItem('active_device_id') || null;
    this.wsConnections = new Map(); // id -> ws
  }

  loadDevices() {
    const stored = localStorage.getItem(DEVICE_STORAGE_KEY);
    return stored ? JSON.parse(stored) : [];
  }

  saveDevices() {
    localStorage.setItem(DEVICE_STORAGE_KEY, JSON.stringify(this.devices));
  }

  addDevice(name, ip) {
    if (this.devices.length >= MAX_DEVICES) return false;
    const id = 'device_' + Date.now();
    this.devices.push({
      id,
      name,
      ip,
      status: 'disconnected',
      lastData: null,
      lastSeen: 0
    });
    this.saveDevices();
    return id;
  }

  removeDevice(id) {
    this.devices = this.devices.filter(d => d.id !== id);
    this.closeWs(id);
    if (this.activeDeviceId === id) this.activeDeviceId = null;
    this.saveDevices();
  }

  setActiveDevice(id) {
    this.activeDeviceId = this.devices.find(d => d.id === id) ? id : null;
    localStorage.setItem('active_device_id', this.activeDeviceId);
    return this.activeDeviceId;
  }

  getActiveDevice() {
    return this.activeDeviceId ? this.devices.find(d => d.id === this.activeDeviceId) : null;
  }

  updateDeviceStatus(id, status) {
    const device = this.devices.find(d => d.id === id);
    if (device) {
      device.status = status;
      device.lastSeen = Date.now();
      this.saveDevices();
    }
  }

  updateDeviceData(id, data) {
    const device = this.devices.find(d => d.id === id);
    if (device) {
      device.lastData = data;
      device.lastSeen = Date.now();
      this.saveDevices();
    }
  }

  connectToDevice(id) {
    const device = this.devices.find(d => d.id === id);
    if (!device) return null;
    
    this.closeWs(id);
    
    const ws = new WebSocket(`ws://${device.ip}:81/ws`);
    
    ws.onopen = () => {
      this.updateDeviceStatus(id, 'connected');
      log('Connected to ' + device.name);
    };
    
    ws.onmessage = (event) => {
      try {
        const data = JSON.parse(event.data);
        this.updateDeviceData(id, data);
        if (id === this.activeDeviceId) {
          updateUI(data);
        }
      } catch (e) {
        log('Parse error');
      }
    };
    
    ws.onclose = () => {
      this.updateDeviceStatus(id, 'disconnected');
      // Optional background reconnect
    };
    
    this.wsConnections.set(id, ws);
    return ws;
  }

  closeWs(id) {
    const ws = this.wsConnections.get(id);
    if (ws && ws.readyState === WebSocket.OPEN) {
      ws.close();
    }
    this.wsConnections.delete(id);
  }
}

const deviceMgr = new DeviceManager();

