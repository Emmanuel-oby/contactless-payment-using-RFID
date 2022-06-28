

#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Adafruit_Fingerprint.h>

#define RST_PIN         27           // Configurable, see typical pin layout above
#define SS_PIN          5         // Configurable, see typical pin layout above
#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // four columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};

byte pin_rows[ROW_NUM]      = {4, 32, 33, 25}; // GIOP19, GIOP18, GIOP5, GIOP17 connect to the row pins
byte pin_column[COLUMN_NUM] = {26, 13, 15, 14};   // GIOP16, GIOP4, GIOP0, GIOP2 connect to the column pins
int numOfOper;
String BALANCE;
String input_password;
String price;
int cursorColumn = 0;
boolean fingra = false;
boolean pay = false;
boolean donebit = true;

HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id;

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
LiquidCrystal_I2C lcd(0x3F, 20, 2); // I2C address 0x27, 16 column and 2 rows


MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

void setup() {
  Serial.begin(115200);        // Initialize serial communications with the PC
  SPI.begin();               // Init SPI bus
  mfrc522.PCD_Init();        // Init MFRC522 card
  finger.begin(57600);
  
  lcd.begin(); // initialize the lcd
  lcd.backlight();
  Serial.println(F("Write personal data on a MIFARE PICC "));
  lcd.print("You are welcome!");
  delay(3000);
  
}

void loop() {
  if(donebit == true){
  lcd.setCursor(0,0);
  lcd.print("press A: REG/Top-up");
  lcd.setCursor(0,1);
  lcd.print("*: check balance");
  lcd.setCursor(0,2);
  lcd.print("#: make payment");
  }
  char key = keypad.getKey();

  if (fingra == true){
     if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }


  uint8_t p = finger.getImage();
  if (p == FINGERPRINT_NOFINGER){
    Serial.println("yes oh!");
    delay(3000);
  }
  if (p == FINGERPRINT_NOFINGER)
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
     
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
     
    default:
      Serial.println("Unknown error");
   
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
     
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
     
    default:
      Serial.println("Unknown error");
      
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
    fingra = false;
    pay = true;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
   
  } else {
    Serial.println("Unknown error");
   
  }
  }

  if (pay == true){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(price);
    lcd.setCursor(0,1);
    lcd.print("place your card over");
    lcd.setCursor(0,2);
    lcd.print("the reader");
    payTime();
    pay = false;
  }

  if (key) {
    donebit = false;
    Serial.println(key);
    lcd.setCursor(cursorColumn, 0); // move cursor to   (cursorColumn, 0)
    lcd.print(key);                 // print key at (cursorColumn, 0)

    cursorColumn++;                 // move cursor to next position

    if (key == '*') {
      lcd.clear();
      lcd.print("waiting for card...");
      checkTime();
      
    }else if(key == '#'){
      lcd.clear();
      lcd.print("How much ?");
      price= "";
      cursorColumn = 0;
      delay(3000);
      lcd.clear();
    }else if(key == 'D'){
      lcd.clear();
      fingra = true;
      Serial.println(price);
      lcd.setCursor(0,0);
      lcd.print("place your hand");
      lcd.setCursor(0,1);
      lcd.print("on the fingerprint");
      lcd.setCursor(0,2);
      lcd.print("sensor");
  } else if (key == 'A') {
      lcd.clear();
      lcd.print("please input your password");
      input_password= "";
      cursorColumn = 0;
      delay(3000);
      lcd.clear();
    }else if(key == 'B'){
      lcd.clear();
      Serial.println(input_password);
      registerTime1();
      delay(2000);
      lcd.clear();
      lcd.print("please enter balance");
      BALANCE = "";
      cursorColumn = 0;
      delay(3000);
      lcd.clear();
    }else if(key == 'C'){
      lcd.clear();
      Serial.println(BALANCE);
      registerTime2();
     
    }
    else{
    input_password += key;
    BALANCE +=key;
    price +=key;
  }
}
}

  

void registerTime1(){
  
  delay(2000);
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print(F("Card UID:"));    //Dump UID
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.print(F(" PICC type: "));   // Dump PICC type
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  byte buffer[34];
  byte block;
  MFRC522::StatusCode status;
  byte len;

//  Serial.setTimeout(20000L) ;     // wait until 20 seconds for input from serial
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  
  Serial.println(F("Type password, ending with #"));
  lcd.clear();
  lcd.print("Type password, ending with #");
  len = (char)input_password.length()+1;
  input_password.getBytes(buffer, len);

//  len = Serial.readBytesUntil('#', (char *) buffer, 30) ; // read family name from serial
  Serial.println(len);
  for (byte i = len; i < 30; i++) buffer[i] = ' ';     // pad with spaces

  block = 1;
  //Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("PCD_Authenticate() success: "));

  // Write block
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));

  block = 2;
  //Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write block
  status = mfrc522.MIFARE_Write(block, &buffer[16], 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else{ Serial.println(F("MIFARE_Write() success: "));
  lcd.print("Accesss granted!");
  }
}
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void registerTime2(){
    
  delay(2000);
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

//  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
//  if ( ! mfrc522.PICC_IsNewCardPresent()) {
//    return;
//  }
//
//  // Select one of the cards
//  if ( ! mfrc522.PICC_ReadCardSerial()) {
//    return;
//  }
  
  byte buffer[34];
  byte block;
  MFRC522::StatusCode status;
  byte len;
  
  Serial.println(F("Enter Balance you want, ending with #"));
  lcd.clear();
  lcd.print("Enter Balance");
  len = (char)BALANCE.length()+3;
  BALANCE.getBytes(buffer, len);

//  len = Serial.readBytesUntil('#', (char *) buffer, 20) ; // read first name from serial
Serial.println(len);
  for (byte i = len; i < 20; i++) buffer[i] = ' ';     // pad with spaces

  block = 4;
  //Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write block
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));

  block = 5;
  //Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write block
  status = mfrc522.MIFARE_Write(block, &buffer[16], 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else {Serial.println(F("MIFARE_Write() success: "));
  delay(2000);
  lcd.print("Successfully updated balance!");
  }

  Serial.println(" ");
  mfrc522.PICC_HaltA(); // Halt PICC
  mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
  delay(2000);
  lcd.clear();
  donebit = true;
}




