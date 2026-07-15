#include <iostream>   // used for input/output
#include <vector>     // used for dynamic arrays 
#include <string>     // used for text handling
#include <cctype>     // used for tolower()

using namespace std;

// struct used to store coordinates (row/col)
struct Position {
    int row;
    int col;

    // Constructors for older C++ standard compatibility
    Position() : row(0), col(0) {}
    Position(int r, int c) : row(r), col(c) {}
};

// struct used to store guard data (position + direction + behavior type)
struct Guard {
    Position pos;
    char dir;
    bool patrol; // true = turns clockwise on hit, false = flips direction (normal)

    // Constructors for older C++ standard compatibility
    Guard() : pos(Position(0,0)), dir(' '), patrol(false) {}
    Guard(Position p, char d, bool pat) : pos(p), dir(d), patrol(pat) {}
};

// struct used to represent a door
struct Door {
    Position pos;
    int group;
    bool isOpen;

    // Constructors for older C++ standard compatibility
    Door() : pos(Position(0,0)), group(0), isOpen(false) {}
    Door(Position p, int g, bool io) : pos(p), group(g), isOpen(io) {}
};

// struct used to represent a switch
struct Switch {
    Position pos;
    int group;

    // Constructors for older C++ standard compatibility
    Switch() : pos(Position(0,0)), group(0) {}
    Switch(Position p, int g) : pos(p), group(g) {}
};

// FUNCTION: converts string to lowercase w/ for-loop
string toLowerCase(string text) {
    for (int i = 0; i < (int)text.size(); i++) {
        text[i] = tolower(text[i]);
    }
    return text;
}

class Level {
private:
    vector<string> board;          // vector used as 2D map
    Position player;               // struct variable
    Position goal;                 // struct variable
    vector<Guard> guards;          // vector of structs
    vector<Door> doors;            // doors in the level
    vector<Switch> switches;       // switches in the level

    // FUNCTION: checks bounds (uses if statements)
    bool inBounds(int r, int c) {
        return r >= 0 && r < (int)board.size() &&
               c >= 0 && c < (int)board[0].size();
    }

    // FUNCTION: checks walls and closed doors (uses indexing)
    bool isWall(int r, int c) {
        if (board[r][c] == '#') return true;
        // Closed doors also block movement and vision
        for (int i = 0; i < (int)doors.size(); i++) {
            if (doors[i].pos.row == r && doors[i].pos.col == c && !doors[i].isOpen) {
                return true;
            }
        }
        return false;
    }

    // FUNCTION: checks guard position w/ loop
    bool guardHere(int r, int c) {
        for (int i = 0; i < (int)guards.size(); i++) {
            if (guards[i].pos.row == r &&
                guards[i].pos.col == c)
                return true;
        }
        return false;
    }

    // FUNCTION: checks door position w/ loop
    bool doorHere(int r, int c, bool& isOpen) {
        for (int i = 0; i < (int)doors.size(); i++) {
            if (doors[i].pos.row == r && doors[i].pos.col == c) {
                isOpen = doors[i].isOpen;
                return true;
            }
        }
        return false;
    }

    // FUNCTION: checks switch position w/ loop
    bool switchHere(int r, int c) {
        for (int i = 0; i < (int)switches.size(); i++) {
            if (switches[i].pos.row == r && switches[i].pos.col == c)
                return true;
        }
        return false;
    }

    // FUNCTION: handles switch toggling logic
    void triggerSwitches(int r, int c) {
        for (int i = 0; i < (int)switches.size(); i++) {
            if (switches[i].pos.row == r && switches[i].pos.col == c) {
                int grp = switches[i].group;
                // Toggle all doors belonging to the same group
                for (int j = 0; j < (int)doors.size(); j++) {
                    if (doors[j].group == grp) {
                        doors[j].isOpen = !doors[j].isOpen;
                    }
                }
            }
        }
    }

    // FUNCTION: vision system w/ loop + if statements
    bool seenByGuard() {
        for (int i = 0; i < (int)guards.size(); i++) {

            int r = guards[i].pos.row;
            int c = guards[i].pos.col;
            char d = guards[i].dir;

            while (true) {

                if (d == '>') c++;
                else if (d == '<') c--;
                else if (d == '^') r--;
                else if (d == 'v') r++;

                if (!inBounds(r, c)) break;
                if (isWall(r, c)) break;
                if (guardHere(r, c)) break;

                if (player.row == r && player.col == c)
                    return true;
            }
        }
        return false;
    }

