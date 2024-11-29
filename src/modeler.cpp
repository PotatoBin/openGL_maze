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
Rect axisToggleButton;
Rect difficultyDecreaseButton;
Rect difficultyIncreaseButton;
Rect angleDecreaseButton;
Rect angleIncreaseButton;
Rect buildOrGameStartButton;

std::vector<scrPt> leftPoints;
std::vector<scrPt> rightPoints;
std::stack<std::vector<scrPt>> leftUndoStack, rightUndoStack;
std::stack<std::vector<scrPt>> leftRedoStack, rightRedoStack;
bool isRedoEnabled = false;

std::vector<int> angleValues = { 1, 2, 3, 4, 5, 6, 8, 9, 10, 15, 20, 30, 45, 60 };
int angleValueIndex = 0;
int numAngleValues = 14;

int difficultyLevel = 1;
char revolvingAxis = 'Y'; // 'Y' 또는 'X'

bool isModelBuilt = false; // 모델이 생성되었는지 여부

std::vector<std::vector<scrPt>> rotatedPoints; // 회전된 점들 저장

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
    int buttonY = 30;
    int buttonWidth = 80;
    int buttonHeight = 30;
    int spacing = 20;

    undoButton = Rect(50, buttonY, buttonWidth, buttonHeight);
    redoButton = Rect(undoButton.x + buttonWidth + spacing, buttonY, buttonWidth, buttonHeight);
    axisToggleButton = Rect(redoButton.x + buttonWidth + spacing, buttonY, buttonWidth, buttonHeight);
    difficultyDecreaseButton = Rect(winWidth - 670, buttonY, 30, buttonHeight);
    difficultyIncreaseButton = Rect(winWidth - 590, buttonY, 30, buttonHeight);
    angleDecreaseButton = Rect(winWidth - 370, buttonY, 30, buttonHeight);
    angleIncreaseButton = Rect(winWidth - 290, buttonY, 30, buttonHeight);
    buildOrGameStartButton = Rect(winWidth - 170, buttonY, 120, buttonHeight); // 위치 조정

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
    drawButton(axisToggleButton, revolvingAxis == 'Y' ? "Axis: Y" : "Axis: X", !isModelBuilt);

    bool isBuildEnabled = leftPoints.size() > 1 && !isModelBuilt;
    bool isGameStartEnabled = isModelBuilt;

    if (isBuildEnabled)
        drawButton(buildOrGameStartButton, "Build Model", true, 0.53f, 0.81f, 0.98f);
    else if (isGameStartEnabled)
        drawButton(buildOrGameStartButton, "Game Start", true, 0.53f, 0.81f, 0.98f);
    else
        drawButton(buildOrGameStartButton, "Build Model", false);

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

    if (!isModelBuilt) {
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

        // 좌우 점 그리기 (강조)
        glPointSize(6.0f);
        glColor3f(1.0f, 0.0f, 0.0f); // 빨간색으로 점 강조
        glBegin(GL_POINTS);
        for (const auto& pt : leftPoints) {
            glVertex2i(static_cast<int>(pt.x + winWidth / 2), static_cast<int>(pt.y + winHeight / 2));
        }
        for (const auto& pt : rightPoints) {
            glVertex2i(static_cast<int>(pt.x + winWidth / 2), static_cast<int>(pt.y + winHeight / 2));
        }
        glEnd();
    }
    else {
        // 모델을 화면에 그리기
        glColor3f(0.0f, 0.0f, 1.0f); // 파란색
        glLineWidth(1.0f);

        const float displayScaleFactor = 1000.0f;

        // 위아래 중심점 계산
        scrPt topCenter, bottomCenter;
        if (revolvingAxis == 'Y') {
            topCenter = scrPt(0.0f, rotatedPoints[0][0].y, 0.0f);
            bottomCenter = scrPt(0.0f, rotatedPoints[0].back().y, 0.0f);
        }
        else { // revolvingAxis == 'X'
            topCenter = scrPt(rotatedPoints[0][0].x, 0.0f, 0.0f);
            bottomCenter = scrPt(rotatedPoints[0].back().x, 0.0f, 0.0f);
        }

        // 측면 그리기
        for (size_t i = 0; i < rotatedPoints.size(); ++i) {
            size_t nextI = (i + 1) % rotatedPoints.size();
            for (size_t j = 0; j < rotatedPoints[i].size() - 1; ++j) {
                // 현재 레이어의 점들
                scrPt p1 = rotatedPoints[i][j];
                scrPt p2 = rotatedPoints[nextI][j];
                scrPt p3 = rotatedPoints[i][j + 1];
                scrPt p4 = rotatedPoints[nextI][j + 1];

                float x1, y1, x2, y2, x3, y3, x4, y4;

                if (revolvingAxis == 'Y') {
                    x1 = p1.x * displayScaleFactor + winWidth / 2;
                    y1 = p1.y * displayScaleFactor + winHeight / 2;

                    x2 = p2.x * displayScaleFactor + winWidth / 2;
                    y2 = p2.y * displayScaleFactor + winHeight / 2;

                    x3 = p3.x * displayScaleFactor + winWidth / 2;
                    y3 = p3.y * displayScaleFactor + winHeight / 2;

                    x4 = p4.x * displayScaleFactor + winWidth / 2;
                    y4 = p4.y * displayScaleFactor + winHeight / 2;
                }
                else { // revolvingAxis == 'X'
                    x1 = p1.z * displayScaleFactor + winWidth / 2;
                    y1 = p1.y * displayScaleFactor + winHeight / 2;

                    x2 = p2.z * displayScaleFactor + winWidth / 2;
                    y2 = p2.y * displayScaleFactor + winHeight / 2;

                    x3 = p3.z * displayScaleFactor + winWidth / 2;
                    y3 = p3.y * displayScaleFactor + winHeight / 2;

                    x4 = p4.z * displayScaleFactor + winWidth / 2;
                    y4 = p4.y * displayScaleFactor + winHeight / 2;
                }

                // 선 그리기
                glBegin(GL_LINES);
                glVertex2f(x1, y1);
                glVertex2f(x2, y2);

                glVertex2f(x1, y1);
                glVertex2f(x3, y3);

                glVertex2f(x2, y2);
                glVertex2f(x4, y4);

                glVertex2f(x3, y3);
                glVertex2f(x4, y4);
                glEnd();

                // 점 그리기 (강조)
                glPointSize(4.0f);
                glColor3f(1.0f, 0.0f, 0.0f); // 빨간색
                glBegin(GL_POINTS);
                glVertex2f(x1, y1);
                glVertex2f(x2, y2);
                glVertex2f(x3, y3);
                glVertex2f(x4, y4);
                glEnd();
                glColor3f(0.0f, 0.0f, 1.0f); // 색상 복원
            }
        }

        // 위아래 면 그리기
        // 위 면
        glBegin(GL_POLYGON);
        for (size_t i = 0; i < rotatedPoints.size(); ++i) {
            scrPt p = rotatedPoints[i][0];
            float x, y;

            if (revolvingAxis == 'Y') {
                x = p.x * displayScaleFactor + winWidth / 2;
                y = p.y * displayScaleFactor + winHeight / 2;
            }
            else {
                x = p.z * displayScaleFactor + winWidth / 2;
                y = p.y * displayScaleFactor + winHeight / 2;
            }
            glVertex2f(x, y);
        }
        glEnd();

        // 아래 면
        glBegin(GL_POLYGON);
        for (size_t i = 0; i < rotatedPoints.size(); ++i) {
            scrPt p = rotatedPoints[i].back();
            float x, y;

            if (revolvingAxis == 'Y') {
                x = p.x * displayScaleFactor + winWidth / 2;
                y = p.y * displayScaleFactor + winHeight / 2;
            }
            else {
                x = p.z * displayScaleFactor + winWidth / 2;
                y = p.y * displayScaleFactor + winHeight / 2;
            }
            glVertex2f(x, y);
        }
        glEnd();
    }

    glutSwapBuffers(); // 더블 버퍼링을 위한 버퍼 교환
}

