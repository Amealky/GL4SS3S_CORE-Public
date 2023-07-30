//MICRO OLED SCREEN LIBRARIES
#include <Wire.h>
#include <SFE_MicroOLED.h> //Click here to get the library: http://librarymanager/All#SparkFun_Micro_OLED

//BLUETOOTH LIB
#include "SoftwareSerial.h"

#include "OneButton.h"


//SERIAL PORT
int baud_rate = 9600;

//DEVICE_NAME
String device_name = "GL4SS3S";
String waiting_message = "wait conn";

//MICRO OLED SETUP
#define DC_JUMPER 1
#define PIN_RESET 6 // Optional - Connect RST on display to pin 6 on Arduino
MicroOLED oled(PIN_RESET, DC_JUMPER); //Example I2C declaration

//BLUETOOTH PORT DIGITAL
SoftwareSerial mySerial(3,2);

//BUTTON SETUP
OneButton button(9, true);


//DATAS SETUP
int blinkState = 0;

int yReadingOffset  = 0;
int currentLine = 0;
int maxLines = 0;

String currentFeatureName = "";
String currentItemName = "";
String textToRead = "";

String tempText = "";

enum BASIC_STATE{START_SCREEN, MENU_SCREEN, SELECT_SCREEN, READ_SCREEN, DIRECT_SCREEN, SHOW_SCREEN};
String SELECT_KEY = "SELECT:";
String TEMP_KEY = "TEMP:";
String FEATS_KEY = "FEATS:";
String FEAT_KEY = "FEAT:";
String ITEM_KEY = "ITEM:";
String DIRECT_KEY = "DIRECT:";
String READ_KEY = "READ:";
String REFRESHING_KEY = "REFR:";
String DISCONNECT_KEY = "OK+LOST";
String CONNECT_KEY = "OK+CONN";
String EXIT_KEY = "SORTIR";

String NEXT_FEATURE_KEY = "NEXT:FEAT";
String NEXT_ITEM_KEY = "NEXT:ITEM";

BASIC_STATE basic_state = START_SCREEN;

void setup() {
  //INIT BLUETOOTh
  Serial.begin(baud_rate);
  
   while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  mySerial.begin(baud_rate);

  
  //INIT SCREEN
  // put your setup code here, to run once:
  delay(100);  //Give display time to power on
  Wire.begin(); //Setup I2C bus

  // Before you can start using the OLED, call begin() to init
  // all of the pins and configure the OLED.
  oled.begin();
  // clear(ALL) will clear out the OLED's graphic memory.
  // clear(PAGE) will clear the Arduino's display buffer.
  oled.clear(ALL); // Clear the display's memory (gets rid of artifacts)
  // To actually draw anything on the display, you must call the
  // display() function.
  oled.display();

  delay(1000); // Delay 1000 ms

  oled.clear(PAGE); // Clear the buffer.

}

void resetDatas() {
  blinkState = 0;
  
  currentFeatureName = "";
  currentItemName = "";
  textToRead = "";

}


void loop() {
  String readvalue = readFromBLEDevice();
  if(readvalue== DISCONNECT_KEY) {
        basic_state = START_SCREEN;
        resetDatas();
   } 

  if(readvalue.startsWith(SELECT_KEY)) {
    readvalue.replace(SELECT_KEY, "");
    Serial.println(readvalue);
    currentItemName = readvalue ;   
    basic_state = SELECT_SCREEN;
  }

  if(readvalue.startsWith(TEMP_KEY)) {
    readvalue.replace(TEMP_KEY, "");
    Serial.println(readvalue);
    tempText = readvalue;
  }


  if(tempText == "") {
    switch(basic_state) {
        case START_SCREEN:
              playStartScreen(readvalue);
              break;
        case MENU_SCREEN:
              playMenuScreen(readvalue);
              break;
        case SHOW_SCREEN:
              playShowScreen(readvalue);
              break;
        case SELECT_SCREEN:
              playSelectScreen(readvalue);
              break;
        case READ_SCREEN:
              playReadScreen(readvalue);
              break;
        case DIRECT_SCREEN:
              playDirectScreen(readvalue);
              break;
        default:
              break;
    }
  } else {
    button.tick();
    printTitle(tempText, 0);
  }

  Serial.println(readvalue);

}

