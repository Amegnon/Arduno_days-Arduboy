🎮 Arduboy Custom Console (Arduino)
📌 Description

Ce projet est une console de jeux portable développée sur Arduino, inspirée du concept de l’Arduboy.
Elle intègre plusieurs mini-jeux (Snake, Tetris, Morpion, Labyrinthe) avec une interface graphique, un système de menu et une gestion des entrées utilisateur.

👉 L’objectif est de créer un mini moteur de jeu embarqué, capable de lancer plusieurs jeux indépendants via une architecture modulaire.

🧠 Fonctionnalités
🎮 Menu interactif pour sélectionner les jeux
🕹️ Gestion des boutons (UP, DOWN, LEFT, RIGHT, A, B)
📺 Affichage via écran TFT (TFT_eSPI)
🔊 Gestion du son avec buzzer
🎯 Système de jeux modulaire (polymorphisme avec Game)
🌌 Écran de démarrage animé (étoiles + barre de chargement)
⏸️ Pause et Game Over
💾 Utilisation de l’EEPROM
🗂️ Structure du projet
.
├── LICENSE
└── main/
    ├── main.ino       # Programme principal (console + boucle)
    ├── Game.h         # Classe abstraite des jeux
    ├── Menu.h         # Gestion du menu
    ├── Snake.h        # Jeu Snake
    ├── Tetris.h       # Jeu Tetris
    ├── Morpion.h      # Jeu Tic-Tac-Toe
    ├── Maze.h         # Jeu Labyrinthe
    └── Sound.h        # Gestion du son
⚙️ Architecture

Le projet repose sur une machine à états :

enum stateConsole {
  STARTING,
  MENU,
  PLAYING
};
🔁 Fonctionnement
STARTING → écran de démarrage
MENU → sélection du jeu
PLAYING → exécution du jeu
🧩 Système de jeux (Polymorphisme)

Tous les jeux héritent d’une classe commune :

Game* currentGame;

Chaque jeu implémente :

update()
render()
init()

👉 Cela permet de lancer n’importe quel jeu dynamiquement :

currentGame = new SnakeGame(&screen);
🔌 Matériel utilisé
Carte Arduino / ESP32
Écran TFT compatible TFT_eSPI
Boutons physiques
Buzzer
🚀 Installation
1. Cloner le projet
git clone https://github.com/Amegnon/Arduno_days-Arduboy.git
cd Arduno_days-Arduboy/main
2. Installer les dépendances

Dans l’IDE Arduino :

Installer la librairie :
TFT_eSPI
3. Configurer le matériel

Modifier les pins si nécessaire :

#define BTN_UP     25
#define BTN_DOWN   26
#define BTN_LEFT   27
#define BTN_RIGHT  32
#define BTN_A      13
#define BTN_B      15
#define BUZZER_PIN 17
4. Compiler et téléverser
Ouvrir main.ino
Sélectionner la carte
Upload 🚀
🎮 Jeux disponibles
Jeu	Description
🐍 Snake	Mange les pommes et grandis
❌ Morpion	Jeu à 2 joueurs
🧱 Tetris	Aligne les lignes
🧭 Maze	Trouve la sortie
⚠️ Limitations
Utilisation de delay() (bloquant)
Gestion mémoire avec new/delete
Optimisation possible pour systèmes embarqués
💡 Améliorations futures
💾 Sauvegarde des scores
🎨 Animations plus fluides (sans delay)
🔋 Optimisation mémoire
🎮 Ajout de nouveaux jeux
🌐 Interface web pour gestion des jeux
📄 Licence

Ce projet est sous licence MIT.
Voir le fichier LICENSE.

👨‍💻 Auteurs
AGNIDE Vital
ALARA Imdad
GANDJI Merveille
HOUNZA Prisca
KANLINHANON Cadnel
KPODEGBE Evodie
SOGADJI Belange

