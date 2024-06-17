class MQQTInfo:

    def __init__(self, timestamp, temperture, humidity, precipitation):
        self.timestamp = timestamp
        self.humidity = humidity
        self.temperture = temperture
        self.precipitation = precipitation
    
    def set_timeStamp(self, timestamp):
        self.timestamp =  timestamp

    def set_temperture(self, temperture):
        self.temperture =  temperture

    def set_humidity(self, humidity):
        self.humidity =  humidity

    def set_precipitation(self, precipitation):
        self.precipitation =  'Sim' if precipitation == 1 else 'Não'
     




