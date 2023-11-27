#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define RST_PIN 9
#define SS_PIN 10

const int servoPin = 5;
const int buzzerPin = 7;

String cardUID ="";

String allowedUIDs[] = {
  "4a197dbd",  // Add more UIDs as needed
  "96141ef9",
  "19cd5c59",
  "59b044c2" // Example UID
};

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myServo;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  myServo.attach(servoPin);
  myServo.write(0);
  mfrc522.PCD_Init();
  lcd.init();
  lcd.backlight();
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  pinMode(buzzerPin, OUTPUT);
}

void readNameAndEmployeeID() {
  byte nameBlockAddr = 2; // Block 2 stores the name and its length.
  byte idBlockAddr = 1;   // Block 1 stores the employee ID.
  byte buffer[18];
  byte size = sizeof(buffer);

  MFRC522::StatusCode status;

  // Authenticate using key A for name block
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, nameBlockAddr, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    return;  // Authentication failed
  }

  // Read the length of the name from byte 0 in block 2
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(nameBlockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    return;  // Read failed
  }

  byte nameLength = buffer[0];

  // Read the name from the remaining bytes in block 2
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(nameBlockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    return;  // Read failed
  }

  char name[16]; // Maximum name length
  for (byte i = 1; i <= nameLength; i++) {
    name[i - 1] = (char)buffer[i];
  }
  name[nameLength] = '\0';

  // Authenticate using key A for employee ID block
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, idBlockAddr, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    return;  // Authentication failed
  }

  // Read the employee ID from byte 0 in block 1
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(idBlockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    return;  // Read failed
  }

  char employeeID[7]; // Maximum 6 characters for the employee ID
  for (byte i = 0; i < 6; i++) {
    employeeID[i] = (char)buffer[i];
  }
  employeeID[6] = '\0';
  Serial.print(cardUID);
  Serial.print(",");
  Serial.print(name);
  Serial.print(",");
  Serial.print(employeeID);
  Serial.print("\n");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Success !");
  digitalWrite(buzzerPin, HIGH);
  delay(200);
  digitalWrite(buzzerPin, LOW);
  myServo.write(180);
  delay(2000);
  myServo.write(0);
  lcd.clear();
}

void loop() {

  cardUID="";

  lcd.setCursor(0,0);
  lcd.print("Scan Your Card");

  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Do Not Remove");
  delay(1000);
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    cardUID += String(mfrc522.uid.uidByte[i], HEX);
  }

  bool isMatch = false;
  
  for (int i = 0; i < sizeof(allowedUIDs) / sizeof(allowedUIDs[0]); i++) {
    if (allowedUIDs[i].equals(cardUID)) {
      isMatch = true;
      break;
    }
  }

  if (isMatch) {
  readNameAndEmployeeID();
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  delay(1000); 
  }
  else{
    digitalWrite(buzzerPin, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Invalid Card");
    delay(1000);
    digitalWrite(buzzerPin, LOW);
    lcd.clear();
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    delay(1000);
  }
}
