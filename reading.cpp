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

void SensorReading::operator-=(const SensorReading &other) {
  this->index -= other.index;
  this->pinky -= other.pinky;
  this->middle -= other.middle;
  this->ring -= other.ring;
  this->thumb -= other.thumb;
}

void replaceByLargest(SensorReading &to, const SensorReading &from) {
  to.index = from.index > to.index ? from.index : to.index;
  to.middle = from.middle > to.middle ? from.middle : to.middle;
  to.ring = from.ring > to.ring ? from.ring : to.ring;
  to.pinky = from.pinky > to.pinky ? from.pinky : to.pinky;
  to.thumb = from.thumb > to.thumb ? from.thumb : to.thumb;
}
void replaceBySmallest(SensorReading &to, const SensorReading &from) {
  to.index = from.index < to.index ? from.index : to.index;
  to.middle = from.middle < to.middle ? from.middle : to.middle;
  to.ring = from.ring < to.ring ? from.ring : to.ring;
  to.pinky = from.pinky < to.pinky ? from.pinky : to.pinky;
  to.thumb = from.thumb < to.thumb ? from.thumb : to.thumb;
}
