#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>
//declare DHT

// Update these with values suitable for your network.

const char* ssid = "Al Bashiir";
const char* password = "r4h4s144";
const char* mqtt_server = "192.168.100.58";
const char* mqtt_username = "ragil";
const char* mqtt_password = "ragil";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


DHT dht;
#define DHTTYPE DHT11

//sesuikan posisi pin select
int s0 = D2;
int s1 = D1;
int s2 = D0;

//sensor digital
int pir = D5;
int dhtPin = D6;

  
//gunakan A0 sebagai input
int analogPin = A0;
  
//variabel untuk menyimpan nilai input
int nilaiSmoke = 0;
float nilaiCurrent = 0;
int nilaiLdr = 0;
int nilaiDht = 0;
int count = 0;
int sensorNow = 0;
int sensorMax = 0;
float Vraw = 0;
float Amp = 0;
float Vout = 0;

String tesss = "";

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
    
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
   //jadikan pin select sebagai output
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(pir, INPUT);
  dht.setup(dhtPin);
  
  //aktifkan komunikasi serial
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  //passing Json
  DynamicJsonBuffer jBuffer;
  JsonArray& root = jBuffer.createArray();
  tesss = Serial.read();
  if(tesss=="status"){
    Serial.println(WiFi.localIP());
  }
  //D5 sebagai input pir
  long state = digitalRead(pir);
  client.publish("lab/pir", String(state).c_str());
  Serial.print("Publish message: ");
  Serial.println(state);
  delay(5000);

  //DHT program gpio 14 D5
  
  int temperature = dht.getTemperature();
  Serial.print("Publish message: ");
  Serial.println(temperature);
  client.publish("lab/temperature", String(temperature).c_str());
  delay(5000);
  
   //memilih y0 sebagai input
  digitalWrite(s0,LOW);
  digitalWrite(s1,LOW);
  digitalWrite(s2,LOW);
  delay(5000);
  nilaiLdr = analogRead(analogPin);
  client.publish("lab/ldr", String(nilaiLdr).c_str());
  Serial.print("Publish message: ");
  Serial.println(nilaiLdr);
  delay(1000);
     
  //memilih y1 sebagai input
  digitalWrite(s0,HIGH);
  digitalWrite(s1,LOW);
  digitalWrite(s2,LOW);
  delay(5000);
  nilaiSmoke = analogRead(analogPin);
  JsonObject& arr1 = jBuffer.createObject();
  arr1["sensor"] = "smoke";
  arr1["nilai"] = nilaiSmoke;
  Serial.print("Publish message: ");
  Serial.println(nilaiSmoke);
  client.publish("lab/smoke", String(nilaiSmoke).c_str());
  root.add(arr1);
  delay(1000);

//memilih y2 sebagai input
  digitalWrite(s0,LOW);
  digitalWrite(s1,HIGH);
  digitalWrite(s2,LOW);
  delay(5000);
  while ( count < 100){
    count++;
    sensorNow = analogRead(analogPin);
    if (sensorNow > sensorMax){
      sensorMax = sensorNow;
    };
    delay(100);
  };
//  Vout = ((sensorMax - 517.0) / 1024.0 * 3.3 / (5/3.3 * 0.1));
  Vraw = sensorMax/1023.0 * 3.3;
  Vout = Vraw - 2.5;
  Amp = Vout / 0.1;
  if(Amp < 0.00 or Amp < 0.33){
    Amp = 0;
  }else if(Amp > 0.32){
    Amp = Amp - 0.32;
  };
  
  Serial.print(" ");
  Serial.print(Vraw);
  Serial.print(" ");
  Serial.print(sensorMax);
  Serial.print(" ");
  Serial.print(Vout);
  Serial.print(" ");
  delay(1000);
  client.publish("lab/current", String(Amp).c_str());
  Serial.print("Publish message: ");
  Serial.println(Amp);
  
  count = 0;
  sensorMax = 0;
  
  delay(1000); 
  

}
