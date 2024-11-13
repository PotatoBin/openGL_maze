// main.cpp

#include <GL/glut.h>
#include "globals.h"
#include "modeler.h"
#include "maze.h"

// 메인 함수: 프로그램 실행 시작점
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(winWidth, winHeight);
    modelerWindow = glutCreateWindow("SOR Modeler Window");
    initModeler();
    glutDisplayFunc(displayModelerWindow);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard); 

    glutMainLoop();
    return 0;
}
