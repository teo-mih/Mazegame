#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <sstream>
#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

vector<pair<int, int>> portals; // Stores all portal positions

struct UserProfile {
    string username;
    int level = 1;
    int lives = 3;
    int coins = 0;
    int coinsCollectedThisLevel = 0;
    int playerX = 0;
    int playerY = 0;
    bool hasKey = false;

    // Load profile from file
    bool loadProfile(const string& username) {
        ifstream file("saves/" + username + ".sav");
        if (!file) {
            cout << "Profile not found! Creating new profile.\n";
            return false;
        }

        string line;
        while (getline(file, line)) {
            istringstream ss(line);
            string key, value;
            if (getline(ss, key, '=') && getline(ss, value)) {
                if (key == "username") this->username = value;
                else if (key == "level") level = stoi(value);
                else if (key == "lives") lives = stoi(value);
                else if (key == "coins") coins = stoi(value);
                else if (key == "positionX") playerX = stoi(value);
                else if (key == "positionY") playerY = stoi(value);
                else if (key == "key") hasKey = (stoi(value) == 1);
            }
        }

        file.close();
        return true;
    }

    // Save profile to file
    void saveProfile() {
        if (username.empty()) {
            cout << "Error: Username is empty! Cannot save profile.\n";
            return;
        }

        ofstream file("saves/" + username + ".sav");
        if (!file) {
            cout << "Error saving profile!\n";
            return;
        }

        file << "username=" << username << '\n'
             << "level=" << level << '\n'
             << "lives=" << lives << '\n'
             << "coins=" << coins << '\n'
             << "positionX=" << playerX << '\n'
             << "positionY=" << playerY << '\n'
             << "key=" << (hasKey ? 1 : 0) << '\n';

        cout << "Progress saved for " << username << endl;
        file.close();
    }
};

class MazeGame {
private:
    vector<string> maze;
    UserProfile user;
    bool hasKey;

    char lastTile = ' ';
    const char WALL = '#';
    const char COIN = 'C';
    const char PLAYER = '@';
    const char PORTAL = '%';
    const char KEY = '&';
    const char CHEST = 'X';
    int coinsCollectedThisLevel = 0;

    void clearConsole() {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }

    void createSaveDir() {
        CreateDirectoryA("saves", NULL); // Windows-specific
    }

    bool profileExists(const string& username) {
        createSaveDir();
        ifstream file("saves/" + username + ".sav");
        return file.good();
    }

public:
    MazeGame() : hasKey(false) {
        createSaveDir();
    }

    void showMainMenu() {
        clearConsole();
        cout << "MAZE ESCAPE\n"
             << "1. New Game\n"
             << "2. Continue\n"  
             << "3. Quit\n";
    }

    void handleLogin() {
        char choice;
        char lvlanswer;
        char selectedlevel;
        while (true) {
            showMainMenu();
            cin >> choice;

            if (choice == '1') { // New Game
                cout << "Enter username (max 50 chars): ";
                cin.ignore();
                getline(cin, user.username);

                if (user.username.length() > 50)
                    user.username = user.username.substr(0, 50);

                if (profileExists(user.username)) {
                    cout << "Profile exists! Overwrite? (y/n): ";
                    cin >> choice;
                    if (tolower(choice) != 'y') continue;
                }

                string tempUsername = user.username; // Store username before resetting
                user = UserProfile(); // Reset everything
                user.username = tempUsername; // Restore username
               
                cout << "New profile created for " << user.username << "!\n";
                user.saveProfile(); // Save the new profile
                break;
            }
            else if (choice == '2') { // Continue
                cout << "Enter username: ";
                cin >> user.username;

                if (user.loadProfile(user.username)) {
                    cout << "Welcome back, " << user.username << "!\n";
                    cout <<"Want to select a level? y/n";
                    cin >> lvlanswer;
                    if (lvlanswer == 'y') {
                        cout << "Enter level: (1-3) ";
                        cin >> selectedlevel;
                        if (selectedlevel > user.level) {
                            cout << "Selected level not reached, starting current level!";
                            break;
                        }
                        if (selectedlevel > 0) {
                            cout << "Please select a correct level (1-3)";
                            return showMainMenu();
                            
                        }
                        if (selectedlevel < user.level)
                            user.level = selectedlevel;
                    }
                    break;
                }
                cout << "Profile not found!\n";
            }
            else if (choice == '3') exit(0);
        }
        loadMaze("level" + to_string(user.level) + ".txt");
    }