    // FUNCTION: turns guard 90 degrees clockwise
    char turnClockwise(char dir) {
        if (dir == '^') return '>';
        if (dir == '>') return 'v';
        if (dir == 'v') return '<';
        if (dir == '<') return '^';
        return dir;
    }

    // FUNCTION: flips guard direction 180 degrees
    char flipDirection(char dir) {
        if (dir == '>') return '<';
        if (dir == '<') return '>';
        if (dir == '^') return 'v';
        if (dir == 'v') return '^';
        return dir;
    }

    // FUNCTION: moves all guards
    void moveGuards() {
        for (int i = 0; i < (int)guards.size(); i++) {

            int newR = guards[i].pos.row;
            int newC = guards[i].pos.col;

            if (guards[i].dir == '>') newC++;
            else if (guards[i].dir == '<') newC--;
            else if (guards[i].dir == '^') newR--;
            else if (guards[i].dir == 'v') newR++;

            if (!inBounds(newR, newC) || isWall(newR, newC) || guardHere(newR, newC)) {
                // Change direction based on behavior
                if (guards[i].patrol) {
                    guards[i].dir = turnClockwise(guards[i].dir);
                } else {
                    guards[i].dir = flipDirection(guards[i].dir);
                }

                // Recalculate step with new direction
                newR = guards[i].pos.row;
                newC = guards[i].pos.col;

                if (guards[i].dir == '>') newC++;
                else if (guards[i].dir == '<') newC--;
                else if (guards[i].dir == '^') newR--;
                else if (guards[i].dir == 'v') newR++;

                // If still blocked after changing direction, don't move
                if (!inBounds(newR, newC) || isWall(newR, newC) || guardHere(newR, newC)) {
                    newR = guards[i].pos.row;
                    newC = guards[i].pos.col;
                }
            }

            guards[i].pos.row = newR;
            guards[i].pos.col = newC;

            // Trigger switch if a guard steps onto one
            triggerSwitches(newR, newC);
        }
    }

    // FUNCTION: handles player movement w/ input
    bool movePlayer(char input) {

        int newR = player.row;
        int newC = player.col;

        if (input == 'w') newR--;
        else if (input == 's') newR++;
        else if (input == 'a') newC--;
        else if (input == 'd') newC++;
        else {
            cout << "Invalid input. Use WASD.\n";
            return true; // Keeps loop running but stops guards
        }

        if (!inBounds(newR, newC) || isWall(newR, newC)) {
            cout << "Blocked by wall or closed door.\n";
            return true;
        }

        if (guardHere(newR, newC)) {
            cout << "Caught by a guard!\n";
            return false;
        }

        player.row = newR;
        player.col = newC;

        // Trigger switch if player steps onto one
        triggerSwitches(newR, newC);

        if (player.row == goal.row && player.col == goal.col) {
            cout << "You reached the goal!\n";
            return false;
        }

        return true;
    }

    // FUNCTION: handles inspecting a specific tile
    void performInspection() {
        int r, c;
        cout << "Enter row (0-indexed): ";
        if (!(cin >> r)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid row number.\n";
            return;
        }
        cout << "Enter column (0-indexed): ";
        if (!(cin >> c)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Invalid column number.\n";
            return;
        }
        cin.ignore(10000, '\n'); // Clear stream buffer

        cout << "\n--- Inspection Results for (" << r << ", " << c << ") ---\n";
        if (!inBounds(r, c)) {
            cout << "Coordinates are out of bounds.\n";
            return;
        }

        // 1. Check Player
        if (player.row == r && player.col == c) {
            cout << "Entity: Player (@)\n";
            cout << "Description: That's you! Sneak safely to the goal.\n";
            return;
        }

        // 2. Check Guard
        for (int i = 0; i < (int)guards.size(); i++) {
            if (guards[i].pos.row == r && guards[i].pos.col == c) {
                cout << "Entity: Guard (" << guards[i].dir << ")\n";
                cout << "Movement Type: " << (guards[i].patrol ? "Patrol (Turns Clockwise)" : "Normal (Flips / Bounces)") << "\n";
                cout << "Facing Direction: " << guards[i].dir << "\n";
                return;
            }
        }

        // 3. Check Doors
        for (int i = 0; i < (int)doors.size(); i++) {
            if (doors[i].pos.row == r && doors[i].pos.col == c) {
                cout << "Entity: Door\n";
                cout << "Group: Group " << doors[i].group << "\n";
                cout << "Status: " << (doors[i].isOpen ? "Open (Passable)" : "Closed (Solid wall)") << "\n";
                return;
            }
        }

        // 4. Check Switches
        for (int i = 0; i < (int)switches.size(); i++) {
            if (switches[i].pos.row == r && switches[i].pos.col == c) {
                cout << "Entity: Switch (S)\n";
                cout << "Controls: Group " << switches[i].group << " doors\n";
                cout << "Description: Stepping on this toggles all doors linked to Group " << switches[i].group << ".\n";
                return;
            }
        }

        // 5. Check Goal
        if (goal.row == r && goal.col == c) {
            cout << "Entity: Goal ($)\n";
            cout << "Description: Reach this coordinate to win the level.\n";
            return;
        }

        // 6. Basic Terrains
        if (board[r][c] == '#') {
            cout << "Entity: Wall (#)\n";
            cout << "Description: A solid brick barrier. Completely blocks movement and line of sight.\n";
        } else {
            cout << "Entity: Empty Tile\n";
            cout << "Description: Clear space. Safe to move through.\n";
        }
    }

public:
    // Overloaded Constructors for Level to remain compatible with pre-C++11 defaults
    Level(vector<string> b, vector<Guard> g, Position p, Position go) {
        board = b;
        guards = g;
        player = p;
        goal = go;
    }

