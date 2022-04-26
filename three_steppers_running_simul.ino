#include <SpeedyStepper.h>

// #include <iom6450.h>                                                          // for p manipulation

// communication
String inputString = "";                                                      // a String to hold incoming data
bool stringComplete = false;                                                  // whether the string is complete
// --

// general
int16_t system_err = 1;                                                       // return values from functions
int32_t homeSpeed = 1000;

typedef struct Motor
{
  const int dir_mask;                                                         // direction pin
  const int pulse_mask;                                                       // pulse pin
  int32_t cur_pos;                                                            // current position
  int32_t req_pos;                                                            // required position
  const int Home_mask;                                                        // Home pin for every axile
} Motor;

//              dir_mask | pulse_mask | cur_pos | req_pos | Home_mask
//              76543210   76543210                         76543210
// Motor M1 = {B00001000, B00010000,       0,       0,     B00100000};
// Motor M2 = {B00010000, B00100000,       0,       0,     B01000000};
// Motor M3 = {B00100000, B00100000,       0,       0,     B10000000};

Motor M1 = {6, 2, 0, 0, 11}; // Y1
Motor M2 = {7, 3, 0, 0, 12}; // Y2
Motor M3 = {8, 4, 0, 0, 13}; // X

SpeedyStepper Y1stepper;
SpeedyStepper Y2stepper;
SpeedyStepper Xstepper;

typedef struct Points
{
  int32_t x;
  int32_t y;
} Points;

Points P1 = {1124, 1150};
Points P2 = {0, 1150};
Points P3 = {1124, 0};
Points P4 = {0, 0};
Points P5 = {562, 575};

/*----------------------------------------------------------------------------------------------------------------*/

void setup()
{
  Serial.begin(115200);
  inputString.reserve(200);                                                   // reserve 200 bytes for the string 

  pinMode(M1.Home_mask, INPUT_PULLUP);
  pinMode(M2.Home_mask, INPUT_PULLUP);
  pinMode(M3.Home_mask, INPUT_PULLUP);

  Y1stepper.connectToPins(M1.pulse_mask, M1.dir_mask);
  Y2stepper.connectToPins(M2.pulse_mask, M2.dir_mask);
  Xstepper.connectToPins(M2.pulse_mask, M2.dir_mask);

  Serial.println("\nHello Gantry\n");
}

/*----------------------------------------------------------------------------------------------------------------*/

void loop()
{
  if (stringComplete)
  {
    Serial.println(inputString);
    if (inputString == "h" || inputString == "H")
    {
      system_err = goHome(M1, M2, M3);
    }
    else
    {
      int coord = inputString.toInt();
      if (((coord == 0 && inputString == "0") || coord) && (!system_err))       // (if coord is 0 & string is 0 or just coord) and system error is false
      {
        if (coord == 1)
        {
          Serial.println("\nGoing to position 1");
          M1.req_pos = Ymovment(P1);
          M2.req_pos = M1.req_pos;
          M3.req_pos = Xmovment(P1);
          system_err = motorGo(M1, M2, M3);
        }
        else if (coord == 2)
        {
          Serial.println("\nGoing to position 2");
          M1.req_pos = Ymovment(P2);
          M2.req_pos = M1.req_pos;
          M3.req_pos = Xmovment(P2);
          system_err = motorGo(M1, M2, M3);
        }
        else if (coord == 3)
        {
          Serial.println("\nGoing to position 3");
          M1.req_pos = Ymovment(P3);
          M2.req_pos = M1.req_pos;
          M3.req_pos = Xmovment(P3);
          system_err = motorGo(M1, M2, M3);
        }
        else if (coord == 4)
        {
          Serial.println("\nGoing to position 4");
          M1.req_pos = Ymovment(P4);
          M2.req_pos = M1.req_pos;
          M3.req_pos = Xmovment(P4);
          system_err = motorGo(M1, M2, M3);
        }
        else if (coord == 5)
        {
          Serial.println("\nGoing to position 5");
          M1.req_pos = Ymovment(P5);
          M2.req_pos = M1.req_pos;
          M3.req_pos = Xmovment(P5);
          system_err = motorGo(M1, M2, M3);
        }
        else
        {
          Serial.println("*** Wrong key ***\n");
        }
      }
      else Serial.println("EMERGENCY STOP");
    }
    inputString = "";                                                          // clear the string for the next loop
    stringComplete = false;                                                    // set the string flag down
  }
  delay(5);
}

/*----------------------------------------------------------------------------------------------------------------*/

// communication ISR
void serialEvent()
{
  while (Serial.available())
  {
    char inChar = (char)Serial.read();                                         // get the new char:
    if (inChar == '\n')
      stringComplete = true;                                                   // if the incoming character is a newline, set a flag so the main loop can do something about it:
    else
      inputString += inChar;                                                   // add it to the inputString:
  }
}

// // -- for 200 clicks
// int32_t Ymovment(Points &PY)
// {
//   return constrain(PY.y, 0, 1150) * 20;                                         // calculate and constrain mm to clicks
// }

// int32_t Xmovment(Points &PX)
// {
//   return constrain(PX.x, 0, 1124) * 20;                                         // calculate and constrain mm to clicks
//}
// -- for 200 clicks
int32_t Ymovment(Points &PY)
{
  return constrain(PY.y, 0, 1150);                                         // calculate and constrain mm to clicks
}

