#include "reading.h"

#include <cmath>

ReadingType readingDist(const SensorReading &a, const SensorReading &b) {
  ReadingType pinky = a.pinky - b.pinky;
  ReadingType ring = a.ring - b.ring;
  ReadingType middle = a.middle - b.middle;
  ReadingType index = a.index - b.index;
  ReadingType thumb = a.thumb - b.thumb;

  return (ReadingType)std::sqrt(pinky * pinky + ring * ring + middle * middle +
                                index * index + thumb * thumb);
}

void SensorReading::copy(const SensorReading &other) {
  this->pinky = other.pinky;
  this->ring = other.ring;
  this->middle = other.middle;
  this->index = other.index;
  this->thumb = other.thumb;
}
