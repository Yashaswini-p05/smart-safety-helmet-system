#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <TinyGPS++.h>
#include <math.h>

// ================= WIFI =================
const char* ssid = "Your Network Name";
const char* password = "Your Password";

// ================= EMERGENCY =================
const char* emergencyNumber = "+91XXXXXXXXXX";

// ================= PINS =================
#define DHT_PIN 4
#define MQ135_PIN 34

#define ACCEL_X 32
#define ACCEL_Y 33
#define ACCEL_Z 35

#define LED_PIN 2
#define BUZZER_PIN 15
#define PANIC_PIN 13
#define COOLING_PIN 25

#define GPS_RX 26
#define GPS_TX 27

#define SIM_RX 16
#define SIM_TX 17

// ================= OBJECTS =================
DHT dht(DHT_PIN, DHT11);
TinyGPSPlus gps;
WebServer server(80);
HardwareSerial gpsSerial(1);
HardwareSerial sim800(2);

// ================= VARIABLES =================
float temp = 0, hum = 0;
int gas = 0;

int xRaw = 0, yRaw = 0, zRaw = 0;
int xBase = 0, yBase = 0, zBase = 0;
float magnitude = 0;

bool fall = false;
bool panicPressed = false;
bool threat = false;

// simulated MAX values
float heartRate = 82;
float spo2 = 98;
unsigned long lastSim = 0;

// GPS
double lat = 0, lng = 0;
bool gpsValid = false;
String gpsStatus = "SEARCHING...";

// WiFi
String wifiStatusText = "CONNECTED";

// timing
unsigned long lastRead = 0;
unsigned long lastSMS = 0;
unsigned long lastBlink = 0;
unsigned long lastWiFiCheck = 0;
bool ledState = false;

// thresholds
float tempTh = 40;
int gasTh = 2000;
float moveTh = 220;
float impactTh = 700;

// ===================================================
void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(PANIC_PIN, INPUT_PULLUP);
  pinMode(COOLING_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(COOLING_PIN, LOW);

  dht.begin();
  analogReadResolution(12);

  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  sim800.begin(9600, SERIAL_8N1, SIM_RX, SIM_TX);

  calibrateAccel();

  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiStatusText = "CONNECTED";
    Serial.println("\nConnected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    wifiStatusText = "FAILED";
    Serial.println("\nWiFi failed");
  }

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("HTTP server started");
}

// ===================================================
void loop() {
  server.handleClient();

  simulateMAX();
  readGPS();
  ensureWiFi();

  if (millis() - lastRead >= 2000) {
    readSensors();
    alertLogic();
    lastRead = millis();
  }
}

// ===================================================
void calibrateAccel() {
  long sx = 0, sy = 0, sz = 0;

  Serial.println("Calibrating accelerometer... keep still");
  for (int i = 0; i < 100; i++) {
    sx += analogRead(ACCEL_X);
    sy += analogRead(ACCEL_Y);
    sz += analogRead(ACCEL_Z);
    delay(10);
  }

  xBase = sx / 100;
  yBase = sy / 100;
  zBase = sz / 100;

  Serial.print("Base X: "); Serial.println(xBase);
  Serial.print("Base Y: "); Serial.println(yBase);
  Serial.print("Base Z: "); Serial.println(zBase);
}

// ===================================================
void simulateMAX() {
  if (millis() - lastSim < 2000) return;
  lastSim = millis();

  heartRate += random(-3, 4);
  spo2 += random(-1, 2);

  heartRate = constrain(heartRate, 72, 105);
  spo2 = constrain(spo2, 95, 100);

  if (random(0, 20) == 5) {
    heartRate = random(120, 140);
    spo2 = random(85, 90);
  }
}

// ===================================================
void readSensors() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t)) temp = t;
  if (!isnan(h)) hum = h;

  gas = analogRead(MQ135_PIN);

  xRaw = analogRead(ACCEL_X);
  yRaw = analogRead(ACCEL_Y);
  zRaw = analogRead(ACCEL_Z);

  int dx = xRaw - xBase;
  int dy = yRaw - yBase;
  int dz = zRaw - zBase;

  magnitude = sqrt((float)(dx * dx) + (float)(dy * dy) + (float)(dz * dz));

  fall = (magnitude > impactTh);
  panicPressed = (digitalRead(PANIC_PIN) == LOW);

  Serial.println("----- DATA -----");
  Serial.print("Temp: "); Serial.println(temp);
  Serial.print("Hum: "); Serial.println(hum);
  Serial.print("Gas: "); Serial.println(gas);
  Serial.print("HR: "); Serial.println(heartRate);
  Serial.print("SpO2: "); Serial.println(spo2);
  Serial.print("Mag: "); Serial.println(magnitude);
  Serial.print("Fall: "); Serial.println(fall ? "YES" : "NO");
  Serial.println("----------------");
}

