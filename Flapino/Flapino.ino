#include <MaxMatrix.h>

#define DATA 4    // DIN pin of MAX7219 module
#define LOAD 3    // CS pin of MAX7219 module
#define CLOCK 2  // CLK pin of MAX7219 module

#define MAX_IN_USE 1    //change this variable to set how many MAX7219's you'll use

MaxMatrix m(DATA, LOAD, CLOCK, MAX_IN_USE); // define module

struct Point
{
  Point(int _x, int _y)
    : x(_x), y(_y)
    {}
  int x;
  int y;
};

PROGMEM const Point zero[] = {{0,0},{0,1},{0,2},{0,3},{0,4},{1,0},{1,4},{2,0},{2,1},{2,2},{2,3},{2,4}};
PROGMEM const Point one[] = {{0,0},{0,4},{1,0},{1,1},{1,2},{1,3},{1,4},{2,4}};
PROGMEM const Point two[] = {{0,0},{0,2},{0,3},{0,4},{1,0},{1,2},{1,4},{2,0},{2,1},{2,2},{2,4}};
PROGMEM const Point three[] = {{0,0},{0,4},{1,0},{1,2},{1,4},{2,0},{2,1},{2,2},{2,3},{2,4}};
PROGMEM const Point four[] = {{0,0},{0,1},{0,2},{1,2},{2,0},{2,1},{2,2},{2,3},{2,4}};
PROGMEM const Point five[] = {{0,0},{0,1},{0,2},{0,4},{1,0},{1,2},{1,4},{2,0},{2,2},{2,3},{2,4}};
PROGMEM const Point six[] = {{0,0},{0,1},{0,2},{0,3},{0,4},{1,0},{1,2},{1,4},{2,0},{2,2},{2,3},{2,4}};
PROGMEM const Point seven[] = {{0,0},{0,3},{0,4},{1,0},{1,2},{2,0},{2,1}};
PROGMEM const Point eight[] = {{0,0},{0,1},{0,2},{0,3},{0,4},{1,0},{1,2},{1,4},{2,0},{2,1},{2,2},{2,3},{2,4}};
PROGMEM const Point nine[] = {{0,0},{0,1},{0,2},{0,4},{1,0},{1,2},{1,4},{2,0},{2,1},{2,2},{2,3},{2,4}};

const int numberSize[] = { 12, 8, 11, 10, 9, 11, 12, 7, 13, 12 };
const Point * numbers[] = { zero, one, two, three, four, five, six, seven, eight, nine };

void PutSprite(int x, int y, const Point * data, int n)
{
  for (int i = 0; i < n; ++i)
    m.setDot(data[i].x + x, data[i].y + y, 1);
}

#define BTN 13

unsigned long * timingPlayer;
unsigned long * timingWalls;

bool TimePassed(unsigned long t, unsigned long * last)
{
  if (millis() - *last > t)
  {
    *last = millis();
    return true;
  }
  return false;
}

#define PLAYER_X 1 // player width position
int playerY = 2;
bool failed = false;

void putPlayer()
{
  m.setDot(PLAYER_X, playerY, 1);
}

class Wall
{
  public:
    Wall()
      : exists(false), hole(random(0,8))
    {}
    Wall(bool _exists)
      : exists(_exists), hole(random(0,8))
    {}

    int Hole() { return hole; }
    void ChangeHole(int min, int max) 
    { 
      hole = random(min, max); 
    }
  
    void Draw(int x)
    {
      for (int y = 0; y < 8; ++y)
      {
        if (y != hole && y != hole + 1)
          m.setDot(x, y, 1);
      }
    }

    bool Exists() { return exists; }
    void Appear() { exists = true; }
    void Disappear() { exists = false; }

  private:
    bool exists;
    int hole;
};

Wall walls[8];
int W_LastUpdate = 0;
int W_Speed = 300;
int W_SpawnRate = 4;

void putWalls()
{
  for (int i = 0; i < 8; ++i)
  {
    if (walls[i].Exists())
      walls[i].Draw(i);
  }
}

void setup()
{
  pinMode(BTN, INPUT_PULLUP);
  m.init();
  m.setIntensity(0); // set intensity of the LEDs (0-15)
  randomSeed(analogRead(0));
  Serial.begin(9600); // start Serial for debug
  while(!Serial); // Wait until Serial is ready
  timingPlayer = (unsigned long *) malloc(sizeof(unsigned long));
  *timingPlayer = 0;
  timingWalls = (unsigned long *) malloc(sizeof(unsigned long));
  *timingWalls = 0;
  putPlayer();
}

int scorel = 0;
int scorer = 0;

bool isFailed()
{
  if (playerY == 8)
    return true;
  if ((playerY == 0 || playerY == -1) && walls[1].Hole() == 0)
    return false;
  if (walls[1].Exists() && playerY != walls[1].Hole() && playerY != walls[1].Hole() + 1)
    return true;
  return false;
}

void UpdatePlayer()
{
  if (digitalRead(BTN) == LOW && playerY >= 0)
    --playerY;
  else
    ++playerY;
  if (walls[0].Exists())
  {
    walls[0].Disappear();
    if (scorer == 9)
    {
      scorel = (scorel == 9) ? 9 : scorel + 1;
      scorer = (scorel == 9) ? 9 : 0;
    }
    else
      scorer = (scorer == 9) ? 9 : scorer + 1;
    if (W_Speed >= 700 && W_SpawnRate > 1)
    {
      W_Speed = 300;
      --W_SpawnRate;
    }
    else
    {
      W_Speed += 50;
    }
  }
}

void UpdateWalls()
{
  if (TimePassed(1000 - W_Speed, timingWalls))
  {
    for (int i = 0; i < 7; ++i)
    {
      walls[i] = walls[i+1];
      walls[i+1].Disappear();
    }
    if (W_LastUpdate == 0)
    {
      walls[7].Appear();
      walls[7].ChangeHole(
        ((walls[6].Hole() <= W_SpawnRate - 1) ? W_SpawnRate: walls[6].Hole() - W_SpawnRate),
        ((walls[6].Hole() >= 8 - W_SpawnRate) ? 8 - W_SpawnRate: walls[6].Hole() + W_SpawnRate)
        );
    }
    W_LastUpdate = (W_LastUpdate + 1) % W_SpawnRate;
  }
}

void loop()
{
  m.clear();
  if (failed)
  {
    PutSprite(0, 1, numbers[scorel], numberSize[scorel]);
    PutSprite(4, 1, numbers[scorer], numberSize[scorer]);
    if (digitalRead(BTN) == LOW)
    {
      scorel = scorer = 0;
      failed = false;
      playerY = 2;
      delay(200);
    }
  }
  else
  {
    if (TimePassed(200, timingPlayer))
    {
      failed = isFailed();
      if (failed)
      {
        for (int i = 0; i < 8; ++i)
          walls[i].Disappear();
        W_Speed = 300;
        W_SpawnRate = 4;
        W_LastUpdate = 0;
        delay(300);
      }
      else
      {
        UpdatePlayer();
      }
    }
    UpdateWalls();
    putPlayer();
    putWalls();
  }
  delay(50);
}