int32_t Xmovment(Points &PX)
{
  return constrain(PX.x, 0, 1124);                                         // calculate and constrain mm to clicks
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------
int motorGo(Motor &M_A, Motor &M_B, Motor &M_C)
{
  Y1stepper.setStepsPerMillimeter(20 * 1);
  Y2stepper.setStepsPerMillimeter(20 * 1);
  Xstepper.setStepsPerMillimeter(20 * 1);

  Y1stepper.setSpeedInMillimetersPerSecond(320.0);
  Y2stepper.setSpeedInMillimetersPerSecond(320.0);
  Xstepper.setSpeedInMillimetersPerSecond(320.0);

  Y1stepper.setAccelerationInMillimetersPerSecondPerSecond(10.0);
  Y2stepper.setAccelerationInMillimetersPerSecondPerSecond(10.0);
  Xstepper.setAccelerationInMillimetersPerSecondPerSecond(10.0);

  Y1stepper.moveToPositionInMillimeters(M1.req_pos);
  Y2stepper.moveToPositionInMillimeters(M2.req_pos);
  Xstepper.moveToPositionInMillimeters(M3.req_pos);

  M1.cur_pos = Y1stepper.getCurrentPositionInMillimeters();
  M2.cur_pos = Y1stepper.getCurrentPositionInMillimeters();
  M3.cur_pos = Y1stepper.getCurrentPositionInMillimeters();

  
  if (Serial.available()) return 1;
  delay(5);
  Serial.println("Done my move");
  Serial.print("Y axis position: ");
  Serial.print(Y1stepper.getCurrentPositionInMillimeters());
  Serial.println(" mm");
  Serial.print("X axis position: ");
  Serial.print(Xstepper.getCurrentPositionInMillimeters());
  Serial.println(" mm\n");
  return 0;
}


// -------------------------------------------------------------------------------------------------------------------------------------------------------
int goHome(Motor &M_A, Motor &M_B, Motor &M_C)
{
    const float homingSpeedInMMPerSec = 10.0;
    const int directionTowardHome = -1;
  while ((PINB >> 5 & B00100000 >> 5) || (PINB >> 6 & B01000000 >> 6))        //digital read of pins 11&12
  {
    const float maxHomingDistanceInMM = 1150;
    Y1stepper.setStepsPerMillimeter(20 * 1);
    Y2stepper.setStepsPerMillimeter(20 * 1);
    Y1stepper.setSpeedInMillimetersPerSecond(100.0);
    Y2stepper.setSpeedInMillimetersPerSecond(100.0);
    Y1stepper.setAccelerationInMillimetersPerSecondPerSecond(10.0);
    Y2stepper.setAccelerationInMillimetersPerSecondPerSecond(10.0);
    Y1stepper.moveToHomeInMillimeters(directionTowardHome, homingSpeedInMMPerSec, maxHomingDistanceInMM, M1.Home_mask);
    Y2stepper.moveToHomeInMillimeters(directionTowardHome, homingSpeedInMMPerSec, maxHomingDistanceInMM, M2.Home_mask);
    // PORTH &= ~M_A.dir_mask;                                                   //backword
    // PORTH &= ~M_B.dir_mask;                                                   //backword
    // PORTE |= M_A.pulse_mask;
    // PORTE |= M_B.pulse_mask;
    // delayMicroseconds(homeSpeed);
    // if (PINB >> 5 & B00100000 >> 5)PORTE &= ~M_A.pulse_mask;                  // if pin 11 is true give  pulse would ya
    // if (PINB >> 6 & B01000000 >> 6)PORTE &= ~M_B.pulse_mask;                  // if pin 12 is true give  pulse would ya   
    // delayMicroseconds(homeSpeed);
    if (Serial.available()) return 1;                                         // if anykey is pressed stop the loop tnd return true to the system error
  }
  M_A.cur_pos = 0;
  M_A.req_pos = 0;
  M_B.cur_pos = 0;
  M_B.req_pos = 0;
  
  while((PINB >> 7 & B10000000 >> 7))                                         //digital read of pins 13
  {
    const float maxHomingDistanceInMM = 1124;
    Xstepper.setStepsPerMillimeter(20 * 1);
    Xstepper.setSpeedInMillimetersPerSecond(100.0);
    Xstepper.setAccelerationInMillimetersPerSecondPerSecond(10.0);
    Xstepper.moveToHomeInMillimeters(directionTowardHome, homingSpeedInMMPerSec, maxHomingDistanceInMM, M3.Home_mask);
    // PORTH |= M_C.dir_mask;
    // PORTG |= M_C.pulse_mask;
    // delayMicroseconds(homeSpeed);
    // if (PINB >> 7 & B10000000 >> 7)PORTG &= ~M_C.pulse_mask; 
    // delayMicroseconds(homeSpeed);
    if (Serial.available()) return 1;                                         // if anykey is pressed stop the loop tnd return true to the system error 
  }
  M_C.cur_pos = 0;
  M_C.req_pos = 0;
  Serial.println("Honey i'm Home!");                                          // if all goes well - set the cur pos and req pos to 0 AKA home
  Serial.print("Y axis position: ");
  Serial.print(M_A.cur_pos / 20);
  Serial.println(" mm");
  Serial.print("X axis position: ");
  Serial.print(M_C.cur_pos / 20);
  Serial.println(" mm\n");
  return 0;
}
