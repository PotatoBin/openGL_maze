﻿// maze.cpp
#include "maze.h"
#include "modeler.h"
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <fstream>
#include <sstream>

// 'M_PI' 정의 (정의되지 않은 경우)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 플레이어 및 미로 데이터 정의
Player player;
Maze maze;
Model model;
std::vector<Item> items;

// 프림 알고리즘을 사용하여 미로를 생성하는 함수
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

// OBJ 파일을 로드하는 함수
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
        // 다른 프리픽스는 무시
    }

    infile.close();
    std::cout << "Loaded OBJ file: " << filename << " with "
        << model.vertices.size() / 3 << " vertices and "
        << model.faces.size() << " faces." << std::endl;
    return true;
}

// 미로를 생성하는 함수
void generateMaze() {
    int currentDifficultyLevel = difficultyLevel; // 모델러에서 가져옴
    int mazeSize = currentDifficultyLevel * 12 + 1;
    maze.mazeData = std::vector<std::vector<int>>(mazeSize, std::vector<int>(mazeSize, 1));
    maze.cellSize = 1.0f;

    maze.mazeData[1][1] = 0;

    primMaze(maze.mazeData);

    maze.mazeData[mazeSize - 2][mazeSize - 2] = 0;

    maze.mazeData[1][1] = 5; // 시작 위치
    maze.mazeData[mazeSize - 2][mazeSize - 2] = 6; // 목표 위치

    // 아이템 배치 (난이도에 따라 랜덤하게 배치)
    int itemCount = currentDifficultyLevel; // 난이도에 따라 아이템 수 조정
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, mazeSize - 2);

    items.clear(); // 아이템 리스트 초기화

    while (items.size() < static_cast<size_t>(itemCount)) {
        int i = dis(gen);
        int j = dis(gen);
        if (maze.mazeData[j][i] == 0) { // 경로에만 아이템 배치
            maze.mazeData[j][i] = 2;
            items.emplace_back(i, j);
        }
    }

    // 플레이어 시작 위치 설정
    player.x = 1.5f;
    player.y = 1.3f; // 카메라 높이 조정
    player.z = 1.5f;
    player.verticalVelocity = 0.0f;
    player.onGround = true;

    // 모델 로드
    if (!loadOBJ("model.obj", model)) { // "model.obj" 파일 경로 확인
        std::cerr << "Failed to load model.obj" << std::endl;
    }

    // 생성된 미로를 콘솔에 출력
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
                std::cout << "I "; // 아이템 위치 표시
            else
                std::cout << "? ";
        }
        std::cout << std::endl;
    }
}

