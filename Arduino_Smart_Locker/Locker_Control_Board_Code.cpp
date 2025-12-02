#include <WiFi.h>
#include <Servo.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>
#include <PubSubClient.h>

Servo servoA;
Servo servoB;

String Locker_A_Lock_Command = "";
String Locker_B_Lock_Command = "";
String Locker_A_Data = "";
String Locker_B_Data = "";
String User_Count_A_Data = "";
String User_Count_B_Data = "";
String Ready_A_Data = "";
String Ready_B_Data = "";
int Ready_A_Delay = 0;
int Ready_B_Delay = 0;
int Time_Box_A = 0;
int Time_Box_B = 0;
int Ready_Box_A = 0;
int Ready_Box_B = 0;
int User_Box_A = -1;
int User_Box_B = -1;
int Locker_A_Status = 0;
int Locker_B_Status = 0;
int servoAPin = 5;
int servoBPin = 11;  // กำหนดพินสำหรับเชื่อมต่อเซอร์โว
int angle = 0;       // กำหนดมุมเริ่มต้น
int BoxItem1 = digitalRead(6);
const int Locker_A_1_trigPin = 6;
const int Locker_A_1_echoPin = 2;
const int Locker_A_2_trigPin = 3;
const int Locker_A_2_echoPin = 4;
const int Locker_B_1_trigPin = 7;
const int Locker_B_1_echoPin = 8;
const int Locker_B_2_trigPin = 9;
const int Locker_B_2_echoPin = 10;
float duration1, distance1, duration2, distance2, duration3, distance3, duration4, distance4;
char Locker_A_String[69];
char Locker_B_String[69];
char User_Count_A_String[69];
char User_Count_B_String[69];
char Ready_A_String[69];
char Ready_B_String[69];

// ตั้งค่า WiFi
const char* ssid = "Ken";            // ใส่ชื่อ WiFi ของคุณ
const char* password = "111111111";  // ใส่รหัสผ่าน WiFi ของคุณ

// ตั้งค่า URL ที่ต้องการเรียกข้อมูล
const char* mqtt_server = "broker.hivemq.com";  //อย่าลืมแก้ไข BROKER
const char* host = "192.168.156.180";           // ใส่ชื่อโดเมนหรือ IP ของเซิร์ฟเวอร์
const int port = 1880;                          // พอร์ต HTTP (ส่วนใหญ่จะใช้ 80)

const char* mqtt_Client = "clientId-3yhG4kHO8W";  //อย่าลืมแก้ไข ClientID
const int mqtt_port = 1883;
const char* mqtt_username = "";  //อย่าลืมแก้ไข Token
const char* mqtt_password = "";  //อย่าลืมแก้ไข Secret


WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  pinMode(Locker_A_1_trigPin, OUTPUT);
  pinMode(Locker_A_1_echoPin, INPUT);
  pinMode(Locker_A_2_trigPin, OUTPUT);
  pinMode(Locker_A_2_echoPin, INPUT);
  pinMode(Locker_B_1_trigPin, OUTPUT);
  pinMode(Locker_B_1_echoPin, INPUT);
  pinMode(Locker_B_2_trigPin, OUTPUT);
  pinMode(Locker_B_2_echoPin, INPUT);
  pinMode(7, OUTPUT);
  Serial.begin(115200);
  servoA.attach(servoAPin);
  servoB.attach(servoBPin);

  // เชื่อมต่อ WiFi
  WiFi.begin(ssid, password);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  client.subscribe("@msg/operator");
  Reconnect_To_Server();
  client.loop();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;

    if (client.connect(host, port)) {
      Serial.println("Connected to server");

      // อ่านค่าจาก US sensor สำหรับ Locker A1
      digitalWrite(Locker_A_1_trigPin, LOW);
      delayMicroseconds(100);
      digitalWrite(Locker_A_1_trigPin, HIGH);
      delayMicroseconds(500);
      digitalWrite(Locker_A_1_trigPin, LOW);
      duration1 = pulseIn(Locker_A_1_echoPin, HIGH);
      distance1 = (duration1 * .0343) / 2;
      Serial.print("Distance1: ");
      Serial.println(distance1);

      // อ่านค่าจาก US sensor สำหรับ Locker A2
      digitalWrite(Locker_A_2_trigPin, LOW);
      delayMicroseconds(100);
      digitalWrite(Locker_A_2_trigPin, HIGH);
      delayMicroseconds(500);
      digitalWrite(Locker_A_2_trigPin, LOW);
      duration2 = pulseIn(Locker_A_2_echoPin, HIGH);
      distance2 = (duration2 * .0343) / 2;
      Serial.print("Distance2: ");
      Serial.println(distance2);

      digitalWrite(Locker_B_1_trigPin, LOW);
      delayMicroseconds(100);
      digitalWrite(Locker_B_1_trigPin, HIGH);
      delayMicroseconds(500);
      digitalWrite(Locker_B_1_trigPin, LOW);
      duration3 = pulseIn(Locker_B_1_echoPin, HIGH);
      distance3 = (duration3 * .0343) / 2;
      Serial.print("Distance3: ");
      Serial.println(distance3);

      digitalWrite(Locker_B_2_trigPin, LOW);
      delayMicroseconds(100);
      digitalWrite(Locker_B_2_trigPin, HIGH);
      delayMicroseconds(500);
      digitalWrite(Locker_B_2_trigPin, LOW);
      duration4 = pulseIn(Locker_B_2_echoPin, HIGH);
      distance4 = (duration4 * .0343) / 2;
      Serial.print("Distance4: ");
      Serial.println(distance4);

      Locker_SensorA(distance1, distance2);
      Locker_SensorB(distance3, distance4);
      
      if (Time_Box_A < 5) {
        Ready_Box_A = 0;
        Publish_Ready_A_To_Nodered(Ready_Box_A);
      } else {
        Ready_Box_A = 1;
        Publish_Ready_A_To_Nodered(Ready_Box_A);
      }

      if (Time_Box_B < 5) {
        Ready_Box_B = 0;
        Publish_Ready_B_To_Nodered(Ready_Box_B);
      } else {
        Ready_Box_B = 1;
        Publish_Ready_B_To_Nodered(Ready_Box_B);
      }

      if (Time_Box_A == 5){
        User_Box_A ++;
      }

      if (Time_Box_B == 5){
        User_Box_B ++;
      }

      Publish_Locker_A_To_Nodered(Locker_A_Status);
      Publish_Locker_B_To_Nodered(Locker_B_Status);
      Publish_User_Count_A_To_Nodered(User_Box_A);
      Publish_User_Count_B_To_Nodered(User_Box_B);

      // ส่ง HTTP GET request ไปยัง Node-RED
      client.print(String("GET ") + "/API_Panel_Status" + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

      // รอรับการตอบกลับ
      Locker_B_Lock_Command = "";
      while (client.available() == 0) {
        delay(100);
      }

      while (client.available()) {
        Locker_B_Lock_Command += client.readStringUntil('\r');
      }
      Serial.println(Locker_B_Lock_Command);

      // Parse JSON ที่ได้รับ
      int jsonIndex_B = Locker_B_Lock_Command.indexOf('{');
      if (jsonIndex_B != -1) {
        String jsonString = Locker_B_Lock_Command.substring(jsonIndex_B);
        JSONVar jsonData = JSON.parse(jsonString);

        if (JSON.typeof(jsonData) == "undefined") {
          Serial.println("Parsing input failed!");
        } else {
          String Servo_A_Status_Value = (const char*)jsonData["Servo_A_Status_Value"];
          String Servo_B_Status_Value = (const char*)jsonData["Servo_B_Status_Value"];
          //Serial.println("Locker_B:" + Locker_B);
          //Serial.println("Locker_A:" + Locker_A);

          // ควบคุม Servo Locker A
          if (Servo_B_Status_Value == "1") {
            for (angle = 90; angle <= 180; angle++) {
              servoB.write(angle);
            }
          } else if (Servo_B_Status_Value == "0") {
            for (angle = 180; angle >= 90; angle--) {
              servoB.write(angle);
            }
          }

          if (Servo_A_Status_Value == "1") {
            for (angle = 90; angle <= 180; angle++) {
              servoA.write(angle);
            }
          } else if (Servo_A_Status_Value == "0") {
            for (angle = 180; angle >= 90; angle--) {
              servoA.write(angle);
            }
          }
        }
      }
      client.stop();
    } else {
      Serial.println("Connection to server failed");
      Reconnect_To_Server();
    }
  } else {
    Serial.println("WiFi Disconnected");
  }
  delay(50);
}

