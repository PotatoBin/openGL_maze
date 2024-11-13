// globals.cpp

#include "globals.h"

// 전역 변수 정의
GLsizei winWidth = 1280, winHeight = 720;
const int controlPanelHeight = 100;
GLint endPtCtr = 0;
GLint modelerWindow, gameWindow;
bool isModeling = true;

bool isRedoEnabled = false;
int angleValueIndex = 7;
const int angleValues[] = { 10, 12, 15, 18, 20, 24, 30, 36, 40, 45, 60, 72, 90, 120 };
const int numAngleValues = sizeof(angleValues) / sizeof(angleValues[0]);

int difficultyLevel = 1; // 난이도 (1, 2, 3)

// Rect 구조체의 멤버 함수 정의
bool Rect::contains(int px, int py) {
    return px >= x && px <= x + width && py >= y && py <= y + height;
}
