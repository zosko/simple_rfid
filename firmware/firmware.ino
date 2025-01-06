#include <EEPROM.h>

// MEMORY
int LAST_CARD_USED = 100;

// PINS
int BUTTON_CLONE_CARD = 4;
int POWER_READER = 7;
int COIL_PIN = 9;

// EMULATION
uint32_t card_generated;

// RDM6300
bool id_is_valid = false;
unsigned long long current_id;
unsigned char received_checksum;

void setup() {
  Serial.begin(9600);

  pinMode(BUTTON_CLONE_CARD, INPUT);
  pinMode(POWER_READER, OUTPUT);
  pinMode(COIL_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(COIL_PIN, LOW);

  delay(1000);
}
void loop() {

  bool buttonMenuPressed = digitalRead(BUTTON_CLONE_CARD) == HIGH;

  if (buttonMenuPressed) {
    digitalWrite(LED_BUILTIN, HIGH);
    int lastSavedLocation = EEPROMReadlong(LAST_CARD_USED);
    bool isCloned = cloneCard(lastSavedLocation);
    if (isCloned) {
      lastSavedLocation++;
      if (lastSavedLocation > 4) {
        lastSavedLocation = 4;
      }
      EEPROMWritelong(LAST_CARD_USED, lastSavedLocation);

      for (int i = 0; i < 5; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(300);
        digitalWrite(LED_BUILTIN, LOW);
        delay(300);
      }
    }
  }

  for (int i = 0; i < 5; i++) {

    uint64_t card_id = EEPROMReadlong(i * 10);
    card_generated = generate_card(card_id);

    for (int y = 0; y < 10; y++) {
      emulateCard(card_generated);
    }
  }
}
