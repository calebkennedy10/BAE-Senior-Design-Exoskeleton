// Assessment to Measure Exoskeleton Effectiveness for Industrial Applications
// reference code: https://github.com/upsidedownlabs/BioAmp-EXG-Pill

#include <math.h>
#include <CircularBuffer.h>

#define SAMPLE_RATE_EMG 500
#define SAMPLE_RATE_ECG 125
#define BAUD_RATE 9600
#define BUFFER_SIZE 128
#define DATA_LENGTH 16

#define INPUT_PIN_ECG A0
#define INPUT_PIN_EMG1 A1
#define INPUT_PIN_EMG2 A2
#define INPUT_PIN_EMG3 A3
#define INPUT_PIN_EMG4 A4
#define INPUT_PIN_EMG5 A5
#define INPUT_PIN_EMG6 A6

int avg = 0;
bool peak = false;
int reading = 0;
float BPM = 0.0;
bool IgnoreReading = false;
bool FirstPulseDetected = false;
unsigned long FirstPulseTime = 0;
unsigned long SecondPulseTime = 0;
unsigned long PulseInterval = 0;
CircularBuffer<int,30> buffer;

int circular_buffer[BUFFER_SIZE];
int data_index, sum;


void setup() {
  // Serial connection begin
  Serial.begin(BAUD_RATE);
}

void loop() {
  // Calculate elapsed time
  static unsigned long past = 0;
  unsigned long present = micros();
  unsigned long interval = present - past;
  past = present;

  // Run timer
  static long timer_EMG = 0;
  static long timer_ECG = 0;
  timer_ECG -= interval;
  timer_EMG -= interval;

  // Sample EMG and get envelop
  if(timer_EMG < 0) {
    timer_EMG += 1000000 / SAMPLE_RATE_EMG;

    int sensor_value_EMG1 = analogRead(INPUT_PIN_EMG1);
    int signal_EMG1 = EMGFilter(sensor_value_EMG1);
    int envelop_EMG1 = getEnvelop(abs(signal_EMG1));

    int sensor_value_EMG2 = analogRead(INPUT_PIN_EMG2);
    int signal_EMG2 = EMGFilter(sensor_value_EMG2);
    int envelop_EMG2 = getEnvelop(abs(signal_EMG2));

    int sensor_value_EMG3 = analogRead(INPUT_PIN_EMG3);
    int signal_EMG3 = EMGFilter(sensor_value_EMG3);
    int envelop_EMG3 = getEnvelop(abs(signal_EMG3));

    int sensor_value_EMG4 = analogRead(INPUT_PIN_EMG4);
    int signal_EMG4 = EMGFilter(sensor_value_EMG4);
    int envelop_EMG4 = getEnvelop(abs(signal_EMG4));

    int sensor_value_EMG5 = analogRead(INPUT_PIN_EMG5);
    int signal_EMG5 = EMGFilter(sensor_value_EMG5);
    int envelop_EMG5 = getEnvelop(abs(signal_EMG5));

    int sensor_value_EMG6 = analogRead(INPUT_PIN_EMG6);
    int signal_EMG6 = EMGFilter(sensor_value_EMG6);
    int envelop_EMG6 = getEnvelop(abs(signal_EMG6));

    Serial.print(-100); // To freeze the lower limit of live plot
    Serial.print(",");
    Serial.print(150); // To freeze the upper limit of live plot
    Serial.print(",");

    //Serial.print(signal_EMG1);
    Serial.print(",");
    Serial.print(envelop_EMG1);
    Serial.print(",");

    //Serial.print(signal_EMG2);
    Serial.print(",");
    Serial.print(envelop_EMG2);
    Serial.print(",");

    //Serial.print(signal_EMG3);
    Serial.print(",");
    Serial.print(envelop_EMG3);
    Serial.print(",");

    //Serial.print(signal_EMG4);
    Serial.print(",");
    Serial.print(envelop_EMG4);
    Serial.print(",");

    //Serial.print(signal_EMG5);
    Serial.print(",");
    Serial.print(envelop_EMG5);
    Serial.print(",");

    //Serial.print(signal_EMG6);
    Serial.print(",");
    Serial.println(envelop_EMG6);

  }
  /*
  // Sample ECG
  if(timer_ECG < 0){
    timer_ECG += 1000000 / SAMPLE_RATE_ECG;
    // Sample and Nomalize input data (-1 to 1)
    float sensor_value_ECG = analogRead(INPUT_PIN_ECG);
    float signal_ECG = ECGFilter(sensor_value_ECG)/512;
    // Get peak
    peak = Getpeak(signal_ECG);

    if(peak && IgnoreReading == false){
        if(FirstPulseDetected == false){
          FirstPulseTime = millis();
          FirstPulseDetected = true;
        }
        else{
          SecondPulseTime = millis();
          PulseInterval = SecondPulseTime - FirstPulseTime;
          buffer.unshift(PulseInterval);
          FirstPulseTime = SecondPulseTime;
        }
        IgnoreReading = true;
      }
      if(!peak){
        IgnoreReading = false;
      }  
      if (buffer.isFull()){
        for(int i = 0 ;i < buffer.size(); i++){
          avg+=buffer[i];
        }
        BPM = (1.0/avg) * 60.0 * 1000 * buffer.size();
        avg = 0;
        buffer.pop();
        if (BPM < 240){
          Serial.print("BPM ");
          Serial.println(BPM);
          Serial.flush();
        }
      }  
  } */
}

