#ifndef SENSOR_READING
#define SENSOR_READING

typedef int ReadingType;

struct SensorReading {
  ReadingType thumb;
  ReadingType index;
  ReadingType middle;
  ReadingType ring;
  ReadingType pinky;

public:
  void copy(const SensorReading &other);
};

ReadingType readingDist(const SensorReading &a, const SensorReading &b);

#endif