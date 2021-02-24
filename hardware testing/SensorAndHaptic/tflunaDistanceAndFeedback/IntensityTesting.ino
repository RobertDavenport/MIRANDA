// test cases for feedbackProcess
test(feedbackProcess) {
  assertEqual(feedback(HAPTIC_MOTOR_1, MIN_DISTANCE), 255);
  assertEqual(feedback(HAPTIC_MOTOR_1, MIN_DISTANCE), 227);
  assertEqual(feedback(HAPTIC_MOTOR_1, MIN_DISTANCE), 198);
  assertEqual(feedback(HAPTIC_MOTOR_1, MIN_DISTANCE), 170);
  assertEqual(feedback(HAPTIC_MOTOR_1, MIN_DISTANCE), 142);
  assertEqual(feedback(HAPTIC_MOTOR_1, MIN_DISTANCE), 113);
  assertEqual(feedback(HAPTIC_MOTOR_1, MIN_DISTANCE), 85);
  assertEqual(feedback(HAPTIC_MOTOR_1, MIN_DISTANCE), 57);
  assertEqual(feedback(HAPTIC_MOTOR_1, MIN_DISTANCE), 28);
  assertEqual(feedback(HAPTIC_MOTOR_1, MIN_DISTANCE), 0);
  assertEqual(feedback(HAPTIC_MOTOR_1, MIN_DISTANCE), 0);
  assertEqual(feedback(HAPTIC_MOTOR_1, MIN_DISTANCE), 0);
  assertEqual(feedback(HAPTIC_MOTOR_1, MIN_DISTANCE), 0);
  assertEqual(feedback(HAPTIC_MOTOR_1, MIN_DISTANCE), 0);
}
