# 🔥 AI-Powered Fire Detection & Monitoring Robot

> A complete embedded AI system for intelligent fire detection and real-time monitoring — built using distributed ESP32 nodes, TinyML on the edge, and a full web dashboard hosted directly on the microcontroller.


## 📖 About the Project

This project is a fully autonomous, AI-powered fire detection and monitoring system designed using a distributed multi-node ESP32 architecture. Unlike conventional fire alarm systems that rely on simple threshold triggers, this system integrates **TinyML (machine learning on edge hardware)** to provide intelligent, context-aware fire detection directly on the ESP32-CAM — without any cloud dependency.

The system uses **ESP-NOW**, a lightweight wireless protocol, to enable real-time communication between multiple ESP32 nodes across the monitoring area. All sensor data, camera feeds, and AI inference results are aggregated and served through a **fully embedded web dashboard hosted on the ESP32 itself** — no external server, no Raspberry Pi, no laptop required.

This was built solo over 3 months as a hands-on exploration of embedded systems, edge AI, IoT architecture, and real-time firmware development.

---

## ✨ Key Features

- 🧠 **Edge AI / TinyML** — Machine learning model deployed directly on ESP32-CAM using Edge Impulse, enabling on-device fire and smoke pattern recognition without cloud inference
- 📡 **Distributed ESP-NOW Network** — Multiple ESP32 nodes communicate wirelessly in real-time using ESP-NOW protocol for low-latency, connectionless data transfer
- 🌐 **Embedded Web Dashboard** — Full web application (HTML + CSS + JavaScript) hosted directly on ESP32 flash memory using LittleFS and AsyncWebServer — the microcontroller is the server
- 📷 **Live Camera Streaming** — Real-time video feed from ESP32-CAM for visual fire monitoring
- 🔔 **Multi-Sensor Fusion** — 5 sensor types integrated for comprehensive environmental awareness
- 📊 **Real-Time Data Visualization** — Live sensor readings displayed on the web dashboard via JSON APIs
- 🔧 **Zero External Dependencies** — Fully standalone system — no cloud, no external server, no Raspberry Pi

