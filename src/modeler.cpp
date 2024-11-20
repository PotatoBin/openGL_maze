// modeler.cpp
#include "modeler.h"
#include "maze.h"
#include <GL/glut.h>
#include <vector>
#include <stack>
#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <cmath>

// 'M_PI' 정의 (정의되지 않은 경우)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 전역 변수 정의 (모델러 전용)
Rect undoButton;
Rect redoButton;
Rect gameStartButton;
Rect difficultyDecreaseButton;
Rect difficultyIncreaseButton;
Rect angleDecreaseButton;
Rect angleIncreaseButton;

std::vector<scrPt> leftPoints;
std::vector<scrPt> rightPoints;
std::stack<std::vector<scrPt>> leftUndoStack, rightUndoStack;
std::stack<std::vector<scrPt>> leftRedoStack, rightRedoStack;
bool isRedoEnabled = false;

std::vector<int> angleValues = { 10, 15, 20, 30, 45, 60 };
int angleValueIndex = 0;
int numAngleValues = 6;

int difficultyLevel = 1;

// 스케일 팩터 정의 (1/1000)
const float scaleFactor = 1.0f / 1000.0f;

// 현재 상태를 Undo 스택에 저장하는 함수
void saveCurrentStateToUndo() {
    leftUndoStack.push(leftPoints);
    rightUndoStack.push(rightPoints);
    // Redo 스택을 비웁니다.
    while (!leftRedoStack.empty()) leftRedoStack.pop();
    while (!rightRedoStack.empty()) rightRedoStack.pop();
    isRedoEnabled = false;
}

// OpenGL 오류 검사 함수
bool checkGLError(const std::string& context) {
    GLenum err;
    bool foundError = false;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL Error in " << context << ": " << gluErrorString(err) << std::endl;
        foundError = true;
    }
    return foundError;
}

// 모델러 윈도우를 초기화하는 함수
void initModeler(void) {
    glClearColor(1.0, 1.0, 1.0, 1.0); // 배경을 흰색으로 설정

    // 투영 설정
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, static_cast<GLdouble>(winWidth), 0.0, static_cast<GLdouble>(winHeight)); // 2D 직교 투영

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 뷰포트 설정
    glViewport(0, 0, winWidth, winHeight);

    // 버튼 위치 초기화 (Y 좌표를 controlPanelHeight 내로 조정)
    // 버튼들을 controlPanelHeight 영역 내에서 적절하게 배치
    undoButton = Rect(50, 30, 80, 30);
    redoButton = Rect(150, 30, 80, 30);
    gameStartButton = Rect(winWidth - 170, 30, 120, 30); // 위치 조정
    difficultyDecreaseButton = Rect(winWidth - 670, 30, 30, 30);
    difficultyIncreaseButton = Rect(winWidth - 590, 30, 30, 30);
    angleDecreaseButton = Rect(winWidth - 370, 30, 30, 30);
    angleIncreaseButton = Rect(winWidth - 290, 30, 30, 30);

    checkGLError("initModeler");
}

// 배경 그리드를 그리는 함수
void drawGrid(void) {
    glColor3f(0.7f, 0.7f, 0.7f);
    glLineWidth(1.0f);

    glBegin(GL_LINES);
    for (int x = 0; x <= winWidth; x += 20) {
        glVertex2i(x, controlPanelHeight);
        glVertex2i(x, winHeight);
    }
    for (int y = controlPanelHeight; y <= winHeight; y += 20) {
        glVertex2i(0, y);
        glVertex2i(winWidth, y);
    }
    glEnd();

    glColor3f(0.5f, 0.5f, 0.5f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2i(0, winHeight / 2);
    glVertex2i(winWidth, winHeight / 2);
    glVertex2i(winWidth / 2, controlPanelHeight);
    glVertex2i(winWidth / 2, winHeight);
    glEnd();

    glLineWidth(1.0f);
}

// 버튼을 그리는 헬퍼 함수
void drawButton(Rect button, const char* text, bool isEnabled, float r, float g, float b) {
    glColor3f(isEnabled ? r : 0.5f, isEnabled ? g : 0.5f, isEnabled ? b : 0.5f);
    glRecti(button.x, button.y, button.x + button.width, button.y + button.height);

    glColor3f(0.0f, 0.0f, 0.0f);
    int textWidth = glutBitmapLength(GLUT_BITMAP_8_BY_13, reinterpret_cast<const unsigned char*>(text));
    int textX = button.x + (button.width - textWidth) / 2;
    int textY = button.y + (button.height - 13) / 2 + 5;
    glRasterPos2i(textX, textY);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
    }
}

