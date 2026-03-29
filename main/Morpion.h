// Morpion.h
#ifndef MORPION_H
#define MORPION_H

#include "Game.h"
#include "Sound.h"

class MorpionGame : public Game {
  private:

    int  board[3][3];
    int  cursorX, cursorY;
    int  prevCursorX, prevCursorY;
    int  currentPlayer;
    int  result;

    bool boardDrawn;
    bool aiWaiting;
    unsigned long aiThinkTime;

    unsigned long cursorBlinkTimer;
    bool          cursorBlinkState;
    static const int CURSOR_BLINK_MS = 400;

    static const int CELL = 36;
    static const int GX   = (240 - CELL * 3) / 2;
    static const int GY   = (135 - CELL * 3) / 2 + 10;

    const uint16_t C_BG   = 0x0005;
    const uint16_t C_GRID = 0x4BAF;
    const uint16_t C_SHD  = 0x0240;
    const uint16_t C_X    = TFT_RED;
    const uint16_t C_X2   = 0xFBE0;
    const uint16_t C_O    = TFT_CYAN;
    const uint16_t C_O2   = 0x03FF;
    const uint16_t C_CUR  = TFT_YELLOW;

    // ═══════════════════════════════════════════
    //  LOGIQUE
    // ═══════════════════════════════════════════
    int checkWinner() {
      for (int i = 0; i < 3; i++) {
        if (board[i][0] && board[i][0]==board[i][1] && board[i][1]==board[i][2]) return board[i][0];
        if (board[0][i] && board[0][i]==board[1][i] && board[1][i]==board[2][i]) return board[0][i];
      }
      if (board[0][0] && board[0][0]==board[1][1] && board[1][1]==board[2][2]) return board[0][0];
      if (board[0][2] && board[0][2]==board[1][1] && board[1][1]==board[2][0]) return board[0][2];
      return 0;
    }

