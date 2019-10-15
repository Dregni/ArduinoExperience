#include <MaxMatrix.h> // include librarie to control the LEDs

#define CLOCK 2 // CLK pin of MAX7219 module
#define LOAD 3  // CS pin of MAX7219 module
#define DATA 4  // DIN pin of MAX7219 module

#define MAX_IN_USE 1 //change this variable to set how many MAX7219's you'll use

#define JS_Y 0   // joystick analog X output
#define JS_X 1   // joystick analog Y output
#define JS_SW 12 // joystick switch output

#define JS_NRG 13 // joystick needs 5 volts but the 5v output is taken by the screen

#define UP 1
#define RIGHT 2
#define DOWN 3
#define LEFT 4

#define PL_MAX_LENGTH 64 // snake max length
#define FOOD_NUMBER 1    // max number of food to appear at the same time

#define STATE 0

MaxMatrix m(DATA, LOAD, CLOCK, MAX_IN_USE); // define Matrix module

// structure to handle coordinates
struct Point
{
  Point()
      : x(0), y(0)
  {
  }
  Point(int _x, int _y)
      : x(_x), y(_y)
  {
  }
  int x;
  int y;
};

// set of coordinates to print digit on the LEDs      I haven't found a better way sry
PROGMEM const Point zero[] = {{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {1, 0}, {1, 4}, {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}};
PROGMEM const Point one[] = {{0, 0}, {0, 4}, {1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}, {2, 4}};
PROGMEM const Point two[] = {{0, 0}, {0, 2}, {0, 3}, {0, 4}, {1, 0}, {1, 2}, {1, 4}, {2, 0}, {2, 1}, {2, 2}, {2, 4}};
PROGMEM const Point three[] = {{0, 0}, {0, 4}, {1, 0}, {1, 2}, {1, 4}, {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}};
PROGMEM const Point four[] = {{0, 0}, {0, 1}, {0, 2}, {1, 2}, {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}};
PROGMEM const Point five[] = {{0, 0}, {0, 1}, {0, 2}, {0, 4}, {1, 0}, {1, 2}, {1, 4}, {2, 0}, {2, 2}, {2, 3}, {2, 4}};
PROGMEM const Point six[] = {{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {1, 0}, {1, 2}, {1, 4}, {2, 0}, {2, 2}, {2, 3}, {2, 4}};
PROGMEM const Point seven[] = {{0, 0}, {0, 3}, {0, 4}, {1, 0}, {1, 2}, {2, 0}, {2, 1}};
PROGMEM const Point eight[] = {{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {1, 0}, {1, 2}, {1, 4}, {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}};
PROGMEM const Point nine[] = {{0, 0}, {0, 1}, {0, 2}, {0, 4}, {1, 0}, {1, 2}, {1, 4}, {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}};

// stocking digit somewhat decently
const int numberSize[] = {12, 8, 11, 10, 9, 11, 12, 7, 13, 12};
const Point *numbers[] = {zero, one, two, three, four, five, six, seven, eight, nine};

// print an array of points
void putSprite(int x, int y, const Point *data, int n)
{
  for (int i = 0; i < n; ++i)
    m.setDot(data[i].x + x, data[i].y + y, 1);
}

// function to wait a certain amount of time without blocking the program
unsigned long timing;

bool TimePassed(unsigned long t)
{
  if (millis() - timing > t)
  {
    timing = millis();
    return true;
  }
  return false;
}

// player class
class Player
{
public:
  Player()
      : dir(0), length(1), heard(false)
  {
    points[0] = Point(4, 4);
    for (int i = 1; i < PL_MAX_LENGTH; ++i)
    {
      points[i] = Point(-1, -1);
    }
  }

  int Length() { return length; }

  void Hear() // used to "hear" what move the players wants to play next this stops a turn back bug
  {
    int tmpDir = dir;

    if (heard == false)
    {
      if (analogRead(JS_Y) > 750 && dir != DOWN)
        dir = UP;
      else if (analogRead(JS_Y) < 250 && dir != UP)
        dir = DOWN;
      else if (analogRead(JS_X) > 750 && dir != LEFT)
        dir = RIGHT;
      else if (analogRead(JS_X) < 250 && dir != RIGHT)
        dir = LEFT;
      if (tmpDir != dir)
        heard = true;
    }
  }

  void Move() // Actual move done 10/3 times per second
  {
    for (int i = length; i >= 1; --i)
    {
      points[i] = points[i - 1];
    }
    if (dir == UP)
      --points[0].y;
    else if (dir == DOWN)
      ++points[0].y;
    else if (dir == LEFT)
      --points[0].x;
    else if (dir == RIGHT)
      ++points[0].x;
    heard = false;
  }

  bool Contains(int x, int y, int id = 0) // check if a point is on the snake
  {
    for (int i = id; i < length; ++i)
    {
      if (points[i].x == x && points[i].y == y)
        return true;
    }
    return false;
  }

  bool Collision() // check if the snake is bumping in a wall or itself
  {
    if (Contains(points[0].x, points[0].y, 1))
      return true;
    if (points[0].x < 0 || points[0].x > 7 || points[0].y < 0 || points[0].y > 7)
      return true;
    return false;
  }

  bool ScaleUp() // increments snake's length
  {
    if (length < PL_MAX_LENGTH)
    {
      ++length;
      return true;
    }
    return false;
  }

  void Draw() // print the snake
  {
    for (int i = 0; i < length; ++i)
      m.setDot(points[i].x, points[i].y, 1);
  }

  void Reset() // reset the snake back to the start
  {
    dir = 0;
    length = 1;
    heard = false;
    points[0] = Point(4, 4);
    for (int i = 1; i < PL_MAX_LENGTH; ++i)
    {
      points[i] = Point(-1, -1);
    }
  }

private:
  int dir;                     // current snake direction
  int length;                  // current snake length
  bool heard;                  // whether the user has made an input between two moves
  Point points[PL_MAX_LENGTH]; // snake's body as an array of points
};

Player p; // player instance

// class for snake food
class Food
{
public:
  Food()
  {
    pos.x = -2;
    pos.y = -2;
  }
  Food(int min, int max) // random init
  {
    while (p.Contains(pos.x, pos.y))
    {
      pos.x = random(min, max);
      pos.y = random(min, max);
    }
  }
  bool isEaten()
  {
    return p.Contains(pos.x, pos.y);
  }
  void Draw()
  {
    m.setDot(pos.x, pos.y, 1);
  }

private:
  Point pos;
};

Food food[FOOD_NUMBER];

void setup()
{
  pinMode(JS_NRG, OUTPUT);
  digitalWrite(JS_NRG, HIGH); // send electricity in the joystick as the other provider is used by the screen
  pinMode(JS_SW, INPUT);
  digitalWrite(JS_SW, HIGH); // set switch to true (we want the input
  m.init();
  m.setIntensity(0); // set intensity of the LEDs (0-15)
  randomSeed(analogRead(0));
  for (int i = 0; i < FOOD_NUMBER; ++i)
  {
    food[i] = Food(0, 8);
  }
  Serial.begin(9600); // start Serial for debug
  while (!Serial)
    ; // Wait until Serial is ready
}

void victoryManagement()
{
  m.clear();
  putSprite(0, 1, numbers[p.Length() / 10], numberSize[p.Length() / 10]);
  if (STATE == 0)
    putSprite(4, 1, numbers[(p.Length() % 10) - 1], numberSize[(p.Length() % 10) - 1]);
  else if (STATE == 1)
    putSprite(4, 1, numbers[(p.Length() % 10)], numberSize[(p.Length() % 10)]);
  if (digitalRead(JS_SW) == LOW)
  {
    p.Reset();
    STATE = 0;
    delay(300);
  }
}

void loop()
{
  victoryManagement();
  p.Hear();
  if (TimePassed(300))
  {
    STATE = ((p.Collision() == true) ? 1 : 0);
    if (STATE == 0)
    {
      m.clear();
      for (int i = 0; i < FOOD_NUMBER; ++i)
      {
        if (food[i].isEaten())
        {
          food[i] = Food(0, 8);
          if (p.Length() < PL_MAX_LENGTH)
          {
            p.ScaleUp();
          }
          else
            STATE = 1;
        }
        food[i].Draw();
      }
      p.Move();
      p.Draw();
    }
  }
  delay(10);
}
