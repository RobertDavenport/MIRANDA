// test cases for feedbackProcess
test(feedbackProcess) {
  assertEqual(feedback(HAPTIC_MOTOR_1, MIN_DISTANCE), 255);
  assertEqual(feedback(HAPTIC_MOTOR_1, 100), 237);
  assertEqual(feedback(HAPTIC_MOTOR_1, 200), 218);
  assertEqual(feedback(HAPTIC_MOTOR_1, 300), 199);
  assertEqual(feedback(HAPTIC_MOTOR_1, 400), 180);
  assertEqual(feedback(HAPTIC_MOTOR_1, 450), 170);
  assertEqual(feedback(HAPTIC_MOTOR_1, 600), 170);
  assertEqual(feedback(HAPTIC_MOTOR_1, 700), 170);
  assertEqual(feedback(HAPTIC_MOTOR_1, 800), 170);
  assertEqual(feedback(HAPTIC_MOTOR_1, 0), 0);
  }
