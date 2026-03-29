// Snake.h
#ifndef SNAKE_H
#define SNAKE_H

#include "Game.h"
#include "Sound.h"
#include <EEPROM.h>

enum snakeDirection {
  SNAKE_UP,
  SNAKE_DOWN,
  SNAKE_LEFT,
  SNAKE_RIGHT
};

struct Segment {
  int x, y;
};

class SnakeGame : public Game {
  private:
    static const int MAX_LENGTH = 99;

    Segment snake[MAX_LENGTH];
    int snakeLength;
    snakeDirection direction;
    int appleX, appleY;

    Segment lastTail;
    bool moved;
    bool appleEaten;
    bool firstDraw;

    int lives;
    static const int MAX_LIVES = 3;

    int highScore;
    static const int EEPROM_ADDR = 0;

    const int gridSize = 8;
    const int gridW    = 28;
    const int gridH    = 14;
    const int offsetX  = 8;
    const int offsetY  = 17;

    const uint16_t COLOR_CELL_DARK  = 0x1A2A;
    const uint16_t COLOR_CELL_LIGHT = 0x1C4E;
    const uint16_t COLOR_BORDER     = 0x2965;
    const uint16_t COLOR_HEAD       = TFT_YELLOW;
    const uint16_t COLOR_BODY       = 0xC600;

    unsigned long lastMove;
    int moveInterval;
    static const int INTERVAL_START = 250;
    static const int INTERVAL_MIN   = 60;
    static const int INTERVAL_STEP  = 10;

    void updateSpeed() {
      moveInterval = max(INTERVAL_MIN, INTERVAL_START - score * INTERVAL_STEP);
    }

    void drawGrid() {
      for (int row = 0; row < gridH; row++) {
        for (int col = 0; col < gridW; col++) {
          uint16_t color = ((row + col) % 2 == 0)
                           ? COLOR_CELL_DARK : COLOR_CELL_LIGHT;
          screen->fillRect(
            offsetX + col * gridSize,
            offsetY + row * gridSize,
            gridSize, gridSize, color
          );
        }
      }
    }

    void drawBorder() {
      screen->drawRect(
        offsetX - 1, offsetY - 1,
        gridW * gridSize + 2, gridH * gridSize + 2,
        COLOR_BORDER
      );
      screen->drawRect(
        offsetX - 2, offsetY - 2,
        gridW * gridSize + 4, gridH * gridSize + 4,
        0x1082
      );
    }

