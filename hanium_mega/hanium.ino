#define millisMAX 0XFFFFFFFF
#define TIME_LIMIT 20000//약 복용에 제한시간을 둔다
#define NUMBER_OF_SPACE 5//약 수납공간개수변경시 이부분만 바꾸면 됨
#define TIME_UNIT 1000
//#include<SoftwareSerial.h>
#include<Wire.h>
#include <HX711_ADC.h>
#include <EEPROM.h>
//SoftwareSerial esp8266(12,13); //RX, TX
const int HX711_dout1 = 2; //mcu > HX711 dout pin
const int HX711_sck1 = 3; //mcu > HX711 sck pin
const int HX711_dout2 = 4; //mcu > HX711 dout pin
const int HX711_sck2 = 5; //mcu > HX711 sck pin
const int HX711_dout3 =22; //mcu > HX711 dout pin
const int HX711_sck3 =23; //mcu > HX711 sck pin
const int HX711_dout4 =24; //mcu > HX711 dout pin
const int HX711_sck4 =25; //mcu > HX711 sck pin
const int HX711_dout5 =26; //mcu > HX711 dout pin
const int HX711_sck5 =27; //mcu > HX711 sck pin

float cur_wgt[NUMBER_OF_SPACE];
float pre_wgt[NUMBER_OF_SPACE];
int upcnt[NUMBER_OF_SPACE];
int dncnt[NUMBER_OF_SPACE];
//HX711 constructor:
HX711_ADC LoadCell1(HX711_dout1, HX711_sck1);
HX711_ADC LoadCell2(HX711_dout2,HX711_sck2);
HX711_ADC LoadCell3(HX711_dout3, HX711_sck3);
HX711_ADC LoadCell4(HX711_dout4,HX711_sck4);
HX711_ADC LoadCell5(HX711_dout5, HX711_sck5);

boolean takemedicine;
const int calVal_eepromAdress = 0;
long t;
String SSID="U+NetB9E3";
String PASSWORD="4000009620";
String HOST="192.168.219.102";
String PORT="3000";
String stamac="                 "; 
String MEDIC_PUSH="/medic_push";
String RECORD_URI="/update";
String FAIL_URI="/fail";
String VAC_URI="/vacate";
int countTrueCommand;
int countTimeCommand;
boolean found = false;
String medic_name[NUMBER_OF_SPACE];
unsigned long medic_interval[NUMBER_OF_SPACE];
unsigned long medic_curtime[NUMBER_OF_SPACE];
unsigned long medic_pretime[NUMBER_OF_SPACE];
int led_pins[NUMBER_OF_SPACE] = {32, 33, 34, 35, 36};
boolean medication[NUMBER_OF_SPACE];
boolean fillmedic[NUMBER_OF_SPACE];
unsigned long now;
unsigned long prev;
boolean cycle;
boolean openlid;
boolean alarm;
int speakerpin = 7; //스피커가 연결된 디지털핀 설정
#define PIEZO_INTERVAL 150
int note[] = {2093, 4186, 2793};
int i = 0;
int elementCount = sizeof(note) / sizeof(int);

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  Serial1.begin(9600);
    Wire.begin();
 
  //esp8266.begin(9600);
  
  //get_macaddress();//회원정보,안드로이드기기 찾아가기위한 맥주소
  //Serial.println(stamac);
  //wifi_setup();
  //pinMode(red, OUTPUT);
  //pinMode(green, OUTPUT);
  //pinMode(blue, OUTPUT);
    LoadCell1.begin();
  LoadCell2.begin();
  LoadCell3.begin();
  LoadCell4.begin();
  LoadCell5.begin();
  float calibrationValue; // calibration value (see example file "Calibration.ino")
  calibrationValue = 696.0; // uncomment this if you want to set the calibration value in the sketch
#if defined(ESP8266)|| defined(ESP32)
  //EEPROM.begin(512); // uncomment this if you use ESP8266/ESP32 and want to fetch the calibration value from eeprom
