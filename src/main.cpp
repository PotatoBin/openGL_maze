#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <vector>
#include <stack>
#include <iostream>
#include <fstream>
#include <string>

GLsizei winWidth = 1280, winHeight = 720;
const int controlPanelHeight = 100;
GLint endPtCtr = 0;
GLint modelerWindow, gameWindow;
bool isModeling = true;

bool isRedoEnabled = false;
int angleValueIndex = 7;
const int angleValues[] = { 10, 12, 15, 18, 20, 24, 30, 36, 40, 45, 60, 72, 90, 120 };
const int numAngleValues = sizeof(angleValues) / sizeof(angleValues[0]);

struct Rect {
    int x, y, width, height;
    bool contains(int px, int py) {
        return px >= x && px <= x + width && py >= y && py <= y + height;
    }
};

Rect undoButton = { 50, 50, 80, 30 };
Rect redoButton = { 150, 50, 80, 30 };
Rect createModelButton = { winWidth - 150, 50, 120, 30 };
Rect angleDecreaseButton = { winWidth - 400, 50, 30, 30 };
Rect angleIncreaseButton = { winWidth - 300, 50, 30, 30 };

class scrPt {
public:
    GLint x, y, z = 0; // 3D 좌표 (x, y, 0) 형태로 저장
};

std::vector<scrPt> leftPoints;
std::vector<scrPt> rightPoints;
std::stack<std::vector<scrPt>> leftUndoStack, rightUndoStack;
std::stack<std::vector<scrPt>> leftRedoStack, rightRedoStack;

void saveCurrentStateToUndo();
void initModeler(void);
void initGame(void);
void drawGrid(void);
void displayButtons();
void displayModelerWindow(void);
void undo();
void redo();
void createModel();  // Create Model 로직 분리
void mouse(int button, int state, int x, int y);

// 현재 상태를 Undo 스택에 저장
void saveCurrentStateToUndo() {
    leftUndoStack.push(leftPoints);
    rightUndoStack.push(rightPoints);
    leftRedoStack = std::stack<std::vector<scrPt>>();
    isRedoEnabled = false;
}

// 모델러 초기화
void initModeler(void) {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.0, winWidth, 0.0, winHeight);
}

// 게임 초기화
void initGame(void) {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.0, winWidth, 0.0, winHeight);
}

// 배경 그리드 그리기
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

// UI 버튼 그리기
void displayButtons() {
    glColor3f(0.8f, 0.8f, 0.8f);
    glRecti(undoButton.x, undoButton.y, undoButton.x + undoButton.width, undoButton.y + undoButton.height);
    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2i(undoButton.x + 15, undoButton.y + 10);
    const char* undoText = "Undo";
    for (const char* c = undoText; *c != '\0'; c++) glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);

    glColor3f(isRedoEnabled ? 0.8f : 0.5f, isRedoEnabled ? 0.8f : 0.5f, isRedoEnabled ? 0.8f : 0.5f);
    glRecti(redoButton.x, redoButton.y, redoButton.x + redoButton.width, redoButton.y + redoButton.height);
    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2i(redoButton.x + 15, redoButton.y + 10);
    const char* redoText = "Redo";
    for (const char* c = redoText; *c != '\0'; c++) glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);

    glColor3f(0.6f, 0.8f, 1.0f);
    glRecti(createModelButton.x, createModelButton.y, createModelButton.x + createModelButton.width, createModelButton.y + createModelButton.height);
    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2i(createModelButton.x + 15, createModelButton.y + 10);
    const char* createText = "Create Model";
    for (const char* c = createText; *c != '\0'; c++) glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);

    glColor3f(0.6f, 0.6f, 0.6f);
    glRecti(angleDecreaseButton.x, angleDecreaseButton.y, angleDecreaseButton.x + angleDecreaseButton.width, angleDecreaseButton.y + angleDecreaseButton.height);
    glRecti(angleIncreaseButton.x, angleIncreaseButton.y, angleIncreaseButton.x + angleIncreaseButton.width, angleIncreaseButton.y + angleIncreaseButton.height);

    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2i(angleDecreaseButton.x + 10, angleDecreaseButton.y + 10);
    glutBitmapCharacter(GLUT_BITMAP_8_BY_13, '<');
    glRasterPos2i(angleIncreaseButton.x + 10, angleIncreaseButton.y + 10);
    glutBitmapCharacter(GLUT_BITMAP_8_BY_13, '>');

    glRasterPos2i(angleDecreaseButton.x - 120, angleDecreaseButton.y + 10);
    const char* angleText = "Revolving Angle:";
    for (const char* c = angleText; *c != '\0'; c++) glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);

    glRasterPos2i(angleDecreaseButton.x + 50, angleDecreaseButton.y + 10);
    std::string angleValueText = std::to_string(angleValues[angleValueIndex]) + "°";
    for (const char& c : angleValueText) glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
}

