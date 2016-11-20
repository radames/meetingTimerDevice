#define CLIENT_IP "10.0.1.65"
#define MAX_TIME 120 //max time 120 minutes
#define MSG_TIME 5000
#define MAX_TIME_FAILED 10000

//Arduino Yun Bridge and HTTP Client
#include <Bridge.h>
#include <HttpClient.h>

//temboo libraries
#include <Temboo.h>
#include "TembooKeys.h"
#include <Process.h>

//LCD I2C
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//encoder
#include <ClickEncoder.h>
#include <TimerOne.h>

#define pinBright 5 //LCD brigthness pin
#define pinContrast 9 //LCD contrast pin

LiquidCrystal_I2C lcd(32, 16, 2); //LCD address and number os characters and lines

//Encoder
ClickEncoder *encoder;


bool ledState = false;
int last = 0;
int timeLength = 0;

//mainTimer variable used for the main timer
long mainTimer = 0;

bool bReset = false;


enum State {
  ENCODERREAD,
  TIMERSTART,
  RESET,
  HOLD
} state; // <-- the actual instance, so can't be a typedef



void setup()
{

  Serial.begin(19200);

  // Bridge startup
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Bridge.begin();
  digitalWrite(13, HIGH);


  //start encoder
  encoder = new ClickEncoder(A1, A2, A0);
  encoder->setAccelerationEnabled(true);

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);
  last = -1;


  lcd.init();                      // initialize the lcd

  // Print a message to the LCD.
  //pinMode(pinBright, OUTPUT);
  //pinMode(pinContrast, OUTPUT);
  analogWrite(pinBright, 128); //max bright
  analogWrite(pinContrast, 0  ); //0% constrast



  // Listen for incoming connection only from localhost
  // (no one from the external network could connect)


  lcd.setCursor(0, 0);

  lcd.print("Bom dia");
  for (int i = 0; i < 3; i++) {
    delay(100);
    lcd.print(".");
  }

  lcd.clear();

  //initial state
  state = ENCODERREAD;


}

void loop()
{
 
  encoderButtonRead();

}

void encoderButtonRead() {

  ClickEncoder::Button b = encoder->getButton();
  if (b != ClickEncoder::Open) {
    switch (b) {
      case ClickEncoder::Held:
        state = HOLD;

        if (!bReset) {
          Serial.println("RESETTING");
          lcd.clear(); //clear lcd
          lcd.setCursor(0, 0);
          lcd.print("RESETTING...");
        }

        bReset = true; // enable Reset
        break;

      case ClickEncoder::Released:

        if (bReset) { //to Reset ?
          lcd.clear(); //clear lcd
          state = RESET;
          bReset = false;
        }
        break;

      case ClickEncoder::Clicked:

        String msgText = "Falhou";
        String linkText = "";

        facebookPost(msgText, linkText);

        break;
    }
  }

}




void facebookPost(String statusMsg, String linkMsg) {

  // print status
  
  Serial.println("Running UpdateFacebookStatus .... ");
  Serial.println(statusMsg);
  Serial.println(linkMsg);


  // define the Process that will be used to call the "temboo" client
  TembooChoreo SetStatusChoreo;

  SetStatusChoreo.begin();

  // set Temboo account credentials
  SetStatusChoreo.setAccountName(TEMBOO_ACCOUNT);
  SetStatusChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
  SetStatusChoreo.setAppKey(TEMBOO_APP_KEY);

  // tell the Temboo client which Choreo to run (Facebook > Publishing > SetStatus)
  SetStatusChoreo.setChoreo("/Library/Facebook/Publishing/Post");  
  SetStatusChoreo.addInput("AccessToken", FACEBOOK_ACCESS_TOKEN);
  SetStatusChoreo.addInput("Message", statusMsg);
  SetStatusChoreo.addInput("Link", linkMsg);


  unsigned int returnCode = SetStatusChoreo.run();

  Serial.println("Response code: " + String(returnCode));

  while (SetStatusChoreo.available()) {
    char c = SetStatusChoreo.read();
    Serial.print(c);
  }

  SetStatusChoreo.close();
  delay(1000);
 
}



void timerIsr() {
  encoder->service();
}


void sendClientCommand(int state) {
  // Make a HTTP request:
  String url = String(CLIENT_IP"/arduino/enableMotors/");
  url += state;

  HttpClient hClient;
  hClient.get(url);

}

