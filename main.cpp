using namespace std;

#include <iostream>
#include <vector>
#include <Windows.h>
#include <string>
#include <conio.h>
#include <winuser.h>
#include <random>
#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77

enum CONTROL_KEYS {UP = 1, DOWN = 2, RIGHT = 3, LEFT = 4};
enum GAME_MODE {SINGLE_PLAYER = 20, MULTI_PLAYER = 21, HIDE_N_SEEK = 22};
int THROUGH = 10;
int CONSUME{ 11 };
int BREAK (12);

class GameObject {
public:
    int collisionType;
    int x;
    int y;
    char represent;

    void setCoords(int x, int y) {
        this->x = x;
        this->y = y;
    }
};

class GameVoid : public GameObject {
public:
    GameVoid() {
        this->represent = '-';
        this->collisionType = THROUGH;
    }
};

class Apple : public GameObject {
public:
    Apple() {
        this->represent = '@';
        this->collisionType = CONSUME;
    }
};

class SnakeSegment : public GameObject {
public:
    SnakeSegment(char segmentRepresent) {
        this->represent = segmentRepresent;
        this->collisionType = BREAK;
    }


};

class Snake : public GameObject {
    char segmentRepresent;

    void addControlToKeys(vector<int> *keys, int control, int altControl = 0) {
        keys->push_back(control);
        if (altControl != 0) {
            keys->push_back(altControl);
        }
    };

    void turnIntoCorpse() {
        char corpseRepresent = '=';
        this->represent = corpseRepresent;
        this->collisionType = CONSUME;

        for (auto &segment : child) {
            segment.represent = corpseRepresent;
            segment.collisionType = CONSUME;
        }
    }

public:
    string name;
    bool canEat = true;
    vector<int> upKeys;
    vector<int> downKeys;
    vector<int> rightKeys;
    vector<int> leftKeys;
    bool isDead;
    int xDirection;
    int yDirection;

    Snake(char snakeRepresent, char segmentRepresent) {
        this->represent = snakeRepresent;
        this->segmentRepresent = segmentRepresent;
        this->collisionType = BREAK;
    };

    vector<SnakeSegment> child;

    bool isThisDirectionItoSegments(int shiftX, int shiftY) {
        if (child.size() > 0) {
            SnakeSegment firstSegment = child[0];
            int newX = x + shiftX;
            int newY = y + shiftY;

            return newX == firstSegment.x && newY == firstSegment.y;
        }
        return false;
    }

    void addSegment(int shiftX, int shiftY) {
        int lenOfSegments = this->child.size();
        int x;
        int y;

        if (lenOfSegments >= 1) {
            GameObject lastSegment;
            GameObject prelastSegment;
            if (lenOfSegments == 1) {
                GameObject snakeCopy = GameObject();
                snakeCopy.setCoords(this->x, this->y);
                lastSegment = this->child[0];
                prelastSegment = snakeCopy;
            }
            else {
                lastSegment = this->child[lenOfSegments - 1];
                prelastSegment = this->child[lenOfSegments - 2];
            }


            // на вертикальной плоскости
            if (lastSegment.x == prelastSegment.x) {
                x = lastSegment.x;
                if (prelastSegment.y - lastSegment.y < 0) {
                    y = lastSegment.y + 1;
                }
                else {
                    y = lastSegment.y - 1;
                }
            }
            // на горизонтальной плоскости
            else if (lastSegment.y == prelastSegment.y) {
                if (prelastSegment.x - lastSegment.x < 0) {
                    x = lastSegment.x + 1;
                }
                else {
                    x = lastSegment.x - 1;
                }
                y = lastSegment.y;
            }
        }
        else {
            x = this->x - shiftX;
            y = this->y - shiftY;
        }
        SnakeSegment newSegment {segmentRepresent};
        newSegment.setCoords(x, y);
        this->child.push_back(newSegment);
    }

    void setControlKeys(int whichKey, int control, int altControl = 0) {
        switch (whichKey) {
            case UP:
                addControlToKeys(&upKeys, control, altControl);
                break;
            case DOWN:
                addControlToKeys(&downKeys, control, altControl);
                break;
            case RIGHT:
                addControlToKeys(&rightKeys, control, altControl);
                break;
            case LEFT:
                addControlToKeys(&leftKeys, control, altControl);
                break;
        }
    }

