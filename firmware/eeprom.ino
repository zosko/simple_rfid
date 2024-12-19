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