    void drawHUD() {
      screen->fillRect(0, 0, 240, 16, TFT_BLACK);

      screen->setTextColor(TFT_WHITE, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(offsetX, 4);
      screen->print("SCORE:");
      char buf[6];
      sprintf(buf, "%04d", score);
      screen->setTextColor(TFT_YELLOW, TFT_BLACK);
      screen->print(buf);

      screen->setTextColor(0x8410, TFT_BLACK);
      screen->setCursor(90, 4);
      screen->print("HI:");
      screen->setTextColor(TFT_CYAN, TFT_BLACK);
      char hbuf[6];
      sprintf(hbuf, "%04d", highScore);
      screen->print(hbuf);

      int heartX = 240 - MAX_LIVES * 14 - offsetX;
      for (int i = 0; i < MAX_LIVES; i++) {
        uint16_t c = (i < lives) ? TFT_RED : 0x3186;
        int hx = heartX + i * 14;
        int hy = 3;
        screen->fillRect(hx + 1, hy,     2, 1, c);
        screen->fillRect(hx + 4, hy,     2, 1, c);
        screen->fillRect(hx,     hy + 1, 6, 1, c);
        screen->fillRect(hx,     hy + 2, 6, 1, c);
        screen->fillRect(hx + 1, hy + 3, 4, 1, c);
        screen->fillRect(hx + 2, hy + 4, 2, 1, c);
        screen->fillRect(hx + 3, hy + 5, 1, 1, c);
      }
    }

    void drawSegment(int col, int row, uint16_t color) {
      screen->fillRect(
        offsetX + col * gridSize + 1,
        offsetY + row * gridSize + 1,
        gridSize - 2, gridSize - 2, color
      );
    }

    void clearCell(int col, int row) {
      uint16_t color = ((row + col) % 2 == 0)
                       ? COLOR_CELL_DARK : COLOR_CELL_LIGHT;
      screen->fillRect(
        offsetX + col * gridSize,
        offsetY + row * gridSize,
        gridSize, gridSize, color
      );
    }

    void spawnApple() {
      bool onSnake;
      do {
        onSnake = false;
        appleX  = random(0, gridW);
        appleY  = random(0, gridH);
        for (int i = 0; i < snakeLength; i++) {
          if (snake[i].x == appleX && snake[i].y == appleY) {
            onSnake = true;
            break;
          }
        }
      } while (onSnake);
    }

    void drawApple() {
      int px = offsetX + appleX * gridSize;
      int py = offsetY + appleY * gridSize;
      screen->fillRect(px + 1, py + 1, gridSize - 2, gridSize - 2, TFT_RED);
      screen->fillRect(px + 2, py + 2, 2, 2, 0xFBEF);
    }

    void moveSnake() {
      if (millis() - lastMove < (unsigned long)moveInterval) return;
      lastMove = millis();

      lastTail = snake[snakeLength - 1];
      for (int i = snakeLength - 1; i > 0; i--)
        snake[i] = snake[i - 1];

      switch (direction) {
        case SNAKE_UP:    snake[0].y -= 1; break;
        case SNAKE_DOWN:  snake[0].y += 1; break;
        case SNAKE_LEFT:  snake[0].x -= 1; break;
        case SNAKE_RIGHT: snake[0].x += 1; break;
      }
      moved = true;
    }

    void respawnSnake() {
      direction  = SNAKE_RIGHT;
      int startX = gridW / 2;
      int startY = gridH / 2;

      snake[0].x = startX;
      snake[0].y = startY;
      for (int i = 1; i < snakeLength; i++) {
        snake[i].x = startX - i;
        snake[i].y = startY;
        if (snake[i].x < 0) {
          snake[i].x = 0;
          snake[i].y = startY + (i / gridW) + 1;
        }
      }
      firstDraw = true;
    }

    void loadHighScore() {
      int lo  = EEPROM.read(EEPROM_ADDR);
      int hi  = EEPROM.read(EEPROM_ADDR + 1);
      int val = lo | (hi << 8);
      highScore = (val == 65535) ? 0 : val;
    }

    void saveHighScore() {
      if (score > highScore) {
        highScore = score;
        EEPROM.write(EEPROM_ADDR,     highScore & 0xFF);
        EEPROM.write(EEPROM_ADDR + 1, (highScore >> 8) & 0xFF);
        EEPROM.commit();
      }
    }

    void checkCollision() {
      bool collision = false;

      if (snake[0].x < 0 || snake[0].x >= gridW ||
          snake[0].y < 0  || snake[0].y >= gridH) {
        collision = true;
      }

      if (!collision) {
        for (int i = 1; i < snakeLength; i++) {
          if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
            collision = true;
            break;
          }
        }
      }

      if (collision) {
        lives--;
        Sound::collision();
        if (lives <= 0) {
          saveHighScore();
          gameOverDrawn       = false;
          gameOverSoundPlayed = false; // ← reset pour que render() joue le son
          state               = GAME_OVER;
        } else {
          respawnSnake();
        }
      }
    }

    void checkApple() {
      if (snake[0].x == appleX && snake[0].y == appleY) {
        if (snakeLength < MAX_LENGTH) snakeLength++;
        score++;
        appleEaten = true;
        Sound::point();
        spawnApple();
        updateSpeed();
      }
    }

