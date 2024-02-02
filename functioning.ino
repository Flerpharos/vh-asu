#include <Arduino.h>
#include <Vector.h>
#include <ArxContainer.h>

#define MAX_QUEUE_LENGTH 5
#define STABILITY_CUTOFF 200
#define POLL_WAIT_TIME 2000          // in ms
#define MAX_CALIBRATING_TIME 10000 // in ms

typedef int ReadingType;

typedef struct SensorReading {
  ReadingType pinky;
  ReadingType ring;
  ReadingType middle;
  ReadingType index;
  ReadingType thumb;

public:
  void copy(const SensorReading &other);
  void operator-=(const SensorReading &other);
  bool const operator <( const SensorReading &other);
  bool const operator ==(const SensorReading &other);
  constexpr bool isInvalid();
  void print();
};

const ReadingType readingDist(const SensorReading &a, const SensorReading &b) {
  ReadingType pinky = a.pinky - b.pinky;
  ReadingType ring = a.ring - b.ring;
  ReadingType middle = a.middle - b.middle;
  ReadingType index = a.index - b.index;
  ReadingType thumb = a.thumb - b.thumb;
  
  const double temp = sqrt(0.0f + pinky * pinky + ring * ring + middle * middle +
                                index * index + thumb * thumb);

  return temp;
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

bool const SensorReading::operator<(const SensorReading &other) {
  return (this->pinky * this->pinky + this->ring * this->ring + this->middle * this->middle + this->index * this->index + this->thumb * this->thumb) <
            (other.pinky * other.pinky + other.ring * other.ring + other.middle * other.middle + other.index * other.index + other.thumb * other.thumb);
}

bool const SensorReading::operator==(const SensorReading &other) {
  return this->pinky == other.pinky && this->ring == other.ring && this->middle == other.middle && this->index == other.index && this->thumb == other.thumb;
}

constexpr bool isValid(ReadingType val) {
  return val >= 0 && val <= 1023;
}

constexpr bool SensorReading::isInvalid() {
  return !(isValid(this->pinky) && isValid(this->ring) && isValid(this->middle) && isValid(this->index) && isValid(this->thumb));
}

void SensorReading::print() {
  Serial.print(this->pinky);
  Serial.print(" ");
  Serial.print(this->ring);
  Serial.print(" ");
  Serial.print(this->middle);
  Serial.print(" ");
  Serial.print(this->index);
  Serial.print(" ");
  Serial.println(this->thumb);
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

SensorReading history_array[MAX_QUEUE_LENGTH];
Vector<SensorReading> history(history_array);
const arx::map<SensorReading, char, 26> mapping {
    {SensorReading{50, 30, 50, 10, 10}, 'a'}, // this kind of thing, but actually
    {SensorReading{0, 0, 0, 0, 30}, 'b'},
    {SensorReading{15, 15, 15, 10, 3}, 'c'},
    {SensorReading{40, 40, 40, 0, 5}, 'd'},
    {SensorReading{30, 30, 50, 15, 30}, 'e'},
    {SensorReading{0, 0, 0, 20, 20}, 'f'},
    {SensorReading{30, 40, 30, 0, 10}, 'g'},
    {SensorReading{40, 40, 0, 0, 10}, 'h'},
    {SensorReading{0, 30, 21, 30, 10}, 'i'},
    {SensorReading{30, 30, 0, 0, 10}, 'k'},
    {SensorReading{50, 40, 60, 50, 5}, 's'},
    {SensorReading{40, 30, 10, 0, 0}, 'l'},
    {SensorReading{25, 20, 5, 5, 10}, 'm'},
    {SensorReading{15, 10, 0, 0, 25}, 'n'},
    {SensorReading{30, 30, 20, 10, 10}, 'o'},
    // {SensorReading{20, 20, 0, 0, 0}, 'p'},
    {SensorReading{30, 30, 35, 0, 20}, 'q'},
    {SensorReading{20, 27, 0, 5, 10}, 'r'},
    {SensorReading{30, 30, 35, 0, 20}, 't'},
    {SensorReading{20, 0, 0, 0, 0}, 'w'},
    {SensorReading{10, 5, 5, 0, 20}, 'x'},
    {SensorReading{0, 15, 15, 5, 0}, 'y'}
      // having real sensor reading sets (calibrated,
                            // assuming that's a thing)
};
char detected;

bool calibrating = false; // state (are we currently calibrating)
long calibratingTime = 0; // time spent calibrating
bool printOutput = true;

SensorReading _min{0, 0, 0, 0, 0};
SensorReading _max{1023, 1023, 1023, 1023, 1023};
SensorReading dist{1023, 1023, 1023, 1023, 1023}; // max - min (calculate after calibrations)

SensorReading *getReading() {
  int thumb = analogRead(A0); // Arya, fill these in with your arduino things
  int index = analogRead(A1);
  int middle = analogRead(A2);
  int ring = analogRead(A3);
  int pinky = analogRead(A4);

  return (SensorReading *)new SensorReading{pinky, ring, middle, index, thumb};
}

void calibrateReading(SensorReading &reading) { // for getting calibrated values
                                                // from raw sensor values
  reading.pinky = static_cast<ReadingType>(static_cast<float>(reading.pinky - _min.pinky) /
                    dist.pinky * 100);
  reading.ring = static_cast<ReadingType>(static_cast<float>(reading.ring - _min.ring) /
                    dist.ring * 100);
  reading.middle = static_cast<ReadingType>(static_cast<float>(reading.middle - _min.middle) /
                      dist.middle * 100);
  reading.index = static_cast<ReadingType>(static_cast<float>(reading.index - _min.index) /
                      dist.index * 100);
  reading.thumb = static_cast<ReadingType>(static_cast<float>(reading.thumb - _min.thumb) /
                      dist.thumb * 100);
}

void raiseDetection(char detected) {
  Serial.print(detected);
} // alert webserver or whatever that we have
                                    // detected a new character. I didn't bother
                                    // doing this; Arya, you figure it out lmao

void wait(long time) {
  delay(time);
} // this is assuming a wait function in milliseconds. Arya,
                      // remove this or change as needed

char getCharacterOf(const SensorReading &reading) {
  char currentCharacter;
  float minDistance = INT8_MAX;

  if (reading.isInvalid()) return -1;

  for (auto read : mapping) {
    float currentDistance = readingDist(read.first, reading);

    // Serial.print(read.second);
    // Serial.print(' ');
    // Serial.println(currentDistance);

    if (currentDistance < minDistance) {
      currentCharacter = read.second;
      minDistance = currentDistance;
    }
  }

  if (minDistance != INT8_MAX)
    return currentCharacter;
  -
  Serial.println(minDistance);
  return 0;
}

void setup() {
  Serial.begin(9600);
  Serial.println("Starting\n\n\n");
}

void loop() {
  SensorReading &reading = *getReading();

  if (calibrating) { // for calibrating, which you said we don't need to do.
                     // the numbers that we were given look like they were
                     // set for values between 1-100, so they won't work
                     // uncalibrated, iirc

    if (calibratingTime == 0) {
      _max.copy(reading);
      _min.copy(reading);
    } else {
      replaceByLargest(_max, reading);
      replaceBySmallest(_min, reading);
    }

    if (calibratingTime > MAX_CALIBRATING_TIME) { // finished calibrating
      calibrating = false;

      dist = *new SensorReading{};
      dist.copy(_max);
      dist -= _min;

      return;
    }

    calibratingTime += POLL_WAIT_TIME;
    wait(POLL_WAIT_TIME);

    return;
  }

  calibrateReading(reading);

  // if (Serial.available()) {
  //   String text = Serial.readString();
  //   if (text.equals("stop")) printOutput = false;
  //   else if (text.equals("start")) printOutput = true;
  // }

  // if (printOutput) {
  //   reading.print();
  // }

  history.push_back(reading);

  while (history.size() > MAX_QUEUE_LENGTH) {
    delete &history.front();
    history.remove(0);
  }

  // Serial.println(readingDist(history.front(), history.back()));

  if (history.size() < MAX_QUEUE_LENGTH) Serial.println("Calibrating...");

  if (history.size() == MAX_QUEUE_LENGTH &&
      readingDist(history.front(), history.back()) < STABILITY_CUTOFF) {
    char temp = getCharacterOf(reading);

    if (temp <= 0) {
      Serial.print("Error ");
      Serial.println(static_cast<int>(temp));
      // reading.print();
    }

    // Serial.println(static_cast<int>(temp));

    // Serial.println("Reading: " + temp);

    if (temp != detected) {
      detected = temp;
      raiseDetection(detected);
    }
  } else if (detected != 0) {
    detected = 0;
  }

  wait(POLL_WAIT_TIME);
}