// Undo 기능을 수행하는 함수
void undo() {
    if (isModelBuilt) {
        clearModel();
    }
    else if (!leftUndoStack.empty() && !rightUndoStack.empty()) {
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

    rotatedPoints.clear(); // 회전된 점들 초기화

    int revolvingAngle = angleValues[angleValueIndex];
    double radian = (M_PI / 180.0) * revolvingAngle;
    int segments = 360 / revolvingAngle;

    // 점 회전 및 스케일링 적용
    for (int i = 0; i < segments; ++i) {
        std::vector<scrPt> rotatedLayer;
        for (const auto& pt : leftPoints) {
            double x, y, z;
            if (revolvingAxis == 'Y') {
                x = pt.x * cos(i * radian) * scaleFactor;
                z = pt.x * sin(i * radian) * scaleFactor;
                y = pt.y * scaleFactor;
            }
            else { // revolvingAxis == 'X'
                y = pt.y * cos(i * radian) * scaleFactor;
                z = pt.y * sin(i * radian) * scaleFactor;
                x = pt.x * scaleFactor;
            }
            // Axis가 X일 때 모델을 Y축 기준으로 -90도 회전
            if (revolvingAxis == 'X') {
                double tempX = x;
                x = -z;
                z = tempX;
            }
            rotatedLayer.push_back(scrPt{ static_cast<float>(x), static_cast<float>(y), static_cast<float>(z) });
        }
        rotatedPoints.push_back(rotatedLayer);
    }

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
    float topCoord, bottomCoord;
    if (revolvingAxis == 'Y') {
        topCoord = rotatedPoints[0][0].y;
        bottomCoord = rotatedPoints[0].back().y;
        outFile << "v 0.0 " << topCoord << " 0.0\n"; // top center point
        outFile << "v 0.0 " << bottomCoord << " 0.0\n"; // bottom center point
    }
    else {
        topCoord = rotatedPoints[0][0].y;
        bottomCoord = rotatedPoints[0].back().y;
        outFile << "v 0.0 " << topCoord << " 0.0\n"; // top center point
        outFile << "v 0.0 " << bottomCoord << " 0.0\n"; // bottom center point
    }

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

    // 모델이 생성되었음을 표시
    isModelBuilt = true;
    glutPostRedisplay();
}

// 모델을 지우는 함수
void clearModel() {
    // 모델을 지우기 위해 필요한 처리를 수행합니다.
    // 여기서는 OBJ 파일을 삭제하지 않고, 모델 생성 상태를 false로 변경합니다.
    isModelBuilt = false;
    rotatedPoints.clear();
    glutPostRedisplay();
}

// 마우스 클릭 이벤트를 처리하는 함수
void mouseModeler(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        int yInverted = winHeight - y;

        if (yInverted <= controlPanelHeight) { // 컨트롤 패널 영역
            if (undoButton.contains(x, yInverted)) undo();
            else if (redoButton.contains(x, yInverted) && isRedoEnabled) redo();
            else if (axisToggleButton.contains(x, yInverted) && !isModelBuilt) {
                // 축 변경 및 점 회전
                if (revolvingAxis == 'Y') {
                    revolvingAxis = 'X';
                    // 점 회전 (90도)
                    for (auto& pt : leftPoints) {
                        float temp = pt.x;
                        pt.x = pt.y;
                        pt.y = -temp;
                    }
                    for (auto& pt : rightPoints) {
                        float temp = pt.x;
                        pt.x = pt.y;
                        pt.y = -temp;
                    }
                }
                else {
                    revolvingAxis = 'Y';
                    // 점 회전 (-90도)
                    for (auto& pt : leftPoints) {
                        float temp = pt.x;
                        pt.x = -pt.y;
                        pt.y = temp;
                    }
                    for (auto& pt : rightPoints) {
                        float temp = pt.x;
                        pt.x = -pt.y;
                        pt.y = temp;
                    }
                }
                glutPostRedisplay();
            }
            else if (buildOrGameStartButton.contains(x, yInverted)) {
                if (!isModelBuilt && leftPoints.size() > 1) {
                    createModel();
                }
                else if (isModelBuilt) {
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
            if (isModelBuilt) {
                // 모델이 생성된 상태에서는 편집 불가
                return;
            }

            saveCurrentStateToUndo();
            int centeredX = x - winWidth / 2;
            int centeredY = yInverted - winHeight / 2;

            scrPt pt(static_cast<float>(centeredX), static_cast<float>(centeredY), 0.0f);
            scrPt mirroredPt;

            if (revolvingAxis == 'Y') {
                mirroredPt = scrPt(-static_cast<float>(centeredX), static_cast<float>(centeredY), 0.0f);
            }
            else { // revolvingAxis == 'X'
                mirroredPt = scrPt(static_cast<float>(centeredX), -static_cast<float>(centeredY), 0.0f);
            }

            if ((revolvingAxis == 'Y' && x < winWidth / 2) || (revolvingAxis == 'X' && yInverted > winHeight / 2)) {
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
