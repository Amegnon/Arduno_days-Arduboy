#ifndef MAZE_H
#define MAZE_H

#include "Game.h"
#include "Sound.h"

class MazeGame : public Game {
  private:

    static const int MAX_COLS = 19;
    static const int MAX_ROWS = 11;

    int cols, rows;
    uint8_t maze[MAX_ROWS][MAX_COLS];
    int cellW, cellH;
    int offsetX, offsetY;

    int playerCol, playerRow;
    int exitCol,   exitRow;

    int level;
    int timeBonus;

    unsigned long levelStartTime;
    int           timeLimit;
    int           timeLeft;
    bool          alertPlayed;

    enum MazeState { PLAYING, LEVEL_WIN, TIME_OVER };
    MazeState mazeState;

    bool winScreenDrawn;
    bool firstDraw;
    int  prevPlayerCol, prevPlayerRow;

    static const int STACK_SIZE = 120;
    int stackCol[STACK_SIZE];
    int stackRow[STACK_SIZE];
    int stackTop;

    unsigned long lastMove;
    const int MOVE_DELAY = 150;

    const uint16_t C_WALL   = 0x2965;
    const uint16_t C_FLOOR  = 0x0841;
    const uint16_t C_PLAYER = TFT_RED;
    const uint16_t C_EXIT   = TFT_YELLOW;
    const uint16_t C_BAR_G  = TFT_GREEN;
    const uint16_t C_BAR_Y  = TFT_YELLOW;
    const uint16_t C_BAR_R  = TFT_RED;

    void generateMaze() {
      for (int r = 0; r < rows; r++)
        for (int c = 0; c < cols; c++)
          maze[r][c] = 0;

      stackTop    = 0;
      maze[1][1]  = 1;
      stackCol[0] = 1;
      stackRow[0] = 1;
      stackTop    = 1;

      const int dc[4] = {0,  0, -2, 2};
      const int dr[4] = {-2, 2,  0, 0};

      while (stackTop > 0) {
        int cc = stackCol[stackTop - 1];
        int cr = stackRow[stackTop - 1];

        int validDir[4], validCount = 0;
        for (int d = 0; d < 4; d++) {
          int nc = cc + dc[d];
          int nr = cr + dr[d];
          if (nc > 0 && nc < cols - 1 &&
              nr > 0 && nr < rows - 1 &&
              maze[nr][nc] == 0) {
            validDir[validCount++] = d;
          }
        }

        if (validCount == 0) {
          stackTop--;
        } else {
          int chosen = validDir[random(0, validCount)];
          int nc = cc + dc[chosen];
          int nr = cr + dr[chosen];
          maze[cr + dr[chosen] / 2][cc + dc[chosen] / 2] = 1;
          maze[nr][nc] = 1;
          if (stackTop < STACK_SIZE) {
            stackCol[stackTop] = nc;
            stackRow[stackTop] = nr;
            stackTop++;
          }
        }
      }

      int extra = level * 2;
      for (int i = 0; i < extra; i++) {
        int c = 1 + random(0, (cols - 2) / 2) * 2;
        int r = 1 + random(0, (rows - 2) / 2) * 2;
        if (c > 0 && c < cols - 1 && r > 0 && r < rows - 1)
          maze[r][c] = 1;
      }

      playerCol = 1;
      playerRow = 1;
      exitCol   = cols - 2;
      exitRow   = rows - 2;
      maze[exitRow][exitCol] = 1;
    }

    void computeLayout() {
      if      (level <= 1) { cols = 11; rows = 7;  }
      else if (level == 2) { cols = 13; rows = 9;  }
      else if (level == 3) { cols = 15; rows = 11; }
      else if (level == 4) { cols = 17; rows = 11; }
      else                 { cols = MAX_COLS; rows = MAX_ROWS; }

      int cell = min(240 / cols, 119 / rows);
      cellW   = cell;
      cellH   = cell;
      offsetX = (240 - cols * cellW) / 2;
      offsetY = 16 + (119 - rows * cellH) / 2;
    }

    int computeTimeLimit() {
      return max(8, 15 - (level - 1) * 1);
    }

    bool isWall(int c, int r) {
      if (c < 0 || c >= cols || r < 0 || r >= rows) return true;
      return maze[r][c] == 0;
    }

    int cellPX(int c) { return offsetX + c * cellW; }
    int cellPY(int r) { return offsetY + r * cellH; }

