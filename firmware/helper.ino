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