/*
  Raspberry Pi Pico - Custom Keyboard (11 columns x 3 rows)
  Board: rp2040:rp2040:rpipico (earlephilhower arduino-pico core)
  -----------------------------------------------------------
  Wiring:
    Columns (11) -> GPIO0 to GPIO10  (Row1..Row3 x Col0..Col10)
    Rows    (3)  -> GPIO11 (Row1), GPIO12 (Row2), GPIO13 (Row3)

    ESC   -> one leg on GPIO28, other leg on Row1 (GPIO11)
    MUTE  -> one leg on GPIO16, other leg on Row1 (GPIO11)
    SPACE -> one leg on GPIO27, other leg on Row3 (GPIO13)

    (In short: ESC/MUTE/SPACE are extra "columns" that share the row
    lines with the main matrix, not standalone GND buttons. So they
    are only read while their matching row is being scanned.)

  Layout:
    Row1: Q W E R T Y U I O P  + Backspace
    Row2: A S D F G H J K L ;  + Enter
    Row3: Ctrl Z X C V B N M , . + Shift

  IMPORTANT Arduino IDE settings:
    Tools -> USB Stack -> "Adafruit TinyUSB" (Keyboard needs this on some
    core versions; agar "Pico SDK" me bhi chal jaye to wahi rakho)

  Note: har switch ke saath diode (1N4148) series me lagao, warna
  ghost keypress aa sakte hain jab multiple keys ek saath dabao.
*/

#include <Keyboard.h>

const int NUM_COLS = 11;
const int NUM_ROWS = 3;

// Column pins: GPIO0 -> GPIO10
int colPins[NUM_COLS] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// Row pins: GPIO11, GPIO12, GPIO13
int rowPins[NUM_ROWS] = {11, 12, 13};

// Extra keys that share row lines (act like extra columns)
const int ESC_PIN   = 28;  // shares Row1 (GPIO11)
const int MUTE_PIN  = 16;  // shares Row1 (GPIO11)
const int SPACE_PIN = 27;  // shares Row3 (GPIO13)

// Keymap (row-major, 11 keys per row)
uint8_t keymap[NUM_ROWS][NUM_COLS] = {
  { 'q','w','e','r','t','y','u','i','o','p', KEY_BACKSPACE },
  { 'a','s','d','f','g','h','j','k','l',';', KEY_RETURN    },
  { KEY_LEFT_CTRL,'z','x','c','v','b','n','m',',','.', KEY_LEFT_SHIFT }
};

// Track previous state to send press/release only once
bool keyState[NUM_ROWS][NUM_COLS] = {false};
bool escState   = false;
bool muteState  = false;
bool spaceState = false;

void setup() {
  // Rows as OUTPUT, default HIGH (idle)
  for (int r = 0; r < NUM_ROWS; r++) {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], HIGH);
  }

  // Columns as INPUT_PULLUP
  for (int c = 0; c < NUM_COLS; c++) {
    pinMode(colPins[c], INPUT_PULLUP);
  }

  // Extra keys as INPUT_PULLUP
  pinMode(ESC_PIN, INPUT_PULLUP);
  pinMode(MUTE_PIN, INPUT_PULLUP);
  pinMode(SPACE_PIN, INPUT_PULLUP);

  Keyboard.begin();
}

void loop() {
  scanMatrix();
  delay(5); // simple debounce
}

void scanMatrix() {
  for (int r = 0; r < NUM_ROWS; r++) {
    // Drive only this row LOW, rest stay HIGH
    digitalWrite(rowPins[r], LOW);
    delayMicroseconds(20); // let signal settle

    for (int c = 0; c < NUM_COLS; c++) {
      bool pressed = (digitalRead(colPins[c]) == LOW);

      if (pressed && !keyState[r][c]) {
        keyState[r][c] = true;
        Keyboard.press(keymap[r][c]);
      } else if (!pressed && keyState[r][c]) {
        keyState[r][c] = false;
        Keyboard.release(keymap[r][c]);
      }
    }

    // ESC and MUTE share Row1 (r == 0, GPIO11)
    if (r == 0) {
      bool escPressed  = (digitalRead(ESC_PIN) == LOW);
      bool mutePressed = (digitalRead(MUTE_PIN) == LOW);

      if (escPressed && !escState) {
        escState = true;
        Keyboard.press(KEY_ESC);
      } else if (!escPressed && escState) {
        escState = false;
        Keyboard.release(KEY_ESC);
      }

      if (mutePressed && !muteState) {
        muteState = true;
        Keyboard.consumerPress(KEY_MUTE);
      } else if (!mutePressed && muteState) {
        muteState = false;
        Keyboard.consumerRelease();
      }
    }

    // SPACE shares Row3 (r == 2, GPIO13)
    if (r == 2) {
      bool spacePressed = (digitalRead(SPACE_PIN) == LOW);

      if (spacePressed && !spaceState) {
        spaceState = true;
        Keyboard.press(' ');
      } else if (!spacePressed && spaceState) {
        spaceState = false;
        Keyboard.release(' ');
      }
    }

    digitalWrite(rowPins[r], HIGH);
  }
}