// 버튼 및 텍스트를 표시하는 함수
void displayButtons() {
    drawButton(undoButton, "Undo", !leftUndoStack.empty());
    drawButton(redoButton, "Redo", isRedoEnabled);

    bool isGameStartEnabled = leftPoints.size() > 1;
    if (isGameStartEnabled)
        drawButton(gameStartButton, "Game Start", true, 0.53f, 0.81f, 0.98f);
    else
        drawButton(gameStartButton, "Game Start", false);

    drawButton(angleDecreaseButton, "<", true);
    drawButton(angleIncreaseButton, ">", true);
    drawButton(difficultyDecreaseButton, "<", true);
    drawButton(difficultyIncreaseButton, ">", true);

    glColor3f(0.0f, 0.0f, 0.0f);
    // Revolving Angle 텍스트
    glRasterPos2i(angleDecreaseButton.x - 150, angleDecreaseButton.y + 10);
    const char* angleText = "Revolving Angle:";
    for (const char* c = angleText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
    }

    // Revolving Angle 값
    glRasterPos2i(angleDecreaseButton.x + 50, angleDecreaseButton.y + 10);
    std::string angleValueText = std::to_string(angleValues[angleValueIndex]) + "°";
    for (const char& c : angleValueText) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
    }

    // Difficulty 텍스트
    glRasterPos2i(difficultyDecreaseButton.x - 120, difficultyDecreaseButton.y + 10);
    const char* difficultyText = "Difficulty:";
    for (const char* c = difficultyText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
    }

    // Difficulty 값
    glRasterPos2i(difficultyDecreaseButton.x + 50, difficultyDecreaseButton.y + 10);
    std::string difficultyValueText = std::to_string(difficultyLevel);
    for (const char& c : difficultyValueText) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
    }
}

// 모델러 윈도우를 표시하는 함수
void displayModelerWindow(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    drawGrid();
    displayButtons();

    glColor3f(0.0f, 0.0f, 0.0f);
    int textX = 20;
    int textY = winHeight - 30;

    size_t pointCount = leftPoints.size();

    int revolvingAngle = angleValues[angleValueIndex];
    double radian = (M_PI / 180.0) * revolvingAngle;
    int segments = static_cast<int>(360.0 / revolvingAngle);

    double estimatedPolygons = pointCount * (segments - 1) + 2;

    std::stringstream ss;
    ss << "Points: " << pointCount << "  Estimated Polygons: " << estimatedPolygons;

    std::string infoText = ss.str();

    glRasterPos2i(textX, textY);
    for (const char& c : infoText) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
    }

    glLineWidth(2.0f);

    // 왼쪽 선분 그리기
    glColor3f(0.53f, 0.81f, 0.98f); // 연한 하늘색
    glBegin(GL_LINE_STRIP);
    for (const auto& pt : leftPoints) {
        glVertex2i(static_cast<int>(pt.x + winWidth / 2), static_cast<int>(pt.y + winHeight / 2));
    }
    glEnd();

    // 오른쪽 선분 그리기
    glBegin(GL_LINE_STRIP);
    for (const auto& pt : rightPoints) {
        glVertex2i(static_cast<int>(pt.x + winWidth / 2), static_cast<int>(pt.y + winHeight / 2));
    }
    glEnd();

    // 노란색 선 그리기
    if (!leftPoints.empty() && !rightPoints.empty()) {
        glLineWidth(2.0f);
        glColor3f(1.0f, 1.0f, 0.0f); // 노란색
        glBegin(GL_LINES);
        // 시작점 연결선
        glVertex2i(static_cast<int>(leftPoints.front().x + winWidth / 2), static_cast<int>(leftPoints.front().y + winHeight / 2));
        glVertex2i(static_cast<int>(rightPoints.front().x + winWidth / 2), static_cast<int>(rightPoints.front().y + winHeight / 2));
        // 끝점 연결선
        glVertex2i(static_cast<int>(leftPoints.back().x + winWidth / 2), static_cast<int>(leftPoints.back().y + winHeight / 2));
        glVertex2i(static_cast<int>(rightPoints.back().x + winWidth / 2), static_cast<int>(rightPoints.back().y + winHeight / 2));
        glEnd();

        // 노란색 선 중앙에 점 그리기
        glPointSize(8.0f);
        glBegin(GL_POINTS);
        // 시작점 연결선의 중앙
        int midStartX = static_cast<int>((leftPoints.front().x + rightPoints.front().x) / 2 + winWidth / 2);
        int midStartY = static_cast<int>((leftPoints.front().y + rightPoints.front().y) / 2 + winHeight / 2);
        glVertex2i(midStartX, midStartY);
        // 끝점 연결선의 중앙
        int midEndX = static_cast<int>((leftPoints.back().x + rightPoints.back().x) / 2 + winWidth / 2);
        int midEndY = static_cast<int>((leftPoints.back().y + rightPoints.back().y) / 2 + winHeight / 2);
        glVertex2i(midEndX, midEndY);
        glEnd();
    }

    // 좌우 점 그리기
    glPointSize(6.0f);
    glColor3f(0.0f, 0.0f, 0.0f); // 검은색
    glBegin(GL_POINTS);
    for (const auto& pt : leftPoints) {
        glVertex2i(static_cast<int>(pt.x + winWidth / 2), static_cast<int>(pt.y + winHeight / 2));
    }
    for (const auto& pt : rightPoints) {
        glVertex2i(static_cast<int>(pt.x + winWidth / 2), static_cast<int>(pt.y + winHeight / 2));
    }
    glEnd();

    glutSwapBuffers(); // 더블 버퍼링을 위한 버퍼 교환
}

