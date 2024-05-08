import paho.mqtt.client as mqtt
import firebase_admin
from firebase_admin import credentials, db

# Initialize Firebase
#cred = credentials.Certificate("C:\\Users\\pc\\esp\\MQTT_TEST\\human-reconition-firebase-adminsdk-pf3ry-b4d1a35cc2.json")
#firebase_admin.initialize_app(cred, {'databaseURL': 'https://human-reconition-default-rtdb.asia-southeast1.firebasedatabase.app/'})
cred = credentials.Certificate("D:\\NCKH\\CODE\\Human-Activity-Recognition\\private_key.json")
firebase_admin.initialize_app(cred, {'databaseURL': 'https://esp32-b024e-default-rtdb.asia-southeast1.firebasedatabase.app/'})
# Callback khi nhận được dữ liệu từ MQTT
def on_message(client, userdata, msg):
    # Xử lý dữ liệu từ msg.payload
    data = msg.payload.decode("utf-8")
    
    # Đường dẫn tới node cần lưu trữ trong Realtime Database
    db_path = 'mqtt_data' 
    
    # Lưu trữ dữ liệu vào Realtime Database
    ref = db.reference(db_path)
    ref.push().set(data)
    print('Message sent to Realtime Database successfully')

# Kết nối với MQTT broker
client = mqtt.Client()
client.on_message = on_message
client.connect("mqtt.eclipseprojects.io", 1883, 60)

# Đăng ký theo dõi tất cả các chủ đề (topics)
client.subscribe("Test")

# Lắng nghe dữ liệu từ MQTT
client.loop_forever()