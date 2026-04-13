// charts.js - Historical data visualization

let chart;

const CHART_MAX_POINTS = 50;

function initCharts() {
  const ctx = document.getElementById('chart').getContext('2d');
  chart = new Chart(ctx, {
    type: 'line',
    data: {
      labels: [],
      datasets: [
        {
          label: 'Temperature (°C)',
          data: [],
          borderColor: '#ff6b6b',
          backgroundColor: 'rgba(255, 107, 107, 0.1)',
          tension: 0.4,
          fill: true
        },
        {
          label: 'Humidity (%)',
          data: [],
          borderColor: '#4ecdc4',
          backgroundColor: 'rgba(78, 205, 196, 0.1)',
          tension: 0.4,
          fill: true,
          yAxisID: 'y1'
        }
      ]
    },
    options: {
      responsive: true,
      interaction: {
        intersect: false
      },
      plugins: {
        legend: {
          position: 'top'
        },
        title: {
          display: true,
          text: 'Sensor Trends'
        }
      },
      scales: {
        x: {
          display: true,
          title: {
            display: true,
            text: 'Time'
          }
        },
        y: {
          type: 'linear',
          position: 'left',
          title: {
            display: true,
            text: 'Temperature (°C)'
          }
        },
        y1: {
          type: 'linear',
          position: 'right',
          title: {
            display: true,
            text: 'Humidity (%)'
          },
          grid: {
            drawOnChartArea: false
          }
        }
      }
    }
  });
}

function updateChart(temp, hum, timestamp) {
  if (!chart) return;
  
  const timeLabel = new Date(timestamp).toLocaleTimeString();
  
  chart.data.labels.push(timeLabel);
  chart.data.datasets[0].data.push(temp);
  chart.data.datasets[1].data.push(hum);
  
  if (chart.data.labels.length > CHART_MAX_POINTS) {
    chart.data.labels.shift();
    chart.data.datasets[0].data.shift();
    chart.data.datasets[1].data.shift();
  }
  
  chart.update('none');
}

