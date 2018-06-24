#include "Adafruit_NeoPixel.h"
#include "avr/power.h"

#ifndef Snake_h
#define Snake_h

class Snake {
  public:
    Snake(int uPin, int rPin, int dPin, int lPin);
    void setup();
    void loop();
    void start();
    void display(Adafruit_NeoPixel &matrix, int (&pixelLocationMap)[16][16]);
    int score;
    int finalScore;
    bool running;
    int gameState;
    int upPin;
    int rightPin;
    int downPin;
    int leftPin;
  private:
    void readButtons();
    void setSnakeStartPosition();
    bool moveSnake();
    void setSnakeLocation();
    void growSnake();
    int isSnakeHere(int x, int y, bool ignoreFirst);
    int isFruitHere(int x, int y);
    void onGameover();
    void onFruitHit();
    void resetFruitArray();
    void spawnFruit();
    void getEmptyFruitLocation();
    int _snakeOldTailX;
    int _snakeOldTailY;
    int _newSnakeHeadY;
    int _newSnakeHeadX;
    int _fruitSpawnTick;
    int _maxFruitSpawned;
    int _maxSize;
    int _hitFruitIndex;
    int _newFruitX;
    int _newFruitY;
    int _direction;
    int _directionButtonState;
    int _gameDelay;
    int _gameMinDelay;
    int _gameDelayDecrease;
};

#endif
