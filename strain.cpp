#include "reading.h"
#include <list>
#include <unordered_map>

#define MAX_QUEUE_LENGTH 10
#define STABILITY_CUTOFF 10
#define POLL_WAIT_TIME 10          // in ms
#define MAX_CALIBRATING_TIME 10000 // in ms

std::list<SensorReading> history;
const std::unordered_map<SensorReading, char> mapping{
    {SensorReading{}, 'a'}, // this kind of thing, but actually
    {SensorReading{}, 'b'}  // having real sensor reading sets (calibrated,
                            // assuming that's a thing)
};
char detected;

bool calibrating = false; // state (are we currently calibrating)
long calibratingTime = 0; // time spent calibrating

SensorReading min{};
SensorReading max{};
SensorReading dist; // max - min (calculate after calibrations)

SensorReading *getReading() {
  int thumb = 0; // Arya, fill these in with your arduino things
  int index = 0;
  int middle = 0;
  int ring = 0;
  int pinky = 0;

  return (SensorReading *)new SensorReading{thumb, index, middle, ring, pinky};
}

void calibrateReading(SensorReading &reading) { // for getting calibrated values
                                                // from raw sensor values
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
                                    // detected a new character. I didn't bother
                                    // doing this; Arya, you figure it out lmao

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

  if (calibrating) { // for calibrating, which you said we don't need to do.
                     // the numbers that we were given look like they were
                     // set for values between 1-100, so they won't work
                     // uncalibrated, iirc

    if (calibratingTime == 0) {
      max.copy(reading);
      min.copy(reading);
    } else {
      replaceByLargest(max, reading);
      replaceBySmallest(min, reading);
    }

    if (calibratingTime > MAX_CALIBRATING_TIME) { // finished calibrating
      calibrating = false;

      dist = *new SensorReading{};
      dist.copy(max);
      dist -= min;

      return;
    }

    calibratingTime += POLL_WAIT_TIME;
    wait(POLL_WAIT_TIME);

    return;
  }

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
