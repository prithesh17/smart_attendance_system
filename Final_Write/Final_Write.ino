#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN 10

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

void setup() {
    Serial.begin(9600);
    while (!Serial);
    SPI.begin();
    mfrc522.PCD_Init();

    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }

    Serial.println(F("Place a MIFARE Classic PICC to write the name and employee ID to blocks 2 and 1."));
    Serial.print(F("Using key (for A and B):"));
    dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);
    Serial.println();
}

void loop() {
    if (!mfrc522.PICC_IsNewCardPresent())
        return;

    if (!mfrc522.PICC_ReadCardSerial())
        return;

    Serial.print(F("Card UID: "));
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println();

    byte nameBlockAddr = 2; // Block 2 is used to store the name and its length.
    byte idBlockAddr = 1;   // Block 1 is used to store the employee ID.
    byte buffer[18];
    byte size = sizeof(buffer);

    // Authenticate using key A
    Serial.println(F("Authenticating using key A..."));
    MFRC522::StatusCode status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, nameBlockAddr, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }

    // Get the name from the user
    Serial.println(F("Enter the name (max 15 characters): "));
    while (!Serial.available()) {
        // Wait for user input
    }
    String nameInput = Serial.readString();
    nameInput.trim();
    if (nameInput.length() > 15) {
        Serial.println(F("Name is too long. Maximum length is 15 characters."));
        return;
    }

    // Write the length of the name to the 0th byte in block 2
    buffer[0] = nameInput.length();

    // Copy the name to the remaining bytes in block 2
    nameInput.getBytes(&buffer[1], nameInput.length() + 1);

    // Write data to block 2
    Serial.print(F("Writing data into block ")); Serial.print(nameBlockAddr);
    Serial.println(F(" ..."));
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(nameBlockAddr, buffer, 16);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Write() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }

    // Get the employee ID from the user
    Serial.println(F("Enter the employee ID (6 characters): "));
    while (!Serial.available()) {
        // Wait for user input
    }
    String employeeIDInput = Serial.readString();
    employeeIDInput.trim();
    if (employeeIDInput.length() != 6) {
        Serial.println(F("Employee ID must be exactly 6 characters."));
        return;
    }

    // Write the 6-character employee ID to block 1
    employeeIDInput.getBytes(buffer, 7);

    // Write data to block 1
    Serial.print(F("Writing employee ID to block ")); Serial.print(idBlockAddr);
    Serial.println(F(" ..."));
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(idBlockAddr, buffer, 16);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Write() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }

    Serial.println(F("Data has been written to the card."));
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    delay(2000); // Wait for a moment before attempting to read.
}
