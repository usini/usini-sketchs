
void websocketEvent() {
  if (!nowifi) {
    if (!wifi_state) {
      if (WiFiMulti.run() == WL_CONNECTED) {
        wifi_state = true;
        
        if(ssl){
          webSocketClient.beginSSL(carrier, port, "/ws");
        } else {
          webSocketClient.begin(carrier, port, "/ws");
        }
        webSocketClient.onEvent(webSocketClientEvent);
      }
    } else {
      if (!disconnected) {
        webSocketClient.loop();
      }
    }
  }
}

void webSocketClientEvent(WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("Carrier disconnected!\n");
      websocket_state = false;

      break;
    case WStype_CONNECTED:
      {
        websocket_state = true;
        //Serial.printf("Connected to %s\n",  payload);


      }
      break;
    case WStype_TEXT:
      readString = (char*) payload;
      //Serial.println(readString);
      
      // LOGGED
      if (logged) {
        if (readString == "@disconnected@") {
          Serial.println("@disconnected@");
        }
      } else {
        // NOT LOGGED
        String cmd = readString.substring(0, 5);
        String cmd2 = readString.substring(0, 25);
        //Serial.println(cmd);

        if (cmd == "{\"pas") {
          if (password != "") {
            //Serial.println(password_state);

            if (password_state) {
              Serial.println("... Error: Wrong Password");
              disconnected = true;
              webSocketClient.disconnect();
            }

            if (!password_state) {
              String passwordCommand = "{\"password\": \"" + password + "\"}";
              webSocketClient.sendTXT(passwordCommand);
              password_state = true;
            }
          }
        }
        if (cmd == "{\"cha") {
          Serial.println("... OK: Connected to libreCarrier");
          password_state = false;
          //String add_command = "{\"connect\": \"" + NAME + "\"}";
          String add_command = "{\"add\": \"" + NAME + "\"}";
          webSocketClient.sendTXT(add_command);
        }

        if (cmd == "{\"err") {
          //Channel wasn't add
          id++;
          //String add_command = "{\"connect\": \"" + NAME + "/" + id + "\"}";
          String add_command = "{\"connect\": \"" + NAME + "\"}";
          webSocketClient.sendTXT(add_command);
        }

        if (cmd2 == "{\"action\": \"connect\", \"er"){
          String add_command = "{\"connect\": \"" + NAME + "\"}";
          webSocketClient.sendTXT(add_command);
        }

        if (cmd2 == "{\"action\": \"connect\", \"ok"){
          logged = true;
          Serial.println("... OK: Channel created");
        }
      }
      break;
  }
}
