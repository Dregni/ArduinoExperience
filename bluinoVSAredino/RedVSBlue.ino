#define RED 7
#define WHITE 6
#define BLUE 5

#define PL1 4
#define PL2 8

#define STEP_WAITING 0
#define STEP_READY 1
#define STEP_PLAY 2
#define STEP_WIN 3
int currentStep = 0;

unsigned long timing = 0;

int WP_out = WHITE;

bool pl1_ready = false;
bool pl2_ready = false;

#define WS_MIN 2000
#define WS_MAX 7000
int waitStart = 0;
bool started = false;

int winner = 0;

void setup()
{
  pinMode(RED, OUTPUT);
  pinMode(WHITE, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(PL1, INPUT_PULLUP);
  pinMode(PL2, INPUT_PULLUP);
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
  randomSeed(analogRead(0));
}

void resetOutput()
{
  digitalWrite(RED, LOW);
  digitalWrite(WHITE, LOW);
  digitalWrite(BLUE, LOW);
}

bool TimePassed(unsigned long t)
{
  if (millis() - timing > t)
  {
    timing = millis();
    return true;
  }
  return false;
}

void stepUp()
{
  resetOutput();
  ++currentStep;
  delay(200);
  timing = millis();
}

void WaitingPlayers()
{
  if (TimePassed(500))
  {
    resetOutput();
    digitalWrite(WP_out++, HIGH);
    if (WP_out > RED)
    {
      WP_out = BLUE;
    }
  }
  if (digitalRead(PL1) == LOW || digitalRead(PL2) == LOW)
  {
    resetOutput();
    ++currentStep;
  }
}

void PlayersReady()
{
  if (digitalRead(PL1) == LOW)
  {
    digitalWrite(BLUE, HIGH);
    pl1_ready = true;
  }
  if (digitalRead(PL2) == LOW)
  {
    digitalWrite(RED, HIGH);
    pl2_ready = true;
  }
  if (pl1_ready && pl2_ready)
  {
    delay(500);
    stepUp();
    waitStart = random(WS_MIN, WS_MAX);
  }
}

void Play()
{
  if (TimePassed(waitStart))
  {
    digitalWrite(WHITE, HIGH);
    started = true;
  }
  if (started)
  {
    if (digitalRead(PL1) == LOW)
    {
      winner = BLUE;
      stepUp();
    }
    if (digitalRead(PL2) == LOW)
    {
      winner = RED;
      stepUp();
    }
  }
  else
  {
    if (digitalRead(PL1) == LOW)
    {
      winner = RED;
      stepUp();
    }
    if (digitalRead(PL2) == LOW)
    {
      winner = BLUE;
      stepUp();
    }
  }
}

void Win()
{
  started = false;
  pl1_ready = false;
  pl2_ready = false;
  digitalWrite(winner, HIGH);
  if (digitalRead(PL1) == LOW || digitalRead(PL2) == LOW || TimePassed(5000))
  {
    resetOutput();
    currentStep = STEP_WAITING;
    WP_out = WHITE;
    delay(100);
  }
}

void loop()
{
  switch (currentStep)
  {
    case STEP_WAITING:
      WaitingPlayers();
      break;
    case STEP_READY:
      PlayersReady();
      break;
    case STEP_PLAY:
      Play();
      break;
    case STEP_WIN:
      Win();
      break;
    default:
      resetOutput();
  }
  delay(50);
}
