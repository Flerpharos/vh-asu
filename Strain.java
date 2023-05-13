import java.util.LinkedList;
import java.util.HashMap;

public class Strain {
    public class SensorReading {
        public final int ring;
        public final int middle;
        public final int index;
        public final int thumb;
        public final int pinky;

        public SensorReading(int pinky, int ring, int middle, int index, int thumb) {
            this.pinky = pinky;
            this.thumb = thumb;
            this.index = index;
            this.middle = middle;
            this.ring = ring;
        }

        // vector distance
        public float getDistance(SensorReading reading) {
            float p = pinky - reading.pinky;
            float t = thumb - reading.thumb;
            float pt = index - reading.index;
            float m = middle - reading.middle;
            float r = ring - reading.ring;

            return (float) Math.sqrt(p * p + t * t + pt * pt + m * m + r * r);
        }
    }

    static HashMap<SensorReading, Character> mapping; // mapping between sensor readings and letters
    LinkedList<SensorReading> history = new LinkedList<>();
    public char detected; // the character that has been detected (should use a getter instead)

    // for calibration
    public SensorReading max;
    public SensorReading min;
    public SensorReading dist;

    public static final int maxQueueLength = 10;
    public static final int letterDetectionDistanceCutoff = 10;

    // in milliseconds
    public static final int pollWaitTime = 100; // time between checking sensor
    public static final int wordDetectionPauseGap = 500; // ignored

    public Strain(SensorReading inputPins) {
       // pins = inputPins;

        calibrateSensors();

        dist = new SensorReading(
                max.pinky - min.pinky,
                max.ring - min.ring,
                max.middle - min.middle,
                max.index - min.index,
                max.thumb - min.thumb);
    }

    static Character getCharacter(SensorReading reading) {
        // get closest sensor reading and corresponding character
        SensorReading closestReading=null;
        float minDistance = Float.MAX_VALUE;

        for (SensorReading read : mapping.keySet()) {
            float currentDistance = reading.getDistance(read);

            if (currentDistance < minDistance) {
                closestReading = read;
                minDistance = currentDistance;
            }
        }

        if (closestReading != null) return mapping.get(closestReading);
        return '\0';
    }

    public void calibrateSensors() {
        //sets max and min for each finger
    }

    public SensorReading getReading() {
        //gets reading from sensors
        return null;
    }

    public SensorReading getCalibratedReading(SensorReading reading) {
        return new SensorReading(
                (reading.pinky - min.pinky) / dist.pinky,
                (reading.ring - min.ring) / dist.ring,
                (reading.middle - min.middle) / dist.middle,
                (reading.index - min.index) / dist.index,
                (reading.thumb - min.thumb) / dist.thumb);
    }

    // main loop
    public void loop() {
        SensorReading uncalibrated = getReading();
        SensorReading calibrated = getCalibratedReading(uncalibrated);

        // keep history queue length under maxQueueLength
        history.add(calibrated);
        while (history.size() > maxQueueLength)
            history.removeFirst();

        // if the change in the last ten readings is minimal
        if (history.size() >= maxQueueLength && history.getFirst().getDistance(history.getLast()) 
            < letterDetectionDistanceCutoff) {
                detected = getCharacter(calibrated);
        }

        wait(pollWaitTime);
    }

    // function for pollWaitTime above
    public void wait(int time){

    }

}