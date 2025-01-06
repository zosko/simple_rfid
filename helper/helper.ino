#include <EEPROM.h>

uint64_t EEPROMReadlong(int address) {
  uint64_t card = 0;
  for (int i = address, b = 0; i < address + 10; i++, b++) {
    uint64_t tmp = EEPROM.read(i);
    tmp = tmp << (36 - b * 4);
    card |= tmp;
  }
  return card;
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

void setup() {
  Serial.begin(9600);

  Serial.println("COMMANDS: ");
  Serial.println("    'l' - list saved cards");
  Serial.println("    'c' - clear saved cards");
}

void clear() {
  Serial.print("EEPROM CLEARING....");
  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0);
  }
  Serial.print("EEPROM CLEAR!");
}

void list() {
  int card_selected = EEPROMReadlong(100);
  Serial.print("LAST CARD USED: ");
  Serial.println(card_selected);

  for (int i = 0; i < 5; i++) {
    Serial.print("ID [");
    Serial.print(i);
    Serial.print("] : ");
    uint64_t card_id = EEPROMReadlong(i * 10);
    print_int64("EMULATE CARD: ", card_id);
  }
}

void loop() {
  if (Serial.available()) {
    char command = Serial.read();
    if (command == 'l') {
      list();
    }
    if (command == 'c') {
      clear();
    }
  }
}
