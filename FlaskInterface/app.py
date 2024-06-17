import paho.mqtt.client as mqtt
import threading
import MQTTInfo
import json
import API
from flask import Flask, render_template, jsonify

app = Flask(__name__)

weatherInfo = MQTTInfo.MQTTInfo(None, None, None, None, None)
connectionStatus = 'Desconectado'

def on_connect(client, userdata, flags, rc):
    global connection_status
    if rc == 0:
        connection_status = 'Conectado'
    else:
        connection_status = 'Desconectado'
    print(f"Connected with result code {rc}")
    client.subscribe("/sensor/data")

def on_message(client, userdata, msg):
    print(msg.topic + " - " + str(msg.payload))

    try:
        data = json.loads(msg.payload.decode())
        payload = data.get("payload", {})
        weatherInfo.set_id(data.get("id"))
        weatherInfo.set_timeStamp(data.get("timestamp"))
        weatherInfo.set_temperature(payload.get("temperature"))
        weatherInfo.set_humidity(payload.get("humidity"))
        weatherInfo.set_precipitation(payload.get("precipitation"))

        print("Updated weatherInfo:")
        print(f"ID: {weatherInfo.id}")
        print(f"Timestamp: {weatherInfo.timestamp}")
        print(f"Temperature: {weatherInfo.temperature}")
        print(f"Humidity: {weatherInfo.humidity}")
        print(f"Precipitation: {weatherInfo.precipitation}")

    except Exception as e:
        print('Error to get MQTT message: ' + str(e))

@app.route('/update_weather_data')
def update_weather_data():

    return jsonify({
        'timestamp': weatherInfo.timestamp,
        'temperature': weatherInfo.temperature,
        'humidity': weatherInfo.humidity,
        'precipitation': weatherInfo.precipitation
    })

def start_MQTT():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect("test.mosquitto.org")

    client.loop_forever()

@app.route('/')
def index():
    locationInfo = API.get_location()
    weatherData = API.get_weather_by_api(location_data=locationInfo)
    return render_template('index.html', city=locationInfo["city"], connection=connectionStatus, weather_data=weatherData, weather_info=weatherInfo)

if __name__ == "__main__":
    threading.Thread(target=start_MQTT).start()
    host = '127.0.0.1'
    port = 5000
    print(f" * Running on http://{host}:{port}/ (Press CTRL+C to quit)")
    app.run(host=host, port=port, debug=True)
