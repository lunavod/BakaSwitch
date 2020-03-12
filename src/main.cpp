#include <Arduino.h>
#include <sys/types.h>

#define LED1 8
#define LED2 15
#define LED3 14

#define YELLOW 1
#define GREEN  2
#define BLUE   3

#define BULB1 6
#define BULB2 7

int leds[3] = {LED1, LED2, LED3};
bool states[3];

void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  pinMode(BULB1, OUTPUT);
  pinMode(BULB2, OUTPUT);

  Serial.begin(9600);
  Serial.setTimeout(100);
}

void setState(int index, bool state) {
  if (index > 3 || index < 0) {
    return;
  }
  if (index == 1) {
    digitalWrite(BULB1, state);
    digitalWrite(BULB2, state);
  }
  index--;
  digitalWrite(leds[index], state);
  states[index] = state;
}

int send(char msg[]) {
  short x = strlen(msg);
  unsigned char buf[2+x];
  buf[0] = (x >> (8*1)) & 0xff;
  buf[1] = (x >> (8*0)) & 0xff;
  for (int i = 0; i < x; i++) {
    buf[sizeof(x)+i] = msg[i];
  }
  int n = Serial.write(buf,sizeof(buf));
  Serial.flush();
  return n;
}

int sendMsg(char msg[], char payload[]) {
  send(msg);
  send(payload);
}

void callback(String event, char payload[]) {
  if (event == "lu/switch/on") {
    int index = atoi(payload);
    setState(index, 1);
    sendMsg("OK", "GOTCHA");
  } else if (event == "lu/switch/off") {
    int index = atoi(payload);
    setState(index, 0);
    sendMsg("OK", "GOTCHA");
  } else if (event == "MQTT_INIT") {
    sendMsg("MQTT_SUBSCRIBE", "lu/switch/on");
    sendMsg("MQTT_SUBSCRIBE", "lu/switch/off");
    sendMsg("MQTT_SUBSCRIBE", "lu/switch/state/request");
  } else if (event == "PING") {
    sendMsg("PONG", "Hi");
  } else if (event == "lu/switch/state/request") {
    sendMsg("lu/switch/state/response", (char*)(states[0]? "on" : "off"));
  }
}

void serialEvent() {
  char eventSizeBuf[2];
  Serial.readBytes(eventSizeBuf, sizeof(eventSizeBuf));

  if (eventSizeBuf[0] == '+' && eventSizeBuf[1] == 'D') {
    setState(BLUE, 1);
    Serial.readString();
    Serial.readString();
    return;
  } else {
    setState(BLUE, 0);
  }

  short eventSize = short(eventSizeBuf[0] << 8 | eventSizeBuf[1]);

  char event[eventSize+1] = {0};
  Serial.readBytes(event, eventSize);
  event[eventSize] = '\0';

  char payloadSizeBuf[2];
  Serial.readBytes(payloadSizeBuf, sizeof(payloadSizeBuf));
  short payloadSize = short(payloadSizeBuf[0] << 8 | payloadSizeBuf[1]);

  char payload[payloadSize+1] = {0};
  Serial.readBytes(payload, payloadSize);
  payload[payloadSize] = '\0';

  callback(String(event), payload);
}

void loop() {
}