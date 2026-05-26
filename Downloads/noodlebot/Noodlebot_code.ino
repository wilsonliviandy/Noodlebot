#include <LiquidCrystal_I2C.h>    // include all libraries we needed, library for LCD I2C
#include <OneWire.h>              // library for DS18b20 temperature sensor
#include <DallasTemperature.h>    // lirary for temperature sensor
#include "ServoTimer2.h"          // library for Servo in timer2
#define ONE_WIRE_BUS 8            // library for one wire communication

LiquidCrystal_I2C lcd(0x27, 16, 2);     // define LCD scale
OneWire oneWire(ONE_WIRE_BUS);          // define onewire
DallasTemperature sensors(&oneWire);    // define dallastemperature

const int trigPin = 7;    // declare trigger pin for ultrasonic sensor
const int echoPin = 6;    // declare echo pin for ultrasonic sensor

float Celsius;          // float value for temperature sensor (celcius)
float Fahrenheit;       // float value for temperature sensor (fahrenheit)
long duration;          // long value for ultrasonic sensor
int distance;           // distance value for ultrasonic sensor
int state;              // state variable for system
int trig;               // trigger variable for certain uses
int dur = 120;          // duration variable on cooking duration
int timer;              // timer variable to count system runtime
int button1state = 0;   // variable to store button state
int button2state = 0;   // variable to store button state

int angle1 = 750;       // angle variable to store servo angle
int angle2 = 750;       // while using timer2, 750PWM equals 0 degree, 2250PWM equals 180 degree
int angle3 = 750;
int angle4 = 750;

ServoTimer2 servo1;     // declare the servo we'll be using on the noodlebot
ServoTimer2 servo2;
ServoTimer2 servo3;
ServoTimer2 servo4;
ServoTimer2 servo5;      // declare the servo used on the stock disposal

void setup() {
  lcd.begin();           // initiate LCD
  sensors.begin();       // initiate temperature sensor

  pinMode(5, INPUT);     // declare input pins for push buttons
  pinMode(4, INPUT);

  servo1.attach(9);      // declare the pins used for servo
  servo2.attach(10);
  servo3.attach(11);
  servo4.attach(12);
  servo5.attach(13);
  pinMode(trigPin, OUTPUT); // declare the trigger pin in ultrasonic as output
  pinMode(echoPin, INPUT);  // declare the echo pin in ultrasonic as input

  Serial.begin(9600);       // serial begin for serial monitor

  TCCR1A = 0;   // set entire TCCR1A register to 0
  TCCR1B = 0;   // set entire TCCR1B register to 0
  TCNT1  = 0;      // setting the counter value as 0
  OCR1A = 15624, 6;      // set compare match register to 1hz
  TCCR1B |= (1 << WGM12);   // turn on CTC mode
  TCCR1B |= (1 << CS12) | (1 << CS10);  // set prescaler value to 1024
  TIMSK1 |= (1 << OCIE1A);    // enable timer compare interrupt
}

void control() {                  // a void for control pushbutton
  button1state = digitalRead(5);  // read button state, if high then function
  if (button1state == HIGH) {
    dur = dur + 30;
  }
}

void enter() {                    // a void for enter pushbutton
  button2state = digitalRead(4);  // read button state, if high then function
  if (button2state == HIGH) {
    state++;
    trig = 1;
  }
}

void loop() {           // main loop function of system
  control();            // read button state
  enter();
  switch (state) {      // switch between state according to state value
    case 0:
      startup();
      break;

    case 1:
      selection();
      break;

    case 2:
      stock();
      break;

    case 3:
      temperature();
      break;

    case 4:
      processing();
      break;

    case 5:
      cooking();
      break;

    case 6:
      serving();
      break;

    case 7:
      bonappetit();
      break;
  }

  Serial.print(state);        // serial print values for troubleshooting
  Serial.print("\n");
  Serial.print(dur);
  Serial.print("\n");
  delay(500);
}

void startup() {          // first function, startup the system
  if (trig == 1) {        // one time function
    lcd.clear();
    trig = 0;
  }
  startpos();             // set all servos to position 0
  lcd.setCursor (0, 0);
  lcd.print ("NoodleBot V1.0");
  lcd.setCursor (0, 1);
  lcd.print ("Start to Begin");       // printout display
}

void selection() {        // second function, duration selection
  if (trig == 1) {        // one time function, set initial duration value
    lcd.clear();
    trig = 0;
    dur = 120;
  }
  lcd.setCursor (0, 0);
  lcd.print ("Cooking Duration: ");

  lcd.setCursor (4, 1);    // logic to limit duration selection
  if (dur > 240) {
    dur = 120;
    lcd.print(dur);
  }

  else {
    lcd.print (dur);
  }
}

void stock() {        // third function, stock process
  ultrasonic();       // read ultrasonic sensor value
  if (trig == 1) {
    lcd.clear();
    trig = 0;
  }
  lcd.setCursor(0, 0);
  lcd.print("Stock Status :");
  lcd.setCursor (0, 1);

  if ( distance < 9) {                // stock logic, if noodle detected : dispense
    Serial.print(" Stock Ready! ");
    lcd.clear();
    lcd.print("Stock Status :");
    lcd.setCursor (0, 1);
    lcd.print ("Ready!");
    delay(2000);
    fetch();          // arm movement to stock dispenser
    delay(1000);
    stockout();       // dispense stock
    state++;
    trig = 1;
  }

  else {
    Serial.print(" Stock Unready! ");
    lcd.print ("Unready!");
  }
}


