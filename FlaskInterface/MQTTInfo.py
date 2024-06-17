class MQTTInfo:
    def __init__(self, timestamp, temperature, humidity, precipitation, id):
        self.timestamp = timestamp
        self.humidity = humidity
        self.temperature = temperature
        self.precipitation = precipitation
        self.id = id
    
    def set_timeStamp(self, timestamp):
        self.timestamp =  timestamp

    def set_temperature(self, temperature):
        self.temperature =  temperature

    def set_humidity(self, humidity):
        self.humidity =  humidity

    def set_id(self, id):
        self.id =  id

    def set_precipitation(self, precipitation):
        self.precipitation =  'Sim' if precipitation == 1 else 'Não'