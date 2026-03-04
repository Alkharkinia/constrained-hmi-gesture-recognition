# Comparative Analysis of Motion and Vision-Based Gesture Recognition for Constrained HMI Systems

The project presents a controlled comparison between motion-based (IMU) and vision-based (camera) gesture recognition systems using architecturally comparable TinyML models. The study evaluates accuracy, latency, and computational efficiency across sensing modalities to determine their suitability for constrained embedded human–machine interaction (HMI) systems.

---

## Overview

Two parallel gesture recognition pipelines are implemented:

###  Motion-Based System (Embedded TinyML)

* Hardware: **ESP32-C3 Super Mini**
* Sensor: **MPU6050 (6-axis IMU)**
* Model: Quantized 1D-CNN (<36k parameters)
* Deployment: TensorFlow Lite Micro
* Inference executed entirely on microcontroller

###  Vision-Based System (Desktop Inference)

* Hardware: Laptop webcam
* Input: 64×64 grayscale frames
* Model: Compact CNN (~31k parameters)
* Deployment: TensorFlow (desktop)
* Real-time webcam inference

Both systems are trained under identical protocols to isolate **sensor modality** as the independent experimental variable.

---

## Key Results

| Metric            | Motion-Based        | Vision-Based |
| ----------------- | ------------------- | ------------ |
| Overall Accuracy  | 82.00%              | 93.60%       |
| Inference Latency | 0.96 ms             | 45.66 ms     |
| Model Size        | ~130 kB (quantized) | ~121 kB      |
| Deployment Target | ESP32 MCU           | Laptop CPU   |

The motion-based system demonstrates ultra-low latency and robustness under constrained conditions, while the vision-based system achieves higher classification accuracy at increased computational cost.

---

## Repository Structure

```
.
├── data/
│   ├── motion/
│   │   ├── hoop.csv
│   │   ├── idle.csv
│   │   ├── inout.csv
│   │   ├── rightleft.csv
│   │   └── updown.csv
│   └── vision/
│       ├── hoop
│       ├── idle
│       ├── inout
│       ├── rightleft
│       └── updown
│
├── models/
│   ├── motion/
│   │   └── model.h
│   └── vision/
│       └── gesture_vision_cnn.keras
│
├── src/
│   ├── motion/
│   │   ├── motion_data_sampling.ino
│   │   └── motion_gesture_prediction.ino
│   └── vision/
│       ├── vision_image_capture.py
│       └── vision_gesture_prediction.py
│
│
│
├── LICENSE
└── README.md
```

---

## Installation & Setup

### Python Environment (Training & Vision Inference)

Required packages include:

* TensorFlow
* NumPy
* OpenCV
* scikit-learn
* matplotlib

---

## Running the Systems

### Train Motion Model

The models were trained in separate Google Colab environments. The links are publicly accesible on the colab_model_training_links.txt file.

### Run Vision-Based Live Inference

```bash
python src/vision/vision_gesture_prediction.py
```

### Deploy Motion Model to ESP32

1. Embed `src/motion/motion_gesture_prediction.ino/` and `models/motion/model.h` in microcontroller (ideally ESP32).
3. Monitor serial output for live predictions.

---

## Gesture Set

Five predefined gestures were used:

* `updown`
* `rightleft`
* `hoop`
* `inout`
* `idle`

Each gesture contains 120 samples × 10 repetitions × 5 gestures per modality.

---

## Experimental Controls

* Identical model scales (~30–35k parameters)
* 60/20/20 train-validation-test split
* Same optimizer (RMSprop, lr = 0.001)
* Same training duration (400 epochs)
* INT8 quantization for embedded deployment

---

## Evaluation Metrics

* Classification Accuracy
* Per-class F1 Score
* Inference Latency (ms)
* CPU Utilization


---

## Reproducibility

The full source code and datasets are publicly available for reproducibility.

GitHub Repository:
[https://github.com/Alkharkinia/constrained-hmi-gesture-comparison](https://github.com/Alkharkinia/constrained-hmi-gesture-comparison)

Archived Version (DOI):
[https://doi.org/10.5281/zenodo.18836677](https://doi.org/10.5281/zenodo.18836677)

---

## License

This project is released under the MIT License (see `LICENSE` file).

---

## Citation

If you use this repository in academic work, please cite:

Karadağ, A., & İlk, H.G. (2026). *Comparative Analysis of Motion and Vision-Based Gesture Recognition for Constrained HMI Systems*. Zenodo. [https://doi.org/10.5281/zenodo.18836677](https://doi.org/10.5281/zenodo.18836677)


