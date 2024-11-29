// maze.h
#ifndef MAZE_H
#define MAZE_H

#include <vector>
#include <string>

// Player 구조체
struct Player {
    float x, y, z;           // 플레이어의 위치
    float angleY, angleX;    // 시선 각도 (Yaw, Pitch)
    float verticalVelocity;  // Y축 속도 (점프 및 중력)
    bool onGround;           // 플레이어가 지면에 있는지 여부

    Player()
        : x(10.0f), y(0.5f), z(10.0f),
        angleY(0.0f), angleX(0.0f),
        verticalVelocity(0.0f),
        onGround(true) {}
};

// Game 구조체
struct Game {
    int exitgridX;
    int exitgridZ;
    int countItem;
    bool isClear = false;
};

// Maze 구조체
struct Maze {
    std::vector<std::vector<int>> mazeData;
    float cellSize;
};

// 모델의 정점과 면을 저장하는 구조체
struct Model {
    std::vector<float> vertices; // x, y, z 순서로 저장
    std::vector<std::vector<int>> faces; // 각 면의 정점 인덱스
};

// 아이템 구조체
struct Item {
    int gridX, gridZ;          // 미로 그리드 좌표
    float oscillateOffset;     // 왕복 운동 오프셋
    bool movingForward;        // 이동 방향 (앞/뒤)
    float rotationAngle;       // 회전 각도
    bool get;                  // 아이템 획득 여부

    Item(int x, int z)
        : gridX(x), gridZ(z), oscillateOffset(0.0f), movingForward(true), rotationAngle(0.0f), get(false) {}
};

// 외부 전역 변수 선언
extern Player player;
extern Maze maze;
extern Model model;
extern std::vector<Item> items;

// 함수 선언
void generateMaze();
void initGame();
void initGameModified(); // 수정된 초기화 함수
void displayGameWindow();
void reshapeGameWindow(int newWidth, int newHeight);
void keyboardGame(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void passiveMouseMotion(int x, int y);
void update(int value);
bool loadOBJ(const std::string& filename, Model& model);

#endif