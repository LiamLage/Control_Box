#include <LiquidCrystal.h>
LiquidCrystal lcd(6, 7, 2, 3, 4, 5);

#include <DHT.h>
#define DHTPIN 17
#define DHTTYPE DHT22
DHT dht (DHTPIN, DHTTYPE);

//Input & Button Logic
const int numOfInputs = 4;
const int inputPins[numOfInputs] = {8, 9, 10, 13};
int inputState[numOfInputs];
int lastInputState[numOfInputs] = {LOW, LOW, LOW, LOW};
bool inputFlags[numOfInputs] = {LOW, LOW, LOW, LOW};
long lastDebounceTime[numOfInputs] = {0, 0, 0, 0};
long debounceDelay = 5;

// Output Logic
const int numOfOutputs = 4;               // Define number of outouts here
const int outputPins[numOfOutputs] = {12, 17, 14, 15}; // Define Output pins here
int outputState[numOfOutputs];
int lastOutputState[numOfOutputs] = {HIGH, HIGH, HIGH, HIGH};
bool outputFlags[numOfOutputs] = {HIGH, HIGH, HIGH, HIGH};

//LCD Menu Logic
const int numOfScreens = 3;
int currentScreen = 0;
String screens[numOfScreens][2] = {{"Temperature", "C"}, {"Humidity", "%"},
  {"Parameters", "unit"}
};
int parameters[numOfScreens];

unsigned long previousMillis = 0;
const long interval = 500;

byte customChar[8] = {
  0b00110,
  0b01001,
  0b01001,
  0b00110,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

void setup() {
  pinMode(18, OUTPUT);
  pinMode(19, OUTPUT);
  pinMode(11, OUTPUT);
  digitalWrite(11, HIGH);
  digitalWrite(18, LOW);
  digitalWrite(19, LOW);

  for (int i = 0; i < numOfInputs; i++) {
    pinMode(inputPins[i], INPUT_PULLUP);

  }
  for (int i = 0; i < numOfOutputs; i++) {
    pinMode(outputPins[i], OUTPUT);

  }


  Serial.begin(9600);
  dht.begin();
  lcd.begin(16, 2);
  lcd.createChar(0, customChar);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Liam Lage");
  lcd.setCursor(0, 1);
  lcd.print("ControlBox2000");
  delay(1000);

}

void loop() {
  setInputFlags();
  resolveInputFlags();
  resolveOutputs();
  printScreen();
  resolveParameters();

}

void setInputFlags() {
  for (int i = 0; i < numOfInputs; i++) {
    int reading = digitalRead(inputPins[i]);
    if (reading != lastInputState[i]) {
      lastDebounceTime[i] = millis();
    }
    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      if (reading != inputState[i]) {
        inputState[i] = reading;
        if (inputState[i] == HIGH) {
          inputFlags[i] = HIGH;
        }
      }
    }
    lastInputState[i] = reading;
  }
}

void resolveInputFlags() {
  for (int i = 0; i < numOfInputs; i++) {
    if (inputFlags[i] == HIGH) {
      inputAction(i);
      inputFlags[i] = LOW;
      //      printScreen();
    }
  }
}

void inputAction(int input) {
  if (input == 0) {
    if (currentScreen == 0) {
      currentScreen = numOfScreens - 1;
    } else {
      currentScreen--;
    }
  } else if (input == 1) {
    if (currentScreen == numOfScreens - 1) {
      currentScreen = 0;
    } else {
      currentScreen++;
    }
  } else if (input == 2) {
    parameterChange(0);
  } else if (input == 3) {
    parameterChange(1);
  }
}

void parameterChange(int key) {
  if (key == 0) {
    parameters[currentScreen]++;
  } else if (key == 1) {
    parameters[currentScreen]--;
  }
}

void printScreen() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    float sensor[numOfScreens][2] = {{dht.readTemperature()}, {dht.readHumidity()}};

    if (currentScreen < numOfScreens - 1) {
      lcd.clear();
      lcd.print(screens[currentScreen][0]);
      lcd.setCursor(0, 1);
      lcd.print(parameters[currentScreen]);
      lcd.print(char(47));
      lcd.print(sensor[currentScreen][0]);
      lcd.print(" ");
      lcd.print(screens[currentScreen][1]);
    }
    else {
      float t = dht.readTemperature();
      float h = dht.readHumidity();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("T: ");
      lcd.print(parameters[0]);
      lcd.print(char(47));
      lcd.print(t);
      lcd.write((uint8_t)0);
      lcd.print("C");
      lcd.setCursor(0, 1);
      lcd.print("H: ");
      lcd.print(parameters[1]);
      lcd.print(char(47));
      lcd.print(h);
      lcd.print("%");
    }
  }
}

void resolveOutputs () {
  //unsigned long currentMillis = millis();
  //if (currentMillis - previousMillis >= interval) {
  //  previousMillis = currentMillis;

  // Temp & Humidity Variables
  float t = dht.readTemperature();
  float h = dht.readHumidity();


  if (t > (parameters[0] + 1)) {            // If current temp is more than set temp
    //    Serial.println("Compressor ON");
    digitalWrite(outputPins[1], LOW); // Turn compressor on
    digitalWrite(outputPins[0], HIGH); // Turn heater off

  }

  else if (t < parameters[0] - 1) { // If current temp is less than set temp
    //    Serial.println("Heater ON");
    digitalWrite(outputPins[0], LOW); // Turn heater on
    digitalWrite(outputPins[1], HIGH); // Turn compressor off

  }

  else {
    ((t - 1) < parameters[0] < (t + 1)); // If t == (t_set +- 1)
    //    Serial.println("Temperature Stable");
    digitalWrite(outputPins[0], HIGH);
    digitalWrite(outputPins[1], HIGH);

  }

  //Humidity Control Loop
  if (h > (parameters[1] + 1)) {            // If current humidity is more than set humidity
    //    Serial.println("Fan On");
    digitalWrite(outputPins[2], LOW); // Turn fan on
    digitalWrite(outputPins[3], HIGH); // Turn humidifier off

  }

  else if (h < parameters[1] - 1) { // If current humidity is less than set humidity
    //    Serial.println("Humidifier ON");
    digitalWrite(outputPins[3], LOW); // Turn humidifier on
    digitalWrite(outputPins[2], HIGH); // Turn fan off

  }

  else {
    ((h - 1) < parameters[1] < (h + 1)); // If h == (h_set +- 1)
    //    Serial.println("Humidity Stable");
    digitalWrite(outputPins[2], HIGH);
    digitalWrite(outputPins[3], HIGH);

  }
}

void resolveParameters() {
  int i = 0;
  if (i <= numOfScreens){
    parameters[0] = max(parameters[0], -20);
    i--;
    parameters[0] = min(parameters[0], 100);
    i++;
  }
  else (i = numOfScreens - 2); {  
    parameters[1] = max(parameters[1], 0);
    i--;
    parameters[1] = min(parameters[1], 100);
    i++;
  }  
}