#endif
  //EEPROM.get(calVal_eepromAdress, calibrationValue); // uncomment this if you want to fetch the calibration value from eeprom

  long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell1.start(stabilizingtime, _tare);
  LoadCell2.start(stabilizingtime, _tare);
  LoadCell3.start(stabilizingtime, _tare);
  LoadCell4.start(stabilizingtime, _tare);
  LoadCell5.start(stabilizingtime, _tare);
  
  if (LoadCell1.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell1.setCalFactor(calibrationValue); // set calibration value (float)
    Serial.println("Startup is complete");
  }
  if(LoadCell2.getTareTimeoutFlag())
  {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell2.setCalFactor(calibrationValue); // set calibration value (float)
    Serial.println("Startup is complete");
  }
    if(LoadCell3.getTareTimeoutFlag())
  {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell3.setCalFactor(calibrationValue); // set calibration value (float)
    Serial.println("Startup is complete");
  }
    if(LoadCell4.getTareTimeoutFlag())
  {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell4.setCalFactor(calibrationValue); // set calibration value (float)
    Serial.println("Startup is complete");
  }
    if(LoadCell5.getTareTimeoutFlag())
  {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell5.setCalFactor(calibrationValue); // set calibration value (float)
    Serial.println("Startup is complete");
  }

}

void loop() {
  reception_process();
  provide_medic();
  weight_check();
  now = millis();
  if (diff_check(now, prev, PIEZO_INTERVAL))
  {
    prev = now;
    if (alarm)
    {
      tone(speakerpin, note[i], 200);
    }

    i = (i + 1) % elementCount;
  }
}

void init_medic()//약 수납공간 개수만큼 약정보 초기화하는 함수
{
 for(int i=0;i<NUMBER_OF_SPACE;i++)
 {
  medic_name[i]="";
  medic_interval[i]=0;
  medic_curtime[i]=0;
  medic_pretime[i]=0;
  fillmedic[i]=false;
  pinMode(led_pins[i], OUTPUT);
 }
}