    void renderSnakeGameOver() {
      if (gameOverDrawn) return;
      gameOverDrawn = true;

      // ── Son de défaite — joué UNE SEULE FOIS ici ──
      if (!gameOverSoundPlayed) {
        gameOverSoundPlayed = true;
        Sound::play(220, 120);
        delay(40);
        Sound::play(185, 120);
        delay(40);
        Sound::play(155, 180);
        delay(60);
        int freqs[] = {147, 139, 131, 123, 117, 110, 104, 98};
        for (int i = 0; i < 8; i++) {
          Sound::play(freqs[i], 60);
          delay(10);
        }
        Sound::play(82, 400);
        delay(80);
        Sound::play(73, 150);
        delay(100);
        Sound::play(65, 300);
      }

      screen->fillScreen(TFT_BLACK);

      screen->setTextColor(TFT_DARKGREY, TFT_BLACK);
      screen->setTextSize(3);
      screen->setCursor(31, 11);
      screen->print("GAME OVER");
      screen->setTextColor(TFT_RED, TFT_BLACK);
      screen->setCursor(29, 9);
      screen->print("GAME OVER");

      screen->drawFastHLine(20, 40, 200, TFT_DARKGREY);

      screen->setTextColor(TFT_WHITE, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(70, 48);
      screen->print("SCORE FINAL");

      char buf[8];
      sprintf(buf, "%04d", score);
      int scoreX = (240 - strlen(buf) * 18) / 2;
      screen->setTextColor(TFT_YELLOW, TFT_BLACK);
      screen->setTextSize(3);
      screen->setCursor(scoreX, 58);
      screen->print(buf);

      screen->drawFastHLine(20, 88, 200, TFT_DARKGREY);

      screen->setTextColor(TFT_DARKGREY, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(45, 94);
      screen->print("MEILLEUR SCORE : ");

      char hbuf[8];
      sprintf(hbuf, "%04d", highScore);
      screen->setTextColor(
        score == highScore && score > 0 ? TFT_GREEN : TFT_CYAN,
        TFT_BLACK
      );
      screen->print(hbuf);

      if (score == highScore && score > 0) {
        screen->setTextColor(TFT_GREEN, TFT_BLACK);
        screen->setCursor(35, 106);
        screen->print("NOUVEAU RECORD !");
      }

      screen->setTextColor(TFT_CYAN, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(10, 124);
      screen->print("[A] Rejouer");
      screen->setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      screen->setCursor(130, 124);
      screen->print("[B] Menu");
    }

  public:
    SnakeGame(TFT_eSPI* display) : Game(display) {}

    void init() override {
      lives               = MAX_LIVES;
      score               = 0;
      state               = IN_PROGRESS;
      moved               = false;
      appleEaten          = false;
      firstDraw           = true;
      lastMove            = 0;
      moveInterval        = INTERVAL_START;
      gameOverDrawn       = false;
      gameOverSoundPlayed = false;

      snakeLength = 3;
      direction   = SNAKE_RIGHT;

      for (int i = 0; i < snakeLength; i++) {
        snake[i].x = (gridW / 2) - i;
        snake[i].y = gridH / 2;
      }

      loadHighScore();
      spawnApple();
    }

    void update(Buttons buttons) override {
      // ── GAME_OVER : update ne fait rien, le son est dans render() ──
      if (state == GAME_OVER) return;
      if (state == PAUSED)    return;

      if (buttons.up    && direction != SNAKE_DOWN)  direction = SNAKE_UP;
      if (buttons.down  && direction != SNAKE_UP)    direction = SNAKE_DOWN;
      if (buttons.right && direction != SNAKE_LEFT)  direction = SNAKE_RIGHT;
      if (buttons.left  && direction != SNAKE_RIGHT) direction = SNAKE_LEFT;

      moved      = false;
      appleEaten = false;

      moveSnake();
      if (state != GAME_OVER) checkCollision();
      if (state != GAME_OVER) checkApple();
    }

    void forceRedraw() override {
      firstDraw = true;
    }

    void render() override {
      if (state == GAME_OVER) {
        renderSnakeGameOver(); // son joué ici, une seule fois
        return;
      }

      if (firstDraw) {
        firstDraw = false;
        drawGrid();
        drawBorder();
        drawApple();
        for (int i = 0; i < snakeLength; i++) {
          drawSegment(snake[i].x, snake[i].y,
                      i == 0 ? COLOR_HEAD : COLOR_BODY);
        }
        drawHUD();
        return;
      }

      if (!moved) return;

      if (!appleEaten) {
        clearCell(lastTail.x, lastTail.y);
      }

      drawSegment(snake[0].x, snake[0].y, COLOR_HEAD);

      if (snakeLength > 1) {
        drawSegment(snake[1].x, snake[1].y, COLOR_BODY);
      }

      if (appleEaten) {
        drawGrid();
        drawApple();
        for (int i = 0; i < snakeLength; i++) {
          drawSegment(snake[i].x, snake[i].y,
                      i == 0 ? COLOR_HEAD : COLOR_BODY);
        }
        drawHUD();
      }
    }

    virtual ~SnakeGame() {}
};

#endif