// 모델러 화면 표시
void displayModelerWindow(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    drawGrid();
    displayButtons();

    glColor3f(0.0, 1.0, 0.0);
    glLineWidth(2.0);

    glBegin(GL_LINE_STRIP);
    for (const auto& pt : leftPoints) glVertex2i(pt.x + winWidth / 2, pt.y + winHeight / 2);
    glEnd();

    glBegin(GL_LINE_STRIP);
    for (const auto& pt : rightPoints) glVertex2i(pt.x + winWidth / 2, pt.y + winHeight / 2);
    glEnd();

    if (!leftPoints.empty() && !rightPoints.empty()) {
        glLineWidth(2.0);
        glColor3f(1.0, 1.0, 0.0);
        glBegin(GL_LINES);
        glVertex2i(leftPoints.front().x + winWidth / 2, leftPoints.front().y + winHeight / 2);
        glVertex2i(rightPoints.front().x + winWidth / 2, rightPoints.front().y + winHeight / 2);
        glVertex2i(leftPoints.back().x + winWidth / 2, leftPoints.back().y + winHeight / 2);
        glVertex2i(rightPoints.back().x + winWidth / 2, rightPoints.back().y + winHeight / 2);
        glEnd();
    }

    glPointSize(6.0);
    glBegin(GL_POINTS);
    for (const auto& pt : leftPoints) glVertex2i(pt.x + winWidth / 2, pt.y + winHeight / 2);
    glEnd();

    glBegin(GL_POINTS);
    for (const auto& pt : rightPoints) glVertex2i(pt.x + winWidth / 2, pt.y + winHeight / 2);
    glEnd();

    glFlush();
}

// Undo 기능 수행
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

// Redo 기능 수행
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

// Create Model
void createModel() {
    int angle = angleValues[angleValueIndex]; // 회전 각도
    std::cout << "Revolving Angle: " << angle << " degrees\n";

    // 왼쪽 선 좌표
    std::cout << "Left Points:\n";
    for (const auto& pt : leftPoints) {
        std::cout << "(" << pt.x << ", " << pt.y << ", " << pt.z << ")\n";
    }

    // 모델링 로직
    // obj 저장 로직
}

// 마우스 클릭 이벤트 처리
void mouse(int button, int state, int x, int y) {
    int yInverted = winHeight - y;

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (yInverted <= controlPanelHeight) {
            if (undoButton.contains(x, yInverted)) undo();
            else if (redoButton.contains(x, yInverted) && isRedoEnabled) redo();
            else if (createModelButton.contains(x, yInverted)) {
                createModel();  // Create Model 함수 호출
            }
            else if (angleDecreaseButton.contains(x, yInverted)) {
                angleValueIndex = (angleValueIndex - 1 + numAngleValues) % numAngleValues;
            }
            else if (angleIncreaseButton.contains(x, yInverted)) {
                angleValueIndex = (angleValueIndex + 1) % numAngleValues;
            }
        }
        else {
            saveCurrentStateToUndo();
            scrPt pt = { x - winWidth / 2, yInverted - winHeight / 2, 0 };
            scrPt mirroredPt = { winWidth / 2 - x, yInverted - winHeight / 2, 0 };
            if (x < winWidth / 2) {
                leftPoints.push_back(pt);
                rightPoints.push_back(mirroredPt);
            }
            else {
                rightPoints.push_back(pt);
                leftPoints.push_back(mirroredPt);
            }
        }
    }
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

    glutInitWindowPosition(100, 100);
    glutInitWindowSize(1280, 720);
    modelerWindow = glutCreateWindow("SOR Modeler Window");
    initModeler();
    glutDisplayFunc(displayModelerWindow);
    glutMouseFunc(mouse);

    glutMainLoop();
    return 0;
}
 