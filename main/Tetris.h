#ifndef TETRIS_H
#define TETRIS_H

#include "Game.h"
#include "Sound.h"

class TetrisGame : public Game {
  private:

    const int COLS    = 12;
    const int ROWS    = 17;
    const int CELL    = 7;
    const int offsetX = 52;
    const int offsetY = 16;

    static const int EEPROM_ADDR = 6;

    uint16_t grid[17][12];

    const int pieces[7][4][2] = {
      {{0,0},{1,0},{2,0},{3,0}},
      {{0,0},{0,1},{1,0},{1,1}},
      {{1,0},{0,1},{1,1},{2,1}},
      {{0,0},{1,0},{1,1},{2,1}},
      {{1,0},{2,0},{0,1},{1,1}},
      {{0,0},{0,1},{1,1},{2,1}},
      {{2,0},{0,1},{1,1},{2,1}},
    };

    const uint16_t pieceColors[7] = {
      TFT_CYAN, TFT_YELLOW, TFT_MAGENTA,
      TFT_GREEN, TFT_RED, TFT_BLUE, TFT_ORANGE
    };

    int curPiece;
    int curColor;
    int curX, curY;
    int rotation;
    int curShape[4][2];

    unsigned long lastFall;
    int  fallInterval;
    bool needsRedraw;

    void applyRotation(int piece, int rot, int shape[4][2]) {
      for (int i = 0; i < 4; i++) {
        int x = pieces[piece][i][0];
        int y = pieces[piece][i][1];
        for (int r = 0; r < rot; r++) {
          int tmp = x;
          x = y;
          y = 3 - tmp;
        }
        shape[i][0] = x;
        shape[i][1] = y;
      }
    }

    bool isValid(int px, int py, int shape[4][2]) {
      for (int i = 0; i < 4; i++) {
        int nx = px + shape[i][0];
        int ny = py + shape[i][1];
        if (nx < 0 || nx >= COLS || ny >= ROWS) return false;
        if (ny >= 0 && grid[ny][nx] != TFT_BLACK) return false;
      }
      return true;
    }

    void lockPiece() {
      for (int i = 0; i < 4; i++) {
        int nx = curX + curShape[i][0];
        int ny = curY + curShape[i][1];
        if (ny >= 0) grid[ny][nx] = curColor;
      }
      Sound::point();
    }

    void clearLines() {
      int cleared = 0;
      for (int r = ROWS - 1; r >= 0; r--) {
        bool full = true;
        for (int c = 0; c < COLS; c++) {
          if (grid[r][c] == TFT_BLACK) { full = false; break; }
        }
        if (full) {
          cleared++;
          for (int row = r; row > 0; row--)
            for (int c = 0; c < COLS; c++)
              grid[row][c] = grid[row-1][c];
          for (int c = 0; c < COLS; c++) grid[0][c] = TFT_BLACK;
          r++;
        }
      }
      if (cleared > 0) {
        score       += cleared * cleared * 10;
        needsRedraw  = true;
        fallInterval = max(100, 500 - score * 2);
        Sound::win();
      }
    }

    void spawnPiece() {
      curPiece = random(0, 7);
      curColor = pieceColors[curPiece];
      curX     = COLS / 2 - 2;
      curY     = 0;
      rotation = 0;
      applyRotation(curPiece, rotation, curShape);

      if (!isValid(curX, curY, curShape)) {
        saveHighScore(EEPROM_ADDR);
        // ── Reset du flag pour que render() joue le son ──
        gameOverSoundPlayed = false;
        state               = GAME_OVER;
      }
    }

    void drawCell(int col, int row, uint16_t color) {
      screen->fillRect(
        offsetX + col * CELL,
        offsetY + row * CELL,
        CELL - 1, CELL - 1,
        color
      );
    }

    void drawHUD() {
      screen->fillRect(0, 0, 50, 135, TFT_BLACK);

      screen->setTextColor(TFT_WHITE, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(2, 4);
      screen->print("SCORE");
      screen->setTextColor(TFT_YELLOW, TFT_BLACK);
      screen->setCursor(2, 14);
      char buf[6];
      sprintf(buf, "%05d", score);
      screen->print(buf);

      screen->setTextColor(0x8410, TFT_BLACK);
      screen->setCursor(2, 30);
      screen->print("BEST");
      screen->setTextColor(TFT_CYAN, TFT_BLACK);
      screen->setCursor(2, 40);
      char hbuf[6];
      sprintf(hbuf, "%05d", highScore);
      screen->print(hbuf);

      screen->setTextColor(TFT_DARKGREY, TFT_BLACK);
      screen->setCursor(2, 60);
      screen->print("LVL");
      screen->setTextColor(TFT_GREEN, TFT_BLACK);
      screen->setCursor(2, 70);
      screen->print((500 - fallInterval) / 40 + 1);
    }

  public:
    TetrisGame(TFT_eSPI* display) : Game(display) {}

    void init() override {
      for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
          grid[r][c] = TFT_BLACK;

      score               = 0;
      state               = IN_PROGRESS;
      fallInterval        = 500;
      needsRedraw         = true;
      lastFall            = 0;
      gameOverDrawn       = false;
      gameOverSoundPlayed = false;

      loadHighScore(EEPROM_ADDR);
      spawnPiece();
    }

    void forceRedraw() override {
      needsRedraw = true;
    }

    void update(Buttons buttons) override {
      // ── GAME_OVER : update ne joue plus aucun son ──
      if (state == GAME_OVER) return;
      if (state == PAUSED)    return;

      int  tmpShape[4][2];
      bool moved = false;

      if (buttons.leftPressed) {
        if (isValid(curX - 1, curY, curShape)) { curX--; moved = true; }
      }

      if (buttons.rightPressed) {
        if (isValid(curX + 1, curY, curShape)) { curX++; moved = true; }
      }

      if (buttons.aPressed || buttons.upPressed) {
        int newRot = (rotation + 1) % 4;
        applyRotation(curPiece, newRot, tmpShape);
        if (isValid(curX, curY, tmpShape)) {
          rotation = newRot;
          applyRotation(curPiece, rotation, curShape);
          moved = true;
        }
      }

      int interval = buttons.down ? 50 : fallInterval;
      if (millis() - lastFall >= (unsigned long)interval) {
        lastFall = millis();
        if (isValid(curX, curY + 1, curShape)) {
          curY++;
          moved = true;
        } else {
          lockPiece();
          clearLines();
          spawnPiece(); // gameOverSoundPlayed = false posé dans spawnPiece()
          needsRedraw = true;
        }
      }

      if (moved) needsRedraw = true;
    }

    void render() override {
      // ── Game Over : son joué dans renderGameOver() de Game.h ──
      if (state == GAME_OVER) {
        renderGameOver();
        return;
      }

      if (!needsRedraw) return;
      needsRedraw = false;

      screen->fillRect(offsetX, offsetY, COLS * CELL, ROWS * CELL, TFT_BLACK);

      for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
          if (grid[r][c] != TFT_BLACK)
            drawCell(c, r, grid[r][c]);

      for (int i = 0; i < 4; i++)
        drawCell(curX + curShape[i][0], curY + curShape[i][1], curColor);

      screen->drawRect(
        offsetX - 1, offsetY - 1,
        COLS * CELL + 2, ROWS * CELL + 2,
        TFT_WHITE
      );

      drawHUD();
    }

    virtual ~TetrisGame() {}
};

#endif