/**
   '########:::::'###::::'####:'##::: ##::'######:::'##::::'##::::'###:::::'######:::'########:
    ##.... ##:::'## ##:::. ##:: ###:: ##:'##... ##:: ##:::: ##:::'## ##:::'##... ##:: ##.....::
    ##:::: ##::'##:. ##::: ##:: ####: ##: ##:::..::: ##:::: ##::'##:. ##:: ##:::..::: ##:::::::
    ########::'##:::. ##:: ##:: ## ## ##: ##::'####: ##:::: ##:'##:::. ##: ##::'####: ######:::
    ##.. ##::: #########:: ##:: ##. ####: ##::: ##:: ##:::: ##: #########: ##::: ##:: ##...::::
    ##::. ##:: ##.... ##:: ##:: ##:. ###: ##::: ##:: ##:::: ##: ##.... ##: ##::: ##:: ##:::::::
    ##:::. ##: ##:::: ##:'####: ##::. ##:. ######:::. #######:: ##:::: ##:. ######::: ########:
   ..:::::..::..:::::..::....::..::::..:::......:::::.......:::..:::::..:::......::::........::
   
   @file RainGauge.ino
   @author minhxl, binhphuong

   @brief This sketch help controlling on-board sensors, measuring battery level. For detail description, 
   please visit: https://github.com/XuanMinh201/RainGauge

   @version 0.0.1
   @date 2023-10-26

   @copyright Copyright (c) 2023

*/

#include "Adafruit_SHTC3.h" //http://librarymanager/All#Adafruit_SHTC3

// Set pin number
#define buttonPin PB5  

// Rain Gauge battery
#define LS_ADC_AREF 3.0f
#define LS_BATVOLT_R1 1.0f 
#define LS_BATVOLT_R2 2.0f
#define LS_BATVOLT_PIN PB4

uint16_t voltage_adc;
uint16_t voltage;

// Set Interrupt
int ledToggle;
int previousState = HIGH;
unsigned int previousPress;
volatile int buttonFlag, buttonFlag_falseDetect;
const int buttonDebounce = 50;
volatile int lastDetect = 0;

volatile int rainFlag;    // turn on this flag if it is rain
volatile int notRainFlag; // turn on this flag if it is not rain
volatile unsigned int rainGaugeCount = 0;
unsigned long time1 = 0;

uint32_t estimatedNextUplink = 0;
int rain_count;
// Set sensor variables
int temper;
int humi;

// Rain Stop Time
uint64_t lastRain = 0; // the last time when it was rain
uint64_t elapsedRain;
uint64_t spendTime; // the remaining time before wake up in period OTAA

bool bucketPositionA = false; // one of the two positions of tipping-bucket
// const double bucketAmount = 0.01610595;   // inches equivalent of ml to trip tipping-bucket
#define ABP_PERIOD   (1800000) //  sleep cycle 30m

//time in case of heavy rain.
#define t_rain   (300000)

// #define RAIN_STOP_TIME   (6000)
/*************************************

   LoRaWAN band setting:
     RAK_REGION_EU433
     RAK_REGION_CN470
     RAK_REGION_RU864
     RAK_REGION_IN865
     RAK_REGION_EU868
     RAK_REGION_US915
     RAK_REGION_AU915
     RAK_REGION_KR920
     RAK_REGION_AS923
     9 (REGION_AS923_2) recommand for vietnam 
 *************************************/

//#define ABP_BAND     (RAK_REGION_EU868)
#define ABP_BAND     (9)

#define ABP_DEVADDR  {0x01, 0x02, 0x03, 0x04}
#define ABP_APPSKEY  {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}
#define ABP_NWKSKEY  {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}


/** Packet buffer for sending */
uint8_t collected_data[64] = { 0 };

Adafruit_SHTC3 shtc3 = Adafruit_SHTC3();

void recvCallback(SERVICE_LORA_RECEIVE_T * data)
{
  if (data->BufferSize > 0) {
    Serial.println("Something received!");
    for (int i = 0; i < data->BufferSize; i++) {
      Serial.printf("%x", data->Buffer[i]);
    }
    Serial.print("\r\n");
  }
}

void sendCallback(int32_t status)
{
  if (status == 0) {
    Serial.println("Successfully sent");
  } else {
    Serial.println("Sending failed");
  }
}

void setup()
{
  Serial.begin(115200, RAK_AT_MODE);

  // Initialize Interrupt
  Serial.println("RAKwireless Arduino Interrupt Example");
  Serial.println("------------------------------------------------------");
  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPin), button_ISR, FALLING);

  buttonFlag = 0;
  buttonFlag_falseDetect = 0;
  lastDetect = 0;
  
  // Initialize SHTC3
  Serial.println("SHTC3 test");
  if (!shtc3.begin())
  {
    Serial.println("Couldn't find SHTC3");
     //while (1) delay(1);
  }
  Serial.println("Found SHTC3 sensor");

  //Initialize AHT20
