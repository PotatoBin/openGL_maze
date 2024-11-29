// maze.cpp
#include "maze.h"
#include "modeler.h"
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <fstream>
#include <sstream>

// 'M_PI' ���� (���ǵ��� ���� ���)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// �÷��̾� �� �̷� ������ ����
Player player;
Maze maze;
Model model;
Game game;
std::vector<Item> items;

// ���� �˰����� ����Ͽ� �̷θ� �����ϴ� �Լ�
void primMaze(std::vector<std::vector<int>>& mazeGrid) {
    int size = mazeGrid.size();

    struct Cell {
        int x, y;
        int px, py;
    };

    std::vector<Cell> walls;

    std::random_device rd;
    std::mt19937 gen(rd());

    int x = 1;
    int y = 1;
    if (x + 2 < size - 1) walls.push_back({ x + 2, y, x, y });
    if (y + 2 < size - 1) walls.push_back({ x, y + 2, x, y });

    while (!walls.empty()) {
        std::uniform_int_distribution<> dis(0, walls.size() - 1);
        int idx = dis(gen);
        Cell wall = walls[idx];

        walls.erase(walls.begin() + idx);

        int nx = wall.x;
        int ny = wall.y;
        int px = wall.px;
        int py = wall.py;

        if (mazeGrid[ny][nx] == 1) {
            mazeGrid[ny][nx] = 0;
            mazeGrid[(ny + py) / 2][(nx + px) / 2] = 0;

            if (nx + 2 < size - 1 && mazeGrid[ny][nx + 2] == 1)
                walls.push_back({ nx + 2, ny, nx, ny });
            if (nx - 2 > 0 && mazeGrid[ny][nx - 2] == 1)
                walls.push_back({ nx - 2, ny, nx, ny });
            if (ny + 2 < size - 1 && mazeGrid[ny + 2][nx] == 1)
                walls.push_back({ nx, ny + 2, nx, ny });
            if (ny - 2 > 0 && mazeGrid[ny - 2][nx] == 1)
                walls.push_back({ nx, ny - 2, nx, ny });
        }
    }
}

// OBJ ������ �ε��ϴ� �Լ�
bool loadOBJ(const std::string& filename, Model& model) {
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        std::cerr << "Unable to open OBJ file: " << filename << std::endl;
        return false;
    }

    model.vertices.clear();
    model.faces.clear();

    std::string line;
    while (std::getline(infile, line)) {
        std::istringstream ss(line);
        std::string prefix;
        ss >> prefix;
        if (prefix == "v") {
            float x, y, z;
            ss >> x >> y >> z;
            model.vertices.push_back(x);
            model.vertices.push_back(y);
            model.vertices.push_back(z);
        }
        else if (prefix == "f") {
            std::vector<int> face;
            std::string vertex;
            while (ss >> vertex) {
                std::istringstream vss(vertex);
                std::string indexStr;
                std::getline(vss, indexStr, '/');
                int index = std::stoi(indexStr);
                face.push_back(index);
            }
            model.faces.push_back(face);
        }
        // �ٸ� �����Ƚ��� ����
    }

    infile.close();
    std::cout << "Loaded OBJ file: " << filename << " with "
        << model.vertices.size() / 3 << " vertices and "
        << model.faces.size() << " faces." << std::endl;
    return true;
}