    bool isControlInKeys(int whichKey, int letterAscii) {
        vector<int> keys;

        switch (whichKey) {
            case UP:
                keys = upKeys;
                break;
            case DOWN:
                keys = downKeys;
                break;
            case RIGHT:
                keys = rightKeys;
                break;
            case LEFT:
                keys = leftKeys;
                break;
        }
        return find(keys.begin(), keys.end(), letterAscii) != keys.end();
    }

    void kill() {
        isDead = true;
        turnIntoCorpse();
    }


};

class GameMap {
    int height;
    int width;
    vector<vector<GameObject>> gameMap;

public:
    void fillWithVoid(int x, int y) {
        GameVoid voidEx = GameVoid();
        voidEx.setCoords(x, y);
        this->setOnGameMap(voidEx);
    };

    void setRandomPos(GameObject *gmObj) {
        int randX, randY;

        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> widthRange(0, this->width - 1);
        uniform_int_distribution<> heightRange(1, this->height - 1);

        while (true) {
            randX = widthRange(gen);
            randY = heightRange(gen);

            GameObject cell = this->gameMap[randY][randX];
            if (cell.collisionType == THROUGH) {
                this->fillWithVoid(randX, randY);
                
                gameMap[randY][randX] = *gmObj;
                gmObj->setCoords(randX, randY);
                return;
            }
        }
    };

    GameObject getCollisionObject(Snake gameObj, int shiftX, int shiftY) {
        int x;
        int y;

        x = gameObj.x;
        y = gameObj.y;
        setCoordsWithBounds(&x, &y, shiftX, shiftY);
        GameObject cell;
        
        cell = this->gameMap[y][x];
        
        return cell;
    }

    void init(int inputHeight, int inputWidth) {
        this->height = inputHeight;
        this->width = inputWidth;

        for (int row = 0; row < this->height; ++row) {
            vector<GameObject> curCol;
            for (int col = 0; col < this->width; ++col) {
                GameVoid voidEx;
                voidEx.setCoords(col, row);
                curCol.push_back(voidEx);
            }
            this->gameMap.push_back(curCol);
        }
    };

    void drawGameMap() {
        for (int row = 0; row < this->height; ++row) {
            for (int col = 0; col < this->width; ++col) {
                cout << this->gameMap[row][col].represent << " ";
            }
            cout << "\n";
        }
    };

    void setOnGameMap(GameObject gameObjEx) {
        this->gameMap[gameObjEx.y][gameObjEx.x] = gameObjEx;
    };

    void setCoordsWithBounds(int* x, int* y, int shiftX, int shiftY) {
        *x = *x + shiftX;
        if (*x >= width) {
            *x -= width;
        }
        else if (*x < 0) {
            *x = width + *x;
        }

        *y = *y + shiftY;
        if (*y >= height) {
            *y -= height;
        }
        else if (*y < 0) {
            *y = height + *y;
        }
    };
};

class GameManager {
    Apple apple;

    void initSnakes(int mode) {
        Snake firstSnake = Snake('1', '#');
        firstSnake.setControlKeys(UP, 'w', 'ц');
        firstSnake.setControlKeys(DOWN, 's', 'ы');
        firstSnake.setControlKeys(RIGHT, 'd', 'в');
        firstSnake.setControlKeys(LEFT, 'a', 'ф');
        firstSnake.name = "Змейка №1";
        snakes.push_back(firstSnake);


        if (mode == MULTI_PLAYER or mode == HIDE_N_SEEK) {
            Snake secondSnake = Snake('2', '&');
            secondSnake.setControlKeys(UP, 'o', 'щ');
            secondSnake.setControlKeys(DOWN, 'l', 'д');
            secondSnake.setControlKeys(RIGHT, ';', 'ж');
            secondSnake.setControlKeys(LEFT, 'k', 'л');
            secondSnake.name = "Змейка №2";
            snakes.push_back(secondSnake);

            if (mode == HIDE_N_SEEK) {
                Snake appleSnake = Snake('@', ' ');
                appleSnake.setControlKeys(UP, 't', 'е');
                appleSnake.setControlKeys(DOWN, 'g', 'п');
                appleSnake.setControlKeys(RIGHT, 'h', 'р');
                appleSnake.setControlKeys(LEFT, 'f', 'а');
                appleSnake.collisionType = CONSUME;
                appleSnake.name = "Яблочко";
                appleSnake.canEat = false;

                snakes.push_back(appleSnake);
            }
        }

    }

