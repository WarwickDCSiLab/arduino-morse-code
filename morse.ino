#include <stdlib.h>
#include <stdio.h>
#include <math.h>

//Library that creates a better noise from buzzer
#include <toneAC.h>

/* PIN LAYOUT
 * 2  -> Shift register serial in
 * 3  -> Shift register memory latch
 * 4  -> Shift register clock
 * 9  -> Buzzer positive (or Pot positive)
 * 10 -> Buzzer negative
 * 13 -> LED positve (no resistor as pin 13 has builtin)
 * A7 -> Pot in (Pot also connected to +5V and GND)
 *
 * INPUT BYTE (LSB) ->
 * A0, A1, A2, A3, D8, D7, D6, D5
 *
 * SHIFT REGISTER
 * Displays current value for processing
 * 8 bit parallel out to 220Ω resistors and LEDs
 * Example setup shown at Arduino ShiftOut tutorial
 */


//Shift input value to LEDs
const int LATCH_PIN = 3;
const int CLOCK_PIN = 4;
const int DATA_PIN = 2;

const int LED_PIN = 13;
const int SPEED_PIN = A7;
const int BEEP_FREQ = 698;


//Morse code timing constants
//Time differences come from the international standard
volatile int FRAME_LENGTH = 200;
const int MAX_FRAME_LENGTH = 500;
const int MIN_FRAME_LENGTH = 10;
#define DOT_TIME FRAME_LENGTH
#define DASH_TIME DOT_TIME*3
#define INNER_SEP_TIME DOT_TIME*1
#define LETTER_SEP_TIME DOT_TIME*3
#define WORD_SEP_TIME DOT_TIME*7

//Morse symbol values
const int term = 0;
const int dit = 1;
const int dah = 2;

//Morse letter definitions
const int A[] = { dit, dah, term };
const int B[] = { dah, dit, dit, dit, term };
const int C[] = { dah, dit, dah, dit, term };
const int D[] = { dah, dit, dit, term };
const int E[] = { dit, term };
const int F[] = { dit, dit, dah, dit, term };
const int G[] = { dah, dah, dit, term };
const int H[] = { dit, dit, dit, dit, term };
const int I[] = { dit, dit, term };
const int J[] = { dit, dah, dah, dah, term };
const int K[] = { dah, dit, dah, term };
const int L[] = { dit, dah, dit, dit, term };
const int M[] = { dah, dah, term };
const int N[] = { dah, dit, term };
const int O[] = { dah, dah, dah, term };
const int P[] = { dit, dah, dah, dit, term };
const int Q[] = { dah, dah, dit, dah, term };
const int R[] = { dit, dah, dit, term };
const int S[] = { dit, dit, dit, term };
const int T[] = { dah, term };
const int U[] = { dit, dit, dah, term };
const int V[] = { dit, dit, dit, dah, term };
const int W[] = { dit, dah, dah, term };
const int X[] = { dah, dit, dit, dah, term };
const int Y[] = { dah, dit, dah, dah, term };
const int Z[] = { dah, dah, dit, dit, term };

const int* morse_code[] =
{
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z
};

const char* numbers[] =
{
    "ZERO",
    "ONE",
    "TWO",
    "THREE",
    "FOUR",
    "FIVE",
    "SIX",
    "SEVEN",
    "EIGHT",
    "NINE"
};

static void checkSpeed()
{
    int raw = analogRead(SPEED_PIN);
    FRAME_LENGTH = map(raw, 0, 1024, MIN_FRAME_LENGTH, MAX_FRAME_LENGTH);
}


//Outputs a single letter of a word from array constants
static void blinkLetter(const int* morse)
{
    int i = 0;
    while(morse[i] != term)
    {
        checkSpeed();
        switch(morse[i]){
            case dit:
                Serial.print(".");
                toneAC(BEEP_FREQ);
                digitalWrite(LED_PIN, HIGH);
                delay(DOT_TIME);
                digitalWrite(LED_PIN, LOW);
                noToneAC();
                delay(INNER_SEP_TIME);
                break;
            case dah:
                Serial.print("-");
                toneAC(BEEP_FREQ);
                digitalWrite(LED_PIN, HIGH);
                delay(DASH_TIME);
                digitalWrite(LED_PIN, LOW);
                noToneAC();
                delay(INNER_SEP_TIME);
                break;
        }
        i++;
    }
    Serial.print(" ");
    checkSpeed();
    delay(LETTER_SEP_TIME);
}

//Convert single digit to Morse Code
static void blinkDigit(int n)
{
    String letters = String(numbers[n]);
    Serial.println(letters);
    for(int i = 0; i < letters.length(); i++)
    {
        blinkLetter(morse_code[letters[i]-'A']);
    }
    Serial.println("");
    checkSpeed();
    delay(WORD_SEP_TIME);
}

//Convert int to Morse Code (0-255)
static void blinkInt(int value)
{
    String digits = String(value);
    for(int i = 0; i < digits.length(); i++)
    {
        blinkDigit(digits[i]-'0');
    }
}

//Shift out value to LEDs
static void displayValue(byte value)
{
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, value);
    digitalWrite(LATCH_PIN, HIGH);
}

//Uses pins A0-A3 and D8-D4 for external digital input
//ACTIVE LOW
static byte readValue()
{
    byte dataIn = 0;
    for(int i = 0; i < 4; i++)
    {
        if(!digitalRead(A0 + i)) dataIn |= (1 << i);
    }
    for(int i = 4; i < 8; i++)
    {
        if(!digitalRead(12 - i)) dataIn |= (1 << i);
    }
    return dataIn;
}

//Arduino setup function (called before loops)
void setup()
{
    //Setup pins
    pinMode(LED_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(DATA_PIN, OUTPUT);

    //Use active low for input due to noise when not connected to ground
    for(int i = A0; i < A4; i++) pinMode(i, INPUT_PULLUP);
    for(int i = 5; i < 9; i++) pinMode(i, INPUT_PULLUP);

    //Open serial console at 9600 Bauds
    Serial.begin(9600);
    Serial.println("Morse Encoder\n-------------\n");
}

//Arduino main loop
void loop()
{
    //Flash value LEDs twice
    for(int i = 0; i < 2; i++)
    {
        displayValue(255);
        delay(100);
        displayValue(0);
        delay(100);
    }

    //Read input value from pins
    Serial.println("Reading input values...");
    byte value = readValue();
    Serial.print("Initial Value: ");
    Serial.println(value);
    displayValue(value);

    //Convert and output morse code
    blinkInt(value);

    Serial.println("Finished\n");
    checkSpeed();
    delay(FRAME_LENGTH*5);
}

