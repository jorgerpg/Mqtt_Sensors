import paho.mqtt.client as mqtt
import threading
import MQTTInfo
import json
import API

from flask import Flask, render_template

app = Flask(__name__)

weatherInfo = MQTTInfo.MQQTInfo(None, None, None, None)
connectionStatus = 'Desconectado'

def on_connect(client, userdata, flags, rc):
  
    # O subscribe fica no on_connect pois, caso perca a conexão ele a renova
    # Lembrando que quando usado o #, você está falando que tudo que chegar após a barra do topico, será recebido
    client.subscribe("/sensor/data")
    
def on_message(client, userdata, msg):
    print(msg.topic + " - " + str(msg.payload))

    try:
        data = json.loads(msg.payload.decode())
        weatherInfo.set_timeStamp = data.get("timestamp")
        weatherInfo.set_temperture = data.get("temperture")
        weatherInfo.set_humidity = data.get("humidity")
        weatherInfo.set_precipitation = data.get("precipitation")

    except Exception as e:
        print('Error to get MQQT menssage ' + e)

def start_MQTT():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    # Seta um usuário e senha para o Broker, se não tem, não use esta linha
    # client.username_pw_set("USUARIO", password="SENHA")

    # Conecta no MQTT Broker, no meu caso, o Mosquitto
    client.connect("test.mosquitto.org")

    # Inicia o loop
    client.loop_forever()

@app.route('/')
def index():
    locationInfo = API.get_location()
    weatherData = API.get_weather_by_api(location_data=locationInfo)
    return render_template('index.html', city=locationInfo["city"], connection=connectionStatus, weather_data=weatherData, weather_info=weatherInfo)

if __name__ == "__main__":
    # Iniciar o cliente MQTT em uma thread separada
    threading.Thread(target=start_MQTT).start()
    # Definir host e port
    host = '127.0.0.1'
    port = 5000
    # Imprimir mensagem de inicialização
    print(f" * Running on http://{host}:{port}/ (Press CTRL+C to quit)")
    # Iniciar o servidor Flask
    app.run(host=host, port=port, debug=True)