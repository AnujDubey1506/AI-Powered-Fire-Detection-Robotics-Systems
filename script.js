// =======================
// GLOBAL VARIABLES
// =======================
let currentMode = null;
let pumpActive = false;
let humanFollowing = false;
let isLogging = false;
let sensorInterval = null;
let flameChart;
let distanceChart;
let sensorHistory = [];
let camStreamUrl = "";
const FIXED_CAM_IP = "192.168.31.77"; // camera
// ✅ CORRECT — auto-detects ESP32 IP always:
const ROBOT_IP = window.location.hostname;

// =======================
// ALERTS
// =======================
let alerts = [];

// =======================
// CAMERA STREAM
// =======================
function setCameraStream(ip) {
    camStreamUrl = `http://${ip}:81/stream`;

    const manualCam = document.getElementById('manualCameraStream');
    const autonomousCam = document.getElementById('autonomousCameraStream');
    const streamUrlWithBust = `${camStreamUrl}?t=${Date.now()}`;

    if (manualCam) {
        manualCam.src = streamUrlWithBust;
        manualCam.style.opacity = "1";
    }

    if (autonomousCam) {
        autonomousCam.src = streamUrlWithBust;
        autonomousCam.style.opacity = "1";
    }

    console.log("Camera stream set:", camStreamUrl);
}

function handleCameraError(img) {
    img.alt = "Camera stream not available";
    img.style.opacity = "0.5";
    console.log("Camera stream failed to load");

    if (camStreamUrl) {
        setTimeout(() => {
            img.src = `${camStreamUrl}?t=${Date.now()}`;
            img.style.opacity = "1";
        }, 3000);
    }
}

// =======================
// MODE SELECTION
// =======================
function selectMode(mode) {
    currentMode = mode;
    document.getElementById('modeSelection')?.classList.add('hidden');

    if (mode === 'manual') {
        document.getElementById('manualMode')?.classList.remove('hidden');
    } else {
        document.getElementById('autonomousMode')?.classList.remove('hidden');
        startAutonomousMode();
    }
}

function changeMode() {
    currentMode = null;
    document.getElementById('manualMode')?.classList.add('hidden');
    document.getElementById('autonomousMode')?.classList.add('hidden');
    document.getElementById('modeSelection')?.classList.remove('hidden');

    if (sensorInterval) {
        clearInterval(sensorInterval);
        sensorInterval = null;
    }
}

// =======================
// MANUAL MODE
// =======================
function sendCommand(direction) {
    console.log(`Moving ${direction}`);
    fetch(`http://${ROBOT_IP}/command?dir=${direction}`)
        .catch(() => { });
    showToast('Command Sent', `Moving ${direction}`);
}

function togglePump() {
    pumpActive = !pumpActive;
    const pumpBtn = document.getElementById('pumpBtn');

    if (!pumpBtn) return;

    if (pumpActive) {
        pumpBtn.classList.add('active');
        pumpBtn.innerHTML = '<span>🔥</span> Pump ON';
        fetch(`http://${ROBOT_IP}/pump?state=on`)
            .catch(() => { });
        showToast('Command Sent', 'Pump turned on');
    } else {
        pumpBtn.classList.remove('active');
        pumpBtn.innerHTML = '<span>💧</span> Pump OFF';
        fetch(`http://${ROBOT_IP}/pump?state=off`)
            .catch(() => { });
        showToast('Command Sent', 'Pump turned off');
    }
}

function sendVoiceCommand() {
    const input = document.getElementById('voiceInput');
    if (!input) return;

    const command = input.value.trim();

    if (command) {
        fetch(`http://${ROBOT_IP}/voice?cmd=${command}`)
            .catch(() => { });

        showToast('Command Sent', `Voice: "${command}"`);
        input.value = '';
    }
}

function handleVoiceEnter(event) {
    if (event.key === 'Enter') sendVoiceCommand();
}

// =======================
// AUTONOMOUS MODE
// =======================
function startAutonomousMode() {
    renderAlerts();

    if (sensorInterval) {
        clearInterval(sensorInterval);
    }

    startSensorFetching();
}

