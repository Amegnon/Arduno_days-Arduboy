// Sound.h
#ifndef SOUND_H
#define SOUND_H

#include <Arduino.h>

class Sound {
  private:
    static unsigned long asyncEndTime;
    static int buzzerPin;

  public:

    // Initialiser le buzzer — à appeler dans setup()
    static void init(int pin) {
      buzzerPin = pin;
      ledcAttach(buzzerPin, 2000, 8);
    }

    // Arrêter le son
    static void stop() {
      ledcWriteTone(buzzerPin, 0);
    }

    // Son bloquant — pour les menus et transitions
    static void play(int frequence, int duree_ms) {
      ledcWriteTone(buzzerPin, frequence);
      delay(duree_ms);
      stop();
    }

    // Son non bloquant — pour pendant le gameplay
    static void playAsync(int frequence, int duree_ms) {
      ledcWriteTone(buzzerPin, frequence);
      asyncEndTime = millis() + duree_ms;
    }

    // À appeler dans loop() à chaque cycle — coupe le son async quand terminé
    static void update() {
      if (asyncEndTime > 0 && millis() >= asyncEndTime) {
        stop();
        asyncEndTime = 0;
      }
    }

    // ── Sons prédéfinis ──

    static void boot() {
      play(523, 150);   // Do
      play(659, 150);   // Mi
      play(784, 300);   // Sol
    }

    static void gameStart() {
      play(523, 100);
      play(784, 100);
      play(1047, 150);
    }

    static void point() {
      playAsync(1047, 80);
    }

    static void collision() {
      playAsync(200, 100);
    }

    static void gameOver() {
      play(392, 200);
      play(330, 200);
      play(262, 400);
    }

    static void menuMove() {
      playAsync(659, 50);
    }

    static void menuConfirm() {
      play(523, 80);
      play(784, 120);
    }

    static void win() {
      play(523, 100);
      play(659, 100);
      play(784, 100);
      play(1047, 300);
    }
};

// Définitions des membres statiques — UNE SEULE fois grâce à inline (C++17)
inline unsigned long Sound::asyncEndTime = 0;
inline int Sound::buzzerPin = 17;

#endif