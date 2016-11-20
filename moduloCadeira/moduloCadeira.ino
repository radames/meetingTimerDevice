#define DEBUG 0
#define SENSOR_THRESHOLD 600 //1021 nao pressionado, 
#define MAX_TIME_ON 10000 //max time motor ligados
#define MOTOR_POWER 80 //0 to 255 (0- 100%)
//Arduino Bridge
#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>


// motor pins
#define mtA1  10
#define mtA1PWM 9

#define mtB1  12
#define mtB1PWM 11

#define nSLEEP 8
#define nFAULT

#define sensorPin A0

YunServer server;
int stateMotors = false;
long lastMillis = 0;
int power = 0;
long timeAlive = 0;

void setup() {

  // Bridge startup
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Bridge.begin();
  digitalWrite(13, HIGH);

  // Listen for incoming connection only from localhost
  // (no one from the external network could connect)
  server.listenOnLocalhost();
  server.begin();

  pinMode(mtA1, OUTPUT);
  pinMode(mtB1, OUTPUT);
  pinMode(nSLEEP, OUTPUT);
  digitalWrite(nSLEEP, HIGH);

  digitalWrite(mtB1, LOW);
  digitalWrite(mtA1, LOW);

  Serial.begin(19200);
  pinMode(sensorPin, INPUT_PULLUP);


}

void loop() {

  // Get clients coming from server
  YunClient client = server.accept();

  // There is a new client?
  if (client) {
    // Process request
    process(client);
    // Close connection and free resources.
    client.stop();
  }

  int sensorVal = analogRead(sensorPin);
  
#ifdef DEBUG 1
  Serial.println(sensorVal);
#endif

  if (stateMotors && (millis() - timeAlive) < MAX_TIME_ON && sensorVal < SENSOR_THRESHOLD) {


    if (millis() - lastMillis > 2) {
      //enable in case someone is sitted on a chair
      digitalWrite(13, true);
      analogWrite(mtB1PWM, power);
      analogWrite(mtA1PWM, MOTOR_POWER - power);
      power = (power + 1) % MOTOR_POWER;
      lastMillis = millis();
    }


  } else {
    //turn off
    digitalWrite(13, false);
    analogWrite(mtB1PWM, 0);
    analogWrite(mtA1PWM, 0);
  }
}

void process(YunClient client) {
  // read the command
  String command = client.readStringUntil('/');
  client.println("Send Command /");

  // is "digital" command?
  if (command == "enableMotors") {
    client.println("Working on ...");

    stateMotors = client.parseInt();


    Serial.print("motors state: ");
    Serial.println(stateMotors);

    client.print("motors state: ");
    client.println(stateMotors);
    timeAlive = millis(); //reset timer for existence

  }

}
