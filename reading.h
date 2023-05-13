#ifndef SENSOR_READING
#define SENSOR_READING

typedef int ReadingType;

struct SensorReading {
  ReadingType thumb;
  ReadingType index;
  ReadingType middle;
  ReadingType ring;
  ReadingType pinky;
};

ReadingType readingDist(const SensorReading &a, const SensorReading &b);

#endif