void playStartScreen(String readvalue) {
    Serial.println(readvalue);
    
   printStartScreen(device_name);
   
   if(readvalue == CONNECT_KEY) {
      mySerial.print(readvalue);
      oled.clear(PAGE);
      basic_state = MENU_SCREEN;
    }

}

void playMenuScreen(String readvalue) {

  button.tick();

  if(readvalue.startsWith(FEATS_KEY)) {
    readvalue.replace(FEATS_KEY, "");
    currentFeatureName = readvalue;
    Serial.println(currentFeatureName);  

    button.attachClick(isButtonClicked);
    button.attachDoubleClick(isButtonDoubleClicked);
    button.attachLongPressStart(isButtonLongClicked);
  }

  if(readvalue.startsWith(DIRECT_KEY)) {
    printText("");
    Serial.println(readvalue);
    basic_state = DIRECT_SCREEN;
  } else {
      printTitle(currentFeatureName, 0);
  }
  

  //oled.display();
}

void playSelectScreen(String readvalue) {
  button.tick();

  if(readvalue.startsWith(READ_KEY)) {
    readvalue.replace(READ_KEY, "");
    Serial.println(readvalue);
    textToRead = readvalue;
    maxLines = (textToRead.length() / 10) - 5;
    
    basic_state = READ_SCREEN;
    
  }

  if(readvalue.startsWith(REFRESHING_KEY)) {
    readvalue.replace(REFRESHING_KEY, "");
    Serial.println(readvalue);
    textToRead = readvalue;

    maxLines = (textToRead.length() / 10) - 5;
    basic_state = SHOW_SCREEN;
  }


  printTitle(currentItemName, 0);
}

void playShowScreen(String readvalue) {
  button.tick();

  if(readvalue.startsWith(REFRESHING_KEY)) {
    readvalue.replace(REFRESHING_KEY, "");
    Serial.println(readvalue);
    textToRead = readvalue;

    maxLines = (textToRead.length() / 10) + 15;
  }

  printText(textToRead);
}

void playReadScreen(String readvalue) {
  button.tick();

  printText(textToRead);
}

void playDirectScreen(String readvalue) {
  button.tick();
  if(readvalue.length() > 0) {
      printText(readvalue);
  }
}

String readFromBLEDevice() {
   if(mySerial.available()) {
    return mySerial.readString();
  }

  return "";
}

void writeToBLEDevice(char* data) {
   mySerial.println(data);
}

void printStartScreen(String title) {
  int middleX = oled.getLCDWidth() / 3;
  int middleY = oled.getLCDHeight() / 2;

  int titleY = oled.getLCDHeight() / 3;

  int subTitleY = oled.getLCDHeight() / 1.8;


  oled.clear(PAGE);

  oled.setFontType(0);

  oled.setCursor(middleX - (oled.getFontWidth() * (title.length() / 3)),
                 titleY);

  oled.print(title);

  if(blinkState == 1) {
      String waitingForBluetooth = waiting_message;

      oled.setCursor(middleX - (oled.getFontWidth() * (title.length() / 2)), subTitleY);

      oled.print(waitingForBluetooth);
      blinkState = 0;
  } else {
      blinkState = 1;
  }
  
  oled.display();
   delay(700);
  oled.clear(PAGE);
}

