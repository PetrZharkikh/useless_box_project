#include <Servo.h>

const int pin_sw      = 5; 
const int pin_trig    = 6; 
const int pin_echo    = 7; 
const int pin_botserv = 10;
const int pin_topserv = 11;

Servo servo_top; 
Servo servo_bot; 

float   distance  = 0; 
int     sw_state  = 0; 
boolean switching = false; 

const int bot_max_angle = 173;                 
const int bot_min_angle = 2;        
const int top_max_angle = 162;                
const int top_min_angle = 10;               
const int max_dist = 100;     

const float threshold_percent = 0.93; 
const float threshCommit = threshold_percent * bot_min_angle ;  

void setup() 
{
  pinMode(pin_trig, OUTPUT);
  pinMode(pin_echo, INPUT);

  servo_top.attach(pin_botserv);              
  servo_bot.attach(pin_topserv);

  servo_top.write(top_min_angle);        
  servo_bot.write(bot_min_angle);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(pin_sw, INPUT_PULLUP);

  Serial.begin(9600);
  Serial.println("Device is ready");
  
  if (digitalRead(pin_sw) == HIGH && sw_state == 0) 
  {
    Serial.println("Upon boot, the switch was seen as turned on. Resetting it...");

    switching = true;
    sw_state = 1;

    servo_top.write(top_max_angle); 
    servo_bot.write(bot_min_angle);

    while (switching == true) 
      if (digitalRead(pin_sw) == LOW) 
      {
        sw_state = 0;
        switching = false;

        servo_top.write(top_min_angle); 
      }
  }

  Serial.println("Starting the main loop");
}

void loop() 
{
  if (digitalRead(pin_sw) == HIGH) 
  {
    Serial.println("The switch was toggled before device was able to do it");

    switching = true;
    resetSwitch(); 
  }

  distance = get_dist();
  if (distance < max_dist & distance > 0) 
  {
    digitalWrite(LED_BUILTIN, HIGH);
    bot_arm_update(distance);
    delay(15);                                                                                                                                            
  }
  else
  {
    digitalWrite(LED_BUILTIN, LOW);
    servo_bot.write(bot_min_angle); 
  }
}

float get_dist()
{                                                                                                                               
  digitalWrite(pin_trig, LOW);
  delayMicroseconds(5);
  digitalWrite(pin_trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pin_trig, LOW);

  float duration = pulseIn(pin_echo, HIGH);
  float dist = (duration * .034) / 2;    

  return dist;
}


void bot_arm_update(int hand_dist)
{
  float motor_pos = map(hand_dist, 0, max_dist, bot_max_angle, bot_min_angle); 

  Serial.print(distance);
  Serial.print(", ");
  Serial.println(motor_pos);

  if (distance < 7)
    commit();
  else
    servo_bot.write(motor_pos);
}

void commit()
{
  Serial.println("Committing to pressing the switch...");
  servo_bot.write(bot_max_angle);

  while (sw_state == 0)
    if (digitalRead(pin_sw) == HIGH)
    {
      sw_state = 1;
      switching = true;

      servo_bot.write(bot_min_angle);
      Serial.println("Switch has been toggled by the bottom arm");
    }

  resetSwitch();
}

void resetSwitch()
{
  Serial.println("Resetting switch...");
  servo_bot.write(bot_min_angle);
  servo_top.write(top_max_angle);

  while (switching == true) 
    if (digitalRead(pin_sw) == LOW) 
    {
      sw_state = 0;
      switching = false;

      servo_top.write(top_min_angle);  
      Serial.println("Switch has been toggled back by the top arm");
    }

  Serial.println("Switch should be reset now");
}
