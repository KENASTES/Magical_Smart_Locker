#include <LiquidCrystal_I2C.h>
#include <DIYables_Keypad.h>
#include <PubSubClient.h>
#include <Arduino_JSON.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include <Wire.h>

// ตั้งค่าการเชื่อมต่อ
WiFiClient espClient;
PubSubClient client(espClient);
const char* ssid = "Ken";
const char* password = "111111111";
const char* mqtt_server = "broker.hivemq.com";
const char* host = "192.168.156.180";
const int port = 1880;
const char* mqtt_Client = "clientId-HvLKk4WQ76";
const int mqtt_port = 1883;
const char* mqtt_username = "";
const char* mqtt_password = "";

// ตั้งค่า URL ที่ต้องการเรียกข้อมูล

// ตั้งค่าเส้นทาง URL
String Get_Locker_1_Status = "/Locker1";
String Get_Locker_2_Status = "/Locker2";
String Locker_Status_Command = "";
String Password_Reset_Command = "";
String Servo_A_Command = "";
String Servo_B_Command = "";
String Status_A_Command = "";
String Status_B_Command = "";

// กำหนดตัวแปรต่างๆ
String response = "";
const int ROW_NUM = 4;
const int COLUMN_NUM = 4;
const int maxInput = 6;

// กำหนดค่า Default
int Servo_A_Value = 0;
int Servo_B_Value = 0;
char inputArray[maxInput];
char ResetArray[maxInput];
char Locker_A_Password_Array[maxInput];
char Locker_B_Password_Array[maxInput];
char Display_Array[6];
char Servo_A_String[69];
char Servo_B_String[69];
char Status_A_String[69];
char Status_B_String[69];
int arrayIndex = 0;
int Choosing_Locker = 0;
int Locker_A_status = 0;
int Locker_B_status = 0;
int Used_A_status = 0;
int Used_B_status = 0;
int Locker_A_status_Check = 0;
int Locker_B_status_Check = 0;
unsigned long lastServerCheck = 0;
unsigned long lastWiFiCheck = 0;
const long intervalServerCheck = 5000;
const long intervalWiFiCheck = 5000;

// กำหนดค่าของปุ่มกดใน Keypad
char keys[ROW_NUM][COLUMN_NUM] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

// กำหนด Pin ของ Keypad
byte pin_rows[ROW_NUM] = { 9, 8, 7, 6 };
byte pin_column[COLUMN_NUM] = { 5, 4, 3, 2 };

DIYables_Keypad keypad = DIYables_Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

// กำหนดตัวแปรของจอ LCD
const String Default_Password = "123456";
const int lcdAddress = 0x27;
const int lcdColumns = 16;
const int lcdRows = 2;
char Input_Clear[20] = " Input  Cleared ";

//กำหนดรูปแบบของ LCD
LiquidCrystal_I2C lcd(lcdAddress, lcdColumns, lcdRows);

void setup() {
  Serial.begin(115200);
  // ตั้งรหัสผ่านให้เป็นค่า Default
  for (int Loop = 0; Loop <= 5; Loop++) {
    Locker_A_Password_Array[Loop] = 'O';
    Locker_B_Password_Array[Loop] = 'O';
  }

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // เชื่อมต่อกับ Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Reconnect_To_Server();

  // แสดงสถาณะ เชื่อมต่อเสร็จสิ้น
  Serial.println("Connected to WiFi");

  // Set หน้าจอตอนเริ่มใช้
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  Clear_Password();
  client.loop();
}