void payTime(){
  
   // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;

  //-------------------------------------------

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  while ( ! mfrc522.PICC_IsNewCardPresent()) {
    
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
   return;
  }

  Serial.println(F("**Card Detected:**"));

  //-------------------------------------------

  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card

  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));      //uncomment this to see all blocks in hex

  //-------------------------------------------

  byte buffer1[18];
  block = 4;
  len = 18;

  //------------------------------------------- GET BALANCE
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  //PRINT BALANCE
  String balance="";
  for (uint8_t i = 0; i < 16; i++)
  {
    if (buffer1[i] != 32)
    {
        balance+=(char)buffer1[i];
    }
  }
  
  //---------------------------------------- GET ID

  byte buffer2[18];
  block = 1;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  //PRINT ID  
  String id="";
  for (uint8_t i = 0; i < 16; i++) {
    id+=(char)buffer2[i];
  }

  
/////////////////////////////////////////////////////////////////////////////////////////////////////
  int balanceInt=balance.toInt();
  Serial.println("");
  Serial.println("Password: "+id);
  Serial.print("Your money: ");
  Serial.println(balanceInt);
  lcd.clear();
  lcd.print(balanceInt);
  
  //----------------------------------------

  int priceInt = price.toInt();
 
  if(id=="12345678"&&balanceInt>=priceInt){
    digitalWrite(15,HIGH);
    balanceInt-=priceInt;
    byte buffer3[18];
    for (int i=0;i<balance.length();i++){
      buffer3[i]=String(balanceInt)[i];
    }
    lcd.print(" Payment successful!  ");
    /////////////////////////////////////////////////////////////////////////////////////////////////
    byte block;
    MFRC522::StatusCode status;
    byte len;
//    Serial.setTimeout(20000L) ;

 
    block = 4;
    //Serial.println(F("Authenticating using key A..."));
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("PCD_Authenticate() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }
  
    // Write block
    status = mfrc522.MIFARE_Write(block, buffer3, 16);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Write() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }
    else Serial.println(F("MIFARE_Write() success: "));
  
    block = 5;
    //Serial.println(F("Authenticating using key A..."));
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("PCD_Authenticate() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }
  
    // Write block
    status = mfrc522.MIFARE_Write(block, &buffer3[16], 16);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Write() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }
    else Serial.println(F("MIFARE_Write() success: "));

    /////////////////////////////////////////////////////////////////////////////////////////////////
    
    delay(5000);
    digitalWrite(15,LOW);
  }else{
    Serial.print("Make sure to use the correct card and your balance is sufficient");
    lcd.clear();
    lcd.print("insufficient balance!");
    delay(2000);
  }
  
  Serial.println("");
  lcd.clear();
  
  Serial.print("Your money now: ");
  Serial.println(balanceInt);
  Serial.println(F("\n**End Reading**\n"));
  lcd.print("balance: ");
  lcd.print(balanceInt);

  delay(3000); //change value if you want to read cards faster
  donebit = true;
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();  
  }





  
void checkTime(){
     // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;
  lcd.clear();
  lcd.print("place your card");
  //-------------------------------------------

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  while ( ! mfrc522.PICC_IsNewCardPresent()) {
    
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    
  }

  Serial.println(F("**Card Detected:**"));

  //-------------------------------------------

  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card

  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));      //uncomment this to see all blocks in hex

  //-------------------------------------------

  byte buffer1[18];
  block = 4;
  len = 18;

  //------------------------------------------- GET BALANCE
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  //PRINT BALANCE
  String balance="";
  for (uint8_t i = 0; i < 16; i++)
  {
    if (buffer1[i] != 32)
    {
        balance+=(char)buffer1[i];
    }
  }
  
  //---------------------------------------- GET ID

  byte buffer2[18];
  block = 1;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  //PRINT ID  
  String id="";
  for (uint8_t i = 0; i < 16; i++) {
    id+=(char)buffer2[i];
  }

  
/////////////////////////////////////////////////////////////////////////////////////////////////////
  int balanceInt=balance.toInt();
  Serial.println("");
  Serial.println("Password: "+id);
  Serial.print("Your money: ");
  Serial.println(balanceInt);
  lcd.clear();
  lcd.print("balance: ");
  lcd.print(balanceInt);
  delay (3000);
  donebit = true;
}
  //----------------------------------------