// ===================================================
void readGPS() {
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isValid()) {
    lat = gps.location.lat();
    lng = gps.location.lng();
    gpsValid = true;
    gpsStatus = "FIXED";
  } else {
    gpsValid = false;
    gpsStatus = "SEARCHING...";
  }
}

// ===================================================
void ensureWiFi() {
  if (millis() - lastWiFiCheck < 3000) return;
  lastWiFiCheck = millis();

  if (WiFi.status() == WL_CONNECTED) {
    wifiStatusText = "CONNECTED";
    return;
  }

  wifiStatusText = "DISCONNECTED";

  if (millis() - lastSMS > 15000) {
    sendSMS("WIFI FAILURE - DASHBOARD OFFLINE");
    lastSMS = millis();
  }

  WiFi.disconnect();
  WiFi.begin(ssid, password);
}

// ===================================================
void alertLogic() {
  threat = false;
  String reason = "";

  if (temp > tempTh) {
    threat = true;
    reason += "HIGH TEMP ";
    digitalWrite(COOLING_PIN, HIGH);
  } else {
    digitalWrite(COOLING_PIN, LOW);
  }

  if (gas > gasTh) {
    threat = true;
    reason += "GAS LEAKAGE ";
  }

  if (fall) {
    threat = true;
    reason += "FALL DETECTED ";
  }

  if (panicPressed) {
    threat = true;
    reason += "PANIC BUTTON ";
  }

  if (heartRate > 120) {
    threat = true;
    reason += "HIGH HR ";
  }

  if (spo2 < 90) {
    threat = true;
    reason += "LOW SPO2 ";
  }

  digitalWrite(BUZZER_PIN, threat ? HIGH : LOW);

  if (threat) {
    if (millis() - lastBlink > 300) {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
      lastBlink = millis();
    }

    if (millis() - lastSMS > 30000) {
      sendSMS(reason);
      lastSMS = millis();
    }
  } else {
    digitalWrite(LED_PIN, LOW);
    ledState = false;
  }
}

// ===================================================
void sendSMS(String reason) {
  String msg = "SMART HELMET ALERT\n";
  msg += "Reason: " + reason + "\n";
  msg += "Temp: " + String(temp, 1) + "\n";
  msg += "Gas: " + String(gas) + "\n";
  msg += "HR: " + String(heartRate, 0) + "\n";
  msg += "SpO2: " + String(spo2, 0) + "\n";

  msg += "Maps: https://maps.google.com/?q=";
  if (gpsValid) msg += String(lat, 6) + "," + String(lng, 6);
  else msg += "17.3850,78.4867";

  sim800.println("AT+CMGF=1");
  delay(500);
  sim800.println("AT+CMGS=\"" + String(emergencyNumber) + "\"");
  delay(500);
  sim800.print(msg);
  delay(500);
  sim800.write(26);
  delay(1500);
}

// ===================================================
void handleRoot() {
  server.send(200, "text/html", getHTML());
}

