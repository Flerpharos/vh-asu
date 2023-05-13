#include "reading.h"
#include <list>
#include <unordered_map>

#define MAX_QUEUE_LENGTH 10
#define STABILITY_CUTOFF 10
#define POLL_WAIT_TIME 10

std::list<SensorReading> history;
const std::unordered_map<SensorReading, char> mapping{{SensorReading{}, 'a'}};
char detected;

SensorReading min;
SensorReading max;
SensorReading dist;

SensorReading *getReading() {
  return (SensorReading *)new SensorReading{0, 0, 0, 0, 0};
}

void calibrateReading(SensorReading &reading) {
  reading.pinky = (ReadingType)(static_cast<float>(reading.pinky - min.pinky) /
                                dist.pinky * 100);
  reading.ring = (ReadingType)(static_cast<float>(reading.ring - min.ring) /
                               dist.ring * 100);
  reading.middle =
      (ReadingType)(static_cast<float>(reading.middle - min.middle) /
                    dist.middle * 100);
  reading.index = (ReadingType)(static_cast<float>(reading.index - min.index) /
                                dist.index * 100);
  reading.thumb = (ReadingType)(static_cast<float>(reading.thumb - min.thumb) /
                                dist.thumb * 100);
}

void raiseDetection(char detected); // alert webserver or whatever that we have
                                    // detected a new character

void wait(long time); // this is assuming a wait function in milliseconds. Arya,
                      // remove this or change as needed

char getCharacterOf(const SensorReading &reading) {
  char currentCharacter;
  float minDistance = FLT_MAX;

  for (auto read : mapping) {
    float currentDistance = readingDist(read.first, reading);

    if (currentDistance < minDistance) {
      currentCharacter = read.second;
      minDistance = currentDistance;
    }
  }

  if (minDistance != FLT_MAX)
    return currentCharacter;
  else
    return 0;
}

void loop() {
  SensorReading &reading = *getReading();
  calibrateReading(reading);

  history.push_back(reading);

  while (history.size() > MAX_QUEUE_LENGTH)
    history.pop_front();

  if (history.size() == MAX_QUEUE_LENGTH &&
      readingDist(history.front(), history.back()) < STABILITY_CUTOFF) {
    char temp = getCharacterOf(reading);

    if (temp != detected) {
      detected = temp;
      raiseDetection(detected);
    }
  } else if (detected != 0) {
    detected = 0;
  }

  wait(POLL_WAIT_TIME);
}
