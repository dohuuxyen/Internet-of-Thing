#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// Uncomment one of the lines bellow for whatever DHT sensor type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// Change the credentials below, so your ESP8266 connects to your router
const char* ssid = "utc2hcm";
const char* password = "123456789aA";

// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char* mqtt_server = "192.168.4.1";

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient espClient;
PubSubClient client(espClient);

// DHT Sensor
const int DHTPin = 14; //D5

// Lamp - LED - GPIO 4 = D2 on ESP-12E NodeMCU board
const int LedPhongNgu = 4; //D1
const int LedPhongKhach = 5; //D2
const int LedPhongBep = 0;  //D3
const int LedPhongTam = 16;  //D8

// Door sensor and Pir hc-502 and Gas MQ135
const int DoorPin = 2;  //D4
const int PirPin = 13; //D7
const int GasPin = 15; //D8
const int buzzer = 12; //D6
int state; // 0 close - 1 open wwitch

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;

char data[80];

// Don't change the function below. This functions connects your ESP8266 to your router
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
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that 
// your ESP8266 is subscribed you can actually do something
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic room/lamp, you check if the message is either on or off. Turns the lamp GPIO according to the message
  if(topic=="room/ledphongngu"){
      Serial.print("Led phong ngu");
      if(messageTemp == "on"){
        digitalWrite(LedPhongNgu, HIGH);
        Serial.print("On");
      }
      else if(messageTemp == "off"){
        digitalWrite(LedPhongNgu, LOW);
        Serial.print("Off");
      }
  }
  if(topic=="room/ledphongkhach"){
      Serial.print("Led phong khach ");
      if(messageTemp == "on"){
        digitalWrite(LedPhongKhach, HIGH);
        Serial.print("On");
      }
      else if(messageTemp == "off"){
        digitalWrite(LedPhongKhach, LOW);
        Serial.print("Off");
      }
  }
  if(topic=="room/ledphongbep"){
      Serial.print("Led phong bep ");
      if(messageTemp == "on"){
        digitalWrite(LedPhongBep, HIGH);
        Serial.print("On");
      }
      else if(messageTemp == "off"){
        digitalWrite(LedPhongBep, LOW);
        Serial.print("Off");
      }
  }
  if(topic=="room/ledphongtam"){
      Serial.print("Led phong tam ");
      if(messageTemp == "on"){
        digitalWrite(LedPhongTam, HIGH);
        Serial.print("On");
      }
      else if(messageTemp == "off"){
        digitalWrite(LedPhongTam, LOW);
        Serial.print("Off");
      }
  }
  Serial.println();
}

// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    /*
     YOU MIGHT NEED TO CHANGE THIS LINE, IF YOU'RE HAVING PROBLEMS WITH MQTT MULTIPLE CONNECTIONS
     To change the ESP device ID, you will have to give a new name to the ESP8266.
     Here's how it looks:
       if (client.connect("ESP8266Client")) {
     You can do it like this:
       if (client.connect("ESP1_Office")) {
     Then, for the other ESP:
       if (client.connect("ESP2_Garage")) {
      That should solve your MQTT multiple connections problem
    */
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("room/ledphongngu");
      client.subscribe("room/ledphongkhach");
      client.subscribe("room/ledphongbep");
      client.subscribe("room/ledphongtam");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// The setup function sets your ESP GPIOs to Outputs, starts the serial communication at a baud rate of 115200
// Sets your mqtt broker and sets the callback function
// The callback function is what receives messages and actually controls the LEDs
void setup() {
  pinMode(DoorPin, INPUT_PULLUP);
  pinMode(PirPin, INPUT_PULLUP);
  pinMode(GasPin, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  pinMode(LedPhongNgu, OUTPUT);
  pinMode(LedPhongKhach, OUTPUT);
  pinMode(LedPhongBep, OUTPUT);
  pinMode(LedPhongTam, OUTPUT);
  dht.begin();
  
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

// For this project, you don't need to change anything in the loop function. Basically it ensures that you ESP is connected to your broker
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("ESP8266Client");

  now = millis();
  // Publishes new temperature and humidity every 30 seconds
  if (now - lastMeasure > 8000) {
    lastMeasure = now;
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Computes temperature values in Celsius
    float hic = dht.computeHeatIndex(t, h, false);
    static char temperatureTemp[7];
    dtostrf(hic, 6, 2, temperatureTemp);
    
    // Uncomment to compute temperature values in Fahrenheit 
    // float hif = dht.computeHeatIndex(f, h);
    // static char temperatureTemp[7];
    // dtostrf(hif, 6, 2, temperatureTemp);
    
    static char humidityTemp[7];
    dtostrf(h, 6, 2, humidityTemp);

    String dhtReadings = "{ \"temperature\": \"" + String(temperatureTemp) + "\", \"humidity\" : \"" + String(humidityTemp) + "\"}";
    dhtReadings.toCharArray(data, (dhtReadings.length() + 1));

    // Publishes Temperature and Humidity values
    client.publish("room/temperature", temperatureTemp);
    client.publish("room/humidity", humidityTemp);
    client.publish("/esp8266/dhtreadings", data);
    Serial.println(data);
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t Temperature: ");
    Serial.print(t);
    Serial.print(" *C ");
    Serial.print(f);
    Serial.print(" *F\t Heat index: ");
    Serial.print(hic);
    Serial.println(" *C ");
    // Serial.print(hif);
    // Serial.println(" *F");
  }

  state = digitalRead(DoorPin);
  if ((state || digitalRead(PirPin) || digitalRead(GasPin)) == HIGH){
     tone(buzzer, 1);
     delay(200);
   }
  else{
    noTone(buzzer);
    delay(200); 
  }
} 