void handleData() {
  StaticJsonDocument<512> doc;
  doc["temp"] = temp;
  doc["hum"] = hum;
  doc["gas"] = gas;
  doc["hr"] = heartRate;
  doc["spo2"] = spo2;
  doc["fall"] = fall;
  doc["mag"] = magnitude;
  doc["motion"] = (magnitude > impactTh ? "IMPACT DETECTED" : (magnitude > moveTh ? "MOVEMENT" : "STABLE"));
  doc["gps"] = gpsStatus;
  doc["lat"] = lat;
  doc["lng"] = lng;
  doc["wifi"] = wifiStatusText;
  doc["panic"] = panicPressed;
  doc["threat"] = threat;

  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

// ===================================================
String getHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Smart Helmet Dashboard</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:Arial,sans-serif;background:linear-gradient(135deg,#0f172a,#1d4ed8);color:white;min-height:100vh;padding:24px}
.header{text-align:center;margin-bottom:24px}
.header h1{font-size:40px;margin-bottom:8px}
.header p{color:rgba(255,255,255,0.8)}
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:18px}
.card{background:rgba(255,255,255,0.10);border:1px solid rgba(255,255,255,0.12);backdrop-filter:blur(10px);border-radius:20px;padding:20px;box-shadow:0 8px 24px rgba(0,0,0,0.25)}
.label{font-size:15px;color:rgba(255,255,255,0.8);margin-bottom:10px}
.value{font-size:22px;font-weight:700;line-height:1.3}
.small{font-size:14px;color:rgba(255,255,255,0.72);margin-top:8px}
.ok{color:#22c55e}.bad{color:#ef4444}.warn{color:#facc15}.wide{grid-column:span 2}
@media (max-width:700px){.wide{grid-column:span 1}.header h1{font-size:30px}}
</style>
</head>
<body>
<div class="header">
  <h1>Smart Helmet Dashboard</h1>
  <p>Construction worker safety monitoring system</p>
</div>

<div class="grid">
  <div class="card"><div class="label">Temperature</div><div class="value" id="temp">--</div></div>
  <div class="card"><div class="label">Humidity</div><div class="value" id="hum">--</div></div>
  <div class="card"><div class="label">Gas Level</div><div class="value" id="gas">--</div></div>
  <div class="card"><div class="label">Heart Rate</div><div class="value" id="hr">--</div></div>
  <div class="card"><div class="label">SpO2</div><div class="value" id="spo2">--</div></div>
  <div class="card"><div class="label">Fall Detection</div><div class="value" id="fall">--</div></div>
  <div class="card"><div class="label">Motion Status</div><div class="value" id="motion">--</div></div>
  <div class="card"><div class="label">GPS Status</div><div class="value" id="gps">--</div><div class="small" id="gpscoord">Lat: --, Lng: --</div></div>
  <div class="card"><div class="label">WiFi Status</div><div class="value" id="wifi">--</div></div>
  <div class="card"><div class="label">Panic Button</div><div class="value" id="panic">--</div></div>
  <div class="card"><div class="label">Threat Status</div><div class="value" id="threat">--</div></div>
  <div class="card wide"><div class="label">Accelerometer Magnitude</div><div class="value" id="mag">--</div></div>
</div>

<script>
function setText(id,text){document.getElementById(id).innerText=text;}
function setStatus(id,text,cls){const el=document.getElementById(id);el.innerText=text;el.className='value '+cls;}

async function updateData(){
  try{
    const res=await fetch('/data');
    const d=await res.json();

    if(d.temp>40) setStatus('temp', d.temp.toFixed(1)+' °C','bad');
    else setStatus('temp', d.temp.toFixed(1)+' °C','ok');

    setText('hum', d.hum.toFixed(1)+' %');

    if(d.gas>2000) setStatus('gas', d.gas,'bad');
    else setStatus('gas', d.gas,'ok');

    if(d.hr>120) setStatus('hr', Number(d.hr).toFixed(0)+' bpm','bad');
    else setText('hr', Number(d.hr).toFixed(0)+' bpm');

    if(d.spo2>0 && d.spo2<90) setStatus('spo2', Number(d.spo2).toFixed(0)+' %','bad');
    else setText('spo2', Number(d.spo2).toFixed(0)+' %');

    if(d.fall) setStatus('fall','FALL DETECTED','bad');
    else setStatus('fall','SAFE','ok');

    if(d.motion==='IMPACT DETECTED') setStatus('motion',d.motion,'bad');
    else if(d.motion==='MOVEMENT') setStatus('motion',d.motion,'warn');
    else setStatus('motion',d.motion,'ok');

    if(d.gps==='FIXED') setStatus('gps',d.gps,'ok');
    else setStatus('gps',d.gps,'warn');

    setText('gpscoord','Lat: '+Number(d.lat).toFixed(6)+' , Lng: '+Number(d.lng).toFixed(6));

    if(d.wifi==='CONNECTED') setStatus('wifi',d.wifi,'ok');
    else setStatus('wifi',d.wifi,'bad');

    if(d.panic) setStatus('panic','PRESSED','bad');
    else setStatus('panic','NORMAL','ok');

    if(d.threat) setStatus('threat','ALERT','bad');
    else setStatus('threat','SAFE','ok');

    if(Number(d.mag)>700) setStatus('mag',Number(d.mag).toFixed(0)+' | IMPACT','bad');
    else if(Number(d.mag)>220) setStatus('mag',Number(d.mag).toFixed(0)+' | MOVEMENT','warn');
    else setStatus('mag',Number(d.mag).toFixed(0)+' | STABLE','ok');

  } catch(e) {
    console.log(e);
  }
}
updateData();
setInterval(updateData,2000);
</script>
</body>
</html>
)rawliteral";
}

