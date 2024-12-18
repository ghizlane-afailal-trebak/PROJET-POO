#include "raylib.h"
#include <vector>
#include <stack>
#include <cstdlib>
#include <ctime>
#include <fstream>
// Constantes globales
#define GRAY CLITERAL(Color){ 64, 69, 77, 255 }
const int screenWidth = 800;
const int screenHeight = 630;
int cellSize = 40;
// Variables globales
int cols, rows;
bool gameStarted = false;
double startTime = 0.0;
double endTime = 0.0;
int playerWidth, playerHeight;
enum GameState { MENU, GAME, VICTORY };
GameState gameState = MENU;
class Cell;
std::vector<Cell> grid;
std::stack<Cell*> stack;
class Cell {
  private :
     int x, y; //x : colonne, et y: ligne
     bool visited; // cellule : visité/Non.
     bool walls[4]; // les quatre murs de la cellule.
  public :
   // Constructeur :
     Cell(int x, int y) : x(x), y(y), visited(false) {
         walls[0] = walls[1] = walls[2] = walls[3] = true; 
     }
    void Draw() {
         //La fonction DrawLine(startX, startY, endX, endY, color) dessine une ligne entre deux points avec la couleur spécifiée (fonction de raylib)
        if (walls[0]) DrawLine(x * cellSize, y * cellSize, (x + 1) * cellSize, y * cellSize, BLACK);
        if (walls[1]) DrawLine((x + 1) * cellSize, y * cellSize, (x + 1) * cellSize, (y + 1) * cellSize, BLACK);
        if (walls[2]) DrawLine((x + 1) * cellSize, (y + 1) * cellSize, x * cellSize, (y + 1) * cellSize, BLACK);
        if (walls[3]) DrawLine(x * cellSize, (y + 1) * cellSize, x * cellSize, y * cellSize, BLACK);
    }
     //La fonction GetCell() est utilisée pour récupérer un pointeur vers une cellule spécifique d'une grille
    // 2D représentée sous la forme d'un tableau 1D.
    Cell* GetCell(int x, int y) {
        if (x < 0 || y < 0 || x >= cols || y >= rows) return nullptr;
        return &grid[y * cols + x];
    }
    //La fonction RemoveWalls() supprime les murs entre deux cellules adjacentes dans une grille de labyrinthe.
    void RemoveWalls(Cell* next) {
        int dx = x - next->x;
        int dy = y - next->y;
        if (dx == 1) {
            walls[3] = false;
            next->walls[1] = false;
        } else if (dx == -1) {
            walls[1] = false;
            next->walls[3] = false;
        }
        if (dy == 1) {
            walls[0] = false;
            next->walls[2] = false;
        } else if (dy == -1) {
            walls[2] = false;
            next->walls[0] = false;
        }
    }
    friend class Maze ;
    friend class Player ;
};

class Maze {
public:
 //Cette fonction génère un labyrinthe en utilisant l'algorithme de parcours
// en profondeur avec un retour en arrière (Depth-First Search).
    void GenerateMaze() {
        Cell* current = &grid[0];
        current->visited = true;
        stack.push(current);
        while (!stack.empty()) {
            current = stack.top();
            std::vector<Cell*> neighbors;
            Cell* top = current->GetCell(current->x, current->y - 1);
            Cell* right = current->GetCell(current->x + 1, current->y);
            Cell* bottom = current->GetCell(current->x, current->y + 1);
            Cell* left = current->GetCell(current->x - 1, current->y);
            if (top && !top->visited) neighbors.push_back(top);
            if (right && !right->visited) neighbors.push_back(right);
            if (bottom && !bottom->visited) neighbors.push_back(bottom);
            if (left && !left->visited) neighbors.push_back(left);
            if (!neighbors.empty()) {
                Cell* next = neighbors[rand() % neighbors.size()];
                next->visited = true;
                current->RemoveWalls(next);
                stack.push(next);
            } else {
                stack.pop();
            }
        }
    }

};
// Les images du joueur et du goal :
Texture2D playerTexture;
Texture2D player2Texture; 

