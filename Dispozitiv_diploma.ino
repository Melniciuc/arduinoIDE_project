#include <WiFi.h>
#include <WebServer.h>
#include <TinyGPS++.h>
#include <Wire.h>
#include <MPU6050.h>
#include <HardwareSerial.h>

// Configurare module
TinyGPSPlus gps;
MPU6050 mpu;
WebServer server(80);

// Configurare GPS
#define RXD2 16  
#define TXD2 17  
HardwareSerial gpsSerial(2);

// Configurare Server
const char* ssid = "iPhone 12 Pro";
const char* password = "123454321";

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConectat la WiFi");
    Serial.print("Adresa IP: ");
    Serial.println(WiFi.localIP());

    server.on("/data", handleData);
    server.on("/", handleRoot);
    server.begin();

    Wire.begin();
    mpu.initialize();

    gpsSerial.begin(115200, SERIAL_8N1, RXD2, TXD2);
}

// Variabile pentru date
float speedKmh = 0.0;
float latitude = 0.0, longitude = 0.0;
float accelX, accelY, accelZ;
float gyroX, gyroY, gyroZ;

// Citire date MPU
void updateSensors() {
    accelX = mpu.getAccelerationX() / 16384.0;
    accelY = mpu.getAccelerationY() / 16384.0;
    accelZ = mpu.getAccelerationZ() / 16384.0;
    gyroX = mpu.getRotationX() / 131.0;
    gyroY = mpu.getRotationY() / 131.0;
    gyroZ = mpu.getRotationZ() / 131.0;
}

// Citire date GPS
void loop() {
    while (gpsSerial.available()) {
        gps.encode(gpsSerial.read());
        if (gps.location.isUpdated()) {
            latitude = gps.location.lat();
            longitude = gps.location.lng();
        }
        if (gps.speed.isUpdated()) {
            speedKmh = gps.speed.kmph();
        }
    }
    updateSensors();

    server.handleClient();
}

unsigned long lastGPSUpdate = 0;
unsigned long gpsUpdateInterval = 500;  

// Pagina Web
void handleRoot() {
    String page = "<html><head><meta charset='UTF-8'>";
    page += "<script>";
    page += "function updateData() {";
    page += "  fetch('/data')"; 
    page += "    .then(response => response.json())";
    page += "    .then(data => {";
    page += "      document.getElementById('speed').innerText = data.speed + ' km/h';";
    page += "      document.getElementById('location').innerText = data.latitude + ', ' + data.longitude;";
    page += "      document.getElementById('accel').innerText = 'X=' + data.accelX + ' Y=' + data.accelY + ' Z=' + data.accelZ;";
    page += "      document.getElementById('gyro').innerText = 'X=' + data.gyroX + ' Y=' + data.gyroY + ' Z=' + data.gyroZ;";
    page += "    });";
    page += "}";
    page += "setInterval(updateData, 500);";
    page += "</script></head><body>";
    page += "<h1>Monitorizare transport public</h1>";
    page += "<p>Viteza: <span id='speed'>0 km/h</span></p>";
    page += "<p>Locatie: <span id='location'>0, 0</span></p>";
    page += "<p>Accelerometru: <span id='accel'>X=0 Y=0 Z=0</span></p>";
    page += "<p>Giroscop: <span id='gyro'>X=0 Y=0 Z=0</span></p>";
    page += "</body></html>";

    server.send(200, "text/html", page);
}

void handleData() {
    String json = "{";
    json += "\"speed\":" + String(speedKmh) + ",";
    json += "\"latitude\":" + String(latitude, 6) + ",";
    json += "\"longitude\":" + String(longitude, 6) + ",";
    json += "\"accelX\":" + String(accelX) + ",";
    json += "\"accelY\":" + String(accelY) + ",";
    json += "\"accelZ\":" + String(accelZ) + ",";
    json += "\"gyroX\":" + String(gyroX) + ",";
    json += "\"gyroY\":" + String(gyroY) + ",";
    json += "\"gyroZ\":" + String(gyroZ);
    json += "}";

    server.send(200, "application/json", json);
}