async function fetchSensorData() {
    try {

        const response = await fetch(`http://${ROBOT_IP}/sensors`);

        const data = await response.json();

        data.timestamp = Date.now();

        updateSensorDisplay(data);
        updateSensorHistory(data);
        updateCharts();

        console.log("Sensor Data:", data);

    } catch (error) {

        console.error("Sensor fetch error:", error);

    }
}

function startSensorFetching() {

    if (sensorInterval) {
        clearInterval(sensorInterval);
    }

    fetchSensorData();

    sensorInterval = setInterval(() => {

        fetchSensorData();

    }, 1000);
}

// =======================
// SENSOR DISPLAY
// =======================
function updateSensorDisplay(data) {
    const flameEl = document.getElementById('flameValue');
    const flameBadge = document.getElementById('flameBadge');

    if (flameEl) flameEl.textContent = data.flame;
    if (flameBadge) {
        if (data.flame >= 700) {
            flameBadge.textContent = 'DANGER';
            flameBadge.className = 'badge badge-danger';
        } else if (data.flame >= 400) {
            flameBadge.textContent = 'WARNING';
            flameBadge.className = 'badge badge-warning';
        } else {
            flameBadge.textContent = 'SAFE';
            flameBadge.className = 'badge badge-safe';
        }
    }

    const smokeEl = document.getElementById('smokeValue');
    const smokeBadge = document.getElementById('smokeBadge');

    if (smokeEl) smokeEl.textContent = data.smoke;
    if (smokeBadge) {
        if (data.smoke >= 600) {
            smokeBadge.textContent = 'DANGER';
            smokeBadge.className = 'badge badge-danger';
        } else if (data.smoke >= 300) {
            smokeBadge.textContent = 'WARNING';
            smokeBadge.className = 'badge badge-warning';
        } else {
            smokeBadge.textContent = 'SAFE';
            smokeBadge.className = 'badge badge-safe';
        }
    }

    document.getElementById('pirValue') && (document.getElementById('pirValue').textContent = data.pir ? 'DETECTED' : 'CLEAR');
    document.getElementById('ultrasonicValue') && (document.getElementById('ultrasonicValue').textContent = data.ultrasonic + ' cm');
    document.getElementById('tempValue') && (document.getElementById('tempValue').textContent = data.irTemperature.toFixed(1) + '°C');
}

// =======================
// SENSOR HISTORY
// =======================
function updateSensorHistory(data) {
    sensorHistory.push(data);
    if (sensorHistory.length > 20) sensorHistory.shift();
}

// =======================
// CHART INIT
// =======================
function initCharts() {
    const flameCanvas = document.getElementById('flameChart');
    const distanceCanvas = document.getElementById('distanceChart');

    if (!flameCanvas || !distanceCanvas || typeof Chart === "undefined") {
        console.error("Chart.js missing or canvas not found");
        return;
    }

    flameChart = new Chart(flameCanvas.getContext('2d'), {
        type: 'line',
        data: {
            labels: [],
            datasets: [
                { label: 'Flame', data: [], borderColor: '#f97316' },
                { label: 'Smoke', data: [], borderColor: '#6b7280' }
            ]
        }
    });

    distanceChart = new Chart(distanceCanvas.getContext('2d'), {
        type: 'line',
        data: {
            labels: [],
            datasets: [
                { label: 'Distance (cm)', data: [], borderColor: '#a855f7' },
                { label: 'Temp (°C)', data: [], borderColor: '#ef4444' }
            ]
        }
    });
}

// =======================
// CHART UPDATE
// =======================
function updateCharts() {
    if (!flameChart || !distanceChart || sensorHistory.length === 0) return;

    const labels = sensorHistory.map(item =>
        new Date(item.timestamp).toLocaleTimeString()
    );

    flameChart.data.labels = labels;
    flameChart.data.datasets[0].data = sensorHistory.map(item => item.flame);
    flameChart.data.datasets[1].data = sensorHistory.map(item => item.smoke);
    flameChart.update();

    distanceChart.data.labels = labels;
    distanceChart.data.datasets[0].data = sensorHistory.map(item => item.ultrasonic);
    distanceChart.data.datasets[1].data = sensorHistory.map(item => item.irTemperature);
    distanceChart.update();
}

