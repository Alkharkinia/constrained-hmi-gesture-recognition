import tensorflow as tf
import cv2
import numpy as np
import time
import psutil
import os

# === Load model ===
model = tf.keras.models.load_model("gesture_vision_cnn.keras")

GESTURES = ["updown", "rightleft", "inout", "hoop", "idle"]
IMG_SIZE = 64

cap = cv2.VideoCapture(0)

process = psutil.Process(os.getpid())

# === Warm-up ===
dummy = np.zeros((1, IMG_SIZE, IMG_SIZE, 1), dtype=np.float32)
for _ in range(20):
    _ = model.predict(dummy, verbose=0)

# === Stats ===
infer_times = []
end_to_end_times = []
frame_count = 0
REPORT_EVERY = 30

while True:
    loop_start = time.perf_counter()

    ret, frame = cap.read()
    if not ret:
        break

    # ---- Preprocessing ----
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    resized = cv2.resize(gray, (IMG_SIZE, IMG_SIZE), interpolation=cv2.INTER_AREA)
    normalized = resized.astype(np.float32) / 255.0
    sample = normalized[np.newaxis, ..., np.newaxis]

    # ---- Inference timing ----
    infer_start = time.perf_counter()
    preds = model.predict(sample, verbose=0)[0]
    infer_end = time.perf_counter()

    loop_end = time.perf_counter()

    infer_ms = (infer_end - infer_start) * 1000.0
    e2e_ms   = (loop_end - loop_start) * 1000.0

    infer_times.append(infer_ms)
    end_to_end_times.append(e2e_ms)

    # ---- Prediction ----
    gesture_id = np.argmax(preds)
    confidence = preds[gesture_id]
    gesture = GESTURES[gesture_id]

    if confidence < 0.7:
        gesture = "idle"

    cv2.putText(
        frame,
        f"{gesture} ({confidence:.2f})",
        (20, 40),
        cv2.FONT_HERSHEY_SIMPLEX,
        1,
        (0, 255, 0),
        2
    )

    cv2.imshow("Gesture Recognition", frame)

    frame_count += 1

    # === Periodic reporting ===
    if frame_count % REPORT_EVERY == 0:
        avg_infer = np.mean(infer_times)
        avg_e2e   = np.mean(end_to_end_times)
        cpu_usage = process.cpu_percent(interval=None)

        print("---- Vision Model Performance ----")
        print(f"Avg inference latency (ms): {avg_infer:.2f}")
        print(f"Avg end-to-end latency (ms): {avg_e2e:.2f}")
        print(f"CPU utilization (%): {cpu_usage:.2f}")
        print("----------------------------------")

        infer_times.clear()
        end_to_end_times.clear()

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