void loop() {
  unsigned long currentMillis = millis();
  // เช็คสถาณะ Wi-Fi
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    PubSubClient Subclient(espClient);
    Subclient.subscribe("Panel_Board/Locker_A_Status");

    // เมื่อสถาณะ Server
    if (client.connect(host, port)) {
      char Input_Key = keypad.getKey();

      // หากมีการกดปุ่มเข้ามา
      if (Input_Key) {
        Serial.println(Input_Key);
        if (Input_Key == 'C') {  // ลบรหัสทั้งหมด
          Clear_Password();
        } else if (Input_Key == 'D') {  // ลบรหัสตัวล่าสุด
          Delete_Last_Letter();
        } else if (Input_Key == 'A') {  // เลือก Locker A
          Locker_A();
          Serial.println(Choosing_Locker);
        } else if (Input_Key == 'B') {  // เลือก Locker B
          Locker_B();
          Serial.println(Choosing_Locker);
        } else if (Input_Key == '*') {  // เช็คสถาณะของกล่อง
          Locker_Status_Check();
        }
        // หากมีการ Input เข้ามา
        else if (Input_Key == '#') {
          // เช็คสถาณะต่างๆ ของกล่อง A
          if (Locker_A_status == 0 && Choosing_Locker == 1) {
            Confirm_Locker_A_Password_Firsttime();
          } else if (Locker_A_status == 1 && Choosing_Locker == 1) {
            if (Verify_Password(Locker_A_Password_Array, inputArray)) {
              Password_A_Currect();
            } else {
              Password_Wrong();
            }
            // เช็คสถาณะต่างๆ ของกล่อง B
          } else if (Locker_B_status == 0 && Choosing_Locker == 2) {
            Confirm_Locker_B_Password_Firsttime();
          } else if ((Locker_B_status == 1 && Choosing_Locker == 2)) {
            if (Verify_Password(Locker_B_Password_Array, inputArray)) {
              Password_B_Currect();
            } else {
              Password_Wrong();
            }
          } else {
            Serial.println(Choosing_Locker);
          }
        } else {
          // คำสั้งแสดงรหัสผ่านที่กดเข้ามา
          if (arrayIndex < maxInput) {
            addCharacter(Input_Key);
          } else {
            // เมื่อรหัสผ่านเต็ม
            displayErrorMessage("Input Is Full");
          }
        }
        Display_To_LCD();
        // แสดงรหัสทั้งหมดบน Serial Monitor
        Serial.println(inputArray);
      }

      if (WiFi.status() == WL_CONNECTED && currentMillis - lastServerCheck >= intervalServerCheck) {
        lastServerCheck = currentMillis;
        // ส่ง HTTP GET request ไปยัง Node-RED
        client.print(String("GET ") + "/API_Locker_Status" + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

        // รอรับการตอบกลับ
        Locker_Status_Command = "";
        while (client.available() == 0) {
          delay(5);
        }

        while (client.available()) {
          Locker_Status_Command += client.readStringUntil('\r');
        }
        Serial.println(Locker_Status_Command);

        // Parse JSON ที่ได้รับ
        int jsonIndex_Locker_Status_Command = Locker_Status_Command.indexOf('{');
        if (jsonIndex_Locker_Status_Command != -1) {
          String jsonString = Locker_Status_Command.substring(jsonIndex_Locker_Status_Command);
          JSONVar jsonData = JSON.parse(jsonString);

          if (JSON.typeof(jsonData) == "undefined") {
            Serial.println("Parsing input failed!");
          } else {
            String Locker_A_Status_Value = (const char*)jsonData["Locker_A_Status_Value"];
            String Locker_B_Status_Value = (const char*)jsonData["Locker_B_Status_Value"];
            Serial.println("Locker_A_Status_Value:" + Locker_A_Status_Value);
            Serial.println("Locker_B_Status_Value:" + Locker_B_Status_Value);
            String Password_A_Status_Value = (const char*)jsonData["Password_A_Status_Value"];
            String Password_B_Status_Value = (const char*)jsonData["Password_B_Status_Value"];
            Serial.println("Password_A_Status_Value:" + Password_A_Status_Value);
            Serial.println("Password_B_Status_Value:" + Password_B_Status_Value);

            // ควบคุม Servo Locker
            if (Locker_A_Status_Value == "1") {
              Locker_A_status_Check = 1;
            } else if (Locker_A_Status_Value == "0") {
              Locker_A_status_Check = 0;
            }

            if (Locker_B_Status_Value == "1") {
              Locker_B_status_Check = 1;
            } else if (Locker_B_Status_Value == "0") {
              Locker_B_status_Check = 0;
            }

            // ควบคุม Password Locker
            if (Password_A_Status_Value == "1") {
              for (int Loop = 0; Loop <= 5; Loop++) {
                Locker_A_Password_Array[Loop] = 'X';
              }
              Servo_A_Value = 0;
              Locker_A_status = 0;
            } else if (Password_A_Status_Value == "0") {
              Serial.println(":>");
            }

            if (Password_B_Status_Value == "1") {
              for (int Loop = 0; Loop <= 5; Loop++) {
                Locker_B_Password_Array[Loop] = 'X';
              }
              Servo_B_Value = 0;
              Locker_B_status = 0;
            } else if (Password_B_Status_Value == "0") {
              Serial.println(":>");
            }
          }
        }
      }
      Publish_Servo_A_To_Nodered(Servo_A_Value);
      Publish_Servo_B_To_Nodered(Servo_B_Value);
      Publish_Locker_A_Password_To_Nodered(Locker_A_status);
      Publish_Locker_B_Password_To_Nodered(Locker_B_status);
      client.stop();
      Subclient.loop();
    }
    // เมื่อการเชื่อมต่อ Server หลุด
    else {
      Connection_Server_Failed_Display();
      Serial.println("Connection to server failed");
      Reconnect_To_Server();
    }
    // เมื่อการเชื่อมต่อ Wi-Fi หลุด
  } else {
    Serial.println("WiFi Disconnected");
  }
  delay(50);
}