// Undo 기능을 수행하는 함수
void undo() {
    if (!leftUndoStack.empty() && !rightUndoStack.empty()) {
        leftRedoStack.push(leftPoints);
        rightRedoStack.push(rightPoints);
        leftPoints = leftUndoStack.top();
        rightPoints = rightUndoStack.top();
        leftUndoStack.pop();
        rightUndoStack.pop();
        isRedoEnabled = true;
        glutPostRedisplay();
    }
}

// Redo 기능을 수행하는 함수
void redo() {
    if (!leftRedoStack.empty() && !rightRedoStack.empty()) {
        leftUndoStack.push(leftPoints);
        rightUndoStack.push(rightPoints);
        leftPoints = leftRedoStack.top();
        rightPoints = rightRedoStack.top();
        leftRedoStack.pop();
        rightRedoStack.pop();
        if (leftRedoStack.empty() && rightRedoStack.empty()) {
            isRedoEnabled = false;
        }
        glutPostRedisplay();
    }
}

// SOR 모델을 생성하는 함수
void createModel() {
    std::cout << "Creating model with " << leftPoints.size() << " points on the left and " << rightPoints.size() << " points on the right." << std::endl;

    std::vector<std::vector<scrPt>> rotatedPoints;
    int revolvingAngle = angleValues[angleValueIndex];
    double radian = (M_PI / 180.0) * revolvingAngle;
    int segments = 360 / revolvingAngle;

    // 점 회전 및 스케일링 적용
    for (int i = 0; i < segments; ++i) {
        std::vector<scrPt> rotatedLayer;
        for (const auto& pt : leftPoints) {
            double x = pt.x * cos(i * radian) * scaleFactor;
            double z = pt.x * sin(i * radian) * scaleFactor;
            double y = pt.y * scaleFactor;
            rotatedLayer.push_back(scrPt{ static_cast<float>(x), static_cast<float>(y), static_cast<float>(z) });
        }
        rotatedPoints.push_back(rotatedLayer);
    }

    // mesh 연결
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < segments - 1; ++i) {
        for (size_t j = 0; j < leftPoints.size() - 1; ++j) {
            const auto& p1 = rotatedPoints[i][j];
            const auto& p2 = rotatedPoints[i + 1][j];
            const auto& p3 = rotatedPoints[i][j + 1];
            const auto& p4 = rotatedPoints[i + 1][j + 1];

            glVertex3f(p1.x, p1.y, p1.z);
            glVertex3f(p2.x, p2.y, p2.z);
            glVertex3f(p3.x, p3.y, p3.z);

            glVertex3f(p2.x, p2.y, p2.z);
            glVertex3f(p4.x, p4.y, p4.z);
            glVertex3f(p3.x, p3.y, p3.z);
        }
    }
    glEnd();

    // 위아래 면
    glBegin(GL_POLYGON);
    for (int i = 0; i < segments; ++i) {
        glVertex3f(rotatedPoints[i].front().x, rotatedPoints[i].front().y, rotatedPoints[i].front().z);
    }
    glEnd();

    glBegin(GL_POLYGON);
    for (int i = 0; i < segments; ++i) {
        glVertex3f(rotatedPoints[i].back().x, rotatedPoints[i].back().y, rotatedPoints[i].back().z);
    }
    glEnd();

    // obj 파일 생성
    std::ofstream outFile("model.obj");
    if (!outFile) {
        std::cerr << "Error creating OBJ file.\n" << std::endl;
        return;
    }

    // 점 좌표 입력
    for (const auto& layer : rotatedPoints) {
        for (const auto& pt : layer) {
            outFile << "v " << pt.x << " " << pt.y << " " << pt.z << "\n";
        }
    }

    // 위아래 중심점 추가
    float topY = rotatedPoints[0][0].y;
    float bottomY = rotatedPoints[0].back().y;
    outFile << "v 0.0 " << topY << " 0.0\n"; // top center point
    outFile << "v 0.0 " << bottomY << " 0.0\n"; // bottom center point

    int totalVertices = segments * leftPoints.size() + 2; // +2 for the top and bottom center points
    int topCenterIndex = totalVertices - 1;
    int bottomCenterIndex = totalVertices;

    // 면 정보 입력
    for (int i = 0; i < segments; ++i) {
        int nextSegment = (i + 1) % segments;
        for (size_t j = 0; j < leftPoints.size() - 1; ++j) {
            int idx1 = i * leftPoints.size() + j + 1;
            int idx2 = nextSegment * leftPoints.size() + j + 1;
            int idx3 = i * leftPoints.size() + j + 2;
            int idx4 = nextSegment * leftPoints.size() + j + 2;

            outFile << "f " << idx1 << " " << idx2 << " " << idx3 << "\n";
            outFile << "f " << idx2 << " " << idx4 << " " << idx3 << "\n";
        }

        // 위 면
        int idxTop1 = i * leftPoints.size() + 1;
        int idxTop2 = nextSegment * leftPoints.size() + 1;
        outFile << "f " << topCenterIndex << " " << idxTop1 << " " << idxTop2 << "\n";

        // 아래 면
        int idxBottom1 = i * leftPoints.size() + leftPoints.size();
        int idxBottom2 = nextSegment * leftPoints.size() + leftPoints.size();
        outFile << "f " << bottomCenterIndex << " " << idxBottom2 << " " << idxBottom1 << "\n";
    }

    outFile.close();
    std::cout << "Model saved as model.obj\n" << std::endl;
}