// Envelop detection algorithm
int getEnvelop(int abs_emg)
{
  sum -= circular_buffer[data_index];
  sum += abs_emg;
  circular_buffer[data_index] = abs_emg;
  data_index = (data_index + 1) % BUFFER_SIZE;
  return (sum/BUFFER_SIZE) * 2;
}

bool Getpeak(float new_sample) {
  // Buffers for data, mean, and standard deviation
  static float data_buffer[DATA_LENGTH];
  static float mean_buffer[DATA_LENGTH];
  static float standard_deviation_buffer[DATA_LENGTH];
  
  // Check for peak
  if (new_sample - mean_buffer[data_index] > (DATA_LENGTH/2) * standard_deviation_buffer[data_index]) {
    data_buffer[data_index] = new_sample + data_buffer[data_index];
    peak = true;
  } else {
    data_buffer[data_index] = new_sample;
    peak = false;
  }

  // Calculate mean
  float sum = 0.0, mean, standard_deviation = 0.0;
  for (int i = 0; i < DATA_LENGTH; ++i){
    sum += data_buffer[(data_index + i) % DATA_LENGTH];
  }
  mean = sum/DATA_LENGTH;

  // Calculate standard deviation
  for (int i = 0; i < DATA_LENGTH; ++i){
    standard_deviation += pow(data_buffer[(i) % DATA_LENGTH] - mean, 2);
  }

  // Update mean buffer
  mean_buffer[data_index] = mean;

  // Update standard deviation buffer
  standard_deviation_buffer[data_index] =  sqrt(standard_deviation/DATA_LENGTH);

  // Update data_index
  data_index = (data_index+1)%DATA_LENGTH;

  // Return peak
  return peak;
}

// Band-Pass Butterworth IIR digital filter, generated using filter_gen.py.
// Sampling rate: 500.0 Hz, frequency: [74.5, 149.5] Hz.
// Filter is order 4, implemented as second-order sections (biquads).
// Reference: 
// https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.butter.html
// https://courses.ideate.cmu.edu/16-223/f2020/Arduino/FilterDemos/filter_gen.py
float EMGFilter(float input)
{
  float output = input;
  {
    static float z1, z2; // filter section state
    float x = output - 0.05159732*z1 - 0.36347401*z2;
    output = 0.01856301*x + 0.03712602*z1 + 0.01856301*z2;
    z2 = z1;
    z1 = x;
  }
  {
    static float z1, z2; // filter section state
    float x = output - -0.53945795*z1 - 0.39764934*z2;
    output = 1.00000000*x + -2.00000000*z1 + 1.00000000*z2;
    z2 = z1;
    z1 = x;
  }
  {
    static float z1, z2; // filter section state
    float x = output - 0.47319594*z1 - 0.70744137*z2;
    output = 1.00000000*x + 2.00000000*z1 + 1.00000000*z2;
    z2 = z1;
    z1 = x;
  }
  {
    static float z1, z2; // filter section state
    float x = output - -1.00211112*z1 - 0.74520226*z2;
    output = 1.00000000*x + -2.00000000*z1 + 1.00000000*z2;
    z2 = z1;
    z1 = x;
  }
  return output;
}

// Band-Pass Butterworth IIR digital filter, generated using filter_gen.py.
// Sampling rate: 125.0 Hz, frequency: [0.5, 44.5] Hz.
// Filter is order 4, implemented as second-order sections (biquads).
// Reference:
// https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.butter.html
// https://courses.ideate.cmu.edu/16-223/f2020/Arduino/FilterDemos/filter_gen.py
float ECGFilter(float input)
{
  float output = input;
  {
    static float z1, z2; // filter section state
    float x = output - 0.70682283*z1 - 0.15621030*z2;
    output = 0.28064917*x + 0.56129834*z1 + 0.28064917*z2;
    z2 = z1;
    z1 = x;
  }
  {
    static float z1, z2; // filter section state
    float x = output - 0.95028224*z1 - 0.54073140*z2;
    output = 1.00000000*x + 2.00000000*z1 + 1.00000000*z2;
    z2 = z1;
    z1 = x;
  }
  {
    static float z1, z2; // filter section state
    float x = output - -1.95360385*z1 - 0.95423412*z2;
    output = 1.00000000*x + -2.00000000*z1 + 1.00000000*z2;
    z2 = z1;
    z1 = x;
  }
  {
    static float z1, z2; // filter section state
    float x = output - -1.98048558*z1 - 0.98111344*z2;
    output = 1.00000000*x + -2.00000000*z1 + 1.00000000*z2;
    z2 = z1;
    z1 = x;
  }
  return output;
}
