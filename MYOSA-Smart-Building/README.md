Date 24-05-2026
MYOSA Smart Building Monitoring and Safety System
<img src="./assets/images/project_views/cover.jpg" width="900">
Introduction

The MYOSA Smart Building Monitoring and Safety System is an IoT-based real-time structural monitoring solution developed using ESP32 and multiple smart sensors. The project is designed to improve building safety by continuously monitoring environmental conditions, vibration intensity, structural tilt, light levels, and emergency situations using embedded electronics and cloud connectivity.

The system combines local monitoring through an OLED display and remote monitoring through the Blynk IoT platform, providing real-time alerts and live data visualization.

Problem Statement

Modern buildings and infrastructures are vulnerable to environmental changes, abnormal vibrations, structural movement, and safety risks. Traditional monitoring systems are often expensive and inaccessible for small-scale applications.

This project aims to develop a compact, affordable, and efficient smart monitoring system capable of detecting abnormal structural conditions and providing real-time alerts using IoT technology.

Objectives
.Real-time structural monitoring
.Tilt and vibration detection
.Earthquake alert monitoring
.Environmental temperature and pressure monitoring
.Ambient light sensing
.OLED-based local monitoring
.IoT-based remote monitoring using Blynk
.Emergency buzzer alert system
.Smart safety monitoring solution
Components Used
Component	   Purpose
.ESP32	       Main microcontroller and WiFi communication
.MPU6050	   Tilt, vibration, and earthquake detection
.BMP180	       Temperature and pressure monitoring
.APDS9960	   Ambient light sensing
.OLED Display  Local real-time monitoring
.Buzzer	       Emergency alert indication
.Blynk         IoT Platform	Remote cloud monitoring

Working Principle

The ESP32 microcontroller continuously collects data from all connected sensors through the I2C communication protocol. The MPU6050 sensor monitors structural movement, tilt angle, and vibration intensity for earthquake and instability detection. The BMP180 sensor measures atmospheric pressure and environmental temperature conditions in real time. The APDS9960 sensor monitors ambient light intensity.

All sensor readings are displayed locally through the OLED display and simultaneously transmitted to the Blynk cloud platform using WiFi connectivity. During abnormal conditions such as excessive vibration, earthquake-like movement, or wall break situations, the system activates emergency alerts through the OLED display and buzzer notification system.

Features

.Real-time sensor monitoring
.OLED live monitoring interface
.WiFi-enabled IoT connectivity
.Remote monitoring using Blynk
.Tilt angle monitoring
.Earthquake and vibration detection
.Temperature and pressure monitoring
.Ambient light sensing
.Emergency buzzer alerts
.Live cloud dashboard visualization

Project Architecture
<img src="./docs/circuit_diagram.png" width="900">

Project Images
Front View
<img src="./assets/images/project_views/front_view.jpg" width="800">
Sensors Used
<img src="./assets/images/sensors/pressure_light_gyro_sensors.jpg" width="800">
<img src="./assets/images/sensors/oled.jpg" width="800">

<img src="./assets/images/sensors/ESP32.jpg" width="800">
OLED Monitoring Interface
Environmental Monitoring
<img src="./assets/images/oled_interface/environment_monitor.jpg" width="700">
Light Sensor Monitoring
<img src="./assets/images/oled_interface/light_sensor.jpg" width="700">
Network Status
<img src="./assets/images/oled_interface/network_status.jpg" width="700">
Live Tilt Monitoring
<img src="./assets/images/oled_interface/tilt_live.jpg" width="700">
Alert System
Earthquake Detection
<img src="./assets/images/alerts/earthquake_alert.jpg" width="700">
Tilt Monitoring Alert
<img src="./assets/images/alerts/tilt_monitor.jpg" width="700">
Wall break status
<img src="./assets/images/alerts/wallbreak_alert.jpg" width="700">
Blynk IoT Dashboard
Mobile Dashboard
<img src="./assets/images/app_blynk/blynk1_.jpg" width="800">

<img src="./assets/images/app_blynk/blynk2.jpg" width="800">

Software Used

.Arduino IDE
.ESP32 Board Package
.Blynk IoT Platform
.GitHub
.Embedded C++

Applications

.Smart building monitoring
.Structural safety systems
.Educational IoT projects
.Smart infrastructure monitoring
.Real-time environmental monitoring
.Earthquake and vibration analysis

Future Scope

Future improvements can include:

.Smoke and gas leak detection
.AI-based structural prediction systems
.CCTV integration
.Smart city infrastructure integration
.Cloud data analytics
.Mobile push notification system
.Advanced automation systems

## Demo Video

<video controls width="100%">
  <source src="./videos/demo_video.mp4" type="video/mp4">
</video>

If the video player does not appear, open directly here:

[Open Demo Video](./videos/demo_video.mp4)
File structure

MYOSA-Smart-Building/
│
├── README.md
│
├── code/
│   └── smart_building.ino
│
├── assets/
│   └── images/
│       ├── project_views/
│       ├── oled_interface/
│       ├── app_blynk/
│       ├── alerts/
│       └── sensors/
│
├── videos/
│   └── demo_video.mp4
│
└── docs/
    └── circuit_diagram.png
    
Conclusion

The MYOSA Smart Building Monitoring and Safety System demonstrates the implementation of IoT technology for real-time structural and environmental monitoring. The project successfully integrates multiple sensors, OLED visualization, cloud connectivity, and emergency alert systems into a compact and efficient monitoring platform.

The system provides an affordable and scalable solution for smart safety monitoring applications.
