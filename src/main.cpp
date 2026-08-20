#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// --- PINES ---
const int PIN_DS18B20 = 4;
const int PIN_ENCODER = 33;
const int PIN_MOTOR_ENA = 25;
const int PIN_MOTOR_IN1 = 26;
const int PIN_MOTOR_IN2 = 27;
const int PIN_SSR = 18;

OneWire oneWire(PIN_DS18B20);
DallasTemperature sensors(&oneWire);
AsyncWebServer server(80);

// Variables de sensores
float temperatura = 0.0;
volatile int pulsos = 0;
int rpm = 0;
int current_pwm = 0;

unsigned long tiempo_anterior = 0;
String log_datos = "Tiempo(s),PWM,RPM,Temperatura(C)\n";
unsigned long inicio_ms = 0;

void IRAM_ATTR contarPulsos() { pulsos++; }

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
    <title>Reactor Batch G5</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        body { font-family: sans-serif; background: #1a1a2e; color: white; padding: 20px; text-align: center; }
        .container { display: flex; flex-wrap: wrap; justify-content: center; gap: 15px; }
        .card { background: #16213e; border-radius: 15px; padding: 15px; width: 300px; border: 1px solid #0f3460; }
        .data-val { font-size: 32px; font-weight: bold; color: #00d2ff; }
        .slider { width: 100%; margin: 15px 0; }
        .btn { background: #e94560; color: white; border: none; padding: 10px; border-radius: 5px; cursor: pointer; width: 100%; font-weight: bold; }
        .chart-box { background: white; border-radius: 10px; margin: 15px auto; padding: 10px; max-width: 600px; }
    </style>
</head>
<body>
    <h1>REACTOR BATCH - GRUPO 5</h1>
    <div class="container">
        <div class="card">
            <div>AGITADOR (RPM)</div>
            <div id="rpm_val" class="data-val">0</div>
            <input type="range" min="0" max="255" value="0" class="slider" oninput="updateMotor(this.value)">
            <div>PWM: <span id="pwm_label">0</span></div>
        </div>
        <div class="card">
            <div>TEMPERATURA (C)</div>
            <div id="temp_val" class="data-val">0.0</div>
            <input type="range" min="0" max="100" value="0" class="slider" oninput="updateHeat(this.value)">
            <div>Potencia: <span id="heat_label">0</span>%</div>
        </div>
    </div>
    <div style="margin-top:15px;"><button class="btn" onclick="downloadData()">DESCARGAR DATOS EXCEL (CSV)</button></div>
    
    <div class="chart-box"><canvas id="tempChart"></canvas></div>
    <div class="chart-box"><canvas id="rpmChart"></canvas></div>

    <script>
        function updateMotor(v) { document.getElementById('pwm_label').innerHTML = v; fetch('/set_pwm?v=' + v); }
        function updateHeat(v) { document.getElementById('heat_label').innerHTML = v; fetch('/set_heat?v=' + v); }
        function downloadData() { window.location.href = "/download"; }

        function createChart(id, label, color) {
            return new Chart(document.getElementById(id), {
                type: 'line',
                data: { labels: [], datasets: [{ label: label, borderColor: color, data: [], fill: false }] },
                options: { responsive: true, animation: false }
            });
        }
        var tChart = createChart('tempChart', 'Temperatura (C)', 'red');
        var rChart = createChart('rpmChart', 'Velocidad (RPM)', 'blue');

        setInterval(function() {
            fetch('/data').then(res => res.json()).then(data => {
                document.getElementById('rpm_val').innerHTML = data.rpm;
                document.getElementById('temp_val').innerHTML = data.temp;
                let t = new Date().toLocaleTimeString();
                [tChart, rChart].forEach((c, i) => {
                    c.data.labels.push(t);
                    c.data.datasets[0].data.push(i==0 ? data.temp : data.rpm);
                    if(c.data.labels.length > 30) { c.data.labels.shift(); c.data.datasets[0].data.shift(); }
                    c.update();
                });
            });
        }, 2000);
    </script>
</body>
</html>)rawliteral";

void setup() {
    Serial.begin(115200);
    pinMode(PIN_MOTOR_ENA, OUTPUT); pinMode(PIN_MOTOR_IN1, OUTPUT); pinMode(PIN_MOTOR_IN2, OUTPUT);
    pinMode(PIN_SSR, OUTPUT); pinMode(PIN_ENCODER, INPUT_PULLUP);
    sensors.begin();
    attachInterrupt(digitalPinToInterrupt(PIN_ENCODER), contarPulsos, RISING);
    WiFi.softAP("Reactor_Batch_G5", "control_industrial");
    inicio_ms = millis();

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){ request->send_P(200, "text/html", index_html); });
    
    server.on("/set_pwm", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("v")) {
            current_pwm = request->getParam("v")->value().toInt();
            digitalWrite(PIN_MOTOR_IN1, HIGH); digitalWrite(PIN_MOTOR_IN2, LOW);
            analogWrite(PIN_MOTOR_ENA, current_pwm);
        }
        request->send(200, "text/plain", "OK");
    });

    server.on("/set_heat", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("v")) {
            digitalWrite(PIN_SSR, request->getParam("v")->value().toInt() > 0 ? HIGH : LOW);
        }
        request->send(200, "text/plain", "OK");
    });

    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "application/json", "{\"temp\":" + String(temperatura) + ",\"rpm\":" + String(rpm) + "}");
    });

    server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/csv", log_datos);
    });

    server.begin();
}

void loop() {
    if (millis() - tiempo_anterior >= 1000) {
        rpm = (pulsos * 60) / 20;
        pulsos = 0;
        sensors.requestTemperatures();
        temperatura = sensors.getTempCByIndex(0);
        
        // Guardar en el LOG para el CSV
        log_datos += String((millis()-inicio_ms)/1000) + "," + String(current_pwm) + "," + String(rpm) + "," + String(temperatura) + "\n";
        
        tiempo_anterior = millis();
    }
}