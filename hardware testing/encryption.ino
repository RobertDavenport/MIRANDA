// TODO: Currently we are not sending sensitive data, This is a test proving encyrption on
// Arduino IDE is possible through the use of AESLIB.h

#include "AESLib.h"

#define BAUD 9600

AESLib aesLib;

#define INPUT_BUFFER_LIMIT (128 + 1) // MIGHT NEED TO PLAY AROUND WITH THIS INPUT BUFFER LIMIT

unsigned char plaintext[INPUT_BUFFER_LIMIT] = {0}; 
unsigned char ciphertext[2*INPUT_BUFFER_LIMIT] = {0};

unsigned char readBuffer[18] = "haptic:intensity";

// AES Encryption Key (same as in node-js example)
byte aes_key[] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };

// General initialization vector (same as in node-js example) (you must use your own IV's in production for full security!!!)
byte aes_iv[N_BLOCK] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };

// Generate IV (once)
void aes_init() {
  aesLib.gen_iv(aes_iv);
  aesLib.set_paddingmode((paddingMode)0);
}

uint16_t encrypt_to_ciphertext(char * msg, uint16_t msgLen, byte iv[]) {
  Serial.println("Calling encrypt (string)...");
  // aesLib.get_cipher64_length(msgLen);
  int cipherlength = aesLib.encrypt((byte*)msg, msgLen, (char*)ciphertext, aes_key, sizeof(aes_key), iv);
                   // uint16_t encrypt(byte input[], uint16_t input_length, char * output, byte key[],int bits, byte my_iv[]);
  return cipherlength;
}

uint16_t decrypt_to_plaintext(byte msg[], uint16_t msgLen, byte iv[]) {
  Serial.print("Calling decrypt...; ");
  uint16_t dec_bytes = aesLib.decrypt(msg, msgLen, (char*)plaintext, aes_key, sizeof(aes_key), iv);
  Serial.print("Decrypted bytes: "); Serial.println(dec_bytes);
  return dec_bytes;
}

void setup() {
  Serial.begin(BAUD);
  Serial.setTimeout(60000);
  delay(2000);

  aes_init();

}

/* wait function */
void wait(unsigned long milliseconds) {
  unsigned long timeout = millis() + milliseconds;
  while (millis() < timeout) {
    yield();
  }
}