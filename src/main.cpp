// main.cpp
#include <GL/glut.h>
#include "modeler.h"
#include "maze.h"
#include <iostream>

// 전역 상수 정의
const int winWidth = 1280;
const int winHeight = 720;
const int controlPanelHeight = 100; // 이전 50에서 100으로 증가

// 윈도우 식별자
int modelerWindow;
int gameWindow;

// 메인 함수: 프로그램 실행 시작점
int main(int argc, char** argv) {
    glutInit(&argc, argv);

    // 모델러 윈도우 생성
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(winWidth, winHeight);
    modelerWindow = glutCreateWindow("SOR Modeler Window");
    initModeler();
    glutDisplayFunc(displayModelerWindow);
    glutMouseFunc(mouseModeler);
    glutKeyboardFunc(keyboardModeler);

    // 게임 윈도우 생성 (초기에는 숨김 상태)
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition(200, 200); // 모델러와 겹치지 않도록 위치 조정
    glutInitWindowSize(winWidth, winHeight);
    gameWindow = glutCreateWindow("Escape the Maze");
    initGameModified(); // 수정된 초기화 함수 호출
    glutDisplayFunc(displayGameWindow);
    glutReshapeFunc(reshapeGameWindow);
    glutPassiveMotionFunc(passiveMouseMotion);
    glutSetCursor(GLUT_CURSOR_NONE);
    glutTimerFunc(0, update, 0);
    glutHideWindow();

    // 메인 루프 시작
    glutMainLoop();
    return 0;
}
