// globals.h
#ifndef GLOBALS_H
#define GLOBALS_H

#include <GL/glut.h>

// ���� ���� ����
extern GLsizei winWidth, winHeight;
extern const int controlPanelHeight;
extern GLint endPtCtr;
extern GLint modelerWindow, gameWindow;
extern bool isModeling;

extern bool isRedoEnabled;
extern int angleValueIndex;
extern const int angleValues[];
extern const int numAngleValues;

extern int difficultyLevel; // ���̵� (1, 2, 3)

// ����ü �� Ŭ���� ����
struct Rect {
    int x, y, width, height;
    Rect(int x_in = 0, int y_in = 0, int width_in = 0, int height_in = 0)
        : x(x_in), y(y_in), width(width_in), height(height_in) {}
    bool contains(int px, int py);
};

struct scrPt {
    GLint x, y, z;
    scrPt(GLint x_in = 0, GLint y_in = 0, GLint z_in = 0) : x(x_in), y(y_in), z(z_in) {}
};

#endif // GLOBALS_H
