#include <Regexp.h>

bool continous = false; //Serial outputs all measurements without needing to send the "f?" command

unsigned int sampling_time = 500;  
unsigned int serial_check_time = 10; // checks the serial port for input every serial_check_time milliseconds

bool is_measuring = false;
bool display_freq = false;
bool serial_interrupt = false; // true serial command was detected

int input_pin_1 = 2;     //This is the input pin on the first device
int interrupt_1 = digitalPinToInterrupt(input_pin_1);
volatile int count_1; //This integer needs to be set as volatile to ensure it updates correctly during the interrupt process.
volatile unsigned long time_trig_1;  // time when measurement was started from  input_pin_1
volatile unsigned long time_end_1;  // time when measurement was ended from  input_pin_1
float frequency_1 = 0;

int input_pin_2 = 3;    //This is the input pin on the second device
int interrupt_2 = digitalPinToInterrupt(input_pin_2);
volatile int count_2; //This integer needs to be set as volatile to ensure it updates correctly during the interrupt process.
volatile unsigned long time_trig_2; // time when measurement was started from  input_pin_2
volatile unsigned long time_end_2; // time when measurement was ended from  input_pin_2
float frequency_2 = 0; 

MatchState ms;

void setup() {
  pinMode(input_pin_1, INPUT);   
  pinMode(input_pin_2, INPUT);           
        
  Serial.begin(9600);  
}

void loop() {
  check_serial();
  is_measuring = true;

  if (is_measuring){
    measure_frequency();
    is_measuring = false;
  }

  if(continous){
    Serial.print(frequency_1);
    Serial.println();
  }
  delay(100);
}

/* CHEKCS FOR SERIAL INPUT */
void check_serial(){
  if (Serial.available() > 0) {
    serial_interrupt = true;
    String ser_input = Serial.readString();
    ser_input.trim();

    ms.Target(ser_input.c_str());
    
    if (ser_input == "f?"){ // "f?" - outputs the frequency from input_pin_1
      Serial.print(frequency_1);
      Serial.println();
    }

    else if (ser_input == "f1?"){ // "f1?" - outputs the frequency from input_pin_1"
      Serial.print(frequency_1);
      Serial.println();
    }

    else if (ser_input == "f2?"){ // "f2?" - outputs the frequency from input_pin_2"
      Serial.print(frequency_2);
      Serial.println();
    }

    else if (ser_input == "*idn?" || ser_input == "*IDN?"){
      Serial.println("Water Flow Monitor");
    }
    else if (ser_input == "t?"){ // "t? - outputs the sampling time"
      Serial.println(sampling_time);
    }
    else if (ms.Match ("^t [+]?[0-9]\d*") > 0){  // "t [time in ms]" - set sampling time
      ser_input.remove(0, 2);
      sampling_time = ser_input.toInt();
    //  Serial.println(measurement_time);
    }

    else if (ms.Match ("^c [+]?[0-9]\d*") > 0){ // "c [n]"" - if n > 0 - enter continous mode. c 0 disable continous mode
      ser_input.remove(0, 2);
      continous = ser_input.toInt() > 0;
    }

  }
  else{
    serial_interrupt = false;
  }
}

void count_pulses_1(){

  if(count_1 == 0){ // if first interrupt, save the time of first interrupt
    time_trig_1 = micros();
  }
  else{
    time_end_1 = micros();  // if not, update the time_end_1 variable with the current time
  }
  count_1++;

}

void count_pulses_2(){
  if(count_2 == 0){ /// if first interrupt, save the time of first interrupt
    time_trig_2 = micros();
  }
  else{
    time_end_2 = micros();  // if not, update the time_end_2 variable with the current time
  }
  count_2++;

}

void measure_frequency(){
  time_end_1 = 0; // Reset the times
  time_trig_1 = 0; // Reset the times

  count_1 = 0;      // Reset the counter
  attachInterrupt(interrupt_1, count_pulses_1, RISING);  //add interrupt from pin

  /* UNCOMMENT BELOW TO ADD SECOND DEVICE */
  // attachInterrupt(interrupt_2, count_pulses_2, RISING);  //add interrupt from pin
  // count_2 = 0;      // Reset the counter

  delay(10);

  /* WAIT serial_check_time ms FOR THE MEASUREMEN TO COMPLETE. DURING THIS TIME, CHECK FOR SERIAL INPUT */
  unsigned long delay_start_time = millis(); //time at which the measurement started. Not the same as time_trig_n, which is the time of the first interrupt. 
  while(millis() - delay_start_time <= sampling_time - serial_check_time){ //while time elapsed since the measurement started is less than the sampling time - serial check time. The total wait time will be approximately sampling_time
    /* check serial every serial_check_time ms. If serial input detected, it aborts the current measurement and executes the command */
    /* if the command asks for a measurement ("f?" for example), the LAST COMPLETED MEASUREMENT will be outputed and NOT the result of the last aborted measurement  */
    delay(serial_check_time); 
    check_serial(); 
    if(serial_interrupt) break;
  }


  if (!serial_interrupt){
    if(count_1 > 0){
      /* calculate the frequency. In the denominator the precise time difference (time_end_1 - time_trig_1), measured using the micros() command is used */
      /* The numerator is (count_1 - 1) beacuse count_1 is the number of rising edges, and not the number of completed periods */
      frequency_1 = 1000* 1000.0 * (count_1 - 1) / (float)(time_end_1 - time_trig_1); 
    }
    else{
      frequency_1 = 0;
    }
  }

  detachInterrupt(interrupt_1); //remove interrupt from pin

  /* UNCOMMENT BELOW TO ADD SECOND DEVICE */
  //if(count_2 > 0){
    /* calculate the frequency. In the denominator the precise time difference (time_end_1 - time_trig_1), measured using the micros() command is used */
    /* The numerator is (count_1 - 1) beacuse count_1 is the number of rising edges, and not the number of completed periods */
  //  frequency_2 = 1000* 1000.0 * (count_2 - 1) / (float)(time_end_2 - time_trig_2); 
  //}
  //else{
  //  frequency_2 = 0;
  //}
  //detachInterrupt(interrupt_2); //remove interrupt from pin



}

