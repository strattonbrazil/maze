#ifndef MAZE_H
#define MAZE_H

#include <QVector>
#include <QPoint>

struct Cell
{
    bool up, down, left, right;
};

class Maze
{
public:
    Maze(const int width, const int height);
    Cell cell(int x, int y);
    int width() { return WIDTH; }
    int height() { return HEIGHT; }
private:
    QVector<bool> _horizontals;
    QVector<bool> _verticals;

    void removeWall(QPoint a, QPoint b);

    const int WIDTH;
    const int HEIGHT;
};

#endif // MAZE_H