boolean diff_check(unsigned long now,unsigned long prev,unsigned long interval)//롤오버시 주기체크에러 방지
{
  if(now>=prev)
  {
    return ((now-prev)>=interval);
  }
  else
  {
    return ((millisMAX-prev)+now>=interval);//음수로서 비교되지 않도록 연산통해 조건식의 결과값을 반환
  }
}
void periodic_check()
{
  for(int i=0;i<NUMBER_OF_SPACE;i++)
  {
    if(medic_interval[i]!=0)//주기가 설정되어있는약을 찾음
    {
      medic_curtime[i]=millis();
      if(diff_check(medic_curtime[i],medic_pretime[i],medic_interval[i]))
      {
        medic_pretime[i]=medic_curtime[i];
        //pre_wgt[i]=cur_wgt[i];
        medication[i]=true;//어떤약 복용해야할 시점인지 신호를준다.
        Serial.print(medic_name[i]);
        Serial.println("서버푸시요청^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^led켜고 부저켜기");
         digitalWrite(led_pins[i], HIGH);
        alarm=true;
        //httpPost(MEDIC_PUSH,medic_name[i]);
        requestesp('^',medic_name[i]);
        
      }
     }
     if (medication[i] == true && diff_check(medic_curtime[i], medic_pretime[i], TIME_LIMIT)) //약복용 시간이 된것을 제한시간내에 복용했는지 검사
     {
       if(!takemedicine)
       {
         //pre_wgt[i]=cur_wgt[i];
         medication[i]=false;
         Serial.print(medic_name[i]);
         Serial.println("약실패 서버요청, LED끄기 알람 끄기");
          digitalWrite(led_pins[i], LOW);
         alarm=false;
         //httpPost(FAIL_URI,medic_name[i]);
         requestesp('*',medic_name[i]);
       }
     }
  }
}
void reception_process()
{
  String medicdata="";
   //시리얼통신을 통해 약업데이트정보 받음
  while(Serial1.available())
  {
    char myChar=(char)Serial1.read();
    if(!isSpace(myChar))//공백을 제외하고 문자열을 구성해야 문자열 비교가 가능
    medicdata+=myChar;
    delay(15);
  }
  if(!medicdata.equals(""))//안드로이드->라즈베리파이,라즈베리파이->아두이노(시리얼통신)수신데이터(약주기 업데이트정보)가 있을시
  {
    Serial.println(medicdata);
    medic_update(medicdata);//수신데이터를 이용해 삭제하는지 등록또는 수정하는지 판단하여 업데이트수행
  }

}
void provide_medic()
{
  periodic_check();
}
void weigh()
{
   static boolean newDataReady1,newDataReady2,newDataReady3,newDataReady4,newDataReady5 = 0;
  const int serialPrintInterval = 0; //increase value to slow down serial print activity

  // check for new data/start next conversion:
  if (LoadCell1.update()) newDataReady1 = true;
    if (LoadCell2.update()) newDataReady2 = true;
  if (LoadCell3.update()) newDataReady3 = true;
  if (LoadCell4.update()) newDataReady4 = true;
  if (LoadCell5.update()) newDataReady5 = true;

  // get smoothed value from the dataset:
  if (newDataReady1&&newDataReady2&&newDataReady3&&newDataReady4&&newDataReady5) {
    if (diff_check(millis(),t,20)) {
      cur_wgt[0] = LoadCell1.getData();
      cur_wgt[1] = LoadCell2.getData();
      cur_wgt[2] = LoadCell3.getData();
      cur_wgt[3] = LoadCell4.getData();
      cur_wgt[4] = LoadCell5.getData();
      Serial.print("[1]:");
      Serial.print(cur_wgt[0]);
      Serial.print(" [2]:");
      Serial.print(cur_wgt[1]);
           Serial.print(" [3]:");
      Serial.print(cur_wgt[2]);
           Serial.print(" [4]:");
      Serial.print(cur_wgt[3]);
           Serial.print(" [5]: ");
      Serial.println(cur_wgt[4]);
      newDataReady1 = 0;
      newDataReady2=0;
           newDataReady3 = 0;
      newDataReady4=0;
           newDataReady5 = 0;
  
      t = millis();
    }
  }
}
void weight_check()
{
  weigh();//무게 재기
  for(int i=0;i<NUMBER_OF_SPACE;i++)
  {

    if(medication[i])//약복용시간이 된약이 있으면
    {
      if(pre_wgt[i]-cur_wgt[i]>0.15)//약무게가 줄어들면
      {
        if(dncnt[i]>=40)//유지되면
        {
          medication[i]=false;//약복용한것으로 간주함
          dncnt[i]=0;
          takemedicine=true;
          Serial.println("복용서버요청알람끄기LED끄기!!!!!!!!!!!!!"); digitalWrite(led_pins[i], LOW);
          
          alarm=false;
          //httpPost(RECORD_URI,medic_name[i]);
          requestesp('&',medic_name[i]);
          if(cur_wgt[i]<1.05)
          {
          //httpPost(VAC_URI,medic_name[i]);
          requestesp('%',medic_name[i]);
            Serial.println("약을 채우세요서버요청");
          }
        }
        else
        {
          dncnt[i]++;
        }
        
      }
      else
      {
        dncnt[i]=0;
      }
    }
    
   
  }
  
  
}
void medic_update(String medicdata)
{
  char buf1[20];
  char buf2[20];
  int seperator1 = medicdata.indexOf(",");// 첫 번째 콤마 위치
  int seperator2 = medicdata.indexOf(",", seperator1 + 1);
  int length = medicdata.length(); // 문자열 길이
  String token1, token2, token3;
  String medicname;
  unsigned long medicinterval;
  int medic_loc;
  token1 = medicdata.substring(0, seperator1); //첫번째 토큰 무조건 저장
  if (seperator2 >= 0) //등록신호가 들어왔을때
  {
    token2 = medicdata.substring(seperator1 + 1, seperator2);
    token3 = medicdata.substring(seperator2 + 1, length);
  }
  else//등록신호가 아닌 삭제,수정일때
  {
    token2 = medicdata.substring(seperator1 + 1, length);
  }

  if (token1.equals("*") && seperator2 < 0) //삭제신호
  {
    medicname = token2; //삭제신호이면 token2가 약이름
    del_medic(medicname);
  }
  else//삭제신호가 아니면 token1은 약이름 token2는 주기
  {
    medicname = token1;
    token2.toCharArray(buf1, 20);
    medicinterval = (unsigned long)(atoi(buf1)) * TIME_UNIT; //주기를 1000ms를 곱하여 초단위로 변경(시연용으로 초단위)
    if (seperator2 >= 0)
    {
      token3.toCharArray(buf2, 20);
      medic_loc = atoi(buf2);
      medic_add(medicname, medicinterval, medic_loc);
    }
    else
    {
      medic_change(medicname, medicinterval); //약수정,추가함수호출
    }
  }
}
void medic_add(String medicname, unsigned long medicinterval, int medic_loc)
{
  medic_loc-=1;
  medic_name[medic_loc] = medicname; //약이름 추가
  medic_interval[medic_loc] = medicinterval; //약 주기 추가
  medic_pretime[medic_loc] = millis(); //이 시점부터 약주기 순환시작
}
void medic_change(String medicname, unsigned long medicinterval) //수정,추가 함수
{
  //순회결과 약이있으면 약주기를 변경
  for (int i = 0; i < NUMBER_OF_SPACE; i++)
  {
    if (medic_name[i].equals(medicname))
    {
      medic_interval[i] = medicinterval;
      medic_pretime[i] = millis(); //약추가 시점부터 주기순환 시작
      break;
    }
  }

}
void del_medic(String medicname)//약 삭제함수
{
  for (int i = 0; i < NUMBER_OF_SPACE; i++)
  {
    if (medic_name[i].equals(medicname)) //순회 결과 삭제하려는 약이름이 있을경우 삭제(이름,주기모두 초기화)
    {
      medic_name[i] = "";
      medic_interval[i] = 0;
      medic_curtime[i] = 0;
      medic_pretime[i] = 0;
      medication[i] = false;
      break;
    }
  }
}
void requestesp(char c,String requestdata)
{
  Wire.beginTransmission(1);
  String data="";
  data+=c;//요청신호(푸시,복용업데이트등)를 뜻하는 문자합치기
  data.concat(",");//구분문자 합치기
  data.concat(requestdata);//약이름 합치기
  Serial.println(data);
    byte*temp=new byte[data.length()+1];
    data.getBytes(temp,data.length()+1);
    for(int i=0;i<data.length();i++)
    {
     Wire.write(*(temp+i));
    }
    Wire.endTransmission();
}
/**void get_macaddress()
{
  int a=0;
  char buf[100];
  esp8266.println("AT+CIFSR");//station 맥주소를 얻는명령어
    

  esp8266.readString().toCharArray(buf,100);//문자열 버퍼에 응답데이터(맥주소를포함한 데이터)저장
  Serial.println(buf);
  while(true)
  {    
    if(buf[a]=='S'&&buf[a+1]=='T'&&buf[a+2]=='A'&&buf[a+3]=='M')
    {
      a+=8;
      for(int i=0;i<17;i++)
      {
        stamac.setCharAt(i,buf[a]);
        a++;
      }
      break;
    }
    a++;
      
  }//문자열중 맥주소만 추출
}
void sendCommandToESP8266(String command, int maxTime, char readReplay[]) {//와이파이 접속실패를 방지하기 위한 함수
 
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while (countTimeCommand < (maxTime * 1))//인자로 받은 maxtime만큼 반복하여 at명령어 수행
  {
    esp8266.println(command);
    if (esp8266.find(readReplay))//AT명령어에 따른 응답데이터중 원하는 데이터를 찾으면 찾은것으로 간주
    {
      found = true;
      break;
    }
    countTimeCommand++;
  }
  if (found == true)
  {
    Serial.println("Success");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  if (found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  found = false;
}
void wifi_setup()
{
  sendCommandToESP8266("AT+RST", 5, "OK");
  sendCommandToESP8266("AT+CWMODE=1", 5, "OK");
  sendCommandToESP8266("AT+CWJAP=\"" + SSID + "\",\"" + PASSWORD + "\"", 20, "OK");
  sendCommandToESP8266("AT+CIPMUX=1", 5, "OK");//와이파이 초기설정
}
void httpPost(String uri,String msg)//웹서버에 post요청을 전송하는 함수
{
  Serial.println(uri);
  sendCommandToESP8266("AT+CIPSTART=0,\"TCP\",\"" + HOST + "\"," +PORT, 5, "OK");//AT명령어를 통해 웹서버에 연결
  String postdata="";
  String key_1="usermac=";
  String key_2="medicname=";
  postdata.concat(key_1);
  postdata.concat(stamac);//맥주소 데이터에 실음
  postdata.concat("&");
  postdata.concat(key_2);
  postdata.concat(msg);//약이름 
  
  
  Serial.print("post data [");
  Serial.print(postdata);
  Serial.println("]");
  String postRequest=
  "POST "+uri+" HTTP/1.1\r\n"+
  "Host: "+HOST+"\r\n"+
  "Content-Type: application/x-www-form-urlencoded\r\n"+
  "Content-Length: " + postdata.length() + "\r\n"+
  "\r\n"+
  postdata+"\r\n";
  String cipSend="AT+CIPSEND=0,"+String(postRequest.length());
  sendCommandToESP8266(cipSend,20,">");//post 요청 전처리
  esp8266.println(postRequest);//완성된 post 데이터 요청
  countTrueCommand++;
}**/
