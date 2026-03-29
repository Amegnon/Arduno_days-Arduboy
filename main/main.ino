#include <TFT_eSPI.h>
#include <EEPROM.h>

#include "Game.h"
#include "Menu.h"
#include "Snake.h"
#include "Morpion.h"
#include "Sound.h"
#include "Tetris.h"
#include "Maze.h"

#define BTN_UP     25
#define BTN_DOWN   26
#define BTN_LEFT   27
#define BTN_RIGHT  32
#define BTN_A      13
#define BTN_B      15
#define BUZZER_PIN 17

Buttons buttons;

enum stateConsole {
  STARTING,
  MENU,
  PLAYING
};

TFT_eSPI     screen        = TFT_eSPI();
Menu         menu(&screen);
stateConsole consoleState  = STARTING;
Game*        currentGame   = nullptr;
int          currentGameId = -1;

void readButtons() {
  bool prevUp    = buttons.up;
  bool prevDown  = buttons.down;
  bool prevLeft  = buttons.left;
  bool prevRight = buttons.right;
  bool prevA     = buttons.a;
  bool prevB     = buttons.b;

  buttons.up    = !digitalRead(BTN_UP);
  buttons.down  = !digitalRead(BTN_DOWN);
  buttons.left  = !digitalRead(BTN_LEFT);
  buttons.right = !digitalRead(BTN_RIGHT);
  buttons.a     = !digitalRead(BTN_A);
  buttons.b     = !digitalRead(BTN_B);

  buttons.upPressed    = buttons.up    && !prevUp;
  buttons.downPressed  = buttons.down  && !prevDown;
  buttons.leftPressed  = buttons.left  && !prevLeft;
  buttons.rightPressed = buttons.right && !prevRight;
  buttons.aPressed     = buttons.a     && !prevA;
  buttons.bPressed     = buttons.b     && !prevB;
}

// ─────────────────────────────────────────
//  Constellation d'étoiles
// ─────────────────────────────────────────
struct Star {
  int x, y;
  uint8_t  size;
  uint16_t color;
};

Star stars[] = {
  {5,   3,  1, 0xFFFF}, {18,  12, 2, 0xAD55}, {35,  7,  1, 0xFFFF},
  {52,  20, 1, 0x8410}, {70,  5,  2, 0xFFFF}, {88,  15, 1, 0xAD55},
  {105, 8,  1, 0xFFFF}, {122, 18, 2, 0x8410}, {140, 4,  1, 0xFFFF},
  {158, 11, 1, 0xAD55}, {175, 6,  2, 0xFFFF}, {192, 14, 1, 0x8410},
  {210, 3,  1, 0xFFFF}, {225, 10, 2, 0xAD55}, {235, 5,  1, 0xFFFF},
  {12,  25, 2, 0xFFFF}, {30,  30, 1, 0x8410}, {48,  28, 1, 0xFFFF},
  {65,  35, 2, 0xAD55}, {83,  22, 1, 0xFFFF}, {100, 32, 1, 0x8410},
  {118, 27, 2, 0xFFFF}, {135, 38, 1, 0xAD55}, {153, 24, 1, 0xFFFF},
  {170, 33, 2, 0x8410}, {188, 19, 1, 0xFFFF}, {205, 29, 1, 0xAD55},
  {220, 36, 2, 0xFFFF}, {232, 22, 1, 0x8410}, {8,   40, 1, 0xFFFF},
  {25,  55, 2, 0xAD55}, {42,  48, 1, 0xFFFF}, {60,  62, 1, 0x8410},
  {78,  50, 2, 0xFFFF}, {95,  65, 1, 0xAD55}, {112, 45, 1, 0xFFFF},
  {130, 60, 2, 0x8410}, {147, 52, 1, 0xFFFF}, {165, 68, 1, 0xAD55},
  {183, 55, 2, 0xFFFF}
};

const int NUM_STARS = sizeof(stars) / sizeof(stars[0]);

void drawStars() {
  for (int i = 0; i < NUM_STARS; i++) {
    if (stars[i].size == 1) {
      screen.drawPixel(stars[i].x, stars[i].y, stars[i].color);
    } else {
      screen.drawPixel(stars[i].x,     stars[i].y,     stars[i].color);
      screen.drawPixel(stars[i].x - 1, stars[i].y,     stars[i].color);
      screen.drawPixel(stars[i].x + 1, stars[i].y,     stars[i].color);
      screen.drawPixel(stars[i].x,     stars[i].y - 1, stars[i].color);
      screen.drawPixel(stars[i].x,     stars[i].y + 1, stars[i].color);
    }
  }
}