void Locker_Status() {
  Clear_Password();
}

// ฟังชั้นเคลียรหัสผ่าน
void Clear_Password() {
  char Choose_Locker[20] = " Choose  Locker ";
  // ทำให้รหัสผ่านไปเริ่มที่ตัวแรกใหม่
  arrayIndex = 0;
  // แทนค่ารหัสทุกหลักด้วยตัว X
  for (int i = 0; i < maxInput; i++) {
    inputArray[i] = 'X';
  }
  lcd.clear();
  // กลับไปหน้าเลือก Locker ใหม่
  for (int Loop = 0; Loop <= 15; Loop++) {
    lcd.print(Choose_Locker[Loop]);
  }
  // รีเซ็ตการเลือก Locker
  Choosing_Locker = 0;
}

// ฟังชั้นลบรหัสตัวล่าสุด
void Delete_Last_Letter() {
  if (arrayIndex > 0) {
    // แทนค่าตัวล่าสุดด้วยตัว X
    arrayIndex--;
    inputArray[arrayIndex] = 'X';
  }
}

// ฟังชั้นแสดงค่ารหัสขี้นบนจอ
void Display_To_LCD() {
  for (int Loop = 0; Loop <= 5; Loop++) {
    // นำ Input ไปใส่ Array ที่จะใช้แสดงบนจอ
    Display_Array[Loop] = inputArray[Loop];
    lcd.setCursor(Loop, 1);
    lcd.print(Display_Array[Loop]);
  }
}

// ฟังชั้นการใส่รหัสผ่านจากเลขที่กดเข้ามา
void addCharacter(char Input_Key) {
  if (arrayIndex < maxInput) {
    inputArray[arrayIndex++] = Input_Key;
  }
}

// ฟังชั้นแจ้งรหัสผ่านเต็ม
void displayErrorMessage(const char* message) {
  char Error_Massage[20] = "Input Error";
  char Full_Input[20] = "Input Is Full";
  char Choose_Locker[20] = " Choose  Locker ";
  lcd.clear();
  lcd.setCursor(0, 0);
  for (int Loop = 0; Loop <= 10; Loop++) {
    lcd.print(Error_Massage[Loop]);
  }
  lcd.setCursor(0, 1);
  for (int Loop = 0; Loop <= 12; Loop++) {
    lcd.print(Full_Input[Loop]);
  }
  lcd.setCursor(0, 1);
  lcd.print(message);
  // แสดงหน้าค้างไว้ 2 วิ
  delay(2000);
  lcd.clear();
  // กลับไปหน้าเลือก Locker ใหม่
  for (int Loop = 0; Loop <= 15; Loop++) {
    lcd.print(Choose_Locker[Loop]);
  }
  // ล้างรหัสผ่านที่ใส่เข้ามา
  Clear_Password();
  // รีเซ็ตการเลือก Locker
  Choosing_Locker = 0;
}

// ฟังชั้นหากมีการเลือก Locker A
void Locker_A() {
  // ดูว่า Locker B ถูกใช้อยู่รึป่าว
  if (Locker_A_status == 1) {
    Clear_Password();
    lcd.clear();
    char Input_A_Password_Display[20] = " Locker A Keys ";
    if (Locker_A_status == 1) {
      lcd.setCursor(0, 0);
      // แสดงสถาณะการเลือกลง LCD
      for (int Loop = 0; Loop <= 14; Loop++) {
        lcd.print(Input_A_Password_Display[Loop]);
      }
    }
  }
  // เมื่อ Locker ว่างให้ผู้ใช้กำหนดรหัสผ่านเอง
  else {
    Clear_Password();
    lcd.clear();
    char Input_A_OneTime_Display[20] = "Onetime Password";
    if (Locker_A_status == 0) {
      lcd.setCursor(0, 0);
      for (int Loop = 0; Loop <= 15; Loop++) {
        lcd.print(Input_A_OneTime_Display[Loop]);
      }
    }
  }
  // แสดงค่าว่ากำลังเลือก Locker A อยู๋
  Choosing_Locker = 1;
}

