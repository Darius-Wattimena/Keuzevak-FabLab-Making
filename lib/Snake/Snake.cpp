#include "Arduino.h"
#include "Snake.h"
#include "Adafruit_NeoPixel.h"
#include "avr/power.h"

int _snakeLocation[10][2] = {
  { -1, -1},
  { -1, -1},
  { -1, -1},
  { -1, -1},
  { -1, -1},
  { -1, -1},
  { -1, -1},
  { -1, -1},
  { -1, -1},
  { -1, -1}
};

int _fruitLocation[3][2] = {
  { -1, -1},
  { -1, -1},
  { -1, -1}
};

Snake::Snake(int uPin, int rPin, int dPin, int lPin) {
  upPin = uPin;
  rightPin = rPin;
  downPin = dPin;
  leftPin = lPin;
  running = false;
}

//Roep dit aan in de setup van de arduino main class
void Snake::setup() {
  pinMode(upPin, INPUT);
  pinMode(rightPin, INPUT);
  pinMode(downPin, INPUT);
  pinMode(leftPin, INPUT);
  _fruitSpawnTick = 0;
  _maxSize = 10;
  _maxFruitSpawned = 3;
  _hitFruitIndex = 0;
  _newFruitX = 0;
  _newFruitY = 0;
  _direction = 2;
  _directionButtonState = 0;
  score = 0;
  setSnakeStartPosition();
}

//Roep dit aan wanneer je de snake minigame wil starten
void Snake::start() {
  running = true;
}

//Roep dit aan in de loop van de arduino main class
void Snake::loop() {
  if (!running) {
    return;
  }

  readButtons();
  _directionButtonState = 0;

  int currentSnakeHeadY = _snakeLocation[0][0];
  int currentSnakeHeadX = _snakeLocation[0][1];

  _newSnakeHeadY = currentSnakeHeadY;
  _newSnakeHeadX = currentSnakeHeadX;

  bool hittingWall = moveSnake();
  bool hittingFruit = isFruitHere(_newSnakeHeadX, _newSnakeHeadY);
  bool hittingSnake = isSnakeHere(_newSnakeHeadX, _newSnakeHeadY);

  if (hittingWall) {
    onGameover();
    return;
  }

  if (hittingFruit) {
    onFruitHit();
  }

  if (hittingSnake) {
    onGameover();
    return;
  }

  if (_fruitSpawnTick == 0) {
    spawnFruit();
    _fruitSpawnTick = random(2, 3);
  }

  _fruitSpawnTick --;
}

//Roep dit aan na de loop hierin wordt alles geregeld voor de matrix
void Snake::display(Adafruit_NeoPixel &matrix, int (&pixelLocationMap)[16][16]) {
  if (!running) {
    return;
  }

  matrix.clear();

  for (int i = 0; i < _maxSize; i++) {
    if (_snakeLocation[i][0] != -1) {
      int snakePixelY = _snakeLocation[i][0];
      int snakePixelX = _snakeLocation[i][1];
      int snakePixel = pixelLocationMap[snakePixelY][snakePixelX];
      matrix.setPixelColor(snakePixel, 0, 10, 0);
    }
  }

  for (int i = 0; i < _maxFruitSpawned; i++) {
    if (_fruitLocation[i][0] != -1) {
      int fruitPixelY = _fruitLocation[i][0];
      int fruitPixelX = _fruitLocation[i][1];
      int fruitPixel = pixelLocationMap[fruitPixelY][fruitPixelX];
      matrix.setPixelColor(fruitPixel, 10, 0, 0);
    }
  }

  matrix.show();
}

//returned -1 als er geen slang is op de gegeven positie anders de index van de slang array
int Snake::isSnakeHere(int x, int y) {
  for (int i = 0; i < _maxSize; i++) {
    if (_snakeLocation[i][0] == y && _snakeLocation[i][1] == x) {
      return i;
    }
  }
  return -1;
}

//returned -1 als er geen fruit is op de gegeven positie anders de index van de fruit array
int Snake::isFruitHere(int x, int y) {
  for (int i = 0; i < _maxFruitSpawned; i++) {
    if (_fruitLocation[i][0] == y && _fruitLocation[i][1] == x) {
      return i;
    }
  }
  return -1;
}

