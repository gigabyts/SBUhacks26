**Built at SB Hacks 2026**

Souvik’s Speed Sensing Super Safety System is a hardware prototype that uses an iPhone's accelerometer to detect rapid movement and automatically trigger a physical response using an Arduino-controlled servo motor.

By combining smartphone sensors, wireless communication, numerical integration, and embedded hardware, we explored how everyday devices can be repurposed into a low-cost motion monitoring system.

## 💡 Inspiration

What if your phone could detect how quickly you're moving and automatically interact with the physical world?

We wanted to explore a simple but interesting engineering challenge: **Can we use acceleration data from a smartphone to detect movement beyond a specified threshold and trigger a mechanical response?**

Rather than purchasing specialized motion-tracking equipment, we decided to use the sensors already built into an iPhone, an Arduino, and an inexpensive servo motor.

## ⚙️ What It Does

Souvik’s Speed Sensing Super Safety System uses the iPhone's accelerometer to monitor movement and wirelessly communicate that information to an Arduino UNO R4 WiFi.

The Arduino processes the acceleration measurements, numerically integrates them to produce a motion-related velocity estimate, and compares that estimate against a predefined threshold.

When the threshold is exceeded, an SG90 servo motor automatically activates.

**The result:** A proof-of-concept system that translates smartphone motion data into a physical hardware response.

## 🛠️ How We Built It

Our system uses four main components:

- **iPhone:** Collects real-time accelerometer measurements.
- **Arduino Cloud:** Transfers sensor readings from the iPhone to the microcontroller.
- **Arduino UNO R4 WiFi:** Processes acceleration data and determines when the threshold is reached.
- **SG90 Servo Motor:** Performs a programmed rotation sequence when triggered.

### System Architecture

```text
          📱 iPhone
               |
       Accelerometer Data
               |
          Arduino Cloud
               |
       Arduino UNO R4 WiFi
               |
      Numerical Integration
               |
       Threshold Detection
               |
          SG90 Servo
               |
       Mechanical Response
```

### The Mathematics

To estimate movement from acceleration, we implemented numerical integration using the trapezoidal rule:

\[
v_{n+1}=v_n+\frac{a_n+a_{n+1}}{2}\Delta t
\]

Our prototype uses an acceleration-magnitude measurement and compensates approximately for its observed 1 g resting baseline.

The resulting value is a **velocity-related proxy rather than a precise physical speed measurement**. This allowed us to experiment with integration, threshold detection, and automated actuation using the available sensor data.

Once the estimate crosses the configured threshold, the Arduino commands the SG90 servo to perform two out-and-back movements.

## 💻 Tech Stack

| Technology | Purpose |
|---|---|
| Arduino C/C++ | Sensor processing and servo control |
| Arduino UNO R4 WiFi | Main microcontroller |
| Arduino Cloud | Wireless communication |
| Arduino IoT Remote | iPhone sensor data collection |
| SG90 Servo | Physical actuation |
| Arduino IDE | Programming and debugging |

## 🚀 Challenges We Faced

**Converting acceleration into velocity**

Acceleration is easy to measure, but velocity is significantly harder to estimate. Numerical integration introduces accumulated errors, particularly when the sensor measurements contain noise or bias.

**Working with gravity**

Our iPhone's acceleration data reported approximately 1 g at rest. We needed to account for that baseline before performing numerical integration, while recognizing that magnitude-based correction is only an approximation.

**Wireless sensor communication**

Getting an iPhone to communicate with an Arduino required configuring multiple devices, synchronizing Arduino Cloud variables, and handling inconsistent data update intervals.

**Coordinating hardware and software**

We had to integrate sensor readings, velocity estimation, threshold logic, and servo control into one working embedded program.

## 🧠 What We Learned

This project gave us hands-on experience with:

- Embedded systems programming and hardware integration
- Working with accelerometers and real-world sensor data
- Numerical integration and the limitations of inertial navigation
- IoT communication between smartphones and microcontrollers
- Controlling physical actuators using software-defined thresholds

One of our biggest takeaways was that **implementing the mathematics is often easier than dealing with the imperfections of real-world measurements**.

## 🔮 What's Next?

There are several ways we'd like to improve Souvik’s Speed Sensing Super Safety System:

- **Better velocity estimation:** Combine gyroscope and accelerometer measurements using sensor fusion.
- **Lower latency:** Replace Cloud communication with direct Bluetooth or Wi-Fi streaming.
- **Improved accuracy:** Implement better gravity compensation and drift correction.
- **Wearable design:** Develop a compact enclosure for the hardware.
- **Live visualization:** Display motion estimates and threshold events in real time.

Ultimately, we'd like to explore how this approach could be expanded into more capable wearable motion-monitoring and interactive feedback devices.

## 🎯 Our Takeaway

Souvik’s Speed Sensing Super Safety System demonstrates how a smartphone, a microcontroller, and a simple motor can work together to turn digital sensor measurements into physical actions.

While our prototype is experimental rather than a validated velocity-measurement or safety system, it showcases the intersection of **electrical engineering, embedded software, IoT, and real-time motion processing**.

---

**Built for SB Hacks 2026 🚀**