// ฟังชั้นหากมีการเลือก Locker B
void Locker_B() {
  // ดูว่า Locker B ถูกใช้อยู่รึป่าว
  if (Locker_B_status == 1) {
    Clear_Password();
    lcd.clear();
    char Input_B_Password_Display[20] = " Locker B Keys ";
    if (Locker_B_status == 1) {
      lcd.setCursor(0, 0);
      // แสดงสถาณะการเลือกลง LCD
      for (int Loop = 0; Loop <= 14; Loop++) {
        lcd.print(Input_B_Password_Display[Loop]);
      }
    }
  }
  // เมื่อ Locker ว่างให้ผู้ใช้กำหนดรหัสผ่านเอง
  else {
    Clear_Password();
    lcd.clear();
    char Input_B_Onetime_Display[20] = "Onetime Password";
    if (Locker_B_status == 0) {
      lcd.setCursor(0, 0);
      for (int Loop = 0; Loop <= 15; Loop++) {
        lcd.print(Input_B_Onetime_Display[Loop]);
      }
    }
  }
  // แสดงค่าว่ากำลังเลือก Locker B อยู๋
  Choosing_Locker = 2;
}

// ฟังชั้นกรอกรหัส Locker A ครั้งแรก
void Confirm_Locker_A_Password_Firsttime() {
  char Locker_Password_Saved_1[20] = " Password Saved ";
  char Locker_Password_Saved_2[20] = " Please re-enter";
  lcd.clear();
  for (int Loop = 0; Loop <= 5; Loop++) {
    Locker_A_Password_Array[Loop] = inputArray[Loop];
  }
  lcd.setCursor(0, 0);
  for (int Loop = 0; Loop <= 15; Loop++) {
    lcd.print(Locker_Password_Saved_1[Loop]);
  }
  delay(2000);
  lcd.setCursor(0, 0);
  for (int Loop = 0; Loop <= 15; Loop++) {
    lcd.print(Locker_Password_Saved_2[Loop]);
  }
  Locker_A_status = 1;
  Clear_Password();
}

// ฟังชั้นกรอกรหัส Locker B ครั้งแรก
void Confirm_Locker_B_Password_Firsttime() {
  char Locker_Password_Saved_1[20] = " Password Saved ";
  char Locker_Password_Saved_2[20] = " Please re-enter";
  lcd.clear();
  for (int Loop = 0; Loop <= 5; Loop++) {
    Locker_B_Password_Array[Loop] = inputArray[Loop];
  }
  lcd.setCursor(0, 0);
  for (int Loop = 0; Loop <= 15; Loop++) {
    lcd.print(Locker_Password_Saved_1[Loop]);
  }
  delay(2000);
  lcd.setCursor(0, 0);
  for (int Loop = 0; Loop <= 15; Loop++) {
    lcd.print(Locker_Password_Saved_2[Loop]);
  }
  Locker_B_status = 1;
  Clear_Password();
}

// ฟังชั่นการ ตรวจรหัสผ่านโดยใช้ Pointer ไปรับค่า
bool Verify_Password(char lockerPasswordArray[], char input_Check_Array[]) {
  for (int i = 0; i < maxInput; i++) {
    if (lockerPasswordArray[i] != input_Check_Array[i]) {
      return false;
    }
  }
  return true;
}

// ฟังชั่นหากใส่รหัสผ่าน Locker A ถูก
void Password_A_Currect() {
  char Currect[20] = "Password Currect";
  lcd.setCursor(0, 0);
  for (int Loop = 0; Loop <= 15; Loop++) {
    lcd.print(Currect[Loop]);
  }
  if (Servo_A_Value == 0) {
    Servo_A_Value = 1;
  } else if (Servo_A_Value == 1) {
    Servo_A_Value = 0;
  }
  delay(2000);
  Clear_Password();
  Choosing_Locker = 0;
}

// ฟังชั่นหากใส่รหัสผ่านผิด
void Password_Wrong() {
  char Wrong[20] = " Wrong Password ";
  lcd.setCursor(0, 0);
  for (int Loop = 0; Loop <= 15; Loop++) {
    lcd.print(Wrong[Loop]);
  }
  delay(2000);
  Clear_Password();
  Choosing_Locker = 0;
}

