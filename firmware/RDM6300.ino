unsigned long long readCard() {
  while (!id_is_valid) {
    while (Serial.available() > 0) {
      parse(Serial.read());
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