    Level(vector<string> b, vector<Guard> g, Position p, Position go, vector<Door> d, vector<Switch> s) {
        board = b;
        guards = g;
        player = p;
        goal = go;
        doors = d;
        switches = s;
    }

    // FUNCTION: displays map w/ loops
    void draw() {
        vector<string> temp = board;

        // Overlay static switches onto the map base
        for (int i = 0; i < (int)switches.size(); i++) {
            temp[switches[i].pos.row][switches[i].pos.col] = 'S';
        }

        // Overlay static doors onto the map base
        for (int i = 0; i < (int)doors.size(); i++) {
            temp[doors[i].pos.row][doors[i].pos.col] = doors[i].isOpen ? ' ' : 'D';
        }

        // Goal representation on the clean map base
        temp[goal.row][goal.col] = '$';

        // Overlay active dynamic guards on top
        for (int i = 0; i < (int)guards.size(); i++) {
            temp[guards[i].pos.row][guards[i].pos.col] = guards[i].dir;
        }

        // Overlay active player position on top
        temp[player.row][player.col] = '@';

        for (int r = 0; r < (int)temp.size(); r++) {
            cout << temp[r] << endl;
        }
    }

    // MAIN GAME LOOP using while
    void play() {

        bool running = true;

        while (running) {

            draw();

            if (seenByGuard()) {
                cout << "You were spotted!\n";
                break;
            }

            cout << "Move (WASD) or type 'inspect': ";
            string action;
            cin >> action;
            action = toLowerCase(action);

            if (action == "inspect") {
                performInspection();
                // Inspection does not cost a turn; continue the loop directly without moving guards
                continue;
            }

            char move = action[0];
            cin.ignore(10000, '\n'); // Clears stream buffer in case user typed out words
            move = tolower(move);

            Position oldPos = player;
            running = movePlayer(move);

            // Only move guards if player actually successfully changed positions
            if (running && (player.row != oldPos.row || player.col != oldPos.col))
                moveGuards();
        }
    }
};


// FUNCTION: creates level 1 (Normal intro level)
Level makeLevel1() {

    vector<string> map;
    map.push_back("########");
    map.push_back("#      #"); 
    map.push_back("#   #  #"); 
    map.push_back("#   #  #"); 
    map.push_back("#      #"); 
    map.push_back("########");

    vector<Guard> guards;

    Guard g1;
    g1.pos.row = 1;
    g1.pos.col = 4;
    g1.dir = 'v';
    g1.patrol = false; // Normal linear guard
    guards.push_back(g1);

    Guard g2;
    g2.pos.row = 2;
    g2.pos.col = 2;
    g2.dir = '>';
    g2.patrol = false; // Normal linear guard
    guards.push_back(g2);

    Position start;
    start.row = 4;
    start.col = 1;

    Position goal;
    goal.row = 4;
    goal.col = 6;

    return Level(map, guards, start, goal);
}