//returned true als er een muur wordt geraakt anders false
bool Snake::moveSnake() {
  switch (_direction) {
    case 1:
      //Hij gaat naar boven
      if (_newSnakeHeadY != 0) {
        _newSnakeHeadY -= 1;
        return false;
      }
      break;
    case 2:
      //Hij gaat naar rechts
      if (_newSnakeHeadX != 15) {
        _newSnakeHeadX += 1;
        return false;
      }
      break;
    case 3:
      //Hij gaat naar beneden
      if (_newSnakeHeadY != 15) {
        _newSnakeHeadY += 1;
        return false;
      }
      break;
    case 4:
      //Hij gaat naar links
      if (_newSnakeHeadX != 0) {
        _newSnakeHeadX -= 1;
        return false;
      }
      break;
  }
  return true;
}

//Zoek een lege locatie voor onze fruit
void Snake::getEmptyFruitLocation() {
  _newFruitX = random(0, 16);
  _newFruitX = random(0, 16);

  if (isSnakeHere(_newFruitX, _newFruitY) || isFruitHere(_newFruitX, _newFruitY)) {
    getEmptyFruitLocation();
  }
}

//Spawn een nieuwe fruit in als de fruitArray nog geen 3 items heeft
void Snake::spawnFruit() {

  //return als er geen plek is voor een fruit
  if (_fruitLocation[0][0] != -1 && _fruitLocation[1][0] != -1 && _fruitLocation[2][0] != -1) {
    return;
  }

  getEmptyFruitLocation();

  if (_fruitLocation[0][0] == -1) {
    _fruitLocation[0][0] = _newFruitY;
    _fruitLocation[0][1] = _newFruitX;
  }
  else if (_fruitLocation[1][0] == -1) {
    _fruitLocation[1][0] = _newFruitY;
    _fruitLocation[1][1] = _newFruitX;
  }
  else if (_fruitLocation[2][0] == -1) {
    _fruitLocation[2][0] = _newFruitY;
    _fruitLocation[2][1] = _newFruitX;
  }
}

//Geef de speler een punt en laat de slang groeien als die nog niet de max size is
void Snake::onFruitHit() {
  score++;

  //Reset de fruit die we hebben gepakt in de array
  _fruitLocation[_hitFruitIndex][0] = -1;
  _fruitLocation[_hitFruitIndex][1] = -1;
}

//Zet running op false en zet de score in finalScore
void Snake::onGameover() {
  running = false;
  finalScore = score;
}

//Zet de snake op zijn default positie
void Snake::setSnakeStartPosition() {
  int randomY = random(6, 9);
  int randomCenterX = random(6, 9);
  _snakeLocation[0][0] = randomY;
  _snakeLocation[0][1] = randomCenterX + 1;

  _snakeLocation[1][0] = randomY;
  _snakeLocation[1][1] = randomCenterX;

  _snakeLocation[2][0] = randomY;
  _snakeLocation[2][1] = randomCenterX - 1;

  _snakeLocation[3][0] = -1;
  _snakeLocation[3][1] = -1;

  _snakeLocation[4][0] = -1;
  _snakeLocation[4][1] = -1;

  _snakeLocation[5][0] = -1;
  _snakeLocation[5][1] = -1;

  _snakeLocation[6][0] = -1;
  _snakeLocation[6][1] = -1;

  _snakeLocation[7][0] = -1;
  _snakeLocation[7][1] = -1;

  _snakeLocation[8][0] = -1;
  _snakeLocation[8][1] = -1;

  _snakeLocation[9][0] = -1;
  _snakeLocation[9][1] = -1;
}

void Snake::readButtons() {
  _directionButtonState = digitalRead(upPin);

  if (_directionButtonState == HIGH && _direction != 3) {
    _direction = 1;
    return;
  }

  _directionButtonState = digitalRead(rightPin);

  if (_directionButtonState == HIGH && _direction != 4) {
    _direction = 2;
    return;
  }

  _directionButtonState = digitalRead(downPin);

  if (_directionButtonState == HIGH && _direction != 1) {
    _direction = 3;
    return;
  }

  _directionButtonState = digitalRead(leftPin);

  if (_directionButtonState == HIGH && _direction != 2) {
    _direction = 4;
    return;
  }
}