void temperature () {     // fourth function, temperature check
  if (trig == 1) {
    lcd.clear();
    trig = 0;
  }
  sensors.requestTemperatures();          // get water temperature
  Celsius = sensors.getTempCByIndex(0);
  lcd.setCursor(0, 0);
  lcd.print("Temperature");
  lcd.setCursor(0, 1);
  lcd.print(Celsius);
  lcd.setCursor(5, 1);
  lcd.print(" C ");
  if (Celsius >= 85) {        // cooking logic, if water boiling : cook
    delay(2000);
    state++;
    trig = 1;
  }
}

void processing () {        // fifth function, boil noodle in pot
  if (trig == 1) {
    lcd.clear();
    trig = 2;
  }

  lcd.setCursor (0, 0);
  lcd.print ("Processing...");
  if (trig == 2) {
    prepare();              // servo movement to cook
    trig = 0;
  }
  state++;
  trig = 1;
}


void cooking () {         // sixth function, boiling the noodle
  if (trig == 1) {
    lcd.clear();
    trig = 0;
  }
  lcd.setCursor(0, 0);
  lcd.print ("Cooking");
  lcd.setCursor(2, 1);
  lcd.print ("  ETA: ");
  lcd.setCursor(10, 1);
  lcd.print (dur);
  if (dur == 99) {
    lcd.clear();
  }
  if (dur == 9) {
    lcd.clear();
  }
  if (dur == 0) {         // countdown function
    state++;
    trig = 1;
  }
}

void serving () {         // seventh function, dry and serve
  if (trig == 1) {
    lcd.clear();
    trig = 2;
  }
  if (trig == 2) {
    lcd.setCursor (0, 0);
    lcd.print ("Drying...");
    dry();                      // servo lifts to dry cooked noodle
    trig = 0;
  }
  lcd.setCursor (0, 0);
  lcd.print ("Serving...");
  serve();                      // servo movement to serve noodle
  delay(5000);
  state++;
}

void bonappetit () {            // finishing function
  if (trig == 1) {
    lcd.clear();
    trig = 0;
  }
  lcd.setCursor (0, 0);
  lcd.print ("Bon Appetit");      // final display
  reset();                        // reset servo position
  delay(3000);
  state = 0;                      // set state value back to 0
}

ISR(TIMER1_COMPA_vect) {  // timer function, used for countdown and run time
  if (state == 5) {
    dur--;
  }
  timer++;
}

void ultrasonic() {
  digitalWrite(trigPin, LOW);     // Clears the trigPin
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);   // Sets the trigPin on HIGH state for 10 micro seconds
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);  // Reads the echoPin, returns the sound wave travel time in microseconds
  distance = duration * 0.034 / 2;  // Calculating the distance

  Serial.print("Distance: ");   // Prints the distance on the Serial Monitor
  Serial.println(distance);
}



void move1(int pos) {                           // function to move servo forward with certain speed
  for (angle1 ; angle1 < pos; angle1 += 12, 5)
  {
    servo1.write(angle1);
    delay(18);
  }
}

void back1(int pos) {                           // function to move servo backward with certain speed
  for (angle1; angle1 > pos; angle1 -= 12, 5)
  {
    servo1 .write(angle1);
    delay(18);
  }
}

void move2(int pos) {                           // function to move servo forward with certain speed
  for (angle2 ; angle2 < pos; angle2 += 12, 5)
  {
    servo2.write(angle2);
    delay(18);
  }
}

void back2(int pos) {                           // function to move servo backward with certain speed
  for (angle2; angle2 > pos; angle2 -= 12, 5)
  {
    servo2 .write(angle2);
    delay(18);
  }
}

void move3(int pos) {                           // function to move servo forward with certain speed
  for (angle3 ; angle3 < pos; angle3 += 12, 5)
  {
    servo3.write(angle3);
    delay(18);
  }
}

void back3(int pos) {                           // function to move servo backward with certain speed
  for (angle3; angle3 > pos; angle3 -= 12, 5)
  {
    servo3 .write(angle3);
    delay(18);
  }
}

void move4(int pos) {                           // function to move servo forward with certain speed
  for (angle4 ; angle4 < pos; angle4 += 12, 5)
  {
    servo4.write(angle4);
    delay(18);
  }
}

void back4(int pos) {                           // function to move servo backward with certain speed
  for (angle4; angle4 > pos; angle4 -= 12, 5)
  {
    servo4 .write(angle4);
    delay(18);
  }
}

void startpos() {       // set servo value to 0 during startup
  servo1.write(750);
  servo2.write(750);
  servo3.write(750);
  servo4.write(750);
}

void fetch() {        // preset servo movement to get noodle from stock
  move3(900);
  move1(2050);
}

void stockout() {     // preset servo movement to dispense stock
  servo5.attach(13);
  servo5.write (750);
  delay(400);
  servo5.write(2250);
  delay(250);

  servo5.detach();
  back1(1100);
}

void prepare() {      // preset servo movement to put noodle in pot
  move3(1200);
  move2(1000);
  delay(2000);
  move3(1550);
  move1(1650);
  delay(2000);
  move2(1150);
  delay(1000);
  move2(1300);
  delay(1000);
  move2(1450);
  delay(2000);
}

void dry() {          // preset servo movement to dry noodle
  back2(1150);
  delay(15000);
}

void serve() {        // preset servo movement to serve cooked noodle
  back2(950);
  delay(1000);
  back1(1050);
  delay(1000);
  back3(1150);
  delay(1000);
  move4(1850);
  delay(5000);
  back4 (750);
}

void reset() {        // preset servo movement to move back to initial position
  back1(750);

  back2(750);

  back3(750);

  back4(750);
}
