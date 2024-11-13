from flask import Flask, render_template, request, jsonify
import paho.mqtt.client as mqtt
import time

app = Flask(__name__)

# MQTT Setup
broker = "192.168.227.177"
mqtt_client = mqtt.Client()
mqtt_client.connect(broker, 1883, 60)

# Topics
led_control_topic = "home/led"
rgb_control_topic = "home/rgb"
light_data_topic = "home/lightData"

# Variable to store the latest light data
light_data = 0

# MQTT callback to receive light data
def on_message(client, userdata, msg):
    global light_data
    if msg.topic == light_data_topic:
        light_data = int(msg.payload.decode())
        print(f"Light data received: {light_data}")  # Print received light data to console


mqtt_client.on_message = on_message
mqtt_client.loop_start()
mqtt_client.subscribe(light_data_topic)

@app.route("/")
def index():
    return render_template("index.html", light_data=light_data)


@app.route("/led", methods=["POST"])
def led():
    state = request.json["state"]
    mqtt_client.publish(led_control_topic, state)
    return jsonify({"status": "success"})


@app.route("/rgb", methods=["POST"])
def rgb():
    r = request.json["red"]
    g = request.json["green"]
    b = request.json["blue"]
    # Publish the RGB values with commas separating them
    mqtt_client.publish(rgb_control_topic, f"{r},{g},{b}")
    return jsonify({"status": "success"})

# New route to fetch the latest light data in JSON format
@app.route("/light-data")
def light_data_route():
    return jsonify({"light_data": light_data})


if __name__ == "__main__":
    app.run(debug=True, host="0.0.0.0", port=3000)