class Player {
 private:
    int x, y;
   public: 
     //constructeur :
    Player(int startX, int startY) : x(startX), y(startY) {}
     // Fonction pour dessiner le joueur :
    void Draw() {
    Vector2 position = {(float)(x * cellSize + 1), (float)(y * cellSize + 1)};
    DrawTextureEx(playerTexture, position, 0.0f, (float)playerWidth / playerTexture.width, WHITE);
    }
    // Fonction pour dessiner le goal :
    void Draw2() {
    Vector2 position = {(float)(x * cellSize + 1), (float)(y * cellSize + 1)};
    DrawTextureEx(player2Texture, position, 0.0f, (float)playerHeight / playerTexture.height, WHITE);
    }
    //ajuste les coordonnées du joueur en fonction des déplacements fournis (dx, dy):
    void Move(int dx, int dy) {
        x += dx;
        y += dy;
    }
    void GameStart (Player& goal, Player& player){
        // gère les déplacements du joueur dans un labyrinthe en utilisant les touches fléchées :
            if (IsKeyPressed(KEY_RIGHT) && !grid[player.y * cols + player.x].walls[1]) player.Move(1, 0);
            if (IsKeyPressed(KEY_LEFT) && !grid[player.y * cols + player.x].walls[3]) player.Move(-1, 0);
            if (IsKeyPressed(KEY_DOWN) && !grid[player.y * cols + player.x].walls[2]) player.Move(0, 1);
            if (IsKeyPressed(KEY_UP) && !grid[player.y * cols + player.x].walls[0]) player.Move(0, -1);
        //Si le joueur atteint la position du goal, le temps de jeu est enregistré et l'état du jeu est mis à "VICTORY"
            if (player.x == goal.x && player.y == goal.y) {
                endTime = GetTime();
                gameState = VICTORY;        }
    }
};

// Image d'accueil :
Texture2D startScreenImage ; 

//les fonctions globale :
void DrawStartScreen(Rectangle startButton) {
    //dessiner l'écran d'accueil du jeu, en affichant un bouton "START":
    ClearBackground(BLACK);
    DrawTexture(startScreenImage, 0, 0, WHITE);
    DrawText("Welcome to the Maze Game!", screenWidth / 2 - 200, screenHeight / 2 - 60, 30, WHITE);
    DrawRectangleRec(startButton, GRAY);
    DrawText("START", startButton.x + 37, startButton.y + 10, 26, WHITE);
}

int SelectDifficultyLevel() {
    //permet à l'utilisateur de choisir le niveau de difficulté
    int level = 1;
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexture(startScreenImage, 0, 0, WHITE);
        DrawText("   Select Difficulty Level:", 150, 230, 40, WHITE);
        DrawText("       1 - Easy", 210, 300, 27, YELLOW);
        DrawText("       2 - Medium",210, 340, 27, ORANGE);
        DrawText("       3 - Hard", 210, 385, 29, RED);
        if (IsKeyPressed(KEY_ONE)) {
            level = 1;
            break;
        }
        if (IsKeyPressed(KEY_TWO)) {
            level = 2;
            break;
        }
        if (IsKeyPressed(KEY_THREE)) {
            level = 3;
            break;
        }
        EndDrawing();
    }
    return level;
}
 
void StartGame(int level) {
    //initialise un jeu de type labyrinthe avec des paramètres adaptés au niveau de difficulté sélectionné :
    gameState = GAME;
    startTime = GetTime();
    Maze m ;
    switch (level) {
        case 1: 
            cols = 11; 
            rows = 8; 
            playerHeight = 50;
            playerWidth = 52 ; // Adjust cell size for easy level
            break;
        case 2: 
            cols = 19; 
            rows = 14; 
            playerHeight =40;
            playerWidth = 40 ;  // Adjust cell size for medium level
            break;
        case 3: 
            cols = 40; 
            rows = 30; 
            playerHeight = 16;
            playerWidth = 17 ;  // Adjust cell size for hard level
            break;
    }
    cellSize = screenWidth / cols;
    srand(time(0)); //Cela est utile pour que la génération soit aléatoire à chaque exécution du programme .
    grid.clear(); 
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            grid.push_back(Cell(x, y));
        }
    }
    m.GenerateMaze();
}


void DrawTimer(double startTime, double endTime, bool hasWon) {
    // Fonction pour afficher le chronomètre à l'écran
    double elapsed = hasWon ? endTime - startTime : GetTime() - startTime;
    // Conversion du temps écoulé en minutes et secondes
    int minutes = static_cast<int>(elapsed) / 60; 
    int seconds = static_cast<int>(elapsed) % 60; 
    char timerText[20];
    sprintf(timerText, "Time: %02d:%02d", minutes, seconds); // Formate le texte du chronomètre

    // Définir la position verticale du chronomètre, 40 pixels au-dessus du bas de l'écran
    int timerY = screenHeight - 40;

    // Affichage du texte à la position spécifiée (10px de marge à gauche, et timerY pour la hauteur)
    // Le texte est affiché en rouge (RED) avec une taille de police de 20
    DrawText(timerText, 10, timerY,20,RED);
}