// �̷θ� �����ϴ� �Լ�
void generateMaze() {
    int currentDifficultyLevel = difficultyLevel; // �𵨷����� ������
    int mazeSize = currentDifficultyLevel * 12 + 1;
    maze.mazeData = std::vector<std::vector<int>>(mazeSize, std::vector<int>(mazeSize, 1));
    maze.cellSize = 1.0f;

    maze.mazeData[1][1] = 0;

    primMaze(maze.mazeData);

    maze.mazeData[mazeSize - 2][mazeSize - 2] = 0;

    maze.mazeData[1][1] = 5; // ���� ��ġ
    maze.mazeData[mazeSize - 2][mazeSize - 2] = 6; // ��ǥ ��ġ

    game.exitgridX = mazeSize - 2;   // ��ǥ ��ǥ ����
    game.exitgridZ = mazeSize - 2;

    // ������ ��ġ (���̵��� ���� �����ϰ� ��ġ)
    int itemCount = currentDifficultyLevel; // ���̵��� ���� ������ �� ����
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, mazeSize - 2);

    items.clear(); // ������ ����Ʈ �ʱ�ȭ
    game.countItem = 0; // ȹ���� ������ �� �ʱ�ȭ
    game.isClear = false; // ���� Ŭ���� ���� �ʱ�ȭ

    while (items.size() < static_cast<size_t>(itemCount)) {
        int i = dis(gen);
        int j = dis(gen);
        if (maze.mazeData[j][i] == 0) { // ��ο��� ������ ��ġ
            maze.mazeData[j][i] = 2;
            items.emplace_back(i, j);
        }
    }

    // �÷��̾� ���� ��ġ ����
    player.x = 1.5f;
    player.y = 1.3f; // ī�޶� ���� ����
    player.z = 1.5f;
    player.verticalVelocity = 0.0f;
    player.onGround = true;

    // �� �ε�
    if (!loadOBJ("model.obj", model)) { // "model.obj" ���� ��� Ȯ��
        std::cerr << "Failed to load model.obj" << std::endl;
    }

    // ������ �̷θ� �ֿܼ� ���
    std::cout << "Generated Maze (" << mazeSize << "x" << mazeSize << "):" << std::endl;
    for (const auto& row : maze.mazeData) {
        for (const auto& cell : row) {
            if (cell == 1)
                std::cout << "# ";
            else if (cell == 0)
                std::cout << "  ";
            else if (cell == 5)
                std::cout << "S ";
            else if (cell == 6)
                std::cout << "E ";
            else if (cell == 2)
                std::cout << "I "; // ������ ��ġ ǥ��
            else
                std::cout << "? ";
        }
        std::cout << std::endl;
    }
}

