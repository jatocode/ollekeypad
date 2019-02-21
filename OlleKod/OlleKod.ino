
#include <Keypad.h>
#include <EEPROM.h>

#define GREENPIN 13
#define LARMPIN 12
#define REDPIN 11
#define BLUEPIN 10
#define CONTACTPIN 9

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};
byte rowPins[ROWS] = {5, 2, 3, 4}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {7, 8, 6};    //connect to the column pinouts of the keypad

/*
    Olles keypad
    1 = Oanvänd

    2 = Col 2 -> DI8
    4 = Col 1 -> DI7
    6 = Col 3 -> DI6

    3 = Row 1 -> DI5
    5 = Row 4 -> DI4
    7 = Row 3 -> DI3
    8 = Row 2 -> DI2

    9 = Oanvänd

*/

Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

unsigned long startTime;
String hold;
String code;
String newCode;
String correctCode = "1234";
int codeAddress = 0;

int state = 0;

// Snodde kod från
// https://learn.adafruit.com/multi-tasking-the-arduino-part-1/a-classy-solution
class Flasher
{
    // Class Member Variables
    // These are initialized at startup
    int ledPin;   // the number of the LED pin
    long OnTime;  // milliseconds of on-time
    long OffTime; // milliseconds of off-time

    // These maintain the current state
    int ledState;                 // ledState used to set the LED
    unsigned long previousMillis; // will store last time LED was updated

    // Constructor - creates a Flasher
    // and initializes the member variables and state
  public:
    Flasher(int pin, long on, long off)
    {
        ledPin = pin;
        pinMode(ledPin, OUTPUT);

        OnTime = on;
        OffTime = off;

        ledState = LOW;
        previousMillis = 0;
    }

    void Update()
    {
        // check to see if it's time to change the state of the LED
        unsigned long currentMillis = millis();

        if ((ledState == HIGH) && (currentMillis - previousMillis >= OnTime))
        {
            ledState = LOW;                 // Turn it off
            previousMillis = currentMillis; // Remember the time
            digitalWrite(ledPin, ledState); // Update the actual LED
        }
        else if ((ledState == LOW) && (currentMillis - previousMillis >= OffTime))
        {
            ledState = HIGH;                // turn it on
            previousMillis = currentMillis; // Remember the time
            digitalWrite(ledPin, ledState); // Update the actual LED
        }
    }
};

Flasher blueflash(BLUEPIN, 200, 200);
Flasher greenflash(GREENPIN, 150, 1000);
Flasher redflash(REDPIN, 150, 1000);
Flasher redflashFast(REDPIN, 150, 150);

void saveCode(String correctCode)
{
    byte codeBuf[5];
    correctCode.getBytes(codeBuf, 5);
    EEPROM.write(codeAddress, codeBuf[0]);
    EEPROM.write(codeAddress + 1, codeBuf[1]);
    EEPROM.write(codeAddress + 2, codeBuf[2]);
    EEPROM.write(codeAddress + 3, codeBuf[3]);

    // For testing simulated
    readCode();
}

String readCode()
{
    byte codeBuf[5];
    codeBuf[0] = EEPROM.read(codeAddress);
    codeBuf[1] = EEPROM.read(codeAddress + 1);
    codeBuf[2] = EEPROM.read(codeAddress + 2);
    codeBuf[3] = EEPROM.read(codeAddress + 3);
    codeBuf[4] = 0; // Nollterminera

    if (codeBuf[0] == 255)
    {
        return "";
    }

    String code = String((char *)codeBuf);

    Serial.print("Läste kod från EEPROM: ");
    Serial.println(code);

    return code;
}

void light1s(int pin)
{
    digitalWrite(pin, HIGH);
    delay(1000);
    digitalWrite(pin, LOW);
}

void alarm(double seconds)
{
    digitalWrite(LARMPIN, HIGH);
    delay(seconds * 1000);
    digitalWrite(LARMPIN, LOW);
}

void setup()
{
    Serial.begin(115200);
    hold = "";
    code = "";
    startTime = millis();
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(GREENPIN, OUTPUT);
    pinMode(REDPIN, OUTPUT);
    pinMode(BLUEPIN, OUTPUT);
    pinMode(LARMPIN, OUTPUT);
    pinMode(CONTACTPIN, INPUT_PULLUP);

    delay(200);

    light1s(GREENPIN);
    light1s(REDPIN);
    light1s(BLUEPIN);
    alarm(0.1);

    String startcode = readCode();
    Serial.println(startcode);
    if (startcode != "")
    {
        Serial.println(startcode);
        correctCode = startcode;
    }

    Serial.println("Olles keyboard");
    Serial.println("Tryck din hemliga kod!");
}

bool doorIsOpen(int pin)
{
    if (digitalRead(pin) == HIGH)
    {
        return true;
    }
    return false;
}

void loop()
{
    if ((millis() - startTime) > 1000)
    {
        code = "";
        hold = "";
    }
    if (doorIsOpen(CONTACTPIN))
    {
        blueflash.Update();
    }

    switch (state)
    {
    case 0: // Larmat
        if (doorIsOpen(CONTACTPIN))
        {
            redflashFast.Update();
            digitalWrite(LARMPIN, HIGH);
        }
        else
        {
            redflash.Update();
            digitalWrite(LARMPIN, LOW);
        }
        digitalWrite(GREENPIN, LOW);
        break;
    case 1: // Byter kod
        digitalWrite(BLUEPIN, HIGH);
        break;
    case 2: // Avlarmat
        greenflash.Update();
        digitalWrite(REDPIN, LOW);
        digitalWrite(LARMPIN, LOW);
        break;
    default:
        break;
    }

    if (kpd.getKeys())
    {
        for (int i = 0; i < LIST_MAX; i++) // Scan the whole key list.
        {
            if (kpd.key[i].stateChanged) // Only find keys that have changed state.
            {
                switch (kpd.key[i].kstate)
                { // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
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
    if (code.length() == correctCode.length())
    {
        if (state == 1)
        {
            digitalWrite(BLUEPIN, LOW);
            Serial.print("DU HAR VALT NY KOD: ");
            Serial.println(code);
            correctCode = code;
            saveCode(code);
            state = 0;
        }
        else if (code == correctCode)
        {
            Serial.println("Rätt kod!");

            // Toggle open or not
            if (state == 0)
            {
                Serial.println("Larmar av");
                state = 2;
            }
            else if (state == 2)
            {
                Serial.println("Larmar på");
                state = 0;
            }
        }

        code = "";
    }

    if (hold.length() == 1 && (hold == "#" || hold == "*#"))
    {
        Serial.println("Byt kod. Tryck en siffra i taget");
        hold = "";
        state = 1;
    }
}