    public:
        vector<Snake> snakes;

        GameMap map;
        bool isLose = false;

        GameManager() {

        };

        void init(int width, int height, int mode) {
            initSnakes(mode);
            this->map.init(width, height);
            for (auto& snake : snakes) {
                map.setRandomPos(&snake);
            }

            if (mode != HIDE_N_SEEK) {
                this->apple = Apple();
                map.setRandomPos(&apple);
            }
            else {
                for (auto& snake : snakes) {
                    if (snake.represent == '@') {
                        break;
                    }

                    for (int ind = 0; ind < 3; ++ind) {
                        snake.addSegment(1, 0);
                        this->map.setCoordsWithBounds(&snake.child[ind].x, &snake.child[ind].y, 0, 0);
                        this->map.setOnGameMap(snake.child[ind]);

                    }
                }                
            }
        };
        
        Snake moveSnakeSegments(Snake snakeObj) {
            if (snakeObj.child.size() == 0) {
                return snakeObj;
            };

            int lastIndex = snakeObj.child.size();
            SnakeSegment lastSegment = snakeObj.child[lastIndex - 1];
            this->map.fillWithVoid(lastSegment.x, lastSegment.y);

            for (int ind = lastIndex - 1; ind >= 0; --ind) {
                SnakeSegment segment = snakeObj.child[ind];

                if (ind == 0) {
                    segment.x = snakeObj.x;
                    segment.y = snakeObj.y;
                    this->map.setCoordsWithBounds(&segment.x, &segment.y, -snakeObj.xDirection, -snakeObj.yDirection);
                }
                else
                {
                    SnakeSegment nextSegment = snakeObj.child[ind - 1];
                    segment.x = nextSegment.x;
                    segment.y = nextSegment.y;
                    this->map.setCoordsWithBounds(&segment.x, &segment.y, 0, 0);
                }

                snakeObj.child[ind] = segment;
                this->map.setOnGameMap(segment);
            }

            return snakeObj;
        }

        Snake moveSnake(Snake snake, int shiftX, int shiftY) {
            this->map.fillWithVoid(snake.x, snake.y);

            this->map.setCoordsWithBounds(&snake.x, &snake.y, shiftX, shiftY);
            this->map.setOnGameMap(snake);

            snake = moveSnakeSegments(snake);

            return snake;
        }

        bool isCollisionWithLastSegment(Snake snake, GameObject collisionObject) {
            if (snake.child.size() == 0) {
                return true;
            }


            int snakeLastIndex = snake.child.size() - 1;
            GameObject lastSegment = snake.child[snakeLastIndex];
            return lastSegment.x == collisionObject.x && lastSegment.y == collisionObject.y;
        }

        bool isAllSnakesDead() {
            bool sumBool = true;

            for (auto& snake : snakes) {
                sumBool &= snake.isDead;
            }

            return sumBool;
        }

        void refreshSnake(Snake snake) {
            this->map.setOnGameMap(snake);
            for (auto& segment : snake.child) {
                this->map.setOnGameMap(segment);
            }
        }

