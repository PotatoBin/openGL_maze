// modeler.cpp

#include "modeler.h"
#include "maze.h"
#include "globals.h"
#include <GL/glut.h>
#include <vector>
#include <stack>
#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>

// 전역 변수 및 데이터 구조
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

// 현재 상태를 Undo 스택에 저장하는 함수
void saveCurrentStateToUndo() {
    leftUndoStack.push(leftPoints);
    rightUndoStack.push(rightPoints);
    leftRedoStack = std::stack<std::vector<scrPt>>();
    rightRedoStack = std::stack<std::vector<scrPt>>();
    isRedoEnabled = false;
}

// 모델러 윈도우를 초기화하는 함수
void initModeler(void) {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.0, winWidth, 0.0, winHeight);

    // 버튼 위치 초기화
    undoButton = Rect(50, 50, 80, 30);
    redoButton = Rect(150, 50, 80, 30);
    gameStartButton = Rect(winWidth - 150, 50, 120, 30);
    difficultyDecreaseButton = Rect(winWidth - 670, 50, 30, 30);
    difficultyIncreaseButton = Rect(winWidth - 590, 50, 30, 30);
    angleDecreaseButton = Rect(winWidth - 370, 50, 30, 30);
    angleIncreaseButton = Rect(winWidth - 290, 50, 30, 30);
}

// 배경 그리드를 그리는 함수
void drawGrid(void) {
    glColor3f(0.7f, 0.7f, 0.7f);
    glLineWidth(1.0);

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
    glLineWidth(2.0);
    glBegin(GL_LINES);
    glVertex2i(0, winHeight / 2);
    glVertex2i(winWidth, winHeight / 2);
    glVertex2i(winWidth / 2, controlPanelHeight);
    glVertex2i(winWidth / 2, winHeight);
    glEnd();

    glLineWidth(1.0);
}

// 버튼을 그리는 헬퍼 함수
void drawButton(Rect button, const char* text, bool isEnabled, float r = 0.8f, float g = 0.8f, float b = 0.8f) {
    glColor3f(isEnabled ? r : 0.5f, isEnabled ? g : 0.5f, isEnabled ? b : 0.5f);
    glRecti(button.x, button.y, button.x + button.width, button.y + button.height);

    glColor3f(0.0f, 0.0f, 0.0f);
    int textX = button.x + (button.width - glutBitmapLength(GLUT_BITMAP_8_BY_13, (const unsigned char*)text)) / 2;
    int textY = button.y + (button.height - 13) / 2 + 5;
    glRasterPos2i(textX, textY);
    for (const char* c = text; *c != '\0'; c++) glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
}

// 버튼 및 텍스트를 표시하는 함수
void displayButtons() {
    drawButton(undoButton, "Undo", true);
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
    glRasterPos2i(angleDecreaseButton.x - 150, angleDecreaseButton.y + 10);
    const char* angleText = "Revolving Angle:";
    for (const char* c = angleText; *c != '\0'; c++) glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);

    glRasterPos2i(angleDecreaseButton.x + 50, angleDecreaseButton.y + 10);
    std::string angleValueText = std::to_string(angleValues[angleValueIndex]) + "°";
    for (const char& c : angleValueText) glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);

    glRasterPos2i(difficultyDecreaseButton.x - 120, difficultyDecreaseButton.y + 10);
    const char* difficultyText = "Difficulty:";
    for (const char* c = difficultyText; *c != '\0'; c++) glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);

    glRasterPos2i(difficultyDecreaseButton.x + 50, difficultyDecreaseButton.y + 10);
    std::string difficultyValueText = std::to_string(difficultyLevel);
    for (const char& c : difficultyValueText) glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
}

