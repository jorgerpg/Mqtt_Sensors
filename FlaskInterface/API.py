import requests

weather_code_transcription = {
    "0": ['Moon.png', 'Sun.png'],
    "1": ['Rainy_Night.png', 'Rain_Cloud.png'],
    "2": ['Rainy_Night.png', 'Rain_Cloud.png'],
    "3": ['Rainy_Night.png', 'Rain_Cloud.png'],
    "45": ['Rainy_Night.png', 'Rain_Cloud.png'],
    "48": ['Rainy_Night.png', 'Rain_Cloud.png'],
    "51": ['Rainy_Night.png', 'Rain_Cloud.png'],
    "53": ['Rain.png', 'Rain.png'],
    "55": ['Cloud_Lightning.png', 'Cloud_Lightning.png'],
    "56": ['Rainy_Night.png', 'Rain_Cloud.png'],
    "57": ['Cloud_Lightning.png', 'Cloud_Lightning.png'],
    "61": ['Rainy_Night.png', 'Rain_Cloud.png'],
    "63": ['Rain.png', 'Rain.png'],
    "65": ['Cloud_Lightning.png', 'Cloud_Lightning.png'],
    "66": ['Rainy_Night.png', 'Rain_Cloud.png'],
    "67": ['Cloud_Lightning.png', 'Cloud_Lightning.png'],
    "80": ['Rainy_Night.png', 'Rain_Cloud.png'],
    "81": ['Rain.png', 'Rain.png'],
    "82": ['Cloud_Lightning.png', 'Cloud_Lightning.png'],
    "95": ['Cloud_Lightning.png', 'Cloud_Lightning.png'],
    "96": ['Cloud_Lightning.png', 'Cloud_Lightning.png'],
    "99": ['Cloud_Lightning.png', 'Cloud_Lightning.png'],
}

def get_ip():
    response = requests.get('https://api64.ipify.org?format=json').json()
    return response["ip"]


def get_location():
    ip_address = get_ip()
    response = requests.get(f'https://ipapi.co/{ip_address}/json/').json()
    location_data = {
        "ip": ip_address,
        "city": response.get("city"),
        "latitude": response.get("latitude"),
        "longitude": response.get("longitude"),
        "timezone": response.get("timezone")
    }
    return location_data

def get_weather_by_api(location_data):
    try:
        timezone = location_data["timezone"].replace('/', f'%2F')
    except:
        timezone = f'America%2FSao_Paulo'

    response = requests.get(f'https://api.open-meteo.com/v1/forecast?latitude={location_data["latitude"]}&longitude={location_data["longitude"]}&current=temperature_2m,is_day,weather_code&daily=temperature_2m_max,temperature_2m_min,sunrise,sunset&timezone={timezone}&forecast_days=1').json()

    wheather_data = {
        "weather_code": weather_code_transcription[str(response.get("current")["weather_code"])][response.get("current")["is_day"]],
        "temperature_max": response.get("daily")["temperature_2m_max"][0],
        "temperature_min": response.get("daily")["temperature_2m_min"][0],
        "sunrise": get_time(response.get("daily")["sunrise"][0]),
        "sunset": get_time(response.get("daily")["sunset"][0])
    }
    print(wheather_data)
    return wheather_data

def get_time(sunTime):
    data, time = sunTime.split('T')
    return time





