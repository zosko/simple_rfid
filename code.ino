#include <EEPROM.h>

//PINS
int LAST_CARD_USED = 100;
int BUTTON_CLONE_CARD = 4;
int POWER_READER = 7;
int COIL_PIN = 9;

//EMULATION
uint32_t card_generated;

//RDM6300
bool id_is_valid = false;
unsigned long long current_id;
unsigned char received_checksum;

// EEPROM FUNCTIONS
void EEPROMWritelong(int address, uint64_t value) {
  for (int i = address, j = 0; i < address + 10; i++, j++) {
    byte b = (value >> 36 - j * 4) & 0xFF;
    EEPROM.write(i, b);
  }
}
uint64_t EEPROMReadlong(int address) {
  uint64_t card = 0;
  for (int i = address, b = 0; i < address + 10; i++, b++) {
    uint64_t tmp = EEPROM.read(i);
    tmp = tmp << (36 - b * 4);
    card |= tmp;
  }
  return card;
}

// EMULATE FUNCTIONS
void set_pin_manchester(int clock_half, int signal) {
  int man_encoded = clock_half ^ signal;
  if (man_encoded == 1) {
    digitalWrite(COIL_PIN, LOW);
  }
  else {
    digitalWrite(COIL_PIN, HIGH);
  }
}
uint32_t generate_card(uint64_t Hex_IDCard) {
  static uint32_t data_card[64];
  static uint32_t card_id[10];

  for (int i = 0; i < 10; i++) card_id[i] = (Hex_IDCard >> 36 - i * 4 ) & 0xF;

  for (int i = 0; i < 9; i++) data_card[i] = 1;

  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 4; j++) data_card[9 + i * 5 + j] = card_id[i] >> (3 - j) & 1;
    data_card[9 + i * 5 + 4] = ( data_card[9 + i * 5 + 0] + data_card[9 + i * 5 + 1] + data_card[9 + i * 5 + 2] + data_card[9 + i * 5 + 3]) % 2;
  }

  for (int i = 0; i < 4; i++) {
    int checksum = 0;
    for (int j = 0; j < 10; j++) checksum += data_card[9 + i + j * 5];
    data_card[i + 59] = checksum % 2;
  }
  data_card[63] = 0;

  return data_card;
}
void emulateCard(uint32_t *data) {
  for (int i = 0; i < 64; i++) {
    set_pin_manchester(0, data[i]);
    delayMicroseconds(255);

    set_pin_manchester(1, data[i]);
    delayMicroseconds(255);
  }
}

// HELPER FUNCTIONS
void blinkEnterMenu() {
  RXLED0; TXLED0;
  delay(500);
  for (int i = 0 ; i < 10; i++) {
    delay(200);
    RXLED1; TXLED0;
    delay(200);
    RXLED0; TXLED1;
  }
  delay(500);
  RXLED0; TXLED0;
}
void cardSelected(int card) {
  RXLED0; TXLED0;
  delay(500);
  for (int i = 0 ; i < card; i++) {
    delay(500);
    RXLED1; TXLED1;
    delay(500);
    RXLED0; TXLED0;
  }
  delay(500);
  RXLED0; TXLED0;
}
void cloneCard(int cardLocation) {
  delay(1000); RXLED1;

  digitalWrite(POWER_READER, HIGH);

  while (0000000000 != readCard()) {
    EEPROMWritelong(cardLocation * 10, readCard());
    print_int64("CARD CLONED: ", readCard());
    break;
  }

  digitalWrite(POWER_READER, LOW);

  RXLED0;
}

// RDM6300 FUNCTIONS
unsigned long long readCard() {
  while (!id_is_valid) {
    while (Serial1.available() > 0) {
      parse(Serial1.read());
    }
  }
  id_is_valid = false;
  return current_id;
}
bool parse(char d) {
  static int state = 0;
  unsigned long long val = 0;
  unsigned long long desl = 0;

  id_is_valid = false;

  switch (state) {
    case 0:
      if (d == 0x02) {
        current_id = 0;
        received_checksum = 0;
        state = 1;
      }
      // else => garbage!
      break;
    case 11: //1st byte of checksum
      val = get_val(d);
      received_checksum = val << 4;
      state++;
      break;
    case 12: //2nd byte of checksum
      val = get_val(d);
      received_checksum |= val;
      state++;
      break;
    case 13:
      if (d == 0x03 && received_checksum == get_checksum(current_id)) {
        id_is_valid = true;
      }
      state = 0;
      break;
    default:
      val = get_val(d);
      desl = (10 - state) * 4;
      current_id |= val << desl;
      state++;
      break;
  }

  return id_is_valid;
}
int get_val(char c) {
  static const char ascii_diff = 48;
  c -= ascii_diff;
  if (c > 9) c -= 7;
  return c;
}

int get_checksum(unsigned long long data) {
  union {
    unsigned char uc[8];
    unsigned long long ul;
  } tmp;
  tmp.ul = data;
  return tmp.uc[0] ^ tmp.uc[1] ^ tmp.uc[2] ^ tmp.uc[3] ^ tmp.uc[4];
}
void print_int64(char *text, unsigned long long data) {
  Serial.print(text);
  union {
    unsigned long long ull;
    unsigned long ul[2];
  } tmp;
  tmp.ull = data;
  Serial.print(tmp.ul[1], HEX);
  unsigned long beacon = 0x10000000;
  while (beacon > 0) {
    if (tmp.ul[0] < beacon)
      Serial.print("0");
    else
      break;
    beacon >>= 4;
  }
  Serial.println(tmp.ul[0], HEX);
}

//MAIN CODE
void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);

  pinMode(BUTTON_CLONE_CARD, INPUT_PULLUP);
  pinMode(POWER_READER, OUTPUT);
  pinMode(COIL_PIN, OUTPUT);

  digitalWrite(COIL_PIN, LOW);

  bool buttonMenuPressed = digitalRead(BUTTON_CLONE_CARD) == LOW;
  if (buttonMenuPressed) {
    bool inMenu = true;
    float pressLength = 0;
    int card_selected = 1;
    blinkEnterMenu();
    delay(3000);
    while (inMenu) {
      while (digitalRead(BUTTON_CLONE_CARD) == LOW) {
        delay(100);
        pressLength = pressLength + 100;
        Serial.print("PRESSED LENGTH: ");
        Serial.println(pressLength);
      }

      if (pressLength >= 1000) { // clone card
        Serial.println("PREPARE TO CLONE CARD");
        cloneCard(card_selected);
        inMenu = false;
      }
      else if (pressLength >= 200 && pressLength <= 1000) {
        if (card_selected++ > 4) {
          card_selected = 1;
        }
        cardSelected(card_selected);
        Serial.print("CARD SELECTED: ");
        Serial.println(card_selected);
        EEPROMWritelong(LAST_CARD_USED, card_selected);
      }

      pressLength = 0;
    }
  }

  int card_selected = EEPROMReadlong(LAST_CARD_USED);
  uint64_t card_id = EEPROMReadlong(card_selected * 10);
  card_generated = generate_card(card_id);
  Serial.print("LAST CARD USED: ");
  Serial.println(card_selected);
  print_int64("EMULATE CARD: ", card_id);

  delay(1000);
  TXLED1;

}
void loop() {
  emulateCard(card_generated);
}
