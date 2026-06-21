# Smart Safety Helmet System

An IoT-based smart helmet developed using ESP32 and multiple sensors to enhance construction worker safety through real-time health monitoring, environmental sensing, fall detection, GPS tracking, and emergency alerts.

---

## Overview

The Smart Safety Helmet System is an IoT-based safety monitoring solution developed for construction workers to improve workplace safety through real-time health, environmental, and emergency monitoring.

The system utilizes an ESP32 microcontroller integrated with multiple sensors to monitor worker health conditions, detect hazardous environmental conditions, identify falls or accidents, track worker location, and generate emergency alerts when necessary.

An HTML-based dashboard was also developed to provide a user-friendly interface for monitoring sensor readings and system status in real time.

---

## Objectives

* Enhance construction worker safety through continuous monitoring.
* Detect hazardous environmental conditions.
* Monitor worker health parameters in real time.
* Provide rapid emergency response through alerts and location tracking.
* Improve situational awareness using a centralized monitoring dashboard.

---

## Features

* Heart Rate Monitoring
* SpO₂ Monitoring
* Hazardous Gas Detection
* Temperature and Humidity Monitoring
* High Temperature Detection
* Fall Detection
* GPS Location Tracking
* GSM-Based Emergency Alerts
* Emergency SOS Push Button
* Real-Time Monitoring Dashboard

---

## Hardware Components

| Component         | Purpose                             |
| ----------------- | ----------------------------------- |
| ESP32             | Main Controller                     |
| MAX30102          | Heart Rate and SpO₂ Monitoring      |
| MQ135             | Gas Detection                       |
| MPU6050           | Fall Detection                      |
| DHT22             | Temperature and Humidity Monitoring |
| MAX6675           | High Temperature Monitoring         |
| NEO-6M GPS        | Location Tracking                   |
| SIM800L GSM       | Emergency Alert Communication       |
| Buzzer            | Local Safety Alarm                  |
| Push Button       | Emergency SOS Trigger               |
| TP4056            | Battery Charging Module             |
| Li-ion Battery    | Power Supply                        |
| Voltage Regulator | Stable Power Distribution           |

---

## System Architecture

The following diagram illustrates the overall architecture of the Smart Safety Helmet System, including sensor integration, communication modules, and monitoring components.

https://github.com/Yashaswini-p05/smart-safety-helmet-system/blob/231fb3a392976602512e0e5977e8472a6532e0a9/IMAGES/Smart%20Safety%20Helmet%20Architecture.png

---

## Dashboard Interface

The project includes an HTML-based dashboard designed to display worker health parameters, environmental conditions, and system status in real time.

---

## Working Principle

1. The ESP32 continuously collects data from all connected sensors.
2. MAX30102 monitors worker heart rate and oxygen saturation levels.
3. MQ135 detects harmful gases in the surrounding environment.
4. DHT22 measures ambient temperature and humidity.
5. MAX6675 monitors high-temperature conditions.
6. MPU6050 detects sudden impacts, abnormal movements, or worker falls.
7. GPS provides real-time location information.
8. SIM800L sends emergency notifications when hazardous conditions are detected.
9. The buzzer provides immediate local alerts.
10. The HTML dashboard displays real-time sensor readings and system status.

---

## Project Demonstration

A demonstration video explaining the system architecture, hardware connections, dashboard functionality, and working process is available below:

🎥 **Demo Video**

[Watch Project Demo](https://drive.google.com/file/d/1TEYZMR_OBnqAHlm1Ht3ouYHAIW8U-2rY/view?usp=drive_link)

---

## Applications

* Construction Site Safety
* Industrial Worker Monitoring
* Hazardous Environment Monitoring
* Emergency Response Systems
* Smart Workplace Safety Solutions

---

## Future Enhancements

* Mobile Application Integration
* Cloud-Based Data Storage
* Real-Time Analytics Dashboard
* Predictive Safety Monitoring
* Multi-Worker Monitoring System

---

## Technologies Used

* ESP32
* Arduino IDE
* Embedded C/C++
* HTML
* IoT Sensors
* GPS Technology
* GSM Communication

---

## Author

**Yashaswini Polakonda**

Bachelor of Engineering (B.E)

Major Project – Smart Safety Helmet System
