#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

#define PIN            6      //Matrix data pin
#define NUMPIXELS      256

const int buttonUpPin = 5;
const int buttonRightPin = 4;
const int buttonDownPin = 3;
const int buttonLeftPin = 2;
const int buttonStartPin = 10;

int directionButtonState = 0;
int startButtonState = 0;

Adafruit_NeoPixel matrix = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Elke pixel locatie
byte pixelLocationMap[16][16] = {
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
  {31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16},
  {32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47},
  {63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48},
  {64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79},
  {95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80},
  {96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111},
  {127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 112},
  {128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143},
  {159, 158, 157, 156, 155, 154, 153, 152, 151, 150, 149, 148, 147, 146, 145, 144},
  {160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175},
  {191, 190, 189, 188, 187, 186, 185, 184, 183, 182, 181, 180, 179, 178, 177, 176},
  {192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207},
  {223, 222, 221, 220, 219, 218, 217, 216, 215, 214, 213, 212, 211, 210, 209, 208},
  {224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239},
  {255, 254, 253, 252, 251, 250, 249, 248, 247, 246, 245, 244, 243, 242, 241, 240}
};

int maxSnakeSize = 10;

int snakeLocation[10][2] = {
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

int snakeOldTailX;
int snakeOldTailY;

int fruitLocation[3][2] = {
  { -1, -1},
  { -1, -1},
  { -1, -1}
};

int fruitSpawnTick = 0;
int maxFruitSpawned = 3;
int hitFruitIndex = 0;
int newFruitX = 0;
int newFruitY = 0;

int snakeDirection;
int score = 0;

int tickDelay = 1000;

bool snakeRunning = false;

void setup() {
  matrix.begin();
  matrix.show();

  Serial.begin(9600);
  startScreenSetup();

  pinMode(buttonUpPin, INPUT);
  pinMode(buttonRightPin, INPUT);
  pinMode(buttonDownPin, INPUT);
  pinMode(buttonLeftPin, INPUT);
  pinMode(buttonStartPin, INPUT);
}

void loop() {
  if (snakeRunning) {
    snakeLoop();
  }
  else {
    startScreenLoop();
  }

  matrix.show();
  delay(tickDelay);
}



void startScreenLoop() {
  startButtonState = digitalRead(buttonStartPin);

  if (startButtonState == HIGH) {
    snakeRunning = true;
    snakeSetup();
  }
}

void snakeSetup() {
  matrix.clear();
  
  score = 0;
  snakeLocation[0][0] = 7;
  snakeLocation[0][1] = 8;

  snakeLocation[1][0] = 7;
  snakeLocation[1][1] = 7;

  snakeLocation[2][0] = 7;
  snakeLocation[2][1] = 6;

  snakeLocation[3][0] = 7;
  snakeLocation[3][1] = 5;

  snakeLocation[4][0] = -1;
  snakeLocation[4][1] = -1;

  snakeLocation[5][0] = -1;
  snakeLocation[5][1] = -1;

  snakeLocation[6][0] = -1;
  snakeLocation[6][1] = -1;

  snakeLocation[7][0] = -1;
  snakeLocation[7][1] = -1;

  snakeLocation[8][0] = -1;
  snakeLocation[8][1] = -1;

  snakeLocation[9][0] = -1;
  snakeLocation[9][1] = -1;

  snakeDirection = 2;
}

void snakeLoop() {
  //Maak de matrix leeg zodat we de nieuwe posities kunnen laten zien
  matrix.clear();

  snakeButtonRead();

  int currentSnakeHeadY = snakeLocation[0][0];
  int currentSnakeHeadX = snakeLocation[0][1];

  int newSnakeHeadY = currentSnakeHeadY;
  int newSnakeHeadX = currentSnakeHeadX;
  bool hittingWall = true;
  bool hittingFruit = false;

  //Bepaal de nieuwe locatie van de slang doormiddel van snakeDirection
  switch (snakeDirection) {
    case 1:
      //Hij gaat naar boven
      if (newSnakeHeadY != 0) {
        newSnakeHeadY -= 1;
        hittingWall = false;
      }
      break;
    case 2:
      //Hij gaat naar rechts
      if (newSnakeHeadX != 15) {
        newSnakeHeadX += 1;
        hittingWall = false;
      }
      break;
    case 3:
      //Hij gaat naar beneden
      if (newSnakeHeadY != 15) {
        newSnakeHeadY += 1;
        hittingWall = false;
      }
      break;
    case 4:
      //Hij gaat naar links
      if (newSnakeHeadX != 0) {
        newSnakeHeadX -= 1;
        hittingWall = false;
      }
      break;
  }

  if (hittingWall) {
    onWallHit();
    return;
  }

  //Zet de locatie van de slang in de array
  if (newSnakeHeadY != currentSnakeHeadY || newSnakeHeadX != currentSnakeHeadX) {
    int newY = newSnakeHeadY;
    int newX = newSnakeHeadX;
    for (int i = 0; i < maxSnakeSize; i++) {
      //Vind de current location
      int currentY = snakeLocation[i][0];
      int currentX = snakeLocation[i][1];

      if (currentY == -1) {
        snakeOldTailX = newX;
        snakeOldTailY = newY;
        break;
      }

      //Zet de nieuwe locatie
      snakeLocation[i][0] = newY;
      snakeLocation[i][1] = newX;

      //Zet de new location voor de volgende item in de array
      newY = currentY;
      newX = currentX;
    }
  }
  bool hitFruit = isFruitHere(newSnakeHeadX, newSnakeHeadY);
  Serial.println(hitFruit);
  hittingFruit = hitFruit;

  if (hittingFruit) {
    onEatFruit();
  }

  //Kijk of we een nieuwe fruit moeten aanmaken
  if (fruitSpawnTick == 0) {
    onFruitSpawn();
    fruitSpawnTick = random(2, 3);
  }

  fruitSpawnTick --;

  //Zet de locatie van de slang op de matrix
  for (int i = 0; i < maxSnakeSize; i++) {
    if (snakeLocation[i][0] != -1) {
      int snakePixelY = snakeLocation[i][0];
      int snakePixelX = snakeLocation[i][1];
      int snakePixel = pixelLocationMap[snakePixelY][snakePixelX];
      matrix.setPixelColor(snakePixel, 0, 10, 0);
    }
  }

  for (int i = 0; i < maxFruitSpawned; i++) {
    if (fruitLocation[i][0] != -1) {
      int fruitPixelY = fruitLocation[i][0];
      int fruitPixelX = fruitLocation[i][1];
      int fruitPixel = pixelLocationMap[fruitPixelY][fruitPixelX];
      matrix.setPixelColor(fruitPixel, 10, 0, 0);
    }
  }
}

void snakeButtonRead() {
  directionButtonState = digitalRead(buttonUpPin);

  if (directionButtonState == HIGH && snakeDirection != 3) {
    snakeDirection = 1;
    return;
  }

  directionButtonState = digitalRead(buttonRightPin);

  if (directionButtonState == HIGH && snakeDirection != 4) {
    snakeDirection = 2;
    return;
  }

  directionButtonState = digitalRead(buttonDownPin);

  if (directionButtonState == HIGH && snakeDirection != 1) {
    snakeDirection = 3;
    return;
  }

  directionButtonState = digitalRead(buttonLeftPin);

  if (directionButtonState == HIGH && snakeDirection != 2) {
    snakeDirection = 4;
  }
}

bool isSnakeHere(int x, int y) {
  for (int i = 0; i < maxSnakeSize; i++) {
    if (snakeLocation[i][0] == y && snakeLocation[i][1] == x) {
      return true;
    }
  }
  return false;
}

bool isFruitHere(int x, int y) {
  for (int i = 0; i < maxFruitSpawned; i++) {
    if (fruitLocation[i][0] == y && fruitLocation[i][1] == x) {
      hitFruitIndex = i;
      return true;
    }
  }
  return false;
}

void getEmptyFruitLocation() {
  newFruitX = random(0, 16);
  newFruitY = random(0, 16);

  if (isSnakeHere(newFruitX, newFruitY) || isFruitHere(newFruitX, newFruitY)) {
    getEmptyFruitLocation();
  }
}

void onFruitSpawn() {
  if (fruitLocation[0][0] != -1 && fruitLocation[1][0] != -1 && fruitLocation[2][0] != -1) {
    return;
  }

  Serial.println("Spawning new fruit");
  getEmptyFruitLocation();

  if (fruitLocation[0][0] == -1) {
    fruitLocation[0][0] = newFruitY;
    fruitLocation[0][1] = newFruitX;
  }
  else if (fruitLocation[1][0] == -1) {
    fruitLocation[1][0] = newFruitY;
    fruitLocation[1][1] = newFruitX;
  }
  else if (fruitLocation[2][0] == -1) {
    fruitLocation[2][0] = newFruitY;
    fruitLocation[2][1] = newFruitX;
  }
}

void onEatFruit() {
  score++;

  fruitLocation[hitFruitIndex][0] = -1;
  fruitLocation[hitFruitIndex][1] = -1;

  for (int i = 0; i < maxSnakeSize; i++) {
    //Vind de current location
    int currentY = snakeLocation[i][0];
    int currentX = snakeLocation[i][1];

    if (currentY == -1) {
      snakeLocation[i][0] = snakeOldTailY;
      snakeLocation[i][1] = snakeOldTailX;
      break;
    }
  }
}

void onWallHit() {
  //TODO
  startScreenSetup();
}

void startScreenSetup() {
  matrix.clear();

  Serial.println("Current score");
  Serial.println(score);
  snakeRunning = false;

  //Laat een snake zien op het start scherm
  matrix.setPixelColor(pixelLocationMap[0][7], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[0][8], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[0][9], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[0][10], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[0][11], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[0][12], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[0][13], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[0][14], 10, 10, 0);

  matrix.setPixelColor(pixelLocationMap[1][8], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[1][9], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[1][13], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[1][14], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[1][15], 0, 10, 0);

  matrix.setPixelColor(pixelLocationMap[2][14], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[2][15], 0, 10, 0);

  matrix.setPixelColor(pixelLocationMap[3][14], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[3][15], 0, 10, 0);


  matrix.setPixelColor(pixelLocationMap[4][4], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[4][5], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[4][6], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[4][7], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[4][8], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[4][9], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[4][13], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[4][14], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[4][15], 0, 10, 0);


  matrix.setPixelColor(pixelLocationMap[5][3], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[5][4], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[5][5], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[5][6], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[5][7], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[5][8], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[5][9], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[5][10], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[5][11], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[5][12], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[5][13], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[5][14], 0, 10, 0);

  matrix.setPixelColor(pixelLocationMap[6][2], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[6][3], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[6][4], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[6][9], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[6][10], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[6][11], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[6][12], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[6][13], 0, 10, 0);

  matrix.setPixelColor(pixelLocationMap[7][1], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[7][2], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[7][3], 0, 10, 0);

  matrix.setPixelColor(pixelLocationMap[8][1], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[8][2], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[8][5], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[8][6], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[8][7], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[8][8], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[8][9], 10, 10, 0);

  matrix.setPixelColor(pixelLocationMap[9][1], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[9][2], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[9][3], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[9][4], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[9][5], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[9][6], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[9][7], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[9][8], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[9][9], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[9][10], 10, 10, 0);

  matrix.setPixelColor(pixelLocationMap[10][2], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[10][3], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[10][4], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[10][5], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[10][9], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[10][10], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[10][11], 10, 10, 0);

  matrix.setPixelColor(pixelLocationMap[11][10], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[11][11], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[11][12], 0, 10, 0);

  matrix.setPixelColor(pixelLocationMap[12][2], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[12][3], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[12][4], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[12][5], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[12][6], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[12][11], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[12][12], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[12][13], 0, 10, 0);

  matrix.setPixelColor(pixelLocationMap[13][1], 10, 10, 10);
  matrix.setPixelColor(pixelLocationMap[13][2], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[13][3], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[13][4], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[13][5], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[13][6], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[13][7], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[13][12], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[13][13], 0, 10, 0);

  matrix.setPixelColor(pixelLocationMap[14][1], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[14][2], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[14][3], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[14][4], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[14][6], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[14][7], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[14][8], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[14][12], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[14][13], 0, 10, 0);

  matrix.setPixelColor(pixelLocationMap[15][1], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[15][2], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[15][3], 10, 10, 10);
  matrix.setPixelColor(pixelLocationMap[15][7], 0, 10, 0);
  matrix.setPixelColor(pixelLocationMap[15][8], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[15][9], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[15][11], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[15][12], 10, 10, 0);
  matrix.setPixelColor(pixelLocationMap[15][13], 0, 10, 0);
}