// 마우스 클릭 이벤트를 처리하는 함수
void mouseModeler(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        int yInverted = winHeight - y;

        if (yInverted <= controlPanelHeight) { // 컨트롤 패널 영역
            if (undoButton.contains(x, yInverted)) undo();
            else if (redoButton.contains(x, yInverted) && isRedoEnabled) redo();
            else if (gameStartButton.contains(x, yInverted)) {
                if (leftPoints.size() > 1) {
                    createModel();
                    generateMaze();

                    // 모델러 윈도우를 숨기고 게임 윈도우를 표시
                    glutSetWindow(modelerWindow);
                    glutHideWindow();

                    glutSetWindow(gameWindow);
                    glutShowWindow();
                    glutPostRedisplay();
                }
            }
            else if (angleDecreaseButton.contains(x, yInverted)) {
                angleValueIndex = (angleValueIndex - 1 + numAngleValues) % numAngleValues;
                glutPostRedisplay();
            }
            else if (angleIncreaseButton.contains(x, yInverted)) {
                angleValueIndex = (angleValueIndex + 1) % numAngleValues;
                glutPostRedisplay();
            }
            else if (difficultyDecreaseButton.contains(x, yInverted)) {
                difficultyLevel = std::max(1, difficultyLevel - 1);
                glutPostRedisplay();
            }
            else if (difficultyIncreaseButton.contains(x, yInverted)) {
                difficultyLevel = std::min(3, difficultyLevel + 1);
                glutPostRedisplay();
            }
        }
        else { // 모델링 영역
            saveCurrentStateToUndo();
            int centeredX = x - winWidth / 2;
            int centeredY = yInverted - winHeight / 2;
            scrPt pt(static_cast<float>(centeredX), static_cast<float>(centeredY), 0.0f);
            scrPt mirroredPt(-static_cast<float>(centeredX), static_cast<float>(centeredY), 0.0f);
            if (x < winWidth / 2) {
                leftPoints.push_back(pt);
                rightPoints.push_back(mirroredPt);
            }
            else {
                rightPoints.push_back(pt);
                leftPoints.push_back(mirroredPt);
            }
            glutPostRedisplay();
        }
    }
}

// 키보드 입력을 처리하는 함수
void keyboardModeler(unsigned char key, int x, int y) {
    if (key == 26) { // Ctrl+Z (ASCII 26)
        undo();
    }
    else if (key == 25) { // Ctrl+Y (ASCII 25)
        redo();
    }
}