// �浹 ���� �Լ�: �÷��̾��� (x, z) ��ġ�� �̷��� ���� �浹�ϴ��� Ȯ��
bool isColliding(float x, float z) {
    // �÷��̾��� �浹 �ݰ� (0.4�� ����)
    const float playerRadius = 0.4f;

    // �÷��̾ ���� �׸��� ��
    int cellX = static_cast<int>(floor(x / maze.cellSize));
    int cellZ = static_cast<int>(floor(z / maze.cellSize));

    // �ֺ� 3x3 �� �˻�
    for (int i = cellX - 1; i <= cellX + 1; ++i) {
        for (int j = cellZ - 1; j <= cellZ + 1; ++j) {
            // �̷��� ������ ����� �ʵ��� �˻�
            if (i >= 0 && i < static_cast<int>(maze.mazeData[0].size()) &&
                j >= 0 && j < static_cast<int>(maze.mazeData.size())) {
                if (maze.mazeData[j][i] == 1) { // �� ��
                    // ���� �߽� ��ġ ���
                    float wallCenterX = (i + 0.5f) * maze.cellSize;
                    float wallCenterZ = (j + 0.5f) * maze.cellSize;

                    // �÷��̾�� �� �߽� ���� �Ÿ� ���
                    float dx = x - wallCenterX;
                    float dz = z - wallCenterZ;

                    // x �� z ���⿡���� �ּ� �Ÿ� ���
                    float overlapX = (playerRadius + maze.cellSize / 2.0f) - fabs(dx);
                    float overlapZ = (playerRadius + maze.cellSize / 2.0f) - fabs(dz);

                    // �浹 ���� Ȯ��
                    if (overlapX > 0 && overlapZ > 0) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

// ���� �����츦 �ʱ�ȭ�ϴ� �Լ�
void initGame(void) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // ����� ���������� ����
    glEnable(GL_DEPTH_TEST);

    // ���̵� ��� ����
    glShadeModel(GL_SMOOTH);

    // ���� �׽�Ʈ ����
    glEnable(GL_DEPTH_TEST);
}

// Ű���� ���� ������ ���� ���� ����
bool keys[256] = { false };

// Ű���� ���� �̺�Ʈ ó��
void keyboardPress(unsigned char key, int x, int y) {
    keys[key] = true;
    keyboardGame(key, x, y);
}

// Ű���� ���� �̺�Ʈ ó��
void keyboardRelease(unsigned char key, int x, int y) {
    keys[key] = false;
}

// ������ �ʱ�ȭ �Լ�: Ű���� �ݹ� ���� ����
void initGameModified(void) {
    initGame();
    // Ű���� ���� �� ���� �ݹ� ����
    glutKeyboardFunc(keyboardPress);
    glutKeyboardUpFunc(keyboardRelease);
}

// ���� �������ϴ� �Լ�
void renderModel() {
    glBegin(GL_TRIANGLES);
    for (const auto& face : model.faces) {
        // OBJ ������ �ε����� 1���� �����ϹǷ� 1�� ���ݴϴ�.
        for (int idx : face) {
            if (idx <= 0 || idx > static_cast<int>(model.vertices.size() / 3)) continue;
            glVertex3f(model.vertices[(idx - 1) * 3],
                model.vertices[(idx - 1) * 3 + 1],
                model.vertices[(idx - 1) * 3 + 2]);
        }
    }
    glEnd();
}

// ������ �ִϸ��̼� ������Ʈ �Լ�
void animateItems(float deltaTime) {
    for (auto& item : items) {
        // �պ� �: oscillateOffset�� �̿��Ͽ� y�� �������� �̵�
        float oscillateSpeed = 0.5f; // �պ� � �ӵ� (units per second)
        float oscillateRange = 0.2f; // �պ� � ���� (units)

        if (item.movingForward) {
            item.oscillateOffset += oscillateSpeed * deltaTime;
            if (item.oscillateOffset >= oscillateRange) {
                item.oscillateOffset = oscillateRange;
                item.movingForward = false;
            }
        }
        else {
            item.oscillateOffset -= oscillateSpeed * deltaTime;
            if (item.oscillateOffset <= -oscillateRange) {
                item.oscillateOffset = -oscillateRange;
                item.movingForward = true;
            }
        }

        // ȸ�� ���� ����
        float rotationSpeed = 90.0f; // �ʴ� ȸ�� ���� (degrees per second)
        item.rotationAngle += rotationSpeed * deltaTime;
        if (item.rotationAngle >= 360.0f)
            item.rotationAngle -= 360.0f;
    }
}

// ī�޶� ��鸲 ȿ���� ���� ����
float cameraTiltOffset = 0.0f;
float cameraTiltIntensity = 0.0f;

// ȭ�鿡 �ؽ�Ʈ�� �������ϴ� �Լ�
void renderText(float x, float y, const std::string& text, void* font = GLUT_BITMAP_HELVETICA_18, float r = 1.0f, float g = 1.0f, float b = 1.0f) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, glutGet(GLUT_WINDOW_WIDTH), 0.0, glutGet(GLUT_WINDOW_HEIGHT));

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING); // ���� ����
    glDisable(GL_DEPTH_TEST); // ���� �׽�Ʈ ����
    glColor3f(r, g, b); // ������ �������� �ؽ�Ʈ ������

    // �ؽ�Ʈ ���� ��ġ ����
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(font, c);
    }

    glEnable(GL_DEPTH_TEST); // ���� �׽�Ʈ �ٽ� Ȱ��ȭ
    glEnable(GL_LIGHTING);   // ���� �ٽ� Ȱ��ȭ

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// ���� �����츦 ǥ���ϴ� �Լ�
void displayGameWindow(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // ī�޶� ƿƮ ����
    float tilt = cameraTiltIntensity * sin(glutGet(GLUT_ELAPSED_TIME) * 0.01f);

    // ī�޶� ����
    float dirX = sinf(player.angleY * static_cast<float>(M_PI) / 180.0f) * cosf((player.angleX + tilt) * static_cast<float>(M_PI) / 180.0f);
    float dirY = sinf((player.angleX + tilt) * static_cast<float>(M_PI) / 180.0f);
    float dirZ = -cosf(player.angleY * static_cast<float>(M_PI) / 180.0f) * cosf((player.angleX + tilt) * static_cast<float>(M_PI) / 180.0f);

    gluLookAt(player.x, player.y, player.z,
        player.x + dirX, player.y + dirY, player.z + dirZ,
        0.0f, 1.0f, 0.0f);

    // ī�޶� ƿƮ ����
    cameraTiltIntensity *= 0.9f; // ������

    // ���� ����
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // �۷ι� �ں��Ʈ ����Ʈ �ణ ����
    GLfloat globalAmbient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

    // ������ ȿ���� ���� ����Ʈ����Ʈ ����
    GLfloat lightPos[] = { player.x, player.y, player.z, 1.0f };
    GLfloat spotDir[] = { dirX, dirY, dirZ };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spotDir);
    glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 35.0f); // ����Ʈ����Ʈ ���� ����
    glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 8.0f); // ����Ʈ����Ʈ ���ߵ� ����

    // ���� ���� ����
    GLfloat ambientLight[] = { 0.1f, 0.1f, 0.1f, 1.0f }; // �ֺ��� �ణ �߰�
    GLfloat diffuseLight[] = { 1.2f, 1.2f, 1.0f, 1.0f }; // Ȯ�걤 ���
    GLfloat specularLight[] = { 1.2f, 1.2f, 1.0f, 1.0f }; // �ݻ籤 ���

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

    // �Ÿ� ���� ���� (���� ���� ����)
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.05f);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.02f);

    // ���� ���� ���
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // �ٴ� �׸���
    glPushMatrix();
    glColor3f(0.3f, 0.3f, 0.3f); // �ٴ��� �ణ �� ���� ȸ��
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(static_cast<float>(maze.cellSize * maze.mazeData[0].size()), 0.0f, 0.0f); // ����: static_cast<float> ���
    glVertex3f(static_cast<float>(maze.cellSize * maze.mazeData[0].size()), 0.0f, static_cast<float>(maze.cellSize * maze.mazeData.size())); // ����
    glVertex3f(0.0f, 0.0f, static_cast<float>(maze.cellSize * maze.mazeData.size())); // ����
    glEnd();
    glPopMatrix();

    // �̷� �� �׸���
    glColor3f(0.7f, 0.7f, 0.7f); // ���� �ణ �� ���� ȸ��
    float wallHeight = 2.0f;

    for (size_t i = 0; i < maze.mazeData.size(); ++i) { // ����: size_t ���
        for (size_t j = 0; j < maze.mazeData[0].size(); ++j) { // ����
            if (maze.mazeData[i][j] == 1) { // �� ��
                glPushMatrix();
                glTranslatef((j + 0.5f) * maze.cellSize, wallHeight / 2.0f, (i + 0.5f) * maze.cellSize);
                glScalef(maze.cellSize, wallHeight, maze.cellSize);
                glutSolidCube(1.0f);
                glPopMatrix();
            }
        }
    }

    // ������ ������
    for (const auto& item : items) {
        if (item.get) continue; // ȹ���� �������� ���������� ����

        glPushMatrix();
        // �������� �׸��� ��ġ�� ���� ���� ��ǥ
        float baseX = (item.gridX + 0.5f) * maze.cellSize;
        float baseZ = (item.gridZ + 0.5f) * maze.cellSize;
        float y = 0.3f; // ���� y ��ġ�� ���� �ٴڰ� ��ġ�� �ʵ��� ��

        // �պ� � ���� (y�� �������� �̵�)
        float oscillateY = item.oscillateOffset;
        glTranslatef(baseX, y + oscillateY, baseZ);

        // ȸ�� ����
        glRotatef(item.rotationAngle, 0.0f, 1.0f, 0.0f);

        // �� ũ�� ����
        glScalef(1.0f, 1.0f, 1.0f);

        // �� ���� ���� (���� �ϴû�)
        glColor3f(0.53f, 0.81f, 0.98f); // ���� �ϴû�

        // �� ������
        renderModel();

        glPopMatrix();
    }

    // �ⱸ ǥ��
    glPushMatrix();
    float exitX = (game.exitgridX + 0.5f) * maze.cellSize;
    float exitZ = (game.exitgridZ + 0.5f) * maze.cellSize;
    float exitY = 0.5f; // �ⱸ ǥ���� ����

    glTranslatef(exitX, exitY, exitZ);

    // �������� ��� ��Ҵ����� ���� ���� ����
    if (game.countItem == items.size()) {
        glColor3f(0.0f, 1.0f, 0.0f); // �ʷϻ� (Ż�� ����)
    }
    else {
        glColor3f(1.0f, 1.0f, 0.0f); // ����� (������ �̼���)
    }

    glutSolidSphere(0.3f, 20, 20); // �ⱸ�� ���� �׷� ǥ��

    glPopMatrix();

    // ȹ���� ������ ���� �ؽ�Ʈ
    std::ostringstream oss;
    oss << "Item : " << game.countItem << " / " << items.size();
    renderText(10.0f, glutGet(GLUT_WINDOW_HEIGHT) - 30.0f, oss.str());

    // Ŭ���� ȭ��
    if (game.isClear) {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // ����� ���� ���� �ʱ�ȭ
        glLoadIdentity(); // ���� �𵨺� ��� �ʱ�ȭ

        // �ؽ�Ʈ ������ ���������� �����Ͽ� ���̵��� ����
        renderText(glutGet(GLUT_WINDOW_WIDTH) / 2 - 50, glutGet(GLUT_WINDOW_HEIGHT) / 2, "Game Clear!", GLUT_BITMAP_TIMES_ROMAN_24, 0.0f, 0.0f, 0.0f);
        glutSwapBuffers();
        return;
    }

    glutSwapBuffers();
}