---

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    SYSTEM OVERVIEW                          │
│                                                             │
│  ┌──────────────┐   ESP-NOW    ┌──────────────────────────┐│
│  │  Sensor Node │ ──────────── │     Master ESP32         ││
│  │  ESP32 #1    │              │  • Web Server (Async)    ││
│  │  • Flame     │   ESP-NOW    │  • LittleFS Dashboard    ││
│  │  • Gas/Smoke │ ──────────── │  • JSON REST APIs        ││
│  │  • PIR       │              │  • Wi-Fi AP/STA Mode     ││
│  └──────────────┘              │  • System Coordinator    ││
│                                └──────────────────────────┘│
│  ┌──────────────┐                          │               │
│  │  Vision Node │   ESP-NOW                │ Wi-Fi         │
│  │  ESP32-CAM   │ ─────────────────────────┘               │
│  │  • TinyML    │                          │               │
│  │  • Camera    │                    ┌─────▼──────┐        │
│  │  • Live Feed │                    │  Browser   │        │
│  └──────────────┘                    │  Dashboard │        │
│                                      └────────────┘        │
│  ┌──────────────┐                                          │
│  │  Motion Node │   ESP-NOW                                │
│  │  ESP32 #2    │ ──────────────────────────────────────── │
│  │  • IR Sensor │                                          │
│  │  • Ultrasonic│                                          │
│  └──────────────┘                                          │
└─────────────────────────────────────────────────────────────┘
```

---

## 🔩 Hardware Components

| Component | Quantity | Purpose |
|---|---|---|
| **ESP32 Development Board** | 2 | Sensor nodes + Master controller |
| **ESP32-CAM** | 1 | TinyML inference + live video stream |
| **Flame Detection Sensor** | 1 | Infrared flame sensing |
| **MQ-2 Gas/Smoke Sensor** | 1 | Smoke and gas concentration detection |
| **PIR Motion Sensor** | 1 | Human/movement presence detection |
| **IR Obstacle Sensor** | 2 | Proximity and obstacle detection |
| **HC-SR04 Ultrasonic Sensor** | 1 | Distance measurement |
| **DC Motors + L298N Driver** | - | Robot movement control |
| **LiPo Battery / Power Bank** | 1 | Portable power supply |
| **Voltage Regulator (AMS1117)** | - | Stable 3.3V regulation for ESP modules |
| **Connecting Wires + PCB** | - | Circuit integration |

---

## 💻 Tech Stack

### Firmware & Embedded
| Technology | Usage |
|---|---|
| **Arduino IDE / C++** | Firmware development for all ESP32 nodes |
| **ESP-NOW Protocol** | Low-latency peer-to-peer wireless communication |
| **AsyncWebServer** | Non-blocking web server on ESP32 |
| **LittleFS** | Flash filesystem for storing dashboard files on ESP32 |
| **JSON (ArduinoJson)** | Structured data transfer via REST APIs |
| **Wi-Fi (Station + AP Mode)** | Network connectivity and dashboard hosting |

### AI / Machine Learning
| Technology | Usage |
|---|---|
| **Edge Impulse** | TinyML model training platform |
| **TensorFlow Lite for Microcontrollers** | On-device model inference |
| **ESP32-CAM** | Hardware target for edge AI deployment |

### Dashboard (Hosted on ESP32)
| Technology | Usage |
|---|---|
| **HTML5 / CSS3** | Dashboard structure and styling |
| **JavaScript** | Real-time data fetching and UI updates |
| **Fetch API / XMLHttpRequest** | Polling JSON endpoints for live sensor data |
| **Chart.js** | Sensor data visualization (if implemented) |

---

## ⚙️ How It Works

### 1. Distributed Sensing via ESP-NOW
Each ESP32 node is responsible for a specific sensing zone. Sensor data is packaged into structured payloads and transmitted wirelessly to the master ESP32 using the **ESP-NOW protocol** — a connectionless, ultra-low-latency communication standard developed by Espressif. Unlike MQTT or HTTP, ESP-NOW does not require a Wi-Fi router and works at the MAC layer, enabling sub-millisecond response times between nodes.

### 2. Edge AI on ESP32-CAM
A **TinyML classification model** was trained using **Edge Impulse** on fire and no-fire image datasets. The trained model was exported as an optimized C library (TensorFlow Lite for Microcontrollers format) and flashed directly onto the ESP32-CAM. This allows the camera module to run AI inference locally — classifying fire presence in the camera feed — without sending data to any external server or cloud service.

### 3. Embedded Web Dashboard
The master ESP32 hosts a complete web application from its internal flash memory using **LittleFS** (a lightweight filesystem) and **AsyncWebServer** (a non-blocking HTTP server library). The dashboard files (HTML, CSS, JS) are uploaded to the ESP32's flash storage and served to any browser connected to the same Wi-Fi network. Live sensor readings are exposed through **JSON REST API endpoints**, which the dashboard polls periodically using JavaScript's Fetch API — creating a real-time monitoring experience.

### 4. Multi-Sensor Fusion
Data from all five sensor types is collected, aggregated at the master node, and cross-referenced to determine overall fire risk. A flame sensor alone might trigger on sunlight; combined with elevated gas readings and camera-confirmed fire patterns from the TinyML model, the system achieves significantly higher detection accuracy.

---

## 📁 Project Structure

```
fire-detection-robot/
│
├── Master_ESP32/
│   ├── Master_ESP32.ino          ← Main coordinator firmware
│   ├── web_server.cpp            ← AsyncWebServer setup
│   ├── espnow_handler.cpp        ← ESP-NOW receive callbacks
│   └── data/                     ← LittleFS files (upload via tool)
│       ├── index.html            ← Dashboard HTML
│       ├── style.css             ← Dashboard styling
│       └── app.js                ← Real-time data fetching
│
├── Sensor_Node_1/
│   ├── Sensor_Node_1.ino         ← Flame + Gas + PIR firmware
│   └── espnow_sender.cpp         ← ESP-NOW transmit to master
│
├── Vision_Node_ESP32CAM/
│   ├── Vision_Node.ino           ← Camera + TinyML inference
│   ├── edge_impulse_model/       ← Exported TinyML model files
│   └── camera_handler.cpp        ← Camera stream configuration
│
├── Motion_Node_2/
│   ├── Motion_Node.ino           ← IR + Ultrasonic firmware
│   └── espnow_sender.cpp
│
├── assets/                        ← Photos, videos, diagrams
│   ├── robot_hardware.jpg
│   ├── dashboard_screenshot.jpg
│   ├── architecture.jpg
│   └── demo.gif
│
├── docs/
│   ├── wiring_diagram.pdf        ← Full circuit diagram
│   └── component_list.md
│
└── README.md
```

---

## 🚀 Setup & Installation

### Prerequisites
- Arduino IDE 2.0+
- ESP32 board package installed in Arduino IDE
- Required libraries (install via Library Manager):
  - `ESPAsyncWebServer`
  - `AsyncTCP`
  - `ArduinoJson`
  - `ESP32 LittleFS Upload Tool`
  - Edge Impulse SDK (for Vision Node)

### Step 1 — Flash the Vision Node (ESP32-CAM)
```
1. Open Vision_Node_ESP32CAM/Vision_Node.ino in Arduino IDE
2. Select board: "AI Thinker ESP32-CAM"
3. Select the correct COM port
4. Click Upload
```

### Step 2 — Flash the Sensor Nodes
```
1. Open Sensor_Node_1/Sensor_Node_1.ino
2. Select board: "ESP32 Dev Module"
3. Upload to first sensor ESP32
4. Repeat for Motion_Node_2
```

### Step 3 — Upload Dashboard to Master ESP32 (LittleFS)
```
1. Install ESP32 LittleFS Upload Tool plugin in Arduino IDE
2. Open Master_ESP32/Master_ESP32.ino
3. Go to Tools → ESP32 LittleFS Data Upload
4. This uploads the HTML/CSS/JS files to flash memory
5. Then upload the main firmware normally
```

### Step 4 — Connect & Monitor
```
1. Power all nodes
2. Connect your phone/laptop to the ESP32 Wi-Fi AP
   SSID: FireBot_Dashboard
   Password: [set in firmware]