//  Serial.begin(115200);
//  Serial.println("Adafruit AHT10/AHT20 demo!");
//
//  if (! aht.begin()) {
//    Serial.println("Could not find AHT? Check wiring");
//    while (1) delay(10);
//  }
//  Serial.println("AHT10 or AHT20 found");

  // Wake-up
  api.system.sleep.setup(RUI_WAKEUP_FALLING_EDGE, PA0);

    // ABP Device Address MSB first
  uint8_t node_dev_addr[4] = ABP_DEVADDR;
  // ABP Application Session Key
  uint8_t node_app_skey[16] = ABP_APPSKEY;
  // ABP Network Session Key
  uint8_t node_nwk_skey[16] = ABP_NWKSKEY;

  if (!api.lorawan.daddr.set(node_dev_addr, 4)) {
    Serial.printf("LoRaWan ABP - set device addr is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.appskey.set(node_app_skey, 16)) {
    Serial.printf
  ("LoRaWan ABP - set application session key is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.nwkskey.set(node_nwk_skey, 16)) {
    Serial.printf
  ("LoRaWan ABP - set network session key is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.band.set(ABP_BAND)) {
    Serial.printf("LoRaWan ABP - set band is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.deviceClass.set(RAK_LORA_CLASS_A)) {
    Serial.printf("LoRaWan ABP - set device class is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.njm.set(RAK_LORA_ABP)) // Set the network join mode to ABP
  {
    Serial.
  printf("LoRaWan ABP - set network join mode is incorrect! \r\n");
    return;
  }

  if (!api.lorawan.adr.set(true)) {
    Serial.
  printf("LoRaWan ABP - set adaptive data rate is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.rety.set(1)) {
    Serial.printf("LoRaWan ABP - set retry times is incorrect! \r\n");
    return;
  }
  if (!api.lorawan.cfm.set(1)) {
    Serial.printf("LoRaWan ABP - set confirm mode is incorrect! \r\n");
    return;
  }

  /** Check LoRaWan Status*/
  Serial.printf("Duty cycle is %s\r\n", api.lorawan.dcs.get()? "ON" : "OFF"); // Check Duty Cycle status
  Serial.printf("Packet is %s\r\n", api.lorawan.cfm.get()? "CONFIRMED" : "UNCONFIRMED");  // Check Confirm status
  uint8_t assigned_dev_addr[4] = { 0 };
  api.lorawan.daddr.get(assigned_dev_addr, 4);
  Serial.printf("Device Address is %02X%02X%02X%02X\r\n", assigned_dev_addr[0], assigned_dev_addr[1], assigned_dev_addr[2], assigned_dev_addr[3]);  // Check Device Address
  Serial.printf("Uplink period is %ums\r\n", ABP_PERIOD);
  Serial.println("");
  api.lorawan.registerRecvCallback(recvCallback);
  api.lorawan.registerSendCallback(sendCallback);
}

void uplink_routine()
{
  /** Payload of Uplink */
  uint8_t data_len = 0;
  collected_data[data_len++] = (uint8_t)buttonFlag;
  collected_data[data_len++] = (uint8_t)temper >> 8;
  collected_data[data_len++] = (uint8_t)(temper / 10) & 0xFF;
  collected_data[data_len++] = (uint8_t)humi & 0xFF;
  collected_data[data_len++] = (uint8_t)(voltage >> 8) & 0xff;
  collected_data[data_len++] = (uint8_t)voltage & 0xff;

  Serial.println("Data Packet:");
  for (int i = 0; i < data_len; i++)
  {
    Serial.printf("0x%02X ", collected_data[i]);
  }
  Serial.println("");

  /** Send the data package */
  if (api.lorawan.send(data_len, (uint8_t *)&collected_data, 2, true, 1))
  {
    Serial.println("Sending is requested");
  }
  else
  {
    Serial.println("Sending failed");
  }
}

void loop()
{
  // Read SHTC3
 
  
  //
  sensors_event_t humidity, temp;
  shtc3.getEvent(&humidity, &temp); // populate temp and humidity objects with fresh data
  temper = (temp.temperature) * 10;
  humi = humidity.relative_humidity;
  Serial.print("Sensors values : temp = ");
  Serial.print(temper / 10);
  Serial.println("deg");
  Serial.print("hum= ");
  Serial.print(humi);
  Serial.println("%");

//  sensors_event_t humidity, temp;
//  aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
//  temper = (temp.temperature) * 10;
//  humi = humidity.relative_humidity;
//  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
//  Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");
  
  Serial.print("Value: ");
  Serial.println(buttonFlag);
  Serial.print(" | False Detect: ");
  Serial.println(buttonFlag_falseDetect);

  //Battery variables
  voltage_adc = (uint16_t)analogRead(LS_BATVOLT_PIN);
  voltage = (uint16_t)((LS_ADC_AREF / 1.024) * (LS_BATVOLT_R1 + LS_BATVOLT_R2) / LS_BATVOLT_R2 * (float)voltage_adc);

  Serial.print("Voltage: ");
  Serial.println(voltage);

  // LoRaWAN Uplink
  uplink_routine();
  buttonFlag = 0;
  rain_count = 0;

  // Set sleep until the next LoRaWAN Uplink
  Serial.printf("Try sleep %ums..", ABP_PERIOD);
  estimatedNextUplink = millis() + ABP_PERIOD;
  api.system.sleep.all(ABP_PERIOD);

  // Re-check the wake up reason. If the wakeup caused by External Interrupt, go back to sleep mode
  while (estimatedNextUplink > millis())
  {
    uint32_t remainingSleepTime = estimatedNextUplink - millis();
    api.system.sleep.all(remainingSleepTime);
  }

  Serial.println("Wakeup..");
}

void button_ISR()
{
  if (rain_count == 0){
    estimatedNextUplink = estimatedNextUplink - (ABP_PERIOD - t_rain);
  }
  rain_count++;
  int _now = millis();
  if ((_now - lastDetect) > buttonDebounce)
  {
    lastDetect = _now;
    buttonFlag++;
  }
  else
  {
    buttonFlag_falseDetect++;
  }
}