// 충돌 감지 함수: 플레이어의 (x, z) 위치가 미로의 벽과 충돌하는지 확인
bool isColliding(float x, float z) {
    // 플레이어의 충돌 반경 (0.4로 증가)
    const float playerRadius = 0.4f;

    // 플레이어가 속한 그리드 셀
    int cellX = static_cast<int>(floor(x / maze.cellSize));
    int cellZ = static_cast<int>(floor(z / maze.cellSize));

    // 주변 3x3 셀 검사
    for (int i = cellX - 1; i <= cellX + 1; ++i) {
        for (int j = cellZ - 1; j <= cellZ + 1; ++j) {
            // 미로의 범위를 벗어나지 않도록 검사
            if (i >= 0 && i < static_cast<int>(maze.mazeData[0].size()) &&
                j >= 0 && j < static_cast<int>(maze.mazeData.size())) {
                if (maze.mazeData[j][i] == 1) { // 벽 셀
                    // 벽의 중심 위치 계산
                    float wallCenterX = (i + 0.5f) * maze.cellSize;
                    float wallCenterZ = (j + 0.5f) * maze.cellSize;

                    // 플레이어와 벽 중심 간의 거리 계산
                    float dx = x - wallCenterX;
                    float dz = z - wallCenterZ;
                    float distance = sqrtf(dx * dx + dz * dz);

                    // 벽과 플레이어의 충돌 여부 확인
                    if (distance < playerRadius + (maze.cellSize / 2.0f)) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

// 게임 윈도우를 초기화하는 함수
void initGame(void) {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    // 조명 설정
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat ambientLight[] = { 0.5f, 0.5f, 0.5f, 1.0f }; // 주변광 (강조)
    GLfloat diffuseLight[] = { 0.7f, 0.7f, 0.7f, 1.0f }; // 확산광
    GLfloat specularLight[] = { 0.9f, 0.9f, 0.9f, 1.0f }; // 반사광
    GLfloat lightPosition[] = { 0.0f, 10.0f, 0.0f, 1.0f }; // 조명 위치 (위에서 비추는 빛)

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    // 색상 재질 사용
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // 쉐이딩 모드 설정
    glShadeModel(GL_SMOOTH);

    // 깊이 테스트 설정
    glEnable(GL_DEPTH_TEST);

    // 그림자 효과를 위해 블렌딩 활성화
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// 키보드 상태 추적을 위한 전역 변수
bool keys[256] = { false };

// 키보드 눌림 이벤트 처리
void keyboardPress(unsigned char key, int x, int y) {
    keys[key] = true;
    keyboardGame(key, x, y);
}

// 키보드 놓임 이벤트 처리
void keyboardRelease(unsigned char key, int x, int y) {
    keys[key] = false;
}

// 수정된 초기화 함수: 키보드 콜백 설정 포함
void initGameModified(void) {
    initGame();
    // 키보드 눌림 및 놓임 콜백 설정
    glutKeyboardFunc(keyboardPress);
    glutKeyboardUpFunc(keyboardRelease);
}

// 모델을 렌더링하는 함수
void renderModel() {
    glBegin(GL_TRIANGLES);
    for (const auto& face : model.faces) {
        // OBJ 파일의 인덱스는 1부터 시작하므로 1을 빼줍니다.
        for (int idx : face) {
            if (idx <= 0 || idx > static_cast<int>(model.vertices.size() / 3)) continue;
            glVertex3f(model.vertices[(idx - 1) * 3],
                model.vertices[(idx - 1) * 3 + 1],
                model.vertices[(idx - 1) * 3 + 2]);
        }
    }
    glEnd();
}

// 아이템 애니메이션 업데이트 함수
void animateItems(float deltaTime) {
    for (auto& item : items) {
        // 왕복 운동: oscillateOffset을 이용하여 y축 방향으로 이동
        float oscillateSpeed = 0.5f; // 왕복 운동 속도 (units per second)
        float oscillateRange = 0.2f; // 왕복 운동 범위 (units)

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

        // 회전 각도 증가
        float rotationSpeed = 90.0f; // 초당 회전 각도 (degrees per second)
        item.rotationAngle += rotationSpeed * deltaTime;
        if (item.rotationAngle >= 360.0f)
            item.rotationAngle -= 360.0f;
    }
}

// 카메라 흔들림 효과를 위한 변수
float cameraTiltOffset = 0.0f;
float cameraTiltIntensity = 0.0f;

// 게임 윈도우를 표시하는 함수
void displayGameWindow(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // 카메라 틸트 적용
    float tilt = cameraTiltIntensity * sin(glutGet(GLUT_ELAPSED_TIME) * 0.01f);

    // 카메라 설정
    float dirX = sinf(player.angleY * static_cast<float>(M_PI) / 180.0f) * cosf((player.angleX + tilt) * static_cast<float>(M_PI) / 180.0f);
    float dirY = sinf((player.angleX + tilt) * static_cast<float>(M_PI) / 180.0f);
    float dirZ = -cosf(player.angleY * static_cast<float>(M_PI) / 180.0f) * cosf((player.angleX + tilt) * static_cast<float>(M_PI) / 180.0f);

    gluLookAt(player.x, player.y, player.z,
        player.x + dirX, player.y + dirY, player.z + dirZ,
        0.0f, 1.0f, 0.0f);

    // 카메라 틸트 감쇠
    cameraTiltIntensity *= 0.9f; // 감쇠율

    // 바닥 그리기
    glPushMatrix();
    glColor3f(0.2f, 0.2f, 0.2f); // 바닥은 어두운 회색
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(maze.cellSize * maze.mazeData[0].size(), 0.0f, 0.0f);
    glVertex3f(maze.cellSize * maze.mazeData[0].size(), 0.0f, maze.cellSize * maze.mazeData.size());
    glVertex3f(0.0f, 0.0f, maze.cellSize * maze.mazeData.size());
    glEnd();
    glPopMatrix();

    // 미로 벽 그리기
    glColor3f(0.8f, 0.8f, 0.8f); // 벽은 밝은 회색
    float wallHeight = 2.0f;

    for (int i = 0; i < maze.mazeData.size(); ++i) {
        for (int j = 0; j < maze.mazeData[0].size(); ++j) {
            if (maze.mazeData[i][j] == 1) { // 벽 셀
                glPushMatrix();
                glTranslatef((j + 0.5f) * maze.cellSize, wallHeight / 2.0f, (i + 0.5f) * maze.cellSize);
                glScalef(maze.cellSize, wallHeight, maze.cellSize);
                glutSolidCube(1.0f);
                glPopMatrix();
            }
        }
    }

    // 아이템 렌더링
    for (const auto& item : items) {
        glPushMatrix();
        // 아이템의 그리드 위치에 대한 월드 좌표
        float baseX = (item.gridX + 0.5f) * maze.cellSize;
        float baseZ = (item.gridZ + 0.5f) * maze.cellSize;
        float y = 0.3f; // 모델의 y 위치를 낮춰 바닥과 겹치지 않도록 함

        // 왕복 운동 적용 (y축 방향으로 이동)
        float oscillateY = item.oscillateOffset;
        glTranslatef(baseX, y + oscillateY, baseZ);

        // 회전 적용
        glRotatef(item.rotationAngle, 0.0f, 1.0f, 0.0f);

        // 모델 크기 조정
        glScalef(1.0f, 1.0f, 1.0f);

        // 조명 활성화
        glEnable(GL_LIGHTING);

        // 모델 색상 설정 (연한 하늘색)
        glColor3f(0.53f, 0.81f, 0.98f); // 연한 하늘색

        // 모델 렌더링
        renderModel();

        glPopMatrix();
    }

    glutSwapBuffers();
}

// 게임 윈도우의 크기가 변경될 때 호출되는 함수
void reshapeGameWindow(int newWidth, int newHeight) {
    if (newHeight == 0) newHeight = 1;
    float aspectRatio = static_cast<float>(newWidth) / static_cast<float>(newHeight);

    glViewport(0, 0, newWidth, newHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, aspectRatio, 0.1f, 1000.0f);
    glMatrixMode(GL_MODELVIEW);
}

// 게임에서 키보드 입력을 처리하는 함수
void keyboardGame(unsigned char key, int x, int y) {
    if (key == 27) { // ESC 키를 누르면 프로그램 종료
        exit(0);
    }
    else if (key == ' ') { // 스페이스바를 눌렀을 때 점프
        if (player.onGround) {
            player.verticalVelocity = 0.10f; // 점프 초기 속도
            player.onGround = false;
            cameraTiltIntensity = 2.0f; // 점프 시 카메라 틸트
        }
    }
}

// 키보드에서 키가 놓였을 때 호출되는 함수
void keyboardUp(unsigned char key, int x, int y) {
    // 추가적인 동작이 필요하면 구현
}

// 마우스 움직임을 처리하는 함수
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

// 게임 로직 업데이트 함수
void update(int value) {
    float speed = 0.05f;

    float dirX = sinf(player.angleY * static_cast<float>(M_PI) / 180.0f);
    float dirZ = -cosf(player.angleY * static_cast<float>(M_PI) / 180.0f);

    float rightX = sinf((player.angleY + 90.0f) * static_cast<float>(M_PI) / 180.0f);
    float rightZ = -cosf((player.angleY + 90.0f) * static_cast<float>(M_PI) / 180.0f);

    float moveX = 0.0f;
    float moveZ = 0.0f;

    // 이동 중인지 확인하기 위한 변수
    bool isMoving = false;

    // W, A, S, D 키를 사용한 이동
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

    // 이동 중이면 카메라 틸트 추가
    if (isMoving && player.onGround) {
        cameraTiltIntensity = 0.5f; // 이동 시 카메라 틸트
    }

    // 축별로 이동 및 충돌 검사
    float tentativeX = player.x + moveX;
    if (!isColliding(tentativeX, player.z)) {
        player.x = tentativeX;
    }

    float tentativeZ = player.z + moveZ;
    if (!isColliding(player.x, tentativeZ)) {
        player.z = tentativeZ;
    }

    // 점프 및 중력 처리
    if (!player.onGround) {
        player.verticalVelocity -= 0.005f; // 중력 가속도
        player.y += player.verticalVelocity;

        if (player.y <= 1.3f) { // 지면에 닿았을 때
            player.y = 1.3f;
            player.verticalVelocity = 0.0f;
            player.onGround = true;
            cameraTiltIntensity = 2.0f; // 착지 시 카메라 틸트
        }
    }

    // 애니메이션 업데이트 (델타 타임 계산)
    static int lastTime = glutGet(GLUT_ELAPSED_TIME);
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    float deltaTime = (currentTime - lastTime) / 1000.0f; // 초 단위
    lastTime = currentTime;

    animateItems(deltaTime);

    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // 약 60 FPS
}