3. Open browser → http://192.168.4.1
4. Dashboard loads from ESP32 flash memory
```

---

## 📡 ESP-NOW Communication Protocol

```
Data Payload Structure (sent from sensor nodes to master):

typedef struct SensorData {
  int   node_id;          // Which node sent this
  float flame_value;      // Flame sensor ADC reading
  float gas_ppm;          // Gas concentration in PPM
  int   pir_state;        // 1 = motion, 0 = no motion
  float distance_cm;      // Ultrasonic distance
  int   ir_state;         // IR obstacle detected
  int   fire_detected;    // TinyML result: 1 = fire, 0 = clear
} SensorData;
```

---

## 🧠 TinyML Model Details

| Parameter | Value |
|---|---|
| **Training Platform** | Edge Impulse |
| **Model Type** | Image Classification |
| **Input** | 96x96 grayscale image |
| **Classes** | `fire`, `no_fire` |
| **Target Device** | ESP32-CAM |
| **Inference Time** | ~300-500ms per frame |
| **Model Size** | < 100KB (optimized for flash) |
| **Framework** | TensorFlow Lite for Microcontrollers |

---

## 📈 Future Improvements

- [ ] Add SMS/Email alert system using ESP32 + SMTP
- [ ] Integrate with MQTT broker for cloud monitoring
- [ ] Train improved TinyML model with larger dataset
- [ ] Add battery level monitoring across all nodes
- [ ] Implement OTA (Over-the-Air) firmware updates
- [ ] Add GPS module for location tracking
- [ ] Upgrade dashboard with WebSocket for true real-time updates (instead of polling)
- [ ] Integrate with Streamlit/FastAPI web app for remote monitoring

---

## 👨‍💻 Author

**Anuj Dubey**
B.Tech CSE | IPS-CTM, RGPV, Gwalior

[![LinkedIn](https://img.shields.io/badge/LinkedIn-Connect-blue)](https://linkedin.com/in/anuj-dubey-60a6b7338)
[![GitHub](https://img.shields.io/badge/GitHub-Follow-black)](https://github.com/AnujDubey1506)

---

## ⭐ If this project helped you or you found it interesting, leave a star!

