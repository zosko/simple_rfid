void set_pin_manchester(int clock_half, int signal) {
  int man_encoded = clock_half ^ signal;
  if (man_encoded == 1) {
    digitalWrite(COIL_PIN, LOW);
  } else {
    digitalWrite(COIL_PIN, HIGH);
  }
}
uint32_t generate_card(uint64_t Hex_IDCard) {
  static uint32_t data_card[64];
  static uint32_t card_id[10];

  for (int i = 0; i < 10; i++) card_id[i] = (Hex_IDCard >> 36 - i * 4) & 0xF;

  for (int i = 0; i < 9; i++) data_card[i] = 1;

  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 4; j++) data_card[9 + i * 5 + j] = card_id[i] >> (3 - j) & 1;
    data_card[9 + i * 5 + 4] = (data_card[9 + i * 5 + 0] + data_card[9 + i * 5 + 1] + data_card[9 + i * 5 + 2] + data_card[9 + i * 5 + 3]) % 2;
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

bool cloneCard(int cardLocation) {
  bool isCloned = false;
  digitalWrite(POWER_READER, HIGH);

  while (0000000000 != readCard()) {
    EEPROMWritelong(cardLocation * 10, readCard());
    print_int64("CARD CLONED: ", readCard());
    isCloned = true;
    break;
  }

  digitalWrite(POWER_READER, LOW);

  return isCloned;
}