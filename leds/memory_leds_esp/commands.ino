void commands() {
  if (readString[0] == '/') {
    if (readString == "/info") {
      multiprint(usb_name);
    }

    if (readString == "/wifi") {
      if(wifi_state){
        multiprint("no");
      } else {
        multiprint("yes");
      }
    }

    if (readString == "/carrier") {

    }

    //Get status
    if (readString == "/status") {
      String buf = "";
      buf = brightness;
      for (int led = 0; led < NUM_LEDS; led++) {
        buf = buf + ";" + hue[led] + ";" + sat[led] + ";" + val[led] + ";" + anim[led] + ";" + params1[led] + ";" + params2[led];
      }
      multiprint(buf);
    }


    if (readString == "/save") {
      save_eeprom();
      multiprint("save");
    }

    //Turn off all leds
    if (readString == "/off") {
      change_color_all(0, 0, 0);
      change_anim_all(0, 0, 0);
      multiprint("off");
    }

    if (readString == "/on") {
      change_color_all(0, 0, 255);
      change_anim_all(0, 0, 255);
      multiprint("on");
    }

    if (readString == "/red") {
      change_color_all(0, 255, 255);
      change_anim_all(0, 255, 255);
      multiprint("red");
    }

    if (readString == "/sync") {
      for (int led = 0; led < NUM_LEDS; led++) {
        val[led] = 255;
      }
      multiprint("sync");
    }

    if (readString == "/reset") {
      read_eeprom();
    }

    if (readString == "/clear") {
      change_color_all(0, 0, 0);
      change_anim_all(0, 0, 0);
      LEDS.setBrightness(255);
      save_eeprom();
      multiprint("clear");
    }
  } else {
    String command = splitString(readString, ';', 0);

    //Change brightness
    if (command == "brightness") {
      brightness = splitString(readString, ';', 1).toInt();
      LEDS.setBrightness(brightness);
      multiprint(readString);
    }

    //Change color
    if (command == "color") {
      int led = splitString(readString, ';', 1).toInt();
      int h = splitString(readString, ';', 2).toInt();
      int s = splitString(readString, ';', 3).toInt();
      int v = splitString(readString, ';', 4).toInt();
      change_color(led, h, s, v);
      //If we turn off led we assume we want to stop the animation too.
      if ( (h == 0) && (s == 0) && (v == 0) ) {
        change_anim(led, 0, 0, 0);
      }
      multiprint(readString);
    }


    //Change animation
    if (command == "animation") {
      int led = splitString(readString, ';', 1).toInt();
      int animation = splitString(readString, ';', 2).toInt();
      int parameter1 = splitString(readString, ';', 3).toInt();
      int parameter2 = splitString(readString, ';', 4).toInt();
      change_anim(led, animation, parameter1, parameter2);
      //Start Rainbow animation with red
      if (animation == 3) {
        change_color(led, 0, 255, 255);
      }
      multiprint(readString);
    }

    //Load previous state
    if (command == "load") {
      int cursor = 1;
      brightness = splitString(readString, ';', cursor++).toInt();

      for (int led = 0; led < NUM_LEDS; led++) {
        int h = splitString(readString, ';', cursor).toInt();
        cursor++;
        int s = splitString(readString, ';', cursor).toInt();
        cursor++;
        int v = splitString(readString, ';', cursor).toInt();
        cursor++;
        int animation = splitString(readString, ';', cursor).toInt();
        cursor++;
        int parameter1 = splitString(readString, ';', cursor).toInt();
        cursor++;
        int parameter2 = splitString(readString, ';', cursor).toInt();
        cursor++;
        change_color(led, h, s, v);
        change_anim(led, animation, parameter1, parameter2);
      }
      multiprint(readString);
    }
  }
}
