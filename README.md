# Arduboy Custom Console (Arduino/ESP32)

Console de jeux portable DIY inspirée de l'Arduboy, basée sur Arduino/ESP32 et un écran TFT couleur. Le projet propose un mini "moteur" de jeux modulaire avec menu, gestion des entrées, effets sonores et sauvegarde de scores.

## Points forts
- Menu interactif avec défilement et sélection rapide
- 4 mini‑jeux intégrés (Snake, Tetris, Morpion, Labyrinthe)
- Écran d'accueil animé (constellation + barre de chargement)
- Gestion des entrées (UP, DOWN, LEFT, RIGHT, A, B)
- Son via buzzer (sons bloquants et non bloquants)
- Architecture orientée objet (classe `Game` + polymorphisme)
- Sauvegarde des meilleurs scores via EEPROM

## Jeux inclus
- Snake : mange les pommes, 3 vies, vitesse progressive, hi‑score sauvegardé
- Tetris : rotation avec `A`/`UP`, accélération avec `DOWN`, hi‑score sauvegardé
- Morpion : joueur contre IA, curseur clignotant, victoire/défaite/nul
- Labyrinthe : niveaux procéduraux, chrono et bonus de temps, progression par niveaux

## Commandes
- Menu
- `UP` / `DOWN` : naviguer
- `A` : lancer le jeu
- En jeu (général)
- `B` : pause
- `A` : reprendre
- `B` : revenir au menu depuis la pause
- Contrôles spécifiques
- Snake : `UP`/`DOWN`/`LEFT`/`RIGHT`
- Tetris : `LEFT`/`RIGHT` déplacer, `DOWN` accélérer, `A` ou `UP` tourner
- Morpion : `UP`/`DOWN`/`LEFT`/`RIGHT` déplacer, `A` placer
- Labyrinthe : `UP`/`DOWN`/`LEFT`/`RIGHT`

## Matériel recommandé
- Carte ESP32 (utilisation de `ledcAttach` pour le buzzer)
- Écran TFT compatible `TFT_eSPI` (résolution 240x135 dans le code)
- 6 boutons (UP, DOWN, LEFT, RIGHT, A, B)
- Buzzer passif
- Alimentation adaptée à la carte et à l'écran

## Câblage (pins par défaut)
Les entrées sont en `INPUT_PULLUP` (boutons actifs à l'état bas).
- `BTN_UP`  : 25
- `BTN_DOWN`: 26
- `BTN_LEFT`: 27
- `BTN_RIGHT`: 32
- `BTN_A`   : 13
- `BTN_B`   : 15
- `BUZZER_PIN`: 17
- Pin 4 mise à `HIGH` au démarrage (souvent utilisée pour le backlight TFT, à adapter si besoin)

Modifie les pins directement dans `main/main.ino` si ton câblage diffère.

## Dépendances
- Bibliothèque `TFT_eSPI`
- Support ESP32 dans l'IDE Arduino
- `EEPROM` (déjà intégrée à l'IDE Arduino)

### Configuration `TFT_eSPI`
Pense à configurer le driver et les broches de ton écran dans `TFT_eSPI/User_Setup.h` ou via `User_Setup_Select.h` selon ton installation.

## Installation et compilation
1. Cloner le projet
```bash
git clone https://github.com/Amegnon/Arduno_days-Arduboy.git
cd Arduno_days-Arduboy/main
```
2. Ouvrir `main/main.ino` dans l'IDE Arduino
3. Installer `TFT_eSPI` via le gestionnaire de bibliothèques
4. Sélectionner la carte (ex. "ESP32 Dev Module")
5. Compiler et téléverser

## Architecture du projet
- `main/main.ino` : boucle principale, machine à états, menu et lancement des jeux
- `main/Game.h` : classe abstraite + pause + game over + hi‑score
- `main/Menu.h` : menu graphique et navigation
- `main/Snake.h` : jeu Snake
- `main/Tetris.h` : jeu Tetris
- `main/Morpion.h` : jeu Morpion (IA)
- `main/Maze.h` : jeu Labyrinthe
- `main/Sound.h` : gestion des sons

## EEPROM
- Utilisée pour sauvegarder les meilleurs scores
- Marqueur d'initialisation à l'adresse 10
- Snake : adresses 0–1
- Tetris : adresses 6–7

## Limitations connues
- Animations et transitions utilisent parfois `delay()`
- `new`/`delete` pour instancier les jeux (optimisation possible)
- Mémoire limitée selon la carte utilisée

## Idées d'amélioration
- Sauvegarde de scores globale et classement
- Suppression des `delay()` pour un rendu plus fluide
- Ajout de nouveaux jeux ou modes
- Menu graphique avec icônes

## Licence
Ce projet est sous licence MIT. Voir `LICENSE`.

## Auteurs
- AGNIDE Vital
- ALARA Imdad
- GANDJI Merveille
- HOUNZA Prisca
- KANLINHANON Cadnel
- KPODEGBE Evodie
- SOGADJI Belange
