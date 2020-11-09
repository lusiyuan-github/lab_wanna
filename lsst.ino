/**门禁加识别,灯光控制完成  制作人、审核人：卢思远2020.11.7*/

//存在问题，调整灯光时候绿灯会亮，暂时不知如何更改

/*门禁加识别完成  制作人、审核人：卢思远2020.11.7*/
#define BLINKER_WIFI
#define BLINKER_MIOT_LIGHT
#include <Blinker.h>
#include <SPI.h>
#include <MFRC522.h>  //RFID
#include <Servo.h>  //电机
#include <Adafruit_NeoPixel.h>  //灯光组

Servo myservo;  // 定义Servo对象来控制

#define RST_PIN         5           // 配置rc522针脚
#define SS_PIN          4           //配置rc522针脚
#define Right_LED       2           
#define Wrong_LED       0
#define Bizzer         15       
#define PIN             3           //灯光组引脚定义
#define NUMPIXELS      30           // 灯泡数目
char auth[] = "5ddceaab9da0";
char ssid[] = "19-723";
char pswd[] = "wzy2458158245";
//char ssid[] = "";
//char pswd[] = "wzy2458158245";

String refid1="";//先声明一个空的字符串全局变量。以便后面存储nuidPICC里面的数据。 
MFRC522 rfid(SS_PIN, RST_PIN); //实例化类RFID
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800); //初始化WS2812
// 新建组件对象
// 初始化数组用于存储读取到的NUID 


byte nuidPICC[4];
byte counter_part = 0;
byte last_one_back = 0;
int pos = 0;    // 角度存储变量
int flag = 0;   //  保证门锁开一次
uint8_t colorR, colorG, colorB, colorW;   //colorW 为亮度


BlinkerButton Button1("btn-abc");
BlinkerNumber Number1("num-abc");
BlinkerText Text1("tex-1");
#define RGB_1 "RGBKey"
BlinkerRGB WS2812(RGB_1);
// 按下按键即会执行该函数

void button1_callback(const String & state)
{
  if(last_one_back==0)
  {
    Text1.print("Lu back");
  }
  if(last_one_back==1)
  {
    Text1.print("son back");
  }  
    
}//按键回调函数

void ws2812_callback(uint8_t r_value, uint8_t g_value, uint8_t b_value, uint8_t bright_value)
{
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    BLINKER_LOG("R value: ", r_value);
    BLINKER_LOG("G value: ", g_value);
    BLINKER_LOG("B value: ", b_value);
    BLINKER_LOG("Rrightness value: ", bright_value);

    colorR = r_value;
    colorG = g_value;
    colorB = b_value;
    colorW = bright_value;

    pixelShow();
}//rgb回调函数

void pixelShow()
{
    pixels.setBrightness(colorW);

    for(int i = 0; i < NUMPIXELS; i++){
        pixels.setPixelColor(i, colorR, colorG, colorB);
    }
    pixels.show();
}//ws2812设置灯光组


void miotPowerState(const String & state)
{
    BLINKER_LOG("need set power state: ", state);

    if (state == BLINKER_CMD_ON) {
        colorR = 255;
        colorG = 255;
        colorB = 255;
        colorW = 255;//调制灯光最亮
        BlinkerMIOT.powerState("on");
        BlinkerMIOT.print();


        
    }
    else if (state == BLINKER_CMD_OFF) {
        digitalWrite(LED_BUILTIN, LOW);
        colorW = 0;
        BlinkerMIOT.powerState("off");
        BlinkerMIOT.print();


    }

    pixelShow();
}

void miotColor(int32_t color)
{
    BLINKER_LOG("need set color: ", color);

    colorR = color >> 16 & 0xFF;
    colorG = color >>  8 & 0xFF;
    colorB = color       & 0xFF;

    BLINKER_LOG("colorR: ", colorR, ", colorG: ", colorG, ", colorB: ", colorB);

    pixelShow();

    BlinkerMIOT.color(color);
    BlinkerMIOT.print();
}


void miotBright(const String & bright)//设置亮度
{
    BLINKER_LOG("need set brightness: ", bright);

    colorW = bright.toInt();

    BLINKER_LOG("now set brightness: ", colorW);

    pixelShow();

    BlinkerMIOT.brightness(colorW);
    BlinkerMIOT.print();
}
//小爱回调函数
void setup()
{

    Serial.begin(9600);// 初始化串口
    BLINKER_DEBUG.stream(Serial);
    BLINKER_DEBUG.debugAll();      // 初始化有LED的IO
    pinMode(Right_LED,OUTPUT);
    pinMode(Wrong_LED,OUTPUT);
    pinMode(Bizzer,OUTPUT);
    digitalWrite(Right_LED,0);
    digitalWrite(Wrong_LED,0);
    digitalWrite(Bizzer,0);
    colorR = 255;
    colorG = 255;
    colorB = 255;
    colorW = 0;
    myservo.attach(1);  // 控制线连接数字
    SPI.begin(); // 初始化SPI总线
    rfid.PCD_Init(); // 初始化 MFRC522 
    pixels.begin();// 初始化ws2812
    pixels.setBrightness(colorW);// 初始化ws2812亮度

    pixelShow();
    BlinkerMIOT.attachPowerState(miotPowerState);     //灯光开关
    BlinkerMIOT.attachColor(miotColor);       //调颜色
    BlinkerMIOT.attachBrightness(miotBright);    //设置亮度
    
    Blinker.begin(auth, ssid, pswd);
    
    Blinker.notify("智能门禁已开启！");
    
    Blinker.attachData(button1_callback);
    WS2812.attach(ws2812_callback);
    //Blinker.attachData(dataRead);

    Button1.attach(button1_callback);
}

