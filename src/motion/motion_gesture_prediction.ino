#include <MPU9250.h>
#include <Wire.h>

// Chirale TensorFlow Lite library
#include <Chirale_TensorFlowLite.h>

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "model.h"

#define I2C_SDA 8
#define I2C_SCL 9

const float accelerationThreshold = 2.5;
const int numSamples = 120;
int samplesRead = numSamples;

MPU9250 IMU(Wire, 0x68);

// === TFLM Globals ===
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

// Tensor arena
constexpr int kTensorArenaSize = 160 * 1024;
alignas(16) uint8_t tensor_arena[kTensorArenaSize];

tflite::AllOpsResolver resolver;

// Gesture labels
const char* GESTURES[] = {
  "updown",
  "rightleft",
  "inout",
  "hoop",
  "idle"
  
};
#define NUM_GESTURES (sizeof(GESTURES) / sizeof(GESTURES[0]))

float input_scale;
int input_zero_point;

void setup() {
  Serial.begin(9600);
  Wire.begin(I2C_SDA, I2C_SCL);

  Serial.print("Free heap: "); 
  Serial.println(ESP.getFreeHeap());

  // === Init IMU ===
  Serial.println("Initializing IMU...");
  if (!IMU.begin()) {
    Serial.println("No MPU6050/MPU9250 detected!");
    while (1);
  }
  Serial.println("MPU initialized.");

  // === Load model ===
  Serial.println("Loading model...");
  model = tflite::GetModel(model_int8_tflite);
  // model = tflite::GetModel(gesture_model_tflite);

  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    while (1);
  }

  // === Interpreter (static, Chirale style) ===
  static tflite::MicroInterpreter static_interpreter(
    model, resolver, tensor_arena, kTensorArenaSize
  );
  interpreter = &static_interpreter;

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("AllocateTensors FAILED");
    while (1);
  }

  input  = interpreter->input(0);
  output = interpreter->output(0);

  input_scale = input->params.scale;
  input_zero_point = input->params.zero_point;

  Serial.println("TensorFlow Lite ready.");
}

void loop() {
  // float aX, aY, aZ, gX, gY, gZ; 
  static unsigned long lastSampleTime = 0; 
  const unsigned long sampleInterval = 10; // 10 ms 
  const unsigned long postPredictDelay = 3000; // 3 seconds

  samplesRead = 0;

  // Countdown
  for (int i = 3; i > 0; i--) {
      Serial.print("Recording starts in ");
      Serial.println(i);
      delay(1000);
  }
  Serial.println("Sampling...");

  TfLiteTensor* input  = interpreter->input(0);
  TfLiteTensor* output = interpreter->output(0);

  float inScale      = input->params.scale;
  int inZeroPoint    = input->params.zero_point;

  // Sample until buffer is full
  while (samplesRead < numSamples) {

      IMU.readSensor();

      float aX = IMU.getAccelX_mss();
      float aY = IMU.getAccelY_mss();
      float aZ = IMU.getAccelZ_mss();

      float gX = IMU.getGyroX_rads();
      float gY = IMU.getGyroY_rads();
      float gZ = IMU.getGyroZ_rads();

      // Normalization BEFORE quantization
      
      float vals[6] = {
        (aX + 4.0f) / 8.0f,
        (aY + 4.0f) / 8.0f,
        (aZ + 4.0f) / 8.0f,

        (gX + 2000.0f) / 4000.0f,
        (gY + 2000.0f) / 4000.0f,
        (gZ + 2000.0f) / 4000.0f
      };
      


      // Quantize input
      for (int i = 0; i < 6; i++) {
          float v = vals[i];
          int8_t q = (int8_t)(v / inScale + inZeroPoint);
          input->data.int8[samplesRead * 6 + i] = q;
      }

      samplesRead++;
      delay(sampleInterval);

      if (samplesRead == numSamples) {
        // === Predict ===
        if (interpreter->Invoke() != kTfLiteOk) {
            Serial.println("Invoke FAILED");
            return;
        }

        // === Read int8 output ===
        float outScale      = output->params.scale;
        int   outZeroPoint  = output->params.zero_point;

        int best = 0;
        float bestScore = (output->data.int8[0] - outZeroPoint) * outScale;

        for (int i = 1; i < NUM_GESTURES; i++) {
            float score = (output->data.int8[i] - outZeroPoint) * outScale;
            if (score > bestScore) {
                bestScore = score;
                best = i;
            }
        }

        // Serial.print("Output type: "); Serial.println(output->type); // 1 -> float32, 9 -> int8

        Serial.print("Predicted: ");
        Serial.println(GESTURES[best]);

        Serial.println("Raw scores:");
        for (int i = 0; i < NUM_GESTURES; i++) {
            float score = (output->data.int8[i] - outZeroPoint) * outScale;
            Serial.print(GESTURES[i]); Serial.print(": ");
            Serial.println(score, 6);
        }

      }
  }

  delay(postPredictDelay);
}