// ���� �������� ũ�Ⱑ ����� �� ȣ��Ǵ� �Լ�
void reshapeGameWindow(int newWidth, int newHeight) {
    if (newHeight == 0) newHeight = 1;
    float aspectRatio = static_cast<float>(newWidth) / static_cast<float>(newHeight);

    glViewport(0, 0, newWidth, newHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, aspectRatio, 0.1f, 1000.0f);
    glMatrixMode(GL_MODELVIEW);
}

// ���ӿ��� Ű���� �Է��� ó���ϴ� �Լ�
void keyboardGame(unsigned char key, int x, int y) {
    if (key == 27) { // ESC Ű�� ������ ���α׷� ����
        exit(0);
    }
    else if (key == ' ') { // �����̽��ٸ� ������ �� ����
        if (player.onGround) {
            player.verticalVelocity = 0.10f; // ���� �ʱ� �ӵ�
            player.onGround = false;
            cameraTiltIntensity = 2.0f; // ���� �� ī�޶� ƿƮ
        }
    }
}

// Ű���忡�� Ű�� ������ �� ȣ��Ǵ� �Լ�
void keyboardUp(unsigned char key, int x, int y) {
    // �߰����� ������ �ʿ��ϸ� ����
}

// ���콺 �������� ó���ϴ� �Լ�
void passiveMouseMotion(int x, int y) {
    static bool warpPointer = false;
    if (warpPointer) {
        warpPointer = false;
        return;
    }

    int centerX = glutGet(GLUT_WINDOW_WIDTH) / 2;
    int centerY = glutGet(GLUT_WINDOW_HEIGHT) / 2;

    int deltaX = x - centerX;
    int deltaY = y - centerY;

    player.angleY += deltaX * 0.1f;
    player.angleX -= deltaY * 0.1f;

    if (player.angleX > 89.0f) player.angleX = 89.0f;
    if (player.angleX < -89.0f) player.angleX = -89.0f;

    warpPointer = true;
    glutWarpPointer(centerX, centerY);
}