// 모델러 윈도우를 표시하는 함수
void displayModelerWindow(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    drawGrid();
    displayButtons();

    glColor3f(0.0f, 0.0f, 0.0f);
    int textX = 20;
    int textY = winHeight - 30;

    size_t pointCount = leftPoints.size();

    int revolvingAngle = angleValues[angleValueIndex];
    double segmentCount = 360.0 / revolvingAngle;
    double estimatedPolygons = pointCount * (segmentCount - 1) + 2;

    std::stringstream ss;
    ss << "Points: " << pointCount << "  Estimated Polygons: " << estimatedPolygons;

    std::string infoText = ss.str();

    glRasterPos2i(textX, textY);
    for (const char& c : infoText) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
    }

    glLineWidth(2.0);

    // 왼쪽 선분 그리기
    glColor3f(0.0f, 1.0f, 0.0f); // 초록색
    glBegin(GL_LINE_STRIP);
    for (const auto& pt : leftPoints) glVertex2i(pt.x + winWidth / 2, pt.y + winHeight / 2);
    glEnd();

    // 오른쪽 선분 그리기
    glBegin(GL_LINE_STRIP);
    for (const auto& pt : rightPoints) glVertex2i(pt.x + winWidth / 2, pt.y + winHeight / 2);
    glEnd();

    // 노란색 선 그리기
    if (!leftPoints.empty() && !rightPoints.empty()) {
        glLineWidth(2.0);
        glColor3f(1.0, 1.0, 0.0); // 노란색
        glBegin(GL_LINES);
        // 시작점 연결선
        glVertex2i(leftPoints.front().x + winWidth / 2, leftPoints.front().y + winHeight / 2);
        glVertex2i(rightPoints.front().x + winWidth / 2, rightPoints.front().y + winHeight / 2);
        // 끝점 연결선
        glVertex2i(leftPoints.back().x + winWidth / 2, leftPoints.back().y + winHeight / 2);
        glVertex2i(rightPoints.back().x + winWidth / 2, rightPoints.back().y + winHeight / 2);
        glEnd();

        // 노란색 선 중앙에 점 그리기
        glPointSize(8.0);
        glBegin(GL_POINTS);
        // 시작점 연결선의 중앙
        int midStartX = (leftPoints.front().x + rightPoints.front().x) / 2 + winWidth / 2;
        int midStartY = (leftPoints.front().y + rightPoints.front().y) / 2 + winHeight / 2;
        glVertex2i(midStartX, midStartY);
        // 끝점 연결선의 중앙
        int midEndX = (leftPoints.back().x + rightPoints.back().x) / 2 + winWidth / 2;
        int midEndY = (leftPoints.back().y + rightPoints.back().y) / 2 + winHeight / 2;
        glVertex2i(midEndX, midEndY);
        glEnd();
    }

    // 좌우 점 그리기
    glPointSize(6.0);
    glColor3f(0.0f, 0.0f, 0.0f); // 검은색
    glBegin(GL_POINTS);
    for (const auto& pt : leftPoints) glVertex2i(pt.x + winWidth / 2, pt.y + winHeight / 2);
    for (const auto& pt : rightPoints) glVertex2i(pt.x + winWidth / 2, pt.y + winHeight / 2);
    glEnd();

    glFlush();
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
}

// 마우스 클릭 이벤트를 처리하는 함수
void mouse(int button, int state, int x, int y) {
    int yInverted = winHeight - y;

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (yInverted <= controlPanelHeight) {
            if (undoButton.contains(x, yInverted)) undo();
            else if (redoButton.contains(x, yInverted) && isRedoEnabled) redo();
            else if (gameStartButton.contains(x, yInverted)) {
                if (leftPoints.size() > 1) {
                    createModel();
                    generateMaze();
                }
            }
            else if (angleDecreaseButton.contains(x, yInverted)) {
                angleValueIndex = (angleValueIndex - 1 + numAngleValues) % numAngleValues;
            }
            else if (angleIncreaseButton.contains(x, yInverted)) {
                angleValueIndex = (angleValueIndex + 1) % numAngleValues;
            }
            else if (difficultyDecreaseButton.contains(x, yInverted)) {
                difficultyLevel = std::max(1, difficultyLevel - 1);
            }
            else if (difficultyIncreaseButton.contains(x, yInverted)) {
                difficultyLevel = std::min(3, difficultyLevel + 1);
            }
            glutPostRedisplay();
        }
        else {
            saveCurrentStateToUndo();
            scrPt pt(x - winWidth / 2, yInverted - winHeight / 2, 0);
            scrPt mirroredPt(winWidth / 2 - x, yInverted - winHeight / 2, 0);
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
void keyboard(unsigned char key, int x, int y) {
    if (key == 26) { // Ctrl+Z
        undo();
    }
    else if (key == 25) { // Ctrl+Y
        redo();
    }
}
