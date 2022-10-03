unsigned long deviceStartTime =  millis();


void waitForConfig(int seconds) {
  // wait for commands on serial
  unsigned long  configWaitTime = millis();
  int counter = seconds;

  while (Serial.available() == 0) {
    if ((millis() - deviceStartTime) > seconds * 1000) {
      // config time over
      Serial.println("Config time expired");
      return;
    }
    if (millis() - configWaitTime > 1000) {
      logo(--counter);
      configWaitTime = millis();
      Serial.println("configWaitTime: " + String(configWaitTime));
    }
  }
  processing("Open for config");
  executeConfig();
}

void executeConfig() {
  while (true) {
    if (Serial.available() == 0) continue;

    String data = Serial.readStringUntil('\n');
    Serial.println("received: " + data);
    KeyValue kv = extractKeyValue(data);
    String commandName = kv.key;
    if (commandName == "/config-done") {
      Serial.println("/config-done");
      return;
    }
    executeCommand(commandName, kv.value);
  }
}

void executeCommand(String commandName, String commandData) {
  Serial.println("executeCommand: " + commandName + " > " + commandData);
  KeyValue kv = extractKeyValue(commandData);
  String path = kv.key;
  String data = kv.value;

  Serial.println("check point 1 ");
  Serial.println("check point 2 " + commandName);
  Serial.println("check point 3 " + commandData);

  if (commandName == "/file-remove") {
    return removeFile(path);
  }
  if (commandName == "/file-append") {
    return appendToFile(path, data);
  }

  if (commandName == "/file-read") {
    Serial.println("prepare to read");
    readFile(path);
    Serial.println("readFile done");
    return;
  }
  if (commandName == "/file-close") {
    return closeFile(path);
  }
  Serial.println("command unknown");
}

void removeFile(String path) {
  Serial.println("removeFile: " + path);
  SPIFFS.remove("/" + path);
}

void appendToFile(String path, String data) {
  Serial.println("appendToFile: " + path);
  File file = SPIFFS.open("/" + path, FILE_APPEND);
  if (!file) {
    file = SPIFFS.open("/" + path, FILE_WRITE);
  }
  if (file) {
    file.println(data);
    file.close();
  }
}

void readFile(String path) {
  Serial.println("readFile: " + path);
  File file = SPIFFS.open("/" + path);
  if (file) {
    while (file.available()) {
      String line = file.readStringUntil('\n');
      Serial.print("/file-read" + line);
    }
    file.close();
  }
  Serial.println("");
  Serial.println("/file-done");
}

void closeFile(String path) {
  Serial.println("closeFile: " + path);
}


KeyValue extractKeyValue(String s) {
  int spacePos = s.indexOf(" ");
  String key = s.substring(0, spacePos);
  if (spacePos == -1) {
    return {key, ""};
  }
  String value = s.substring(spacePos + 1, s.length());
  return {key, value};
}