void Publish_Ready_A_To_Nodered(int Ready_A_Status_Send) {

  Ready_A_Data = "\"" + String(Ready_A_Status_Send) + "\"";

  Ready_A_Data.toCharArray(Ready_A_String, 69);
  Serial.print("Publish Ready: ");
  Serial.println(Ready_A_String);
  client.publish("Locker_Board/Ready_A_Status", Ready_A_String);
}

void Publish_Ready_B_To_Nodered(int Ready_B_Status_Send) {

  Ready_B_Data = "\"" + String(Ready_B_Status_Send) + "\"";

  Ready_B_Data.toCharArray(Ready_B_String, 69);
  Serial.print("Publish Ready: ");
  Serial.println(Ready_B_String);
  client.publish("Locker_Board/Ready_B_Status", Ready_B_String);
}

void Publish_User_Count_A_To_Nodered(int User_Count_A_Status_Send) {

  User_Count_A_Data = "\"" + String(User_Count_A_Status_Send) + "\"";

  User_Count_A_Data.toCharArray(User_Count_A_String, 69);
  Serial.print("Publish User: ");
  Serial.println(User_Count_A_String);
  client.publish("Locker_Board/User_Count_A_Status", User_Count_A_String);
}

void Publish_User_Count_B_To_Nodered(int User_Count_B_Status_Send) {

  User_Count_B_Data = "\"" + String(User_Count_B_Status_Send) + "\"";

  User_Count_B_Data.toCharArray(User_Count_B_String, 69);
  Serial.print("Publish User: ");
  Serial.println(User_Count_B_String);
  client.publish("Locker_Board/User_Count_B_Status", User_Count_B_String);
}

void Publish_Locker_A_To_Nodered(int Locker_A_Status_Send) {

  Locker_A_Data = "\"" + String(Locker_A_Status_Send) + "\"";

  Locker_A_Data.toCharArray(Locker_A_String, 69);
  Serial.print("Publish Status: ");
  Serial.println(Locker_A_String);
  client.publish("Locker_Board/Locker_A_Status", Locker_A_String);
}

void Publish_Locker_B_To_Nodered(int Locker_B_Status_Send) {

  Locker_B_Data = "\"" + String(Locker_B_Status_Send) + "\"";

  Locker_B_Data.toCharArray(Locker_B_String, 69);
  Serial.print("Publish Status: ");
  Serial.println(Locker_B_String);
  client.publish("Locker_Board/Locker_B_Status", Locker_B_String);
}

void Reconnect_To_Server() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    client.connect(mqtt_Client);
    if (client.connect(mqtt_Client)) {
      Serial.println("connected");
      WiFiClient client;
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void Locker_SensorA(int Sensor1, int Sensor2) {
  if (Sensor1 < 15 || Sensor2 < 15 && Sensor1 != 0 && Sensor2 != 0) {
    Serial.print("Locker A Found\n");
    Locker_A_Status = 1;
    Time_Box_A = 0;
  } else {
    Serial.print("Locker A Not Found\n");
    Locker_A_Status = 0;
    Time_Box_A++;
  }
}

void Locker_SensorB(int Sensor3, int Sensor4) {
  if (Sensor3 < 15 || Sensor4 < 15 && Sensor3 != 0 && Sensor4 != 0) {
    Serial.print("Locker B Found\n");
    Locker_B_Status = 1;
    Time_Box_B = 0;
  } else {
    Serial.print("Locker B Not Found\n");
    Locker_B_Status = 0;
    Time_Box_B++;
  }
}