// ฟังชั่นหากใส่รหัสผ่าน Locker B ถูก
void Password_B_Currect() {
  char Currect[20] = "Password Currect";
  lcd.setCursor(0, 0);
  for (int Loop = 0; Loop <= 15; Loop++) {
    lcd.print(Currect[Loop]);
  }
  if (Servo_B_Value == 0) {
    Servo_B_Value = 1;
  } else if (Servo_B_Value == 1) {
    Servo_B_Value = 0;
  }
  delay(2000);
  Clear_Password();
  Choosing_Locker = 0;
}

// ฟังชั่นตรวจสอบสถาณะของ Locker
void Locker_Status_Check() {
  char Locker_Status_Check_Display[20] = " Locker  Status ";
  char Choose_Locker[20] = " Choose  Locker ";
  char Locker_Ready[20] = "Ready";
  char Locker_Busy[20] = "Busy";
  lcd.clear();
  lcd.setCursor(0, 0);
  for (int Loop = 0; Loop <= 15; Loop++) {
    lcd.print(Locker_Status_Check_Display[Loop]);
  }
  lcd.setCursor(0, 1);
  lcd.print("1");
  lcd.print(".");
  lcd.setCursor(8, 1);
  lcd.print("2");
  lcd.print(".");
  lcd.setCursor(2, 1);
  if (Locker_A_status_Check == 0) {
    for (int Loop = 0; Loop <= 4; Loop++) {
      lcd.print(Locker_Ready[Loop]);
    }
  } else {
    for (int Loop = 0; Loop <= 3; Loop++) {
      lcd.print(Locker_Busy[Loop]);
    }
  }
  lcd.setCursor(10, 1);
  if (Locker_B_status_Check == 0) {
    for (int Loop = 0; Loop <= 4; Loop++) {
      lcd.print(Locker_Ready[Loop]);
    }
  } else {
    for (int Loop = 0; Loop <= 3; Loop++) {
      lcd.print(Locker_Busy[Loop]);
    }
  }
  Choosing_Locker = 0;
  delay(5000);
  lcd.clear();
  lcd.setCursor(0, 0);
  for (int Loop = 0; Loop <= 15; Loop++) {
    lcd.print(Choose_Locker[Loop]);
  }
}

void Connection_Server_Failed_Display() {
  char Connect_Server_Failed_Array_1[20] = "   Server   ";
  char Connect_Server_Failed_Array_2[20] = "Connection  Fail";
  lcd.clear();
  lcd.setCursor(0, 0);
  for (int Loop = 0; Loop <= 11; Loop++) {
    lcd.print(Connect_Server_Failed_Array_1[Loop]);
  }
  lcd.setCursor(0, 1);
  for (int Loop = 0; Loop <= 15; Loop++) {
    lcd.print(Connect_Server_Failed_Array_2[Loop]);
  }
  delay(500);
  lcd.clear();
  char Choose_Locker[20] = " Choose  Locker ";
  for (int Loop = 0; Loop <= 15; Loop++) {
    lcd.print(Choose_Locker[Loop]);
  }
}

void Connection_WIFI_Failed_Display() {
  char Connect_WIFI_Failed_Array_1[20] = "      Wi-Fi     ";
  char Connect_WIFI_Failed_Array_2[20] = "Connection  Fail";
  lcd.clear();
  lcd.setCursor(0, 0);
  for (int Loop = 0; Loop <= 15; Loop++) {
    lcd.print(Connect_WIFI_Failed_Array_1[Loop]);
  }
  lcd.setCursor(0, 1);
  for (int Loop = 0; Loop <= 15; Loop++) {
    lcd.print(Connect_WIFI_Failed_Array_2[Loop]);
  }
  delay(500);
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
  if (client.connect(mqtt_Client)) {
    Serial.println("Connected to MQTT");
  }
}