void twinkleStars(int cycles) {
  for (int c = 0; c < cycles; c++) {
    int i = random(0, NUM_STARS);
    screen.drawPixel(stars[i].x, stars[i].y, TFT_BLACK);
    delay(30);
    if (stars[i].size == 1) {
      screen.drawPixel(stars[i].x, stars[i].y, stars[i].color);
    } else {
      screen.drawPixel(stars[i].x,     stars[i].y,     stars[i].color);
      screen.drawPixel(stars[i].x - 1, stars[i].y,     stars[i].color);
      screen.drawPixel(stars[i].x + 1, stars[i].y,     stars[i].color);
      screen.drawPixel(stars[i].x,     stars[i].y - 1, stars[i].color);
      screen.drawPixel(stars[i].x,     stars[i].y + 1, stars[i].color);
    }
    delay(20);
  }
}

// ─────────────────────────────────────────
//  Écran de démarrage
// ─────────────────────────────────────────
void showBoot() {
  screen.fillScreen(TFT_BLACK);
  drawStars();

  screen.drawRect(10, 10, 220, 80, TFT_CYAN);
  screen.drawRect(12, 12, 216, 76, TFT_CYAN);
  screen.fillRect(13, 13, 214, 74, TFT_BLACK);

  screen.setTextColor(TFT_CYAN, TFT_BLACK);
  screen.setTextSize(3);
  screen.setCursor(30, 25);
  screen.print(" ARDUBOY ");

  screen.setTextSize(1);
  screen.setTextColor(TFT_WHITE, TFT_BLACK);
  screen.setCursor(55, 60);
  screen.print("UNE CONSOLE DE JEUX");

  twinkleStars(20);
  Sound::boot();

  screen.drawRect(20, 100, 200, 15, TFT_WHITE);
  for (int i = 0; i <= 100; i += 2) {
    int largeur = (i * 196) / 100;
    screen.fillRect(22, 102, largeur, 10, TFT_GREEN);
    if (i % 10 == 0) twinkleStars(3);
    delay(20);
  }

  delay(500);
  consoleState = MENU;
}

// ─────────────────────────────────────────
//  Lancement d'un jeu
// ─────────────────────────────────────────
void launchGame(int gameId) {
  if (currentGame != nullptr) {
    delete currentGame;
    currentGame = nullptr;
  }

  switch (gameId) {
    case 0: currentGame = new SnakeGame(&screen);   break;
    case 1: currentGame = new MorpionGame(&screen); break;
    case 2: currentGame = new TetrisGame(&screen);  break;
    case 3: currentGame = new MazeGame(&screen);    break;

  if (currentGame != nullptr) {
    currentGame->init();
    consoleState = PLAYING;
  }
}

// ─────────────────────────────────────────
//  SETUP
// ─────────────────────────────────────────
void setup() {
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  screen.init();
  screen.setRotation(1);
  screen.fillScreen(TFT_BLACK);

  pinMode(BTN_UP,    INPUT_PULLUP);
  pinMode(BTN_DOWN,  INPUT_PULLUP);
  pinMode(BTN_LEFT,  INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_A,     INPUT_PULLUP);
  pinMode(BTN_B,     INPUT_PULLUP);

  Sound::init(BUZZER_PIN);
  EEPROM.begin(20);

  if (EEPROM.read(10) != 0xAA) {
    for (int i = 0; i < 10; i++) EEPROM.write(i, 0);
    EEPROM.write(10, 0xAA);
    EEPROM.commit();
  }

  menu.addGame("Snake",      "Mange les pommes!",         "", 0);
  menu.addGame("Morpion",    "2 joueurs - A pour jouer!", "", 1);
  menu.addGame("Tetris",     "Aligne les lignes!",        "", 2);
  menu.addGame("Labyrinthe", "Trouve la sortie!",         "", 3);


  showBoot();
}

// ─────────────────────────────────────────
//  LOOP — Machine à états
// ─────────────────────────────────────────
void loop() {
  readButtons();
  Sound::update();

  switch (consoleState) {

    case STARTING:
      break;

    case MENU: {
      int selectedId = menu.update(buttons);
      menu.render();
      if (selectedId != -1) {
        currentGameId = selectedId;
        launchGame(currentGameId);
      }
      break;
    }

    case PLAYING: {
      if (currentGame == nullptr) break;

      // ── GAME OVER ──
      if (currentGame->isGameOver()) {
        currentGame->render();
        if (buttons.aPressed) {
          launchGame(currentGameId);
        }
        if (buttons.bPressed) {
          consoleState = MENU;
          menu.forceRedraw();
        }
        break;
      }

      // ── PAUSE ──
      // renderPause() dessiné UNE SEULE FOIS grâce à pauseDrawn
      if (currentGame->isPaused()) {
        currentGame->renderPause();

        // A = Reprendre
        if (buttons.aPressed) {
          currentGame->resume();
          currentGame->forceRedraw();  // effacer la boîte pause
        }

        // B = Quitter vers menu
        if (buttons.bPressed) {
          consoleState = MENU;
          menu.forceRedraw();
        }
        break;
      }

      // ── Jeu en cours : B = mettre en pause ──
      if (buttons.bPressed) {
        currentGame->pause();
        break;
      }

      // ── Cycle normal ──
      currentGame->update(buttons);
      currentGame->render();
      break;
    }
  }

  delay(1);
}