// FUNCTION: creates level 2 (Multi-Group Door & Switch Level)
Level makeLevel2() {

    vector<string> map;
    map.push_back("###########");
    map.push_back("#         #"); 
    map.push_back("# ### #   #"); 
    map.push_back("#     #   #"); 
    map.push_back("#         #"); 
    map.push_back("###########");

    vector<Guard> guards;

    Guard g1;
    g1.pos.row = 1;
    g1.pos.col = 4;
    g1.dir = '>';
    g1.patrol = false;
    guards.push_back(g1);

    Position start;
    start.row = 3;
    start.col = 2;

    Position goal;
    goal.row = 3;
    goal.col = 9;

    // Two independent door systems (Group 1 and Group 2)
    vector<Door> doors;
    doors.push_back(Door(Position(2, 5), 1, false)); // Closed Group 1 door
    doors.push_back(Door(Position(3, 5), 1, false)); // Closed Group 1 door
    doors.push_back(Door(Position(2, 7), 2, false)); // Closed Group 2 door
    doors.push_back(Door(Position(3, 7), 2, false)); // Closed Group 2 door

    vector<Switch> switches;
    switches.push_back(Switch(Position(1, 2), 1));     // Switch 1 toggles Group 1
    switches.push_back(Switch(Position(4, 2), 2));     // Switch 2 toggles Group 2

    return Level(map, guards, start, goal, doors, switches);
}


// FUNCTION: creates level 3 (Rectangular Patrol Guard Level)
Level makeLevel3() {
    // Shows off rectangular movement pattern of a patrol guard
    vector<string> map;
    map.push_back("#######");
    map.push_back("#     #");
    map.push_back("# ### #");
    map.push_back("#     #");
    map.push_back("#######");

    vector<Guard> guards;
    Guard g1;
    g1.pos.row = 1;
    g1.pos.col = 1;
    g1.dir = '>';
    g1.patrol = true; // Turn clockwise on wall collision (creates rectangular route)
    guards.push_back(g1);

    Position start;
    start.row = 3;
    start.col = 1;

    Position goal;
    goal.row = 3;
    goal.col = 5;

    return Level(map, guards, start, goal);
}


// FUNCTION: creates level 4 (Complex Patrol Route Level)
Level makeLevel4() {
    // Patrolling guard navigating a winding, complex corridor
    vector<string> map;
    map.push_back("#########");
    map.push_back("#   #   #");
    map.push_back("# # # # #");
    map.push_back("# #   # #");
    map.push_back("#########");

    vector<Guard> guards;
    Guard g1;
    g1.pos.row = 1;
    g1.pos.col = 1;
    g1.dir = 'v';
    g1.patrol = true; // Loops through the labyrinth structures clockwise
    guards.push_back(g1);

    Position start;
    start.row = 3;
    start.col = 4;

    Position goal;
    goal.row = 1;
    goal.col = 7;

    return Level(map, guards, start, goal);
}


// FUNCTION: creates level 5 (Guard walks over switch level)
Level makeLevel5() {
    // Shows a guard stepping on a switch to open/close doors dynamically for the player
    vector<string> map;
    map.push_back("###########");
    map.push_back("#         #");
    map.push_back("# ####### #");
    map.push_back("#         #");
    map.push_back("###########");

    vector<Guard> guards;
    Guard g1;
    g1.pos.row = 1;
    g1.pos.col = 9;
    g1.dir = '<';
    g1.patrol = false; // Linear guard patrolling across a switch at (1, 5)
    guards.push_back(g1);

    Position start;
    start.row = 3;
    start.col = 1;

    Position goal;
    goal.row = 3;
    goal.col = 9;

    vector<Door> doors;
    doors.push_back(Door(Position(3, 5), 3, false)); // Closed Group 3 Door blocks path to goal

    vector<Switch> switches;
    switches.push_back(Switch(Position(1, 5), 3));     // Switch 3 placed directly in the path of Guard 1

    return Level(map, guards, start, goal, doors, switches);
}


// MAIN FUNCTION
int main() {

    while (true) {

        cout << "\nWelcome to Ultra-Spy!\n";
        cout << "1) A New Enemy\n";
        cout << "2) The Switch & Door Networks\n";
        cout << "3) The Patrol Loop (Rectangular)\n";
        cout << "4) The Maze Loop (Complex Route)\n";
        cout << "5) Guard Assisted Passage (Guard on Switch)\n";
        cout << "Enter choice: ";

        string input;
        getline(cin, input); // Uses getline to accept multi-word option text cleanly

        input = toLowerCase(input);

        if (input == "1" || input == "a new enemy") {
            Level lvl = makeLevel1(); 
            lvl.play();
        }
        else if (input == "2" || input == "the switch & door networks") {
            Level lvl = makeLevel2(); 
            lvl.play();
        }
        else if (input == "3" || input == "the patrol loop") {
            Level lvl = makeLevel3(); 
            lvl.play();
        }
        else if (input == "4" || input == "the maze loop") {
            Level lvl = makeLevel4(); 
            lvl.play();
        }
        else if (input == "5" || input == "guard assisted passage") {
            Level lvl = makeLevel5(); 
            lvl.play();
        }
        else {
            cout << "Invalid choice.\n";
        }
    }

    return 0;
}