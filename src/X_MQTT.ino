////////////////////////////////////////////////////////////////////////
//  Matthew Kavanagh
//
//  Nest
//  MQTT.ino
//  2017
//  *********
////////////////////////////////////////////////////////////////////////
//
//  #     #  #####  ####### #######
//  ##   ## #     #    #       #
//  # # # # #     #    #       #
//  #  #  # #     #    #       #
//  #     # #   # #    #       #
//  #     # #    #     #       #
//  #     #  #### #    #       #
//
///////////////////////////////////////////////////////////////////////
void startMQTT() {
  mqtt.setServer(mqttServerIP, 1883);
  mqtt.setCallback(messageReceived);
}

void handleMQTT() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!mqtt.connected()) {
      long now = millis();
      if (now - lastMQTTReconnectAttempt > connectionTimeout) {
        lastMQTTReconnectAttempt = now;

        if (mqtt.connect(nodeName, nodeName, 1, 0, disconnectMsg)) {  // Attempt to connect using a login
          Serial << "| MQTT connection established |" << endl;        // Dont publish here, causes crashes
          subscribeToTopics();
          digitalWrite(connectionLED, OFF);
        }

        else {  // Not connected
          Serial << "| MQTT connection failed, rc=" << mqtt.state() << " Trying again |" << endl;

          digitalWrite(connectionLED, ON);
          delay(250);
          digitalWrite(connectionLED, OFF);
        }
      }
    } else {
      mqtt.loop();
    }
  }
}

void messageReceived(char* topic, byte* payload, unsigned int length) {
  printMessage(payload, length);
  char state = (char)payload[0];

  if (length > 1)  // Colours
  {
    mode = 9;
    StaticJsonDocument<256> doc;
    deserializeJson(doc, payload, length);

    red = doc["red"];
    green = doc["green"];
    blue = doc["blue"];

    Serial << "Red :" << red << " Green :" << green << " Blue: " << blue << endl;

    for (int i = 0; i < totalLEDs; i++) {
      currentLED[i].setRGB(red, green, blue);
    }
    FastLED.show();

    publishAll();
  } else {
    mode = (int)payload[0] - 48;  // Stupid ascii I think
    publishAll();
  }

  // Serial << mode << endl;
  // Wire.beginTransmission(1);
  // // Wire.write((char)payload[0]);
  // Wire.write((char)payload[0]);
  // Wire.endTransmission();
}

void printMessage(byte* payload, int length) {
  for (int i = 0; i < length; i++) {
    Serial << (char)payload[i];  // No endl here
  }
  Serial << endl;
}

////////////////////////////////////////////////////////////////////////
//
//   #####
//  #     # #    # #####   ####   ####  #####  # #####  ######
//  #       #    # #    # #      #    # #    # # #    # #
//   #####  #    # #####   ####  #      #    # # #####  #####
//        # #    # #    #      # #      #####  # #    # #
//  #     # #    # #    # #    # #    # #   #  # #    # #
//   #####   ####  #####   ####   ####  #    # # #####  ######
//
////////////////////////////////////////////////////////////////////////
void subscribeToTopics() {
  mqtt.subscribe("Hexagon Lights Control");
}

////////////////////////////////////////////////////////////////////////
//
//  ######
//  #     # #    # #####  #      #  ####  #    #
//  #     # #    # #    # #      # #      #    #
//  ######  #    # #####  #      #  ####  ######
//  #       #    # #    # #      #      # #    #
//  #       #    # #    # #      # #    # #    #
//  #        ####  #####  ###### #  ####  #    #
//
////////////////////////////////////////////////////////////////////////
void publishAll() {
  const size_t capacity = JSON_OBJECT_SIZE(6);
  DynamicJsonDocument doc(capacity);

  doc["Node"] = nodeName;
  doc["red"] = red;
  doc["green"] = green;
  doc["blue"] = blue;
  doc["mode"] = mode;

  char buffer[512];

  size_t n = serializeJson(doc, buffer);

  Serial << buffer << endl;

  mqtt.publish(nodeName, buffer, n);
}