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