void Get_Password_Reset_Status() {
  WiFiClient client;
  PubSubClient Subclient(espClient);
  unsigned long currentMillis = millis();

  if (WiFi.status() == WL_CONNECTED && currentMillis - lastServerCheck >= intervalServerCheck) {
    lastServerCheck = currentMillis;
    // ส่ง HTTP GET request ไปยัง Node-RED
    client.print(String("GET ") + "/API_Password_Reset" + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

    // รอรับการตอบกลับ
    Password_Reset_Command = "";
    while (client.available() == 0) {
      delay(100);
    }

    while (client.available()) {
      Password_Reset_Command += client.readStringUntil('\r');
    }
    Serial.println(Password_Reset_Command);

    // Parse JSON ที่ได้รับ
    int jsonIndex_Password_Reset_Command = Password_Reset_Command.indexOf('{');
    if (jsonIndex_Password_Reset_Command != -1) {
      String jsonString = Password_Reset_Command.substring(jsonIndex_Password_Reset_Command);
      JSONVar jsonData = JSON.parse(jsonString);

      if (JSON.typeof(jsonData) == "undefined") {
        Serial.println("Parsing input failed!");
      } else {
        String Password_A_Status_Value = (const char*)jsonData["Password_A_Status_Value"];
        String Password_B_Status_Value = (const char*)jsonData["Password_B_Status_Value"];
        Serial.println("Password_A_Status_Value:" + Password_A_Status_Value);
        Serial.println("Password_B_Status_Value:" + Password_B_Status_Value);

        // ควบคุม Servo Locker A
        if (Password_A_Status_Value == "1") {
          for (int Loop = 0; Loop <= 5; Loop++) {
            Locker_A_Password_Array[Loop] = 'X';
          }
          Servo_A_Value = 1;
          Locker_A_status = 1;
        } else if (Password_A_Status_Value == "0") {
          Serial.println(":>");
        }

        if (Password_B_Status_Value == "1") {
          for (int Loop = 0; Loop <= 5; Loop++) {
            Locker_B_Password_Array[Loop] = 'X';
          }
          Servo_B_Value = 1;
          Locker_A_status = 1;
        } else if (Password_B_Status_Value == "0") {
          Serial.println(":>");
        }
      }
    } else {
      Serial.println("DaDa");
    }
  }
}

void Get_Locker_Status_Data() {
  WiFiClient client;
  PubSubClient Subclient(espClient);

  // ส่ง HTTP GET request ไปยัง Node-RED
  client.print(String("GET ") + "/API_Locker_Status" + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");

  // รอรับการตอบกลับ
  Locker_Status_Command = "";
  while (client.available() == 0) {
    delay(10);
  }

  while (client.available()) {
    Locker_Status_Command += client.readStringUntil('\r');
  }
  Serial.println(Locker_Status_Command);

  // Parse JSON ที่ได้รับ
  int jsonIndex_Locker_Status_Command = Locker_Status_Command.indexOf('{');
  if (jsonIndex_Locker_Status_Command != -1) {
    String jsonString = Locker_Status_Command.substring(jsonIndex_Locker_Status_Command);
    JSONVar jsonData = JSON.parse(jsonString);

    if (JSON.typeof(jsonData) == "undefined") {
      Serial.println("Parsing input failed!");
    } else {
      String Locker_A_Status_Value = (const char*)jsonData["Locker_A_Status_Value"];
      String Locker_B_Status_Value = (const char*)jsonData["Locker_B_Status_Value"];
      Serial.println("Locker_A_Status_Value:" + Locker_A_Status_Value);
      Serial.println("Locker_B_Status_Value:" + Locker_B_Status_Value);

      // ควบคุม Servo Locker A
      if (Locker_A_Status_Value != "0") {
        Locker_A_status = 1;
      } else if (Locker_A_Status_Value == "0") {
        Locker_A_status = 0;
      }

      if (Locker_B_Status_Value != "0") {
        Locker_B_status = 1;
      } else if (Locker_B_Status_Value == "0") {
        Locker_B_status = 0;
      }
    }
  }
}

void Publish_Servo_A_To_Nodered(int Servo_A_Status_Send) {

  Servo_A_Command = "\"" + String(Servo_A_Status_Send) + "\"";

  Servo_A_Command.toCharArray(Servo_A_String, 69);
  client.publish("Panel_Board/Servo_A_Status", Servo_A_String);
}

void Publish_Servo_B_To_Nodered(int Servo_B_Status_Send) {

  Servo_B_Command = "\"" + String(Servo_B_Status_Send) + "\"";

  Servo_B_Command.toCharArray(Servo_B_String, 69);
  client.publish("Panel_Board/Servo_B_Status", Servo_B_String);
}

void Publish_Locker_A_Password_To_Nodered(int Locker_A_Status_Send) {

  Status_A_Command = "\"" + String(Locker_A_Status_Send) + "\"";

  Status_A_Command.toCharArray(Status_A_String, 69);
  client.publish("Panel_Board/Status_A_Status", Status_A_String);
}

void Publish_Locker_B_Password_To_Nodered(int Locker_B_Status_Send) {

  Status_B_Command = "\"" + String(Locker_B_Status_Send) + "\"";

  Status_B_Command.toCharArray(Status_B_String, 69);
  client.publish("Panel_Board/Status_B_Status", Status_B_String);
}
