char intensity[8] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
int intensity_values[8] = {0, 130, 155, 175, 190, 205, 220, 255};
//int pins[5] = {9, 10, 3, 5, 6};
int pins[5] = {6, 5, 3, 10, 9};

void setup() {
  for(int i = 0; i<5; i++){
    pinMode(pins[i], OUTPUT);
  }
  Serial.begin(115200);
}

void loop() {
  if(Serial.available()){
    int vals[5] = {0,0,0,0,0};
    for(int i = 0; i<5; i++){
      char c = Serial.read();
      Serial.println(c);
      int j = 0;
      for(j = 0; j < 8; j++){
        if(c == intensity[j])
          break; 
      }
      vals[i] = intensity_values[j];
    }
    for(int i = 0; i<5; i++){
      analogWrite(pins[i], vals[i]);
    }
  }
}
