import cv2
import os
import time

# =========================
# CONFIGURATION
# =========================
TOTAL_FRAMES = 750
EXECUTIONS = 10
FPS = 25
COUNTDOWN_SECONDS = 10
IMG_SIZE = 64

FRAMES_PER_EXEC = TOTAL_FRAMES // EXECUTIONS
ROI = (100, 100, 300, 300)  # x1, y1, x2, y2

cap = cv2.VideoCapture(0)

print("Press 's' to start capture")
print("Press 'q' to quit")

# =========================
# WAIT FOR START
# =========================
while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = cv2.flip(frame, 1)
    cv2.putText(frame, "Press 's' to start capture",
                (30, 40), cv2.FONT_HERSHEY_SIMPLEX,
                0.8, (0, 255, 0), 2)

    x1, y1, x2, y2 = ROI
    cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)

    cv2.imshow("Frame", frame)

    key = cv2.waitKey(1) & 0xFF
    if key == ord('s'):
        break
    elif key == ord('q'):
        cap.release()
        cv2.destroyAllWindows()
        exit()

GESTURE_NAME = input("Enter gesture name: ")
SAVE_DIR = f"dataset/{GESTURE_NAME}"
os.makedirs(SAVE_DIR, exist_ok=True)

# =========================
# COUNTDOWN
# =========================
for i in range(COUNTDOWN_SECONDS, 0, -1):
    ret, frame = cap.read()
    frame = cv2.flip(frame, 1)

    cv2.putText(frame, f"Starting in {i}",
                (140, 200), cv2.FONT_HERSHEY_SIMPLEX,
                1.2, (0, 0, 255), 3)

    x1, y1, x2, y2 = ROI
    cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)

    cv2.imshow("Frame", frame)
    cv2.waitKey(1000)

# =========================
# CAPTURE LOOP
# =========================
print("Capturing frames...")

frame_interval = 1.0 / FPS
last_time = time.time()

execution_id = 1
frame_id = 0
captured_total = 0

while captured_total < TOTAL_FRAMES:
    ret, frame = cap.read()
    if not ret:
        break

    current_time = time.time()
    if current_time - last_time < frame_interval:
        continue

    last_time = current_time

    frame = cv2.flip(frame, 1)

    x1, y1, x2, y2 = ROI
    roi = frame[y1:y2, x1:x2]

    # -------------------------
    # PREPROCESSING
    # -------------------------
    gray = cv2.cvtColor(roi, cv2.COLOR_BGR2GRAY)
    resized = cv2.resize(gray, (IMG_SIZE, IMG_SIZE))

    # -------------------------
    # SAVE IMAGE
    # -------------------------
    filename = (
        f"{SAVE_DIR}/"
        f"{GESTURE_NAME}_e{execution_id:02d}_f{frame_id:03d}.png"
    )
    cv2.imwrite(filename, resized)

    frame_id += 1
    captured_total += 1

    # -------------------------
    # EXECUTION TRANSITION
    # -------------------------
    if frame_id >= FRAMES_PER_EXEC:
        execution_id += 1
        frame_id = 0

        # short pause between executions (optional but good)
        time.sleep(0.5)

    # -------------------------
    # DISPLAY
    # -------------------------
    cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
    cv2.putText(
        frame,
        f"Exec {execution_id}/10 | "
        f"Frame {frame_id}/{FRAMES_PER_EXEC}",
        (20, 40),
        cv2.FONT_HERSHEY_SIMPLEX,
        0.8, (255, 0, 0), 2
    )

    cv2.imshow("Frame", frame)
    cv2.imshow("Captured ROI", resized)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# =========================
# CLEANUP
# =========================
cap.release()
cv2.destroyAllWindows()
print("Capture complete.")
