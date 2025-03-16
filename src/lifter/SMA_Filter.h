/*
A moving average, also called rolling average, rolling mean or running average, 
is a type of finite impulse response filter (FIR) used to analyze a set of values
by creating a series of averages of different subsets of the full data set.
Modified after: https://github.com/sebnil/Moving-Avarage-Filter--Arduino-Library-
*/

#define MAXSMADATAPOINTS 20

class SMA_Filter {
private:
  float values[MAXSMADATAPOINTS];
  int k; // k stores the index of the current array read
  int dataPointsCount;
public:

// Constructor
SMA_Filter(unsigned int newDataPointsCount) {
  k = 0; //initialize so that we start to write at index 0
  if (newDataPointsCount < MAXSMADATAPOINTS)
    dataPointsCount = newDataPointsCount;
  else
    dataPointsCount = MAXSMADATAPOINTS;
  for (uint8_t i = 0; i < dataPointsCount; i++)
    values[i] = 0; // fill the array with 0's
}

void presetFilter(float current_value) {
  for (uint8_t i = 0; i < dataPointsCount; i++)
    values[i] = current_value; // fill the array with current_value
}

float getFilteredValue(float in) {
  float out = 0;
  values[k] = in;
  k = (k + 1) % dataPointsCount;
  for (uint8_t i = 0; i < dataPointsCount; i++)
    out += values[i];
  return out / dataPointsCount;
}

};