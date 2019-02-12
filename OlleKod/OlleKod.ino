
#include <Keypad.h>

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {5, 2, 3, 4}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {7, 8, 6}; //connect to the column pinouts of the keypad

/*
    Olles keypad
    1 = Oanvänd

    2 = Col 2
    4 = Col 1
    6 = Col 3

    3 = Row 1
    5 = Row 4
    7 = Row 3
    8 = Row 2

    9 = Oanvänd

*/

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

unsigned long startTime;
String hold;
String code;
String newCode;
const String correctCode = "1234";

int state = 0;

void setup() {
  Serial.begin(115200);
  hold = "";
  code = "";
  startTime = millis();

  delay(200);

  Serial.println("Olles keyboard");
  Serial.println("Tryck din hemliga kod!");

}

void loop() {
  if ( (millis() - startTime) > 1000 ) {
    code = "";
    hold = "";
  }
  
  if (kpd.getKeys())
  {
    for (int i = 0; i < LIST_MAX; i++) // Scan the whole key list.
    {
      if ( kpd.key[i].stateChanged )   // Only find keys that have changed state.
      {
        switch (kpd.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
          case PRESSED:
            code += kpd.key[i].kchar;
            startTime = millis();
            break;
          case HOLD:
            hold += kpd.key[i].kchar;
            break;
          case RELEASED:
            break;
          case IDLE:
            break;
        }
      }
    }
  }
  if (code.length() == correctCode.length()) {
    if(state == 1) {
      Serial.println("NY KOD!");
      Serial.println(code);
      correctCode = code;
      state = 0;
    } else if (code == correctCode ) {
      Serial.println("VÄLKOMMEN IN TILL OLLE!");
    } else {
      Serial.println("* * * * * ALARM ALARM ALARM * * * * * *");
    }
    delay(3000);
    code = "";
  }

  if(hold.length() == 2 && (hold == "#*" || hold == "*#")) {
    Serial.println("Byt kod. Tryck en siffra i taget");
    hold = "";
    state = 1;
  }
}
