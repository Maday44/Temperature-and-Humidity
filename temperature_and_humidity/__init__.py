from datetime import datetime, timezone
import requests
from flask import Flask
from flask_sqlalchemy import SQLAlchemy
from apscheduler.schedulers.background import BackgroundScheduler
from models import TemperatureHumidity

app = Flask(__name__)
app.config["SQLALCHEMY_DATABASE_URI"] = "sqlite:///temperature_and_humidity_data.db"
app.config["SQLALCHEMY_TRACK_MODIFICATIONS"] = False


db = SQLAlchemy(app)

with app.app_context():
    db.create_all()


def fetch_temperature_and_humidity_data():
    """Fetch temp/humidity endpoints, parse floats, and write to SQLAlchemy DB."""
    with app.app_context():
        try:
            temp_response = requests.get("http://192.168.1.69/temperature", timeout=5)
            hum_response = requests.get("http://192.168.1.69/humidity", timeout=5)

            temp_text = temp_response.text
            hum_text = hum_response.text

            if (temp_text or hum_text) is None:
                print("!!!! ESP32 returned invalid sensor reading !!!")
                return

            temp_val = float(temp_text)
            hum_val = float(hum_text)

            reading = TemperatureHumidity(temperature=temp_val, humidity=hum_val)

            db.session.add(reading)
            db.session.commit()

            print(
                f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] Saved -> Temp: {temp_val}°C, Humidity: {hum_val}%"
            )

        except (ValueError, TypeError) as e:
            print(f"[Error] Failed to parse sensor reading into a float: {e}")
        except requests.exceptions.Timeout:
            print("[Error] Request timed out. Check if 192.168.1.69 is reachable.")
        except requests.exceptions.ConnectionError:
            print(
                "[Error] Could not connect to sensor. Ensure you are on the same local network."
            )
        except requests.exceptions.RequestException as e:
            print(f"[Error] HTTP request failed: {e}")
        except Exception as e:
            db.session.rollback()
            print(f"[Error] Database operation failed: {e}")


scheduler = BackgroundScheduler(daemon=True)


scheduler.add_job(fetch_temperature_and_humidity_data, "interval", seconds=30)
# scheduler.add_job(fetch_temperature_and_humidity_data, 'interval', minutes=15)

scheduler.start()
