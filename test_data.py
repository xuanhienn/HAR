import csv
import json
import time
import paho.mqtt.client as mqtt

# MQTT setup
broker_address = "mqtt://mqtt.eclipseprojects.io"
client = mqtt.Client("CSVReaderClient")
client.connect(broker_address)

def send_data_to_mqtt(data):
    client.publish("Test", json.dumps(data))

# Read the CSV file
with open('"D:\JupyterNotebook_Workspace\HAR Project\data\har.csv"', mode='r') as csvfile:
    csvreader = csv.DictReader(csvfile)
    
    line_number = 1
    for row in csvreader:
        data = {
            "ax": float(row["accel_x"]),
            "ay": float(row["accel_y"]),
            "az": float(row["accel_z"]),
            "gx": float(row["gyro_x"]),
            "gy": float(row["gyro_y"]),
            "gz": float(row["gyro_z"]),
            "Time": line_number
        }
        send_data_to_mqtt(data)
        line_number += 1
        time.sleep(0.05)  # wait for 50ms

client.disconnect()
