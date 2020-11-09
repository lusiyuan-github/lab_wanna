/*2020.11.7卢思远审核，窗帘加温度计*/
/*2020.11.8卢思远审核，窗帘加温度计,可控制窗帘开关*/
//存在问题，舵机在第二次错误会抽风，暂判断暂歇掉电
/**窗帘加温度计完成  制作人、审核人：卢思远2020.11.8*/
#define BLINKER_WIFI //blinkerWiFi
#define BLINKER_MIOT_SENSOR//引入小爱传感器
#include <Blinker.h>
#include <DHT.h>
#include <U8g2lib.h>
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 14, /* data=*/ 2, /* reset=*/ U8X8_PIN_NONE);
char auth[] = "df3299543667"; 
char ssid[] = "19-723";
char pswd[] = "wzy2458158245";
//char ssid[] = "lu";
//char pswd[] = "17354354317";
BlinkerNumber HUMI("humi"); 
BlinkerNumber TEMP("temp");
BlinkerButton Button1("btn-1");
BlinkerButton Button2("btn-2");
BlinkerButton Button3("btn-3");
BlinkerButton Button4("btn-4");
BlinkerButton Button5("btn-5");
#define DHTPIN 5 //#define DHTTYPE DHT11
#define DHTTYPE DHT11 //#define DHTTYPE DHT21
#define Lightsensor 4 //#define light sensor
#define motor_left 1
#define motor_right 0

DHT dht(DHTPIN, DHTTYPE); 
float temp_read = 0;
float humi_read = 0;
byte light_read = 0;
int flag = 0;//判断手动自动
int open_light = 0;//判断是否开窗过
int close_light = 0;

void button1_callback(const String & state)
{
    flag = 1;
}

void button2_callback(const String & state)
{
    flag = 0;
}
void button3_callback(const String & state)
{
    digitalWrite(motor_left,LOW);
    digitalWrite(motor_right,HIGH);
}
void button4_callback(const String & state)
{
    digitalWrite(motor_left,LOW);
    digitalWrite(motor_right,LOW);
}
void button5_callback(const String & state)
{
    digitalWrite(motor_left,HIGH);
    digitalWrite(motor_right,LOW);
}
void heartbeat() 
{
  
  HUMI.print(humi_read); 
  TEMP.print(temp_read);
}//获取心跳包
void dataStorage() 
{
  Blinker.dataStorage("temp", temp_read); 
  Blinker.dataStorage("humi", humi_read);
}//存储数据至云端

void miotQuery(int32_t queryCode)
{
    BLINKER_LOG("MIOT Query codes: ", queryCode);

    switch (queryCode)
    {
        case BLINKER_CMD_QUERY_HUMI_NUMBER :
            BLINKER_LOG("MIOT Query HUMI");
            BlinkerMIOT.humi(humi_read);
            BlinkerMIOT.print();
            break;
        case BLINKER_CMD_QUERY_TEMP_NUMBER :
            BLINKER_LOG("MIOT Query TEMP");
            BlinkerMIOT.temp(temp_read);
            BlinkerMIOT.print();
            break;
        default :
            BlinkerMIOT.temp(temp_read);
            BlinkerMIOT.humi(int(humi_read));
            BlinkerMIOT.print();
            break;
    }
}//小爱语言包
void setup() {
  Serial.begin(115200); 
  BLINKER_DEBUG.stream(Serial); 
  BLINKER_DEBUG.debugAll();
  Blinker.begin(auth, ssid, pswd); 
  Blinker.attachHeartbeat(heartbeat); 
  Blinker.attachDataStorage(dataStorage); 
  pinMode(motor_left,OUTPUT);
  pinMode(motor_right,OUTPUT);
  digitalWrite(motor_left,LOW);
  digitalWrite(motor_right,LOW);
  dht.begin();
  u8g2.begin();
  Blinker.vibrate();//初始化成功震动
  Blinker.notify("智能家居已启动");   
  BlinkerMIOT.attachQuery(miotQuery);//小爱回调函数 
  Button1.attach(button1_callback);
  Button2.attach(button2_callback);
  Button3.attach(button3_callback);
  Button4.attach(button4_callback);
  Button5.attach(button5_callback);
}
void loop() {
  Blinker.run();
  float h = dht.readHumidity(); 
  float t = dht.readTemperature();
  light_read = digitalRead(Lightsensor);//读光敏电阻
  if(flag == 1)
  {
    if(light_read == 1&& open_light == 0)
    {
      BLINKER_LOG("OPEN");
      digitalWrite(motor_left,LOW);
      digitalWrite(motor_right,HIGH);
      delay(2000);
      digitalWrite(motor_left,LOW);
      digitalWrite(motor_right,LOW);
      open_light = 1;
      close_light = 0;
    }
    if(light_read == 0&& close_light == 0)
    {
      BLINKER_LOG("CLOSE");
      digitalWrite(motor_left,HIGH);
      digitalWrite(motor_right,LOW);
      delay(2000);
      digitalWrite(motor_left,LOW);
      digitalWrite(motor_right,LOW);
      open_light = 0;
      close_light = 1;
    }
    
  }
  
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_inb16_mf); // choose a suitable font
  u8g2.setCursor(0, 15);
  u8g2.print("T:"); 
  u8g2.print(t); 
  u8g2.setCursor(0, 40);
  u8g2.print("H:"); 
  u8g2.print(h);   
  u8g2.sendBuffer();          // transfer internal memory to the display
  if (isnan(h) || isnan(t)) 
  {
    BLINKER_LOG("Failed to read from DHT sensor!");
  } 
  else 
  {
    BLINKER_LOG("Humidity: ", h, " %"); BLINKER_LOG("Temperature: ", t, " *C"); 
    humi_read = h; 
    temp_read = t;
    if(t>=30)
    {
    
      u8g2.setCursor(0, 65);
      u8g2.print("Too hot!!!");   
      u8g2.sendBuffer();    
    }
    if(t<=10)
    {
      u8g2.setCursor(0, 65);
      u8g2.print("Too cold!!!");   
      u8g2.sendBuffer();   
    }
  }
  Blinker.delay(2000);//每两秒测温湿度
}
