// Game.h
#ifndef GAME_H
#define GAME_H

#include <TFT_eSPI.h>
#include <EEPROM.h>
#include "Sound.h"

struct Buttons {
  bool up, down, left, right, a, b;
  bool upPressed, downPressed;
  bool leftPressed, rightPressed;
  bool aPressed, bPressed;
};

enum stateGame {
  IN_PROGRESS,
  GAME_OVER,
  PAUSED
};

class Game {
  protected:
    TFT_eSPI*  screen;
    int        score;
    stateGame  state;
    bool       gameOverDrawn;
    bool       gameOverSoundPlayed;
    bool       pauseDrawn;        // ← flag anti-fluctuation
    int        highScore;

  public:
    Game(TFT_eSPI* display) {
      screen              = display;
      score               = 0;
      state               = IN_PROGRESS;
      gameOverDrawn       = false;
      gameOverSoundPlayed = false;
      pauseDrawn          = false;
      highScore           = 0;
    }

    virtual void init()                  = 0;
    virtual void update(Buttons buttons) = 0;
    virtual void render()                = 0;
    virtual void forceRedraw() {}

    bool isGameOver() { return state == GAME_OVER; }
    bool isPaused()   { return state == PAUSED; }
    int  getScore()   { return score; }

    // ── Pause ──────────────────────────────────
    void pause() {
      if (state == IN_PROGRESS) {
        state      = PAUSED;
        pauseDrawn = false;   // ← forcer redessin à l'entrée en pause
      }
    }

    void resume() {
      if (state == PAUSED) {
        state      = IN_PROGRESS;
        pauseDrawn = false;
      }
    }

    // Appelé UNE SEULE FOIS grâce à pauseDrawn
    void renderPause() {
      if (pauseDrawn) return;   // ← dessiné une fois, plus de fluctuation
      pauseDrawn = true;

      // Boîte centrale semi-transparente
      screen->fillRect(40, 35, 160, 65, TFT_BLACK);
      screen->drawRect(40, 35, 160, 65, TFT_CYAN);
      screen->drawRect(42, 37, 156, 61, 0x0299);

      // Titre PAUSE
      screen->setTextColor(TFT_CYAN, TFT_BLACK);
      screen->setTextSize(2);
      screen->setCursor(78, 44);
      screen->print("PAUSE");

      // Séparateur
      screen->drawFastHLine(48, 65, 144, TFT_DARKGREY);

      // A = Reprendre
      screen->setTextColor(TFT_GREEN, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(58, 72);
      screen->print("[A]  Reprendre");

      // B = Menu
      screen->setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      screen->setCursor(58, 86);
      screen->print("[B]  Quitter");
    }

    // ── High Score EEPROM ───────────────────────
    void loadHighScore(int addr) {
      int val = EEPROM.read(addr) | (EEPROM.read(addr + 1) << 8);
      highScore = (val == 65535) ? 0 : val;
    }

    void saveHighScore(int addr) {
      if (score > highScore) {
        highScore = score;
        EEPROM.write(addr,     highScore & 0xFF);
        EEPROM.write(addr + 1, (highScore >> 8) & 0xFF);
        EEPROM.commit();
      }
    }

    // ── Game Over unifié ────────────────────────
    void renderGameOver() {
      if (gameOverDrawn) return;
      gameOverDrawn = true;

      // Son de défaite
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
        Sound::play(65, 300);
      }

      // Flash rouge
      for (int i = 0; i < 3; i++) {
        screen->fillScreen(TFT_RED);
        delay(80);
        screen->fillScreen(TFT_BLACK);
        delay(60);
      }
      screen->fillScreen(TFT_BLACK);

      // Titre GAME OVER
      screen->setTextColor(TFT_DARKGREY, TFT_BLACK);
      screen->setTextSize(3);
      screen->setCursor(31, 11);
      screen->print("GAME OVER");
      screen->setTextColor(TFT_RED, TFT_BLACK);
      screen->setCursor(29, 9);
      screen->print("GAME OVER");

      screen->drawFastHLine(20, 42, 200, TFT_DARKGREY);

      // Score final
      screen->setTextColor(TFT_WHITE, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(70, 50);
      screen->print("SCORE FINAL");

      char buf[8];
      sprintf(buf, "%04d", score);
      int scoreX = (240 - strlen(buf) * 18) / 2;
      screen->setTextColor(TFT_YELLOW, TFT_BLACK);
      screen->setTextSize(3);
      screen->setCursor(scoreX, 60);
      screen->print(buf);

      screen->drawFastHLine(20, 92, 200, TFT_DARKGREY);

      // Meilleur score
      screen->setTextColor(TFT_DARKGREY, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(45, 98);
      screen->print("MEILLEUR SCORE : ");

      char hbuf[8];
      sprintf(hbuf, "%04d", highScore);
      screen->setTextColor(
        score >= highScore && score > 0 ? TFT_GREEN : TFT_CYAN,
        TFT_BLACK
      );
      screen->print(hbuf);

      if (score >= highScore && score > 0) {
        screen->setTextColor(TFT_GREEN, TFT_BLACK);
        screen->setCursor(45, 110);
        screen->print("** NOUVEAU RECORD ! **");
      }

      // Instructions
      screen->setTextColor(TFT_CYAN, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(10, 124);
      screen->print("[A] Rejouer");
      screen->setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      screen->setCursor(130, 124);
      screen->print("[B] Menu");
    }

    void displayScore() {
      screen->fillRect(0, 0, 240, 15, TFT_BLACK);
      screen->setTextColor(TFT_WHITE, TFT_BLACK);
      screen->setTextSize(1);
      screen->setCursor(2, 4);
      screen->print("SCORE:");
      screen->print(score);
    }

    virtual ~Game() {}
};

#endif