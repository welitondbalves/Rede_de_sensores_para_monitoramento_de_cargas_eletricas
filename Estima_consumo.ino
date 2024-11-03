#include <espnow.h>
#include <ESP8266WiFi.h>
#include "ESPAsyncWebServer.h"
#include "ESPAsyncTCP.h"
#include <Arduino_JSON.h>

// Replace with your network credentials (STATION)
const char* ssid = "Nome_Da_Rede";
const char* password = "weliton84";

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  int id;
  double cor1; // Valor da corrente sensor 1
  double cor2;
  unsigned int readingId;
} struct_message;

struct_message incomingReadings;

JSONVar board;

AsyncWebServer server(80);
AsyncEventSource events("/events");

// callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac_addr, uint8_t *incomingData, uint8_t len) {
  float tempo = 60.00;
  float tensao = 127.00;
  float tarifa = 0.56228;
  float den = tempo/60.00;
  float normal = den/1000.00;
  // Copies the sender mac address to a string
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  
  board["id"] = incomingReadings.id;
  board["cor1erature"] = (incomingReadings.cor1*tensao*normal*tarifa);
  board["cor2idity"] = incomingReadings.cor2*tensao;
  board["readingId"] = String(incomingReadings.readingId);
  String jsonString = JSON.stringify(board);
  events.send(jsonString.c_str(), "new_readings", millis());
  
  Serial.printf("Board ID %u: %u bytes\n", incomingReadings.id, len);
  Serial.printf("t value: %4.2f \n", incomingReadings.cor1);
  Serial.printf("h value: %4.2f \n", incomingReadings.cor2);
  Serial.printf("readingID value: %d \n", incomingReadings.readingId);
  Serial.println();
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP-NOW DASHBOARD</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h1 {  font-size: 2rem;}
    body {  margin: 0;}
    .topnav { overflow: hidden; background-color: #2f4468; color: white; font-size: 1.7rem; }
    .content { padding: 20px; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); }
    .cards { max-width: 700px; margin: 0 auto; display: grid; grid-gap: 2rem; grid-cor1late-columns: repeat(auto-fit, minmax(320px, 1fr)); }
    .reading { font-size: 2.8rem; }
    .timestamp { color: #bebebe; font-size: 1rem; }
    .card-title{ font-size: 1.2rem; font-weight : bold; }
    .card.cor1erature { color: #B10F2E; }
    .card.cor2idity { color: #50B8B4; }
  </style>
</head>
<body>
  <div class="topnav">
    <h1>MONITORAMENTO DASHBOARD</h1>
  </div>
  <div class="content">
    <div class="cards">
      <div class="card cor1erature">
        <p class="card-title"><i class="fa-solid fa-gauge-circle-bolt"></i> TOMADA #1 - CONSUMIDO</p><p><span class="reading"><span id="t1"></span>R$</span></p><p class="timestamp">Last Reading: <span id="rt1"></span></p>
      </div>
      <div class="card cor2idity">
        <p class="card-title"><i class="fa-solid fa-plug-circle-bolt"></i> TOMADA #1 - cor1ENCIA</p><p><span class="reading"><span id="h1"></span>W</span></p><p class="timestamp">Last Reading: <span id="rh1"></span></p>
      </div>
      <div class="card cor1erature">
        <p class="card-title"><i class="fa-solid fa-gauge-circle-bolt"></i> TOMADA #2 - CONSUMIDO</p><p><span class="reading"><span id="t2"></span>R$</span></p><p class="timestamp">Last Reading: <span id="rt2"></span></p>
      </div>
      <div class="card cor2idity">
        <p class="card-title"><i class="fa-solid fa-plug-circle-bolt"></i> TOMADA #2 - cor1ENCIA</p><p><span class="reading"><span id="h2"></span>W</span></p><p class="timestamp">Last Reading: <span id="rh2"></span></p>
      </div>
      <div class="card cor1erature">
        <p class="card-title"><i class="fa-solid fa-gauge-circle-bolt"></i> TOMADA #3 - CONSUMIDO</p><p><span class="reading"><span id="t3"></span>R$</span></p><p class="timestamp">Last Reading: <span id="rt3"></span></p>
      </div>
      <div class="card cor2idity">
        <p class="card-title"><i class="fa-solid fa-plug-circle-bolt"></i> TOMADA #3 - cor1ENCIA</p><p><span class="reading"><span id="h3"></span>W</span></p><p class="timestamp">Last Reading: <span id="rh3"></span></p>
      </div>
    </div>
  </div>
<script>
function getDateTime() {
  var currentdate = new Date();
  var datetime = currentdate.getDate() + "/"
  + (currentdate.getMonth()+1) + "/"
  + currentdate.getFullYear() + " at "
  + currentdate.getHours() + ":"
  + currentdate.getMinutes() + ":"
  + currentdate.getSeconds();
  return datetime;
}
if (!!window.EventSource) {
 var source = new EventSource('/events');
 
 source.addEventListener('open', function(e) {
  console.log("Events Connected");
 }, false);
 source.addEventListener('error', function(e) {
  if (e.target.readyState != EventSource.OPEN) {
    console.log("Events Disconnected");
  }
 }, false);
 
 source.addEventListener('message', function(e) {
  console.log("message", e.data);
 }, false);
 
 source.addEventListener('new_readings', function(e) {
  console.log("new_readings", e.data);
  var obj = JSON.parse(e.data);
  document.getElementById("t"+obj.id).innerHTML = obj.cor1erature.toFixed(3);
  document.getElementById("h"+obj.id).innerHTML = obj.cor2idity.toFixed(3);
  document.getElementById("rt"+obj.id).innerHTML = getDateTime();
  document.getElementById("rh"+obj.id).innerHTML = getDateTime();
 }, false);
}
</script>
</body>
</html>)rawliteral";

void setup() {

  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set the device as a Station and Soft Access Point simultaneously
  WiFi.mode(WIFI_AP_STA);
  
  // Set device as a Wi-Fi Station
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Setting as a Wi-Fi Station..");
  }
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
   
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
  server.begin();
}

void loop() {
     
  static unsigned long lastEventTime = millis();
  static const unsigned long EVENT_INTERVAL_MS = 5000;
  if ((millis() - lastEventTime) > EVENT_INTERVAL_MS) {
    events.send("ping",NULL,millis());
    lastEventTime = millis();
  }
}