    bool checkDraw() {
      for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++)
          if (!board[r][c]) return false;
      return true;
    }

    void playAI() {
      // Gagner si possible
      for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++)
          if (!board[r][c]) {
            board[r][c] = 2;
            if (checkWinner() == 2) return;
            board[r][c] = 0;
          }
      // Bloquer le joueur (70% du temps)
      if (random(0, 10) < 7) {
        for (int r = 0; r < 3; r++)
          for (int c = 0; c < 3; c++)
            if (!board[r][c]) {
              board[r][c] = 1;
              if (checkWinner() == 1) { board[r][c] = 2; return; }
              board[r][c] = 0;
            }
      }
      // Jouer aléatoire
      int vides[9][2], count = 0;
      for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++)
          if (!board[r][c]) { vides[count][0]=r; vides[count][1]=c; count++; }
      if (count > 0) {
        int idx = random(0, count);
        board[vides[idx][0]][vides[idx][1]] = 2;
      }
    }

    // ═══════════════════════════════════════════
    //  DESSIN — différentiel, jamais de fillScreen
    //  pendant le jeu
    // ═══════════════════════════════════════════
    void drawBackground() {
      screen->fillScreen(C_BG);
      for (int x = 0; x < 240; x += 20)
        for (int y = 0; y < 135; y += 20)
          screen->drawPixel(x, y, 0x2104);
    }

    void drawGrid() {
      screen->drawRoundRect(GX-2, GY-2, CELL*3+4, CELL*3+4, 5, C_GRID);
      for (int i = 1; i < 3; i++) {
        screen->drawFastVLine(GX + i*CELL,   GY, CELL*3, C_GRID);
        screen->drawFastVLine(GX + i*CELL+1, GY, CELL*3, C_SHD);
        screen->drawFastHLine(GX, GY + i*CELL,   CELL*3, C_GRID);
        screen->drawFastHLine(GX, GY + i*CELL+1, CELL*3, C_SHD);
      }
    }

    void drawHUD() {
      screen->fillRect(0, 0, 240, GY - 2, C_BG);
      screen->setTextSize(1);
      screen->setTextColor(currentPlayer == 1 ? TFT_RED : TFT_CYAN, C_BG);
      screen->setCursor(5, 4);
      screen->print(currentPlayer == 1 ? "Ton tour  [X]" : "Tour IA   [O]");
    }

    void drawX(int col, int row) {
      int m  = 8;
      int x0 = GX + col*CELL, y0 = GY + row*CELL;
      screen->drawLine(x0+m,        y0+m,      x0+CELL-m,   y0+CELL-m, C_X);
      screen->drawLine(x0+CELL-m,   y0+m,      x0+m,        y0+CELL-m, C_X);
      screen->drawLine(x0+m+1,      y0+m,      x0+CELL-m+1, y0+CELL-m, C_X2);
      screen->drawLine(x0+CELL-m+1, y0+m,      x0+m+1,      y0+CELL-m, C_X2);
    }

    void drawO(int col, int row) {
      int cx = GX + col*CELL + CELL/2;
      int cy = GY + row*CELL + CELL/2;
      int r  = CELL/2 - 6;
      screen->drawCircle(cx, cy, r,   C_O);
      screen->drawCircle(cx, cy, r-1, C_O2);
      screen->drawCircle(cx, cy, r-2, C_O);
    }

    void clearCell(int col, int row) {
      screen->fillRect(GX + col*CELL + 1, GY + row*CELL + 1,
                       CELL-2, CELL-2, C_BG);
    }

    void drawCursor(int col, int row, bool visible) {
      uint16_t color = visible ? C_CUR : C_BG;
      screen->drawRect(GX + col*CELL + 4, GY + row*CELL + 4,
                       CELL-8,  CELL-8,  color);
      screen->drawRect(GX + col*CELL + 5, GY + row*CELL + 5,
                       CELL-10, CELL-10, color);
    }

    // Redessine une case proprement (fond + symbole éventuel)
    void redrawCell(int col, int row) {
      clearCell(col, row);
      if      (board[row][col] == 1) drawX(col, row);
      else if (board[row][col] == 2) drawO(col, row);
    }

    // ═══════════════════════════════════════════
    //  ÉCRAN FIN DE PARTIE
    //  Pattern identique à Snake / Memory
    // ═══════════════════════════════════════════
    void renderMorpionGameOver() {
      if (gameOverDrawn) return;
      gameOverDrawn = true;

      if (!gameOverSoundPlayed) {
        gameOverSoundPlayed = true;
        if      (result == 1) Sound::win();
        else if (result == 2) Sound::gameOver();
        else { Sound::play(440, 150); delay(60); Sound::play(370, 250); }
      }

      screen->fillScreen(TFT_BLACK);

      if (result == 1) {
        // Victoire
        screen->setTextColor(0x0300, TFT_BLACK);
        screen->setTextSize(2);
        screen->setCursor(31, 11);
        screen->print("TU AS GAGNE!");
        screen->setTextColor(TFT_GREEN, TFT_BLACK);
        screen->setCursor(29, 9);
        screen->print("TU AS GAGNE!");
        screen->drawFastHLine(20, 32, 200, TFT_GREEN);
        screen->setTextColor(TFT_RED, TFT_BLACK);
        screen->setTextSize(5);
        screen->setCursor(100, 55);
        screen->print("X");

      } else if (result == 2) {
        // Défaite
        screen->setTextColor(TFT_DARKGREY, TFT_BLACK);
        screen->setTextSize(2);
        screen->setCursor(31, 11);
        screen->print("GAME OVER");
        screen->setTextColor(TFT_RED, TFT_BLACK);
        screen->setCursor(29, 9);
        screen->print("GAME OVER");
        screen->drawFastHLine(20, 32, 200, TFT_RED);
        screen->setTextColor(TFT_CYAN, TFT_BLACK);
        screen->setTextSize(5);
        screen->setCursor(100, 55);
        screen->print("O");

      } else {
        // Match nul
        screen->setTextColor(TFT_YELLOW, TFT_BLACK);
        screen->setTextSize(2);
        screen->setCursor(45, 20);
        screen->print("MATCH NUL");
        screen->drawFastHLine(20, 42, 200, TFT_YELLOW);
        screen->setTextColor(TFT_WHITE, TFT_BLACK);
        screen->setTextSize(1);
        screen->setCursor(40, 70);
        screen->print("Personne ne gagne !");
      }

      screen->drawFastHLine(20, 105, 200, TFT_DARKGREY);
      screen->setTextColor(TFT_CYAN, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(10, 114);
      screen->print("[A] Rejouer");
      screen->setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      screen->setCursor(130, 114);
      screen->print("[B] Menu");
    }

  public:
    MorpionGame(TFT_eSPI* display) : Game(display) {}

    // ═══════════════════════════════════════════
    //  INIT
    // ═══════════════════════════════════════════
    void init() override {
      for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++)
          board[r][c] = 0;

      cursorX             = 1;
      cursorY             = 1;
      prevCursorX         = 1;
      prevCursorY         = 1;
      currentPlayer       = 1;
      result              = 0;
      score               = 0;
      state               = IN_PROGRESS;
      boardDrawn          = false;
      aiWaiting           = false;
      aiThinkTime         = 0;
      cursorBlinkTimer    = 0;
      cursorBlinkState    = true;
      gameOverDrawn       = false;
      gameOverSoundPlayed = false;
    }

    // ═══════════════════════════════════════════
    //  UPDATE
    // ═══════════════════════════════════════════
    void update(Buttons buttons) override {
      if (state == GAME_OVER) return;

      if (currentPlayer == 1) {
        bool moved = false;

        if      (buttons.upPressed    && cursorY > 0) { cursorY--; moved = true; }
        else if (buttons.downPressed  && cursorY < 2) { cursorY++; moved = true; }
        else if (buttons.leftPressed  && cursorX > 0) { cursorX--; moved = true; }
        else if (buttons.rightPressed && cursorX < 2) { cursorX++; moved = true; }

        if (moved) {
          Sound::menuMove();
          // Déplacement immédiat dans render() via la détection de changement
        }

        if (buttons.aPressed && !board[cursorY][cursorX]) {
          board[cursorY][cursorX] = 1;
          Sound::point();
          drawCursor(cursorX, cursorY, false);
          drawX(cursorX, cursorY);

          result = checkWinner();
          if (!result && checkDraw()) result = 3;
          if (result) {
            score               = result;
            state               = GAME_OVER;
            gameOverDrawn       = false;
            gameOverSoundPlayed = false;
            return;
          }
          currentPlayer = 2;
          drawHUD();
        }

      } else {
        if (!aiWaiting) { aiThinkTime = millis() + 500; aiWaiting = true; }
        if (millis() < aiThinkTime) return;

        aiWaiting = false;
        playAI();
        Sound::collision();

        for (int r = 0; r < 3; r++)
          for (int c = 0; c < 3; c++)
            if (board[r][c] == 2) { clearCell(c, r); drawO(c, r); }

        result = checkWinner();
        if (!result && checkDraw()) result = 3;
        if (result) {
          score               = result;
          state               = GAME_OVER;
          gameOverDrawn       = false;
          gameOverSoundPlayed = false;
          return;
        }
        currentPlayer = 1;
        drawHUD();
      }
    }

    // ═══════════════════════════════════════════
    //  RENDER — différentiel, zéro fluctuation
    // ═══════════════════════════════════════════
    void render() override {
      if (state == GAME_OVER) {
        renderMorpionGameOver();
        return;
      }

      // Premier dessin complet — une seule fois
      if (!boardDrawn) {
        boardDrawn = true;
        drawBackground();
        drawGrid();
        drawHUD();
        for (int r = 0; r < 3; r++)
          for (int c = 0; c < 3; c++)
            if (board[r][c]) redrawCell(c, r);
        drawCursor(cursorX, cursorY, true);
        prevCursorX      = cursorX;
        prevCursorY      = cursorY;
        cursorBlinkTimer = millis();
        cursorBlinkState = true;
        return;
      }

      // Pas de rendu si c'est le tour de l'IA
      if (currentPlayer != 1 || state != IN_PROGRESS) return;

      // Déplacement curseur — rendu différentiel uniquement
      if (cursorX != prevCursorX || cursorY != prevCursorY) {
        redrawCell(prevCursorX, prevCursorY); // efface l'ancien
        prevCursorX      = cursorX;
        prevCursorY      = cursorY;
        cursorBlinkState = true;
        cursorBlinkTimer = millis();
        drawCursor(cursorX, cursorY, true);   // dessine le nouveau
      }

      // Clignotement — touche uniquement la case courante
      if (millis() - cursorBlinkTimer > (unsigned long)CURSOR_BLINK_MS) {
        cursorBlinkTimer = millis();
        cursorBlinkState = !cursorBlinkState;
        if (!board[cursorY][cursorX])
          drawCursor(cursorX, cursorY, cursorBlinkState);
      }
    }

    void forceRedraw() override {
      boardDrawn = false;
    }

    virtual ~MorpionGame() {}
};

#endif