// modeler.h
#ifndef MODELER_H
#define MODELER_H

#include <vector>
#include <stack>
#include <string>

// 버튼을 위한 Rect 구조체 정의
struct Rect {
    int x, y;
    int width, height;
    Rect(int _x = 0, int _y = 0, int _w = 0, int _h = 0)
        : x(_x), y(_y), width(_w), height(_h) {}
    bool contains(int px, int py) const {
        return px >= x && px <= x + width && py >= y && py <= y + height;
    }
};

// 점 좌표 구조체
struct scrPt {
    float x, y, z;
    scrPt(float _x = 0, float _y = 0, float _z = 0)
        : x(_x), y(_y), z(_z) {}
};

// 외부 전역 변수 선언
extern const int winWidth;
extern const int winHeight;
extern const int controlPanelHeight;
extern int modelerWindow;
extern int gameWindow;

// 모델러 전용 전역 변수 선언
extern Rect undoButton;
extern Rect redoButton;
extern Rect gameStartButton;
extern Rect difficultyDecreaseButton;
extern Rect difficultyIncreaseButton;
extern Rect angleDecreaseButton;
extern Rect angleIncreaseButton;

extern std::vector<scrPt> leftPoints;
extern std::vector<scrPt> rightPoints;
extern std::stack<std::vector<scrPt>> leftUndoStack, rightUndoStack;
extern std::stack<std::vector<scrPt>> leftRedoStack, rightRedoStack;
extern bool isRedoEnabled;

extern std::vector<int> angleValues;
extern int angleValueIndex;
extern int numAngleValues;

extern int difficultyLevel;

// 함수 선언
void initModeler();
void displayModelerWindow();
void drawGrid();
void displayButtons();
void drawButton(Rect button, const char* text, bool isEnabled, float r = 0.8f, float g = 0.8f, float b = 0.8f);
void mouseModeler(int button, int state, int x, int y);
void keyboardModeler(unsigned char key, int x, int y);
void undo();
void redo();
void createModel();

#endif
