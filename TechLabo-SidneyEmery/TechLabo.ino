#include <WiFiNINA.h>
#include <PubSubClient.h>  // Include PubSubClient for MQTT

// WiFi and MQTT Setup
const char ssid[] = "saskialaverge";   // Your WiFi SSID
const char pass[] = "Rita1956";        // Your WiFi password
const char broker[] = "192.168.0.178"; // IP address of the MQTT broker
int port = 1883;                       // MQTT port (default: 1883 for non-secure connection)
WiFiClient wifiClient; 
PubSubClient mqttClient(wifiClient);    // Initialize PubSubClient with WiFiClient

// Pin Definitions
const byte ledPin = 13;     // Simple LED on Pin 7
const byte redPin = 11;     // Red for RGB LED
const byte greenPin = 10;  // Green for RGB LED
const byte bluePin = 9;   // Blue for RGB LED
const byte photoResistorPin = A0;  // Analog input for photoresistor

// Topics
const char ledControlTopic[] = "home/led";
const char rgbControlTopic[] = "home/rgb";
const char lightDataTopic[] = "home/lightData";

// Variables
int lightValue = 0;

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // Connect to WiFi
  connectWiFi();

  // Initialize MQTT client
  mqttClient.setServer(broker, port);  // Set the MQTT broker
  mqttClient.setCallback(messageReceived); // Set the message callback
  
  // Attempt to connect to MQTT broker
  while (!mqttClient.connected()) {
    Serial.print(".");
    if (mqttClient.connect("arduinoClient")) {
      Serial.println("Connected to MQTT broker!");
      mqttClient.subscribe(ledControlTopic);
      mqttClient.subscribe(rgbControlTopic);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Try again in 5 seconds.");
      delay(5000);  // Wait 5 seconds before retrying
    }
  }
}

void loop() {
  mqttClient.loop();  // Process incoming MQTT messages

  // Read the value from the photoresistor and send it to the MQTT broker
  lightValue = analogRead(photoResistorPin);
  char lightData[10];
  itoa(lightValue, lightData, 10);  // Convert the integer to a string
  mqttClient.publish(lightDataTopic, lightData);  // Publish the light data

  delay(1000);  // Wait for 1 second before reading again

  lightValue = analogRead(photoResistorPin);
  Serial.print("Light value: ");
  Serial.println(lightValue);
  delay(500);  // Print every 500 milliseconds
}


void connectWiFi() {
  Serial.print("Connecting to WiFi...");
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    delay(5000);
    Serial.print(".");  // Print a dot every 5 seconds to show progress
  }
  Serial.println("Connected to WiFi!");
}

void messageReceived(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];  // Convert payload bytes to string
  }

  if (String(topic) == ledControlTopic) {
    if (message == "ON") {
      digitalWrite(ledPin, HIGH);  // Turn on LED
    } else {
      digitalWrite(ledPin, LOW);   // Turn off LED
    }
  } 
  else if (String(topic) == rgbControlTopic) {
    // Assuming the message is in the format "255,0,128"
    int firstComma = message.indexOf(',');
    int secondComma = message.indexOf(',', firstComma + 1);

    int r = message.substring(0, firstComma).toInt();  // Get the red value
    int g = message.substring(firstComma + 1, secondComma).toInt();  // Get the green value
    int b = message.substring(secondComma + 1).toInt();  // Get the blue value

    // Debugging: Print the RGB values to the Serial Monitor
    Serial.print("Received RGB: ");
    Serial.print(r);
    Serial.print(", ");
    Serial.print(g);
    Serial.print(", ");
    Serial.println(b);

    // Ensure that the values are within the valid range
    r = constrain(r, 0, 255);
    g = constrain(g, 0, 255);
    b = constrain(b, 0, 255);

    // For common anode LED, HIGH turns it off and LOW turns it on
    analogWrite(redPin, 255 - r);    // Invert for common anode
    analogWrite(greenPin, 255 - g);  // Invert for common anode
    analogWrite(bluePin, 255 - b);   // Invert for common anode
  }
}