void DrawReStarticon(Rectangle restartButton) { 
    //permet d'afficher un bouton interactif à l'écran
      DrawRectangleRec(restartButton, RED);
    DrawText("RESTART", restartButton.x + 23, restartButton.y + 10, 15, WHITE);
}

void DrawVictoryScreen(Rectangle restartButton, double elapsedTime) {

    //afficher un écran de victoire 
    ClearBackground(BLACK);// fond noir
    DrawTexture(startScreenImage, 0, 0, WHITE);
    DrawText("Congratulations! You reached the goal!", screenWidth / 2 - 180, screenHeight / 2 - 60, 20, YELLOW);//Affiche un texte au centre de l'écran
    char scoreText[50];//tableau de caractères pour stocker le texte du temps final
    sprintf(scoreText, "Final Time: %.2f seconds", elapsedTime);//Par exemple : "Final Time: 45.67 seconds"
    DrawText(scoreText, screenWidth / 2 - 100, screenHeight / 2, 20, GREEN);
    DrawRectangleRec(restartButton, WHITE);//Dessine un rectangle de button restartButton
    DrawText("RESTART", restartButton.x + 18, restartButton.y + 10, 25, BLACK); //Affiche le mot "RESTART" au centre du bouton.
}
int main() {
    InitWindow(screenWidth  , screenHeight  , "Maze Game with Victory Screen"); // Initialisation de la fenêtre de jeu
    SetTargetFPS(60);
    // Chargement des textures du joueur, du joueur goal et de l'écran de démarrage :
     playerTexture = LoadTexture("player/tommy.png");
     player2Texture = LoadTexture("player/jerry.png");
     startScreenImage = LoadTexture("player/maze intro.png");
     // Définition des rectangles pour les boutons "Start" et "Restart" :
    Rectangle startButton = {screenWidth / 2 - 75, screenHeight / 2 - 25, 150, 50};
    Rectangle restartButton = {screenWidth / 2 - 75, screenHeight / 2 + 50, 150, 50};
    Rectangle restartButton2 = { 600 ,585, 110, 30};
    int difficultyLevel = 1; 
    Player player(0, 0); // Initialisation du joueur au debut de labyrinthe .
    Player goal(cols - 1, rows - 1); // Initialisation du joueur a la fin .
    while (!WindowShouldClose()) {
        if (gameState == MENU) {
            BeginDrawing();
            DrawStartScreen(startButton);

            if (CheckCollisionPointRec(GetMousePosition(), startButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                difficultyLevel = SelectDifficultyLevel();
                StartGame(difficultyLevel);
                player = Player( 0 ,  0); // Réinitialise les positions du joueur et de l'objectif :
                goal = Player (cols - 1, rows -1 );
            }
            EndDrawing();
 } else if (gameState == GAME) {
            BeginDrawing();
            ClearBackground(WHITE);
            for (auto& cell : grid) cell.Draw();
            // Dessine chaque cellule de la grille.
              player.Draw();
              goal.Draw2() ;// Dessine le joueur et l'objectif
              player.GameStart(goal, player);
            DrawReStarticon(restartButton2); // Gère la logique du début du jeu, en vérifiant si le joueur a atteint l'objectif
            if (CheckCollisionPointRec(GetMousePosition(), restartButton2) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                gameState = MENU; // Change l'état du jeu vers le menu
            }
            DrawTimer(startTime, endTime, gameState == VICTORY); //// Affiche le chronomètre du jeu 
            EndDrawing();

        } else if (gameState == VICTORY) {
            BeginDrawing();
            DrawVictoryScreen(restartButton, endTime - startTime);

            if (CheckCollisionPointRec(GetMousePosition(), restartButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                gameState = MENU;
            }

            EndDrawing();
        }
  }
    UnloadTexture(playerTexture); // Libère la mémoire des textures chargées avant de fermer la fenêtre
    CloseWindow();  // Ferme la fenêtre et libère les ressources
  return 0 ;
  
}