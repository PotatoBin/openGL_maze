// modeler.h
#ifndef MODELER_H
#define MODELER_H

#include "globals.h"
#include <vector>
#include <stack>

// �Լ� ����
void saveCurrentStateToUndo();
void initModeler(void);
void drawGrid(void);
void displayButtons();
void displayModelerWindow(void);
void undo();
void redo();
void createModel();
void mouse(int button, int state, int x, int y);
bool hasOverlapWithLeftLines(const scrPt& p1, const scrPt& p2);

void keyboard(unsigned char key, int x, int y);

// �ܺο��� ���� ������ ���� ����
extern std::vector<scrPt> leftPoints;
extern std::vector<scrPt> rightPoints;
extern std::stack<std::vector<scrPt>> leftUndoStack, rightUndoStack;
extern std::stack<std::vector<scrPt>> leftRedoStack, rightRedoStack;

extern Rect undoButton;
extern Rect redoButton;
extern Rect gameStartButton;
extern Rect difficultyDecreaseButton;
extern Rect difficultyIncreaseButton;
extern Rect angleDecreaseButton;
extern Rect angleIncreaseButton;

#endif // MODELER_H
