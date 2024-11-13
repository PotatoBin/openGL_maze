// maze.cpp

#include "maze.h"
#include "globals.h"
#include <iostream>
#include <vector>
#include <random>

// 프림 알고리즘을 사용하여 미로를 생성하는 함수
void primMaze(std::vector<std::vector<int>>& maze) {
    int mazeSize = maze.size();

    struct Cell {
        int x, y;
        int px, py;
    };

    std::vector<Cell> walls;

    std::random_device rd;
    std::mt19937 gen(rd());

    int x = 1;
    int y = 1;
    if (x + 2 < mazeSize - 1) walls.push_back({ x + 2, y, x, y });
    if (y + 2 < mazeSize - 1) walls.push_back({ x, y + 2, x, y });

    while (!walls.empty()) {
        std::uniform_int_distribution<> dis(0, walls.size() - 1);
        int idx = dis(gen);
        Cell wall = walls[idx];

        walls.erase(walls.begin() + idx);

        int nx = wall.x;
        int ny = wall.y;
        int px = wall.px;
        int py = wall.py;

        if (maze[ny][nx] == 1) {
            maze[ny][nx] = 0;
            maze[(ny + py) / 2][(nx + px) / 2] = 0;

            if (nx + 2 < mazeSize - 1 && maze[ny][nx + 2] == 1)
                walls.push_back({ nx + 2, ny, nx, ny });
            if (nx - 2 > 0 && maze[ny][nx - 2] == 1)
                walls.push_back({ nx - 2, ny, nx, ny });
            if (ny + 2 < mazeSize - 1 && maze[ny + 2][nx] == 1)
                walls.push_back({ nx, ny + 2, nx, ny });
            if (ny - 2 > 0 && maze[ny - 2][nx] == 1)
                walls.push_back({ nx, ny - 2, nx, ny });
        }
    }
}

// 미로를 생성하는 함수
void generateMaze() {
    int mazeSize = difficultyLevel * 12 + 1;
    std::vector<std::vector<int>> maze(mazeSize, std::vector<int>(mazeSize, 1));

    maze[1][1] = 0;

    primMaze(maze);

    maze[mazeSize - 2][mazeSize - 2] = 0;

    maze[1][1] = 5;
    maze[mazeSize - 2][mazeSize - 2] = 6;

    for (int i = 0; i < mazeSize; ++i) {
        for (int j = 0; j < mazeSize; ++j) {
            std::cout << maze[i][j] << " ";
        }
        std::cout << std::endl;
    }
}