    void drawMaze() {
      screen->fillScreen(TFT_BLACK);
      for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
          uint16_t col = (maze[r][c] == 0) ? C_WALL : C_FLOOR;
          screen->fillRect(cellPX(c), cellPY(r), cellW, cellH, col);
          if (maze[r][c] == 1 && cellW > 4)
            screen->drawRect(cellPX(c), cellPY(r), cellW, cellH, 0x1082);
        }
      }
      screen->fillRect(cellPX(exitCol) + 1, cellPY(exitRow) + 1,
                       cellW - 2, cellH - 2, C_EXIT);
      drawPlayer(playerCol, playerRow);
    }

    void drawPlayer(int c, int r) {
      screen->fillRect(cellPX(c) + 1, cellPY(r) + 1,
                       cellW - 2, cellH - 2, C_PLAYER);
    }

    void erasePlayer(int c, int r) {
      screen->fillRect(cellPX(c), cellPY(r), cellW, cellH, C_FLOOR);
      if (cellW > 4)
        screen->drawRect(cellPX(c), cellPY(r), cellW, cellH, 0x1082);
    }

    void drawHUD() {
      screen->fillRect(0, 0, 240, 15, TFT_BLACK);

      screen->setTextColor(TFT_CYAN, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(4, 4);
      screen->print("LVL:");
      screen->print(level);

      screen->setTextColor(TFT_YELLOW, TFT_BLACK);
      screen->setCursor(50, 4);
      screen->print("SCR:");
      screen->print(score);

      const int BAR_X = 155;
      const int BAR_Y = 4;
      const int BAR_W = 80;
      const int BAR_H = 7;

      screen->drawRect(BAR_X - 1, BAR_Y - 1,
                       BAR_W + 2, BAR_H + 2, TFT_DARKGREY);
      screen->fillRect(BAR_X, BAR_Y, BAR_W, BAR_H, 0x2104);

      if (timeLeft > 0) {
        int fillW = constrain((timeLeft * BAR_W) / timeLimit, 0, BAR_W);
        uint16_t barCol;
        if      (timeLeft > timeLimit / 2) barCol = C_BAR_G;
        else if (timeLeft > timeLimit / 4) barCol = C_BAR_Y;
        else                               barCol = C_BAR_R;
        screen->fillRect(BAR_X, BAR_Y, fillW, BAR_H, barCol);
      }

      screen->setTextColor(TFT_WHITE, TFT_BLACK);
      screen->setCursor(BAR_X - 20, 4);
      screen->print(timeLeft);
      screen->print("s");
    }

    void drawWinScreen() {
      screen->fillScreen(TFT_BLACK);

      screen->setTextColor(TFT_GREEN, TFT_BLACK);
      screen->setTextSize(2);
      screen->setCursor(28, 14);
      screen->print("TU AS GAGNE !");

      screen->drawFastHLine(20, 36, 200, TFT_DARKGREY);

      screen->setTextColor(TFT_CYAN, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(60, 44);
      screen->print("NIVEAU ");
      screen->print(level);
      screen->print(" COMPLETE !");

      screen->setTextColor(TFT_WHITE, TFT_BLACK);
      screen->setCursor(40, 58);
      screen->print("Temps restant : ");
      screen->setTextColor(timeLeft > 5 ? TFT_GREEN : TFT_YELLOW, TFT_BLACK);
      screen->print(timeLeft);
      screen->print("s");

      screen->setTextColor(TFT_YELLOW, TFT_BLACK);
      screen->setCursor(40, 72);
      screen->print("Bonus temps  : +");
      screen->print(timeLeft * 10);
      screen->print(" pts");

      int previewScore = level * 100 + timeBonus + timeLeft * 10;
      screen->setTextColor(TFT_WHITE, TFT_BLACK);
      screen->setCursor(40, 86);
      screen->print("Score total  :  ");
      screen->setTextColor(TFT_YELLOW, TFT_BLACK);
      screen->print(previewScore);

      screen->drawFastHLine(20, 102, 200, TFT_DARKGREY);

      screen->setTextColor(TFT_CYAN, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(10, 110);
      screen->print("[A] Niveau suivant");
      screen->setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      screen->setCursor(148, 110);
      screen->print("[B] Menu");

      int bx = 88; int by = 124;
      uint16_t deco[5] = {TFT_GREEN, TFT_CYAN, TFT_YELLOW, TFT_GREEN, TFT_CYAN};
      for (int i = 0; i < 5; i++)
        screen->fillRect(bx + i * 14, by, 10, 6, deco[i]);
    }

    void tryMove(int dc, int dr) {
      int nc = playerCol + dc;
      int nr = playerRow + dr;
      if (!isWall(nc, nr)) {
        prevPlayerCol = playerCol;
        prevPlayerRow = playerRow;
        playerCol     = nc;
        playerRow     = nr;
        Sound::menuMove();
      }
    }

    void prepareNextLevel() {
      timeBonus += timeLeft * 10;
      score      = level * 100 + timeBonus;
      level++;
      firstDraw      = true;
      alertPlayed    = false;
      winScreenDrawn = false;
      computeLayout();
      generateMaze();
      timeLimit      = computeTimeLimit();
      timeLeft       = timeLimit;
      levelStartTime = millis();
      prevPlayerCol  = playerCol;
      prevPlayerRow  = playerRow;
      mazeState      = PLAYING;
    }

  public:
    MazeGame(TFT_eSPI* display) : Game(display) {}

    void init() override {
      level          = 1;
      score          = 0;
      timeBonus      = 0;
      state          = IN_PROGRESS;
      mazeState      = PLAYING;
      firstDraw      = true;
      alertPlayed    = false;
      winScreenDrawn = false;
      lastMove       = 0;
      gameOverDrawn       = false;
      gameOverSoundPlayed = false;

      computeLayout();
      generateMaze();

      timeLimit      = computeTimeLimit();
      timeLeft       = timeLimit;
      levelStartTime = millis();
      prevPlayerCol  = playerCol;
      prevPlayerRow  = playerRow;
    }

    // ← forceRedraw() CORRECTEMENT placé HORS de init()
    void forceRedraw() override {
      firstDraw = true;
    }

    void update(Buttons buttons) override {
      if (state == GAME_OVER) {
        if (!gameOverSoundPlayed) {
          gameOverSoundPlayed = true;
          Sound::gameOver();
        }
        return;
      }

      if (mazeState == LEVEL_WIN) {
        if (buttons.aPressed) {
          prepareNextLevel();
          Sound::gameStart();
        }
        return;
      }

      // Minuteur
      int elapsed = (int)((millis() - levelStartTime) / 1000);
      timeLeft    = max(0, timeLimit - elapsed);

      if (timeLeft <= 5 && !alertPlayed) {
        alertPlayed = true;
        Sound::collision();
      }

      if (timeLeft == 0) {
        state = GAME_OVER;
        return;
      }

      // Déplacement
      if (millis() - lastMove > MOVE_DELAY) {
        if      (buttons.upPressed)    { tryMove(0, -1); lastMove = millis(); }
        else if (buttons.downPressed)  { tryMove(0,  1); lastMove = millis(); }
        else if (buttons.leftPressed)  { tryMove(-1, 0); lastMove = millis(); }
        else if (buttons.rightPressed) { tryMove( 1, 0); lastMove = millis(); }
      }

      // Victoire niveau
      if (playerCol == exitCol && playerRow == exitRow) {
        mazeState      = LEVEL_WIN;
        winScreenDrawn = false;
        Sound::win();
      }
    }

    void render() override {
      if (state == GAME_OVER) {
        renderGameOver();
        return;
      }

      if (mazeState == LEVEL_WIN) {
        if (!winScreenDrawn) {
          drawWinScreen();
          winScreenDrawn = true;
        }
        return;
      }

      // Premier dessin complet — déclenché aussi par forceRedraw()
      if (firstDraw) {
        firstDraw = false;
        drawMaze();
        drawHUD();
        return;
      }

      // Rendu différentiel
      if (playerCol != prevPlayerCol || playerRow != prevPlayerRow) {
        erasePlayer(prevPlayerCol, prevPlayerRow);
        if (prevPlayerCol == exitCol && prevPlayerRow == exitRow) {
          screen->fillRect(cellPX(exitCol) + 1, cellPY(exitRow) + 1,
                           cellW - 2, cellH - 2, C_EXIT);
        }
        drawPlayer(playerCol, playerRow);
        prevPlayerCol = playerCol;
        prevPlayerRow = playerRow;
      }

      // HUD mis à jour chaque seconde
      static int lastTimeDrawn = -1;
      if (timeLeft != lastTimeDrawn) {
        lastTimeDrawn = timeLeft;
        drawHUD();
      }
    }

    virtual ~MazeGame() {}
};

#endif