// ���� ���� ������Ʈ �Լ�
void update(int value) {
    float speed = 0.05f;

    float dirX = sinf(player.angleY * static_cast<float>(M_PI) / 180.0f);
    float dirZ = -cosf(player.angleY * static_cast<float>(M_PI) / 180.0f);

    float rightX = sinf((player.angleY + 90.0f) * static_cast<float>(M_PI) / 180.0f);
    float rightZ = -cosf((player.angleY + 90.0f) * static_cast<float>(M_PI) / 180.0f);

    float moveX = 0.0f;
    float moveZ = 0.0f;

    // �̵� ������ Ȯ���ϱ� ���� ����
    bool isMoving = false;

    // W, A, S, D Ű�� ����� �̵�
    if (keys['w'] || keys['W']) {
        moveX += dirX * speed;
        moveZ += dirZ * speed;
        isMoving = true;
    }
    if (keys['s'] || keys['S']) {
        moveX -= dirX * speed;
        moveZ -= dirZ * speed;
        isMoving = true;
    }
    if (keys['a'] || keys['A']) {
        moveX -= rightX * speed;
        moveZ -= rightZ * speed;
        isMoving = true;
    }
    if (keys['d'] || keys['D']) {
        moveX += rightX * speed;
        moveZ += rightZ * speed;
        isMoving = true;
    }

    // �̵� ���̸� ī�޶� ƿƮ �߰�
    if (isMoving && player.onGround) {
        cameraTiltIntensity = 0.5f; // �̵� �� ī�޶� ƿƮ
    }

    // ���ο� ��ġ ���
    float newX = player.x + moveX;
    float newZ = player.z + moveZ;

    bool collisionX = isColliding(newX, player.z);
    bool collisionZ = isColliding(player.x, newZ);

    // �浹 ó��: ���� �̲������� �̵�
    if (!collisionX && !collisionZ) {
        player.x = newX;
        player.z = newZ;
    }
    else if (!collisionX && collisionZ) {
        player.x = newX;
    }
    else if (collisionX && !collisionZ) {
        player.z = newZ;
    }
    // else: �� �� ��� �浹�ϸ� �̵����� ����

    // ���� �� �߷� ó��
    if (!player.onGround) {
        player.verticalVelocity -= 0.005f; // �߷� ���ӵ�
        player.y += player.verticalVelocity;

        if (player.y <= 1.3f) { // ���鿡 ����� ��
            player.y = 1.3f;
            player.verticalVelocity = 0.0f;
            player.onGround = true;
            cameraTiltIntensity = 2.0f; // ���� �� ī�޶� ƿƮ
        }
    }

    // ������ ȹ�� Ȯ��
    for (auto& item : items) {
        if (item.get) continue;

        // �������� �׸��� ��ġ�� ���� ���� ��ǥ
        float itemX = (item.gridX + 0.5f) * maze.cellSize;
        float itemZ = (item.gridZ + 0.5f) * maze.cellSize;

        // �����ۿ� ������ ���� ȹ��
        if (fabs(player.x - itemX) < 0.3f && fabs(player.z - itemZ) < 0.3f) {
            std::cout << "������ ȹ��" << std::endl;
            item.get = true;
            game.countItem++;
        }
    }

    // Ŭ���� Ȯ��
    if (!game.isClear) {
        // ��ǥ ������ �׸��� ��ġ�� ���� ���� ��ǥ
        float exitX = (game.exitgridX + 0.5f) * maze.cellSize;
        float exitZ = (game.exitgridZ + 0.5f) * maze.cellSize;

        // ��ǥ ������ ������ ���� Ŭ���� ���� �˻�
        if (fabs(player.x - exitX) < 0.3f && fabs(player.z - exitZ) < 0.3f) {
            if (game.countItem == items.size()) {
                std::cout << "Ŭ����" << std::endl;
                game.isClear = true;
            }
            else {
                // �������� ��� ������ ������ ���
                std::cout << "��� �������� ��ƾ� �մϴ�!" << std::endl;
                // �ȳ� �޽����� ȭ�鿡 ǥ��
                renderText(glutGet(GLUT_WINDOW_WIDTH) / 2 - 100, glutGet(GLUT_WINDOW_HEIGHT) / 2, "Collect all items before exiting!", GLUT_BITMAP_TIMES_ROMAN_24, 1.0f, 1.0f, 1.0f);
            }
        }
    }

    // �ִϸ��̼� ������Ʈ (��Ÿ Ÿ�� ���)
    static int lastTime = glutGet(GLUT_ELAPSED_TIME);
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    float deltaTime = (currentTime - lastTime) / 1000.0f; // �� ����
    lastTime = currentTime;

    animateItems(deltaTime);

    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // �� 60 FPS
}
