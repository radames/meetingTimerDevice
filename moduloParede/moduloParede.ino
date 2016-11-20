#define CLIENT_IP "10.0.1.65"
#define MAX_TIME 120 //max time 120 minutes
#define MSG_TIME 5000
#define MAX_TIME_FAILED 10000

//LCD I2C
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//encoder
#include <ClickEncoder.h>
#include <TimerOne.h>



//Arduino Yun Bridge and HTTP Client
#include <Bridge.h>
#include <HttpClient.h>

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


  // Bridge startup
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Bridge.begin();
  digitalWrite(13, HIGH);

  // Listen for incoming connection only from localhost
  // (no one from the external network could connect)


  Serial.begin(19200);


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
  switch (state) {

    case ENCODERREAD:
      //mode for reading the encoder and set timeValue;

      lcd.setCursor(0, 0);
      lcd.print("SET MEETING TIME");

      timeLength += int(encoder->getValue());
      timeLength = constrain(timeLength, 0, MAX_TIME); //


      if (timeLength != last) {
        last = timeLength;
        Serial.print("Time Length: ");
        Serial.println(timeLength);
        lcd.setCursor(0, 1);
        lcd.print(timeLength);
        lcd.print(" Minutes  ");
      }
      break;

    case TIMERSTART:

      lcd.setCursor(0, 0);
      lcd.print("YOUR MEETING");

      long elapsedTime, minutes, seconds, maxTime;

      maxTime = (long)timeLength * 60 * 1000;

      elapsedTime = millis() - mainTimer; // elapsed time since it started
      minutes = (elapsedTime / 60000) % 60; //minutes
      seconds = (elapsedTime / 1000) % 60;

      lcd.setCursor(0, 1);
      lcd.print(minutes < 10 ? "0" : "");
      lcd.print(minutes);
      lcd.print(":");
      lcd.print(seconds < 10 ? "0" : "");
      lcd.print(seconds);

      if (elapsedTime % 1000  == 0) {
        Serial.println(elapsedTime);
        Serial.println(maxTime);
      }

      if (elapsedTime > maxTime) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("YOU'VE FAILED");
        lcd.setCursor(0, 1);
        lcd.print("TOO LONG!");
        sendClientCommand(true); // send command to turn ON CHAIR motors
        
        
        //blick Screen while you'e failed
        int c = 0;
        while(c < MAX_TIME_FAILED/1000){
         analogWrite(pinBright, 0);
         delay(500);
         analogWrite(pinBright, 255);
         delay(500);
         c++;
        } 
    
        state = RESET;
      }

      break;

    case RESET:
      state = ENCODERREAD;
      timeLength = 0;
      break;

    case HOLD:
      break;

  }
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

        if (!(state == TIMERSTART)) {
          //if is not running already then process de start click

          if (timeLength > 0) {

            Serial.println("Start timer");
            state = TIMERSTART; //change state
            mainTimer = millis(); // reset global timer
            lcd.clear(); //clear lcd
          } else {

            lcd.clear(); //clear lcd
            lcd.setCursor(0, 0);
            lcd.print("THE TIME MUST BE");
            lcd.setCursor(0, 1);
            lcd.print("MORE THAN ZERO");
            delay(MSG_TIME);
            lcd.clear(); //clear lcd


          }
        } else {
          //if ONE click and it o running timer, it means SUCCES
          lcd.setCursor(0, 0);
          lcd.print("WELL DONE       ");
          delay(MSG_TIME);

          state = RESET;

        }
        break;
    }
  }

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

