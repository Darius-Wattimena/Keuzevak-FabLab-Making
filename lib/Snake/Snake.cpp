#include "Arduino.h"
#include "Snake.h"
#include "Adafruit_NeoPixel.h"
#include "avr/power.h"

int _snakeLocation[20][2] = {
  { -1, -1},
  { -1, -1},
  { -1, -1},
  { -1, -1},
  { -1, -1},
  { -1, -1},
  { -1, -1},
  { -1, -1},
  { -1, -1},
  { -1, -1},
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
}

//Roep dit aan in de setup van de arduino main class
void Snake::setup() {
  pinMode(upPin, INPUT);
  pinMode(rightPin, INPUT);
  pinMode(downPin, INPUT);
  pinMode(leftPin, INPUT);
  _maxSize = 20;
  _maxFruitSpawned = 3;
  running = false;
  gameState = 0;
}

//Roep dit aan wanneer je de snake minigame wil starten
void Snake::start() {
  running = true;
  _fruitSpawnTick = 0;
  _hitFruitIndex = 0;
  _newFruitX = 0;
  _newFruitY = 0;
  _direction = 2;
  _directionButtonState = 0;
  score = 0;
  gameState = 1;
  _gameDelay = 1000;
  _gameMinDelay = 50;
  _gameDelayDecrease = (_gameDelay - _gameMinDelay) / 20;
  setSnakeStartPosition();
  resetFruitArray();
}

//Roep dit aan in de loop van de arduino main class
void Snake::loop() {
  if (running == false) {
    return;
  }

  readButtons();
  _directionButtonState = 0;

  int currentSnakeHeadY = _snakeLocation[0][0];
  int currentSnakeHeadX = _snakeLocation[0][1];

  _newSnakeHeadY = currentSnakeHeadY;
  _newSnakeHeadX = currentSnakeHeadX;

  bool hittingWall = moveSnake();
  setSnakeLocation();
  int hittingFruit = isFruitHere(_newSnakeHeadX, _newSnakeHeadY);
  int hittingSnake = isSnakeHere(_newSnakeHeadX, _newSnakeHeadY, true);

  if (hittingWall) {
    onGameover();
    return;
  }

  if (hittingFruit != -1) {
    _hitFruitIndex = hittingFruit;
    onFruitHit();
  }

  if (hittingSnake != -1 && hittingSnake != 0) {
    onGameover();
    return;
  }

  if (_fruitSpawnTick == 0) {
    spawnFruit();
    _fruitSpawnTick = random(5, 8);
  }

  _fruitSpawnTick --;
}

//Roep dit aan na de loop hierin wordt alles geregeld voor de matrix
void Snake::display(Adafruit_NeoPixel &matrix, int (&pixelLocationMap)[16][16]) {
  if (running == false) {
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

  delay(_gameDelay);
}

//Zet de locatie van de slang in de array
void Snake::setSnakeLocation() {
  int newY = _newSnakeHeadY;
  int newX = _newSnakeHeadX;
  for (int i = 0; i < _maxSize; i++) {
    //Vind de current location
    int currentY = _snakeLocation[i][0];
    int currentX = _snakeLocation[i][1];

    if (currentY == -1) {
      _snakeOldTailX = newX;
      _snakeOldTailY = newY;
      break;
    }

    //Zet de nieuwe locatie
    _snakeLocation[i][0] = newY;
    _snakeLocation[i][1] = newX;

    //Zet de new location voor de volgende item in de array
    newY = currentY;
    newX = currentX;
  }
}

//returned -1 als er geen slang is op de gegeven positie anders de index van de slang array
int Snake::isSnakeHere(int x, int y, bool ignoreFirst) {
  int i;
  if (ignoreFirst) {
    i = 1;
  }
  else {
    i = 0;
  }

  for (; i < _maxSize; i++) {
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
  _newFruitY = random(0, 16);

  if (isSnakeHere(_newFruitX, _newFruitY, false) != -1 || isFruitHere(_newFruitX, _newFruitY) != -1) {
    getEmptyFruitLocation();
  }
}

void Snake::resetFruitArray() {
  _fruitLocation[0][0] = -1;
  _fruitLocation[0][1] = -1;
  _fruitLocation[1][0] = -1;
  _fruitLocation[1][1] = -1;
  _fruitLocation[2][0] = -1;
  _fruitLocation[2][1] = -1;
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

  if (score < 21) {
    _gameDelay -= _gameDelayDecrease;
  }

  //Reset de fruit die we hebben gepakt in de array
  _fruitLocation[_hitFruitIndex][0] = -1;
  _fruitLocation[_hitFruitIndex][1] = -1;

  growSnake();
}

void Snake::growSnake() {
  for (int i = 0; i < _maxSize; i++) {

    int currentY = _snakeLocation[i][0];

    if (currentY == -1) {
      _snakeLocation[i][0] = _snakeOldTailY;
      _snakeLocation[i][1] = _snakeOldTailX;
      break;
    }
  }
}

//Zet running op false en zet de score in finalScore
void Snake::onGameover() {
  running = false;
  finalScore = score;
  gameState = 2;
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

  _snakeLocation[10][0] = -1;
  _snakeLocation[10][1] = -1;

  _snakeLocation[11][0] = -1;
  _snakeLocation[11][1] = -1;

  _snakeLocation[12][0] = -1;
  _snakeLocation[12][1] = -1;

  _snakeLocation[13][0] = -1;
  _snakeLocation[13][1] = -1;

  _snakeLocation[14][0] = -1;
  _snakeLocation[14][1] = -1;

  _snakeLocation[15][0] = -1;
  _snakeLocation[15][1] = -1;

  _snakeLocation[16][0] = -1;
  _snakeLocation[16][1] = -1;

  _snakeLocation[17][0] = -1;
  _snakeLocation[17][1] = -1;

  _snakeLocation[18][0] = -1;
  _snakeLocation[18][1] = -1;

  _snakeLocation[19][0] = -1;
  _snakeLocation[19][1] = -1;
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