void loop() {
    Blinker.run();
  /*本模块设置rc522寻找卡并且进行动作 
 * 2020.10.16
 * 卢思远最后审查*/
 
  
  if ( ! rfid.PICC_IsNewCardPresent())
    return;  // 找卡，无则返回 
  if ( ! rfid.PICC_ReadCardSerial())
    return;// 验证NUID是否可读
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak); 
  
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K) 
  {
    //Serial.println("不支持读取此卡类型");
    return;// 检查是否MIFARE卡类型
  }
  
  // 将NUID保存到nuidPICC数组
  for (byte i = 0; i < 4; i++) 
  {
    nuidPICC[i] = rfid.uid.uidByte[i];
  }   
  //Serial.print("十进制UID：");
  printHex(rfid.uid.uidByte, rfid.uid.size);
  //Serial.println();
  
  if(refid1=="231527698")    
  { 
    //Serial.println("已验证");//判断为真，执行需要的程序。
    counter_part = 0;             //在错误超过后重置
    digitalWrite(Bizzer,0);       //在错误超过后重置
    digitalWrite(Wrong_LED,0);    //在错误超过后重置
    last_one_back = 0;
    Bizzer_right_normalcard();    //卡验证正确时反应 
    if(flag == 0)
    {
        for (pos = 0; pos <= 100; pos ++) 
        { // 0°到180°
        // in steps of 1 degree
          myservo.write(pos);              // 舵机角度写入
          delay(5);                        // 等待转动到指定角度
        }
        flag = 1;
    }   
    Blinker.notify("卢思远用门禁卡开门了");  
    //Blinker.wechat("智能门禁已开启！"); 
  }
  else if(refid1=="103150167199")    
  { 
    //Serial.println("已验证");//判断为真，执行需要的程序。
    counter_part = 0;             //在错误超过后重置
    digitalWrite(Bizzer,0);       //在错误超过后重置
    digitalWrite(Wrong_LED,0);    //在错误超过后重置
    last_one_back = 1;
    Bizzer_right_normalcard();    //卡验证正确时反应      
    if(flag == 0)
    {
        for (pos = 0; pos <= 100; pos ++) 
        { // 0°到180°
        // in steps of 1 degree
          myservo.write(pos);              // 舵机角度写入
          delay(5);                       // 等待转动到指定角度
        }
        flag = 1;
    }
     Blinker.notify("儿子用小米手环开门了");  
  }
  else
  { 
    if(flag == 1)
    {
        for (pos = 100; pos >= 0; pos --) 
        { // 0°到180°
        // in steps of 1 degree
          myservo.write(pos);              // 舵机角度写入
          delay(5);                       // 等待转动到指定角度
        }
        flag = 0;
    } 
    if(counter_part<=4)
    {  
      //Serial.println("验证失败");//判断为假，执行需要的程序。
      Bizzer_wrong_normalcard();  //卡验证错误时反应     
      counter_part++;
      //Serial.print("你已经输入错误");
      //Serial.print(counter_part);
      //Serial.println("次"); 
    } 
    if(counter_part>4)
    {
      //Serial.print("错误次数超过4次，请用正确卡解除报警");
      digitalWrite(Bizzer,1);
      digitalWrite(Wrong_LED,1);
      Blinker.notify("您家中门禁刷卡错误次数过多，请刷入正确卡或检查是否有小偷！！！"); 
    }
  }
 
  rfid.PICC_HaltA();  // 使放置在读卡区的IC卡进入休眠状态，不再重复读卡
  rfid.PCD_StopCrypto1();  // 停止读卡模块编码
  refid1="";
}
void printHex(byte *buffer, byte bufferSize) 
{
  for (byte i = 0; i < bufferSize; i++) {
    //Serial.print(buffer[i] < 0x10 ? " 0" : "");
        refid1+=nuidPICC[i]; //对nuidPICC进行存储     
  }
   
  //Serial.print(refid1);//对nuidPICC打印，看是是自己需要的数据。 
}//转换为十进制UID


void Bizzer_right_normalcard()
{
    digitalWrite(Right_LED,1);
    analogWrite(Bizzer,1024);

    delay(400);
    digitalWrite(Bizzer,0);
    delay(200);
    analogWrite(Bizzer,1024);
    delay(400);
    digitalWrite(Right_LED,0);
    digitalWrite(Bizzer,0);
    
}
void Bizzer_wrong_normalcard()
{
    digitalWrite(Wrong_LED,1);
    analogWrite(Bizzer,800);
    delay(400);
    digitalWrite(Bizzer,0);
    delay(200);
    analogWrite(Bizzer,800);
    delay(400);
    digitalWrite(Wrong_LED,0);
    digitalWrite(Bizzer,0);
    
    
}  
