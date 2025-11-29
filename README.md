# SmartSite - IoT Construction Safety Monitoring System

Real-time health monitoring for construction workers using wearable sensors and automated emergency response. Built with ESP32, MQTT, and various actuators to help supervisors respond faster to critical situations.

## What It Does

Workers wear wristbands that track heart rate and body temperature. An ESP32 gateway monitors all the data coming in and checks it against safety thresholds. If someone's vitals hit dangerous levels, the system automatically:
- Calls emergency services via GSM module
- Shuts down nearby heavy machinery 
- Alerts supervisors on the dashboard
- Logs everything for incident reports

The goal is to speed up emergency response, not replace human judgment. Supervisors still make the final calls, but they get alerted instantly when something's wrong.

## How It Works

```
Wristband sensors → MQTT → ESP32 gateway → Checks thresholds → Triggers emergency actions
                                         → Dashboard updates in real-time
```

Each wristband publishes vitals to MQTT topics like `/worker/W003/vitals`. The gateway subscribes to all of them, evaluates the data, and publishes commands to `/emergency/call` or `/equipment/shutdown` when needed.

Dashboard connects via WebSocket to see everything in real-time.

## Hardware

**Worker Wristband:**
- MAX30102: heart rate and SpO2 sensor
- MAX30208: temperature sensor (±0.1°C accuracy)
- ESP32-WROOM: WiFi + MQTT client
- Li-Po battery (runs ~18 hours)

Data packet looks like:
```json
{
  "worker_id": "W003",
  "timestamp": 1703251415,
  "heart_rate": 125,
  "temperature": 39.1,
  "spo2": 94,
  "location": "Zone-B-Floor-3",
  "battery": 78
}
```

**ESP32 Gateway:**
- Collects data from all wristbands
- Runs threshold checks every time data comes in
- Publishes emergency commands
- Logs events to SD card

Safety thresholds:
```c
#define HR_MAX_SAFE      120   // BPM
#define TEMP_MAX_SAFE   38.5   // Celsius
#define SPO2_MIN_SAFE     90   // Percentage
```

**GSM Module (SIM800L/SIM900):**
- Dials emergency services
- Sends SMS to supervisors
- Plays pre-recorded emergency message with worker ID and location

**IoT Power Relay:**
- Remotely cuts power to equipment in the affected zone
- Prevents accidents when someone's having a medical emergency near machinery

## MQTT Topics

```
/worker/{id}/vitals          # Sensor data
/worker/{id}/status          # Worker status
/emergency/call/{id}         # Trigger emergency call
/emergency/sms/{id}          # Send SMS alert
/equipment/{zone}/shutdown   # Cut power
/admin/alerts                # Dashboard notifications
/system/discovery            # SSDP device discovery
```

## Example Flow

Worker gets heat exhaustion:

1. Wristband detects temp 39.1°C, HR 125
2. Publishes to `/worker/W003/vitals`
3. ESP32 sees threshold exceeded
4. Publishes to `/emergency/call/W003` and `/equipment/zone-b/shutdown`
5. GSM module calls medics and texts supervisor
6. Power relay cuts machinery in Zone B
7. Dashboard shows critical alert
8. Supervisor acknowledges and coordinates response

Total time from detection to action: <3 seconds

## Device Discovery

Uses SSDP so new wristbands are automatically detected when they join the network. No manual IP configuration needed - just power on a new wristband and the gateway picks it up.

## Dashboard

Web-based interface showing:
- Real-time vitals for all workers
- Zone-by-zone status
- Active alerts with severity levels
- Recent event log
- Manual override controls

Critical alerts show worker ID, current vitals, location, and action buttons to acknowledge or manually intervene.

## Project Structure

```
SmartSite/
├── firmware/
│   ├── wristband/
│   │   ├── main.cpp
│   │   ├── max30102_driver.cpp
│   │   ├── max30208_driver.cpp
│   │   └── mqtt_client.cpp
│   └── gateway/
│       ├── main.cpp
│       ├── threshold_checker.cpp
│       ├── emergency_handler.cpp
│       └── ssdp_discovery.cpp
├── actuators/
│   ├── gsm_module/
│   │   ├── call_handler.cpp
│   │   └── sms_sender.cpp
│   └── power_relay/
│       └── relay_controller.cpp
├── backend/
│   ├── mqtt_broker/
│   │   └── mosquitto.conf
│   ├── api_server/
│   │   ├── server.js
│   │   └── routes/
│   └── logger/
│       └── event_logger.js
├── dashboard/
│   ├── index.html
│   ├── app.js
│   ├── mqtt_client.js
│   └── styles.css
└── simulation/
    ├── sensor_simulator.py
    ├── scenarios.py
    └── network_sim.py
```

## Setup

Install and configure Mosquitto:
```bash
sudo apt-get install mosquitto mosquitto-clients
sudo nano /etc/mosquitto/mosquitto.conf
```

Add to config:
```
listener 1883
allow_anonymous false
password_file /etc/mosquitto/passwd
```

Flash wristband firmware:
```bash
cd firmware/wristband/
platformio run --target upload --upload-port /dev/ttyUSB0
```

Edit `config.h` with your WiFi and MQTT broker details.

Flash gateway:
```bash
cd firmware/gateway/
platformio run --target upload
```

Test MQTT:
```bash
mosquitto_sub -h 192.168.1.100 -t "/worker/+/vitals"
```

Run dashboard:
```bash
cd dashboard/
npm install
npm start
```

## Testing

Included Python scripts to simulate different scenarios:

Heat exhaustion - gradual temp increase:
```python
simulate_worker_vitals(
    worker_id="W003",
    initial_temp=36.5,
    final_temp=39.5,
    duration_minutes=10
)
```

Cardiac event - sudden HR spike:
```python
simulate_cardiac_event(
    worker_id="W007",
    initial_hr=72,
    spike_hr=145,
    duration_seconds=30
)
```

Device failure:
```python
simulate_device_offline(
    worker_id="W012",
    reason="battery_depleted"
)
```

## Performance

Average response times during testing:
- Sensor reading to MQTT: 85ms
- MQTT to ESP32 processing: 32ms
- Emergency protocol trigger: 380ms
- GSM call initiation: 1.7s
- Dashboard update: 150ms
- **Total detection to action: 2.3s**

System uptime: 99.7% over 30-day test
False positives: <2%
Missed alerts: 0%

## Safety Levels

**Warning** (yellow alert):
- HR 100-120 BPM
- Temp 37.2-38.5°C
- SpO2 90-95%

**Critical** (red alert + emergency actions):
- HR >120 or <40 BPM
- Temp >38.5 or <35°C
- SpO2 <90%

## What Could Be Added

- GPS tracking for outdoor sites
- Fall detection with accelerometer
- Environmental sensors (CO, dust levels, noise)
- ML for predicting issues before they become critical
- Integration with access control systems
- Wearable camera for incident documentation
- Bluetooth mesh for underground/enclosed areas
- Voice-activated emergency button

## Tech Stack

- Microcontroller: ESP32-WROOM-32
- Sensors: MAX30102, MAX30208
- Communication: WiFi 802.11 b/g/n, MQTT
- GSM: SIM800L/SIM900
- Power relay: Digital Loggers
- Broker: Mosquitto MQTT v5.0
- Dashboard: HTML5 + WebSockets
- Backend: Node.js + MongoDB

## Author

Nina Dragićević

## License

MIT
