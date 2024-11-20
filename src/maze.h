// maze.h
#ifndef MAZE_H
#define MAZE_H

#include <vector>
#include <string>

// Player ����ü
struct Player {
    float x, y, z;           // �÷��̾��� ��ġ
    float angleY, angleX;    // �ü� ���� (Yaw, Pitch)
    float verticalVelocity;  // Y�� �ӵ� (���� �� �߷�)
    bool onGround;           // �÷��̾ ���鿡 �ִ��� ����

    Player()
        : x(0.0f), y(0.5f), z(0.0f),
        angleY(0.0f), angleX(0.0f),
        verticalVelocity(0.0f),
        onGround(true) {}
};

// Maze ����ü
struct Maze {
    std::vector<std::vector<int>> mazeData;
    float cellSize;
};

// ���� ������ ���� �����ϴ� ����ü
struct Model {
    std::vector<float> vertices; // x, y, z ������ ����
    std::vector<std::vector<int>> faces; // �� ���� ���� �ε���
};

// ������ ����ü
struct Item {
    int gridX, gridZ;          // �̷� �׸��� ��ǥ
    float oscillateOffset;     // �պ� � ������
    bool movingForward;        // �̵� ���� (��/��)
    float rotationAngle;       // ȸ�� ����

    Item(int x, int z)
        : gridX(x), gridZ(z), oscillateOffset(0.0f), movingForward(true), rotationAngle(0.0f) {}
};

// �ܺ� ���� ���� ����
extern Player player;
extern Maze maze;
extern Model model;
extern std::vector<Item> items;

// �Լ� ����
void generateMaze();
void initGame();
void initGameModified(); // ������ �ʱ�ȭ �Լ�
void displayGameWindow();
void reshapeGameWindow(int newWidth, int newHeight);
void keyboardGame(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void passiveMouseMotion(int x, int y);
void update(int value);
bool loadOBJ(const std::string& filename, Model& model);

#endif