void printTitle(String title, int font)
{
  int middleX = oled.getLCDWidth() / 2;
  int middleY = oled.getLCDHeight() / 2;
  
  oled.clear(PAGE);
  oled.setFontType(font);
  // Try to set the cursor in the middle of the screen
    if(title.length() < 9) {
   oled.setCursor(middleX - (oled.getFontWidth() * (title.length() / 2)),
                 middleY - (oled.getFontHeight() / 2));
  } else {
     oled.setCursor(1, 1);
  }

  // Print the title:
  oled.print(title);
  oled.display();
  oled.clear(PAGE);
}

void printText(String text) {
  oled.clear(PAGE);
  oled.setFontType(0);

  oled.setCursor(0, -yReadingOffset);

  oled.print(text);
  oled.display();
  oled.clear(PAGE);
}

void isButtonClicked() {

  if(tempText != "") {
    Serial.println(tempText);
    tempText = "";
  } else {
      switch(basic_state) {
      case MENU_SCREEN:
            goToNextFeature();
            break;
      case SELECT_SCREEN:
            goToNextItem();
            break;
      case READ_SCREEN:
            scrollText();
            break;
       case SHOW_SCREEN: 
            scrollText();
            break;
      default:
            break;
      }
  }
  
  
}

void isButtonDoubleClicked() {
  switch(basic_state) {
      case MENU_SCREEN:
            selectFeature();
            break;
      case SELECT_SCREEN:
            selectItem();
            break;
      case READ_SCREEN:
           exitCurrentState();
           break;
      case DIRECT_SCREEN:
           mySerial.print("DIRECT:"+EXIT_KEY); 
           exitCurrentState();
           break;
      case SHOW_SCREEN:
           refreshItem();
           break;
      default:
            break;
  }
}

void isButtonLongClicked() {
  switch(basic_state) {
    case SHOW_SCREEN:
          exitCurrentState();
          break;
  }
}
void goToNextFeature() {
  mySerial.print(NEXT_FEATURE_KEY); 
}

void goToNextItem() {
   mySerial.print(NEXT_ITEM_KEY); 
}

void scrollText() {
  if(currentLine < maxLines) {
      currentLine++;
      yReadingOffset+=15;
  } else {
    resetScroll();
  }
}

void resetScroll() {
    yReadingOffset = 0;
    currentLine = 0;
}

void selectFeature() {
 
  mySerial.print(FEAT_KEY+currentFeatureName); 
  
}

void selectItem() {
  if(currentItemName == EXIT_KEY) {
    exitCurrentState();
    mySerial.print(ITEM_KEY+currentItemName); 
  } else {
    mySerial.print(ITEM_KEY+currentItemName); 
  }
}

void refreshItem() {
  mySerial.print(ITEM_KEY+currentItemName); 
}

void exitCurrentState() {
  resetScroll();
  switch(basic_state) {
    case READ_SCREEN: 
         basic_state = SELECT_SCREEN;
         break;
    case SELECT_SCREEN:
         basic_state = MENU_SCREEN;
         break;
    case DIRECT_SCREEN:
         basic_state = MENU_SCREEN;
    case SHOW_SCREEN:
         basic_state = SELECT_SCREEN;
         break;
  }
}


/*int countOccurence(String data, char characterToDetect) {

  int occurence = 0;

  for(int i = 0; i < data.length(); i++) {
    if(data.charAt(i) == characterToDetect) {
      occurence++;  
    }
  }

  return occurence+1;
}

void splitString(String* dateSplitted, String data, char separator) {
    int occurence = 0;
    int lastStartIndex = 0;

        
    for(int i = 0; i < data.length(); i++){
      if(data.charAt(i) == separator) {
        dateSplitted[occurence] = data.substring(lastStartIndex, i);
        occurence++;
        lastStartIndex = i+1;
      }
      if(i == data.length() -1 ) {
        dateSplitted[occurence] = EXIT_KEY ;
      }
    }

}*/

/*char* stringToChar(String value) {
    char* charArray;
    value.toCharArray(charArray, value.length());
    return charArray;
}*/
