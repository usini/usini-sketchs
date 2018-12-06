void websocketEvent() {
  if (!nowifi) {
    if (!wifi_state) {
      if (WiFiMulti.run() == WL_CONNECTED) {
        wifi_state = true;
        webSocket.begin(carrier, port, "/ws");
        //webSocket.beginSSL(carrier, 80, "/ws");
        webSocket.onEvent(webSocketClient);
      }
    } else {
      if (!disconnected) {
        webSocket.loop();
      }
    }
  }
}

void webSocketClient(WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("Carrier disconnected!\n");
      websocket_state = false;

      break;
    case WStype_CONNECTED:
      {
        websocket_state = true;
        //Serial.printf("Connected to %s\n",  payload);

        //String add_command = "{\"add\": \"" + usb_name + "\"}";
        //webSocket.sendTXT(add_command);
      }
      break;
    case WStype_TEXT:
      readString = (char*) payload;
      Serial.println(readString);
      if (logged) {
        commands();

        if (readString == "@disconnected@") {
          Serial.println("@disconnected@");
        }
      } else {
        String cmd = readString.substring(0, 5);
        Serial.println(cmd);

        if (cmd == "{\"pas") {
          if (password != "") {
            Serial.println(password_state);

            if (password_state) {
              Serial.println("Failed password");
              disconnected = true;
              digitalWrite(LED_BUILTIN, LOW);
              webSocket.disconnect();
            }

            if (!password_state) {
              String passwordCommand = "{\"password\": \"" + password + "\"}";
              webSocket.sendTXT(passwordCommand);
              password_state = true;
            }
          }
        }
        if (cmd == "{\"cha") {
          Serial.println("CHANNEL");
          password_state = false;
          String add_command = "{\"add\": \"" + usb_name + "\"}";
          webSocket.sendTXT(add_command);
        }

        if (cmd == "{\"err") {
          //Channel wasn't add
          id++;
          String add_command = "{\"add\": \"" + usb_name + "/" + id + "\"}";
          webSocket.sendTXT(add_command);
        }

        if (cmd == "{\"act") {
          digitalWrite(LED_BUILTIN, HIGH);
          logged = true;
          Serial.println("ok");
        }
      }
      break;
  }
}
