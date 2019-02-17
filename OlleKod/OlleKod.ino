
#include <Keypad.h>

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
byte colPins[COLS] = {7, 8, 6};	//connect to the column pinouts of the keypad

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

int state = 0;

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

	Serial.println("Olles keyboard");
	Serial.println("Tryck din hemliga kod!");
}

void loop()
{
	if ((millis() - startTime) > 1000)
	{
		code = "";
		hold = "";
	}
	switch (state)
	{
	case 0: // Larmat
		digitalWrite(LED_BUILTIN, LOW);
		digitalWrite(GREENPIN, LOW);
		digitalWrite(REDPIN, HIGH);
		if (digitalRead(CONTACTPIN) == HIGH)
		{
			digitalWrite(BLUEPIN, HIGH);
			digitalWrite(LARMPIN, HIGH);

			delay(50);
			digitalWrite(BLUEPIN, LOW);
			digitalWrite(LARMPIN, LOW);
		}
		break;
	case 1: // Byter kod
		digitalWrite(LED_BUILTIN, HIGH);
		break;
	case 2: // Avlarmat
		digitalWrite(LED_BUILTIN, LOW);
		digitalWrite(GREENPIN, HIGH);
		digitalWrite(REDPIN, LOW);
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
			Serial.print("DU HAR VALT NY KOD: ");
			Serial.println(code);
			correctCode = code;
			state = 0;
		}
		else if (code == correctCode)
		{
			Serial.println("VÄLKOMMEN IN TILL OLLE!");
			digitalWrite(LED_BUILTIN, HIGH);
			delay(3000);
			digitalWrite(LED_BUILTIN, LOW);

			// Toggle open or not
			if (state == 0)
			{
				state = 2;
			}
			else if (state == 2)
			{
				state = 0;
			}
			Serial.println(state);
		}

		code = "";
	}

	if (hold.length() == 2 && (hold == "#*" || hold == "*#"))
	{
		Serial.println("Byt kod. Tryck en siffra i taget");
		hold = "";
		state = 1;
	}
}
