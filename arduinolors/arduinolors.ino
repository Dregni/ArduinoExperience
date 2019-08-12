#define RED 9
#define GREEN 10
#define BLUE 11

float r=0;
float g=0;
float b=0;
float freq = 0.3;
float nInc = 0.3;
float n = 0;

void setup() {
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
}

void loop() {
  analogWrite(RED, r);
  analogWrite(GREEN, g);
  analogWrite(BLUE, b);

  r = sin(freq*n) * 10 + 10;
  g = sin(freq*n + 2*PI/3) * 10 + 10;
  b = sin(freq*n + 4*PI/3) * 10 + 10;
  n += nInc;
  if (n >= 21)
    n=0;
  delay(50);
}