    void loadMaze(string filename) {
        ifstream file(filename);
        if (!file) {
            cout << "Error loading " << filename << endl;
            return;
        }

        maze.clear();
        portals.clear();

        string line;
        int y = 0;
        bool playerPlaced = false;
        while (getline(file, line)) {
            maze.push_back(line);
            for (int x = 0; x < line.size(); x++) {
                char character = line[x];

                //if (character == '@') {
                if (character == '@') { //&& user.playerX == 0 && user.playerY == 0) {  // Only set position if it's a new game 
                    user.playerX = x;
                    user.playerY = y;
                }
                if (line[x] == PLAYER && !playerPlaced) {
                    if (user.playerX == 0 && user.playerY == 0) {
                        user.playerX = x;
                        user.playerY = y;
                        playerPlaced = true;
                    }
                }
                else if (line[x] == PORTAL) {
                    portals.push_back({x, y});
                }
            }
            y++;
        }
        file.close();
    }

    void displayMaze() {
        clearConsole();
        cout << "Level: " << user.level << " | Lives: " << user.lives << " | Coins: " << user.coins << " | Key: " << (hasKey ? "Found" : "Not Found") << "\n";
        for (const auto& row : maze) cout << row << endl;
    }

    void movePlayer(char direction) {
        int newX = user.playerX;
        int newY = user.playerY;

        switch (tolower(direction)) {
            case 'w': newY--; break;
            case 's': newY++; break;
            case 'a': newX--; break;
            case 'd': newX++; break;
            default: return;
        }

        if (newX < 0 || newX >= maze[0].size() || newY < 0 || newY >= maze.size()) return;

        char newTile = maze[newY][newX];

        if (newTile == WALL) {
            user.lives--;
            cout << "Ouch! You hit a wall! Lives left: " << user.lives << endl << flush;

            if (user.lives <= 0) {
                cout << "Game Over! Restarting..." << endl;
                user.lives = 3;
                hasKey = false;
                user.coins -= user.coinsCollectedThisLevel;
                user.coinsCollectedThisLevel = 0;
                coinsCollectedThisLevel = 0;
                loadMaze("level" + to_string(user.level) + ".txt");
                return;
            }
            return;
        }

        if (lastTile == CHEST) {
            maze[user.playerY][user.playerX] = CHEST;
        }
        else if (lastTile == PORTAL) {
            maze[user.playerY][user.playerX] = PORTAL;
        }
        else {
            maze[user.playerY][user.playerX] = ' ';
        }

        if (newTile == COIN) {
            user.coins++;
            user.coinsCollectedThisLevel++;
            coinsCollectedThisLevel++;
            cout << "You collected a coin! Total: " << user.coins << endl;
            newTile = ' ';
        }

        if (newTile == KEY) {
            hasKey = true;
            cout << "Key acquired" << endl;
        }

        if (newTile == CHEST && hasKey) {
            user.level++;
            user.coinsCollectedThisLevel = 0;
            coinsCollectedThisLevel = 0;
            hasKey = false;
            cout << "Level completed! Moving to level " << user.level << endl;
            loadMaze("level" + to_string(user.level) + ".txt");
            return;
        }

        if (newTile == PORTAL) {
            for (size_t i = 0; i < portals.size(); ++i) {
                if (portals[i] == make_pair(newX, newY)) {
                    size_t otherPortal = (i + 1) % portals.size();
                    newX = portals[otherPortal].first;
                    newY = portals[otherPortal].second;
                    break;
                }
            }
        }

        lastTile = maze[newY][newX];
        user.playerX = newX;
        user.playerY = newY;
        maze[user.playerY][user.playerX] = PLAYER;
    }

    void play() {
        displayMaze();
        char input;

        while (true) {
            cout << "Enter command (w/a/s/d to move, q to quit and save progress): ";
            cin >> input;
            input = tolower(input);

            if (input == 'q') {
                user.saveProfile();
                break;
            }
            movePlayer(input);
            displayMaze();
        }
    }
};

int main() {
    MazeGame game;
    game.handleLogin();
    game.play();
    return 0;
}