// =======================
// INIT
// =======================
document.addEventListener("DOMContentLoaded", () => {
    initCharts();

    // Fixed ESP32-CAM stream IP for direct browser access
    setCameraStream(FIXED_CAM_IP);
});

// =======================
// UI FUNCTIONS
// =======================
function toggleHumanFollowing() {
    humanFollowing = !humanFollowing;

    const statusEl = document.getElementById('followingStatus');

    if (!statusEl) return;

    if (humanFollowing) {
        statusEl.textContent = 'Following active';

        fetch(`http://${ROBOT_IP}/human_follow?state=on`)
            .catch(err => console.log(err));

        showToast('Enabled', 'Human following ON');

        //startHumanDetection();

    } else {
        statusEl.textContent = 'Autonomous navigation';

        fetch(`http://${ROBOT_IP}/human_follow?state=off`)
            .catch(err => console.log(err));

        //stopHumanDetection();

        showToast('Disabled', 'Human following OFF');
    }
}

function addAIAlert(type, message, confidence = null) {
    alerts.unshift({
        id: Date.now().toString(),
        type,
        message,
        timestamp: new Date(),
        confidence
    });

    if (alerts.length > 8) {
        alerts.pop();
    }

    renderAlerts();
}

function renderAlerts() {
    const container = document.getElementById('alertsContainer');
    if (!container) return;

    if (alerts.length === 0) {
        container.innerHTML = `<div class="alert-item"><div class="alert-content"><p>No alerts yet</p></div></div>`;
        return;
    }

    container.innerHTML = alerts.map(alert => `
        <div class="alert-item">
            <span class="alert-badge badge ${getAlertBadgeClass(alert.type)}">
                ${getAlertIcon(alert.type)}
            </span>
            <div class="alert-content">
                <p>${alert.message}</p>
                <small>${alert.timestamp.toLocaleTimeString()}</small>
            </div>
        </div>
    `).join('');
}

function getAlertIcon(type) {
    return {
        fire: '🔥',
        person: '👤',
        obstacle: '⚠️',
        info: 'ℹ️'
    }[type] || 'ℹ️';
}

function getAlertBadgeClass(type) {
    return {
        fire: 'badge-danger',
        person: 'badge-safe',
        obstacle: 'badge-warning',
        info: 'badge-outline'
    }[type] || 'badge-outline';
}

function toggleLogging() {
    isLogging = !isLogging;

    const badge = document.getElementById('logBadge');
    const downloadBtn = document.getElementById('downloadBtn');

    if (badge) {
        badge.textContent = isLogging ? 'LOGGING ON' : 'LOGGING OFF';
        badge.className = isLogging ? 'badge badge-safe' : 'badge badge-outline';
    }

    if (downloadBtn) downloadBtn.disabled = !isLogging;
}

function downloadLog() {
    if (!isLogging || sensorHistory.length === 0) return;

    const csvContent = [
        'Timestamp,Flame,Smoke,Ultrasonic,Temperature',
        ...sensorHistory.map(item =>
            `${new Date(item.timestamp).toISOString()},${item.flame},${item.smoke},${item.ultrasonic},${item.irTemperature.toFixed(2)}`
        )
    ].join('\n');

    const blob = new Blob([csvContent], { type: 'text/csv' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');

    a.href = url;
    a.download = `sensor_log_${Date.now()}.csv`;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);

    URL.revokeObjectURL(url);
}

function showToast(title, description) {
    const toast = document.getElementById('toast');
    if (!toast) return;

    toast.innerHTML = `
        <div class="toast-title">${title}</div>
        <div class="toast-description">${description}</div>
    `;
    toast.classList.remove('hidden');

    setTimeout(() => {
        toast.classList.add('hidden');
    }, 3000);
}