        void gameCycle() {
            for (auto& snake : snakes) {
                if (snake.isDead || (snake.xDirection == 0 && snake.yDirection == 0) ) {
                    continue;
                }

                GameObject collisionObject = this->map.getCollisionObject(snake, snake.xDirection, snake.yDirection);
                int collision = collisionObject.collisionType;
                if (collision == BREAK) {
                    if (snake.child.size() > 0) {
                        if (!isCollisionWithLastSegment(snake, collisionObject)) {
                            snake.kill();
                            refreshSnake(snake);
                            continue;
                        }
                    }
                    else {
                        snake.kill();
                        refreshSnake(snake);
                        continue;
                    }
                    
                }
                else if (collision == CONSUME) {
                    if (!snake.canEat) {
                        continue;
                    }
                    snake.addSegment(snake.xDirection, snake.yDirection);
                    int snakeLastIndex = snake.child.size() - 1;
                    this->map.setCoordsWithBounds(&snake.child[snakeLastIndex].x, &snake.child[snakeLastIndex].y, 0, 0);
                    if (collisionObject.represent == '@' && apple.x > 0) {
                        this->map.setRandomPos(&this->apple);
                    }
                }
                else if (collision == -1) {
                    int newX, newY = 0;
                }

                snake = moveSnake(snake, snake.xDirection, snake.yDirection);
            }

            if (isAllSnakesDead()) {
                isLose = true;
            }
        }

        int getScore(int snakeIndex) {
            return this->snakes[snakeIndex].child.size();
        }
};


int width = 20;
int height = 30;
GameManager gm;

void handleInput() {
    char letter = '0';
    if (_kbhit()) {
        letter = _getch();
    };
    letter = tolower(letter);
    for (auto &snake : gm.snakes) {
        if (snake.isControlInKeys(UP, letter) && !snake.isThisDirectionItoSegments(0, -1) ) {
            snake.xDirection = 0;
            snake.yDirection = -1;
        }
        else if (snake.isControlInKeys(DOWN, letter) && !snake.isThisDirectionItoSegments(0, 1) ) {
            snake.xDirection = 0;
            snake.yDirection = 1;
        }
        else if (snake.isControlInKeys(LEFT, letter) && !snake.isThisDirectionItoSegments(-1, 0) ) {
            snake.xDirection = -1;
            snake.yDirection = 0;
        }
        else if (snake.isControlInKeys(RIGHT, letter) && !snake.isThisDirectionItoSegments(1, 0) ) {
            snake.xDirection = 1;
            snake.yDirection = 0;
        }
    }    
}

void tick(int time) {
    int sumScore = 0;

    for (int ind = 0; ind < gm.snakes.size(); ++ind) {
        int scoreOfSnake = gm.getScore(ind);
        sumScore += scoreOfSnake;
    }
    int increasingSpeedValue = 10 * (sumScore);
    Sleep(time - increasingSpeedValue);
}

void drawUi() {
    for (int ind = 0; ind < gm.snakes.size(); ++ind) {
        string dead = "";
        if (gm.snakes[ind].isDead) {
            dead = " (ПРОИГРАЛ)";
        }
        string score = to_string(gm.getScore(ind));
        cout << "\n\n== Очки " + gm.snakes[ind].name + " " + dead + ": " + score + " ==";
        char upKey = toupper(gm.snakes[ind].upKeys[0]);
        char downKey = toupper(gm.snakes[ind].downKeys[0]);
        char rightKey = toupper(gm.snakes[ind].rightKeys[0]);
        char leftKey = toupper(gm.snakes[ind].leftKeys[0]);
        cout << "\nУправление: ВВЕРХ - " << upKey << \
            ", ВНИЗ - " << downKey << \
            ", ВПРАВО - " << rightKey << \
            ", ВЛЕВО - " << leftKey;

    }
   
    if (gm.isLose) {
        cout << "\n\n============\nИгра окончена\n============\n";
    }
}

int chooseGameMode() {
    int mode;

    try {
        cout << "Выберите режим игры:\n20 - режим кампании. 21 - мультиплеер. 22 - Прячься и Ищи\n";
        scanf_s("%d", &mode);
        if (mode > 22 || mode < 20) {
            throw exception("Нет такого режима игры");
        }
    }
    catch (exception e) {
        cout << "Неверный формат ввода";
        mode = chooseGameMode();
    }

    system("cls");
    return mode;
}

int main() {
    SetConsoleCP(1251);
    setlocale(LC_ALL, "RUS");
    int mode = chooseGameMode();
    gm.init(width, height, mode);
    int gameTime = 500;
    
    while (!gm.isLose) {
        handleInput();
        gm.gameCycle();
        system("cls");
        gm.map.drawGameMap();
        drawUi();
        tick(gameTime);
    }
}