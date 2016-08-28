#include "maze.h"

#include <QPoint>
#include <QSet>
#include <algorithm>

#include <iostream>

inline std::string toString(QPoint p) {
    return QString("(%1, %2)").arg(p.x()).arg(p.y()).toStdString();
}

inline uint qHash(const QPoint p) {
    return qHash(QString::fromStdString(toString(p)), 0xa03f); // arbitrary value
}

inline float randomFloat() {
    return ((float) rand()) / (float) RAND_MAX;
}

QPoint randomInSet(QSet<QPoint> points)
{
    //QSet<QPoint>::iterator i = points.begin();

    int index = points.size() * randomFloat();

    int i = 0;
    foreach (QPoint p, points) {
        if (i == index)
            return p;
        i++;
    }
}

Maze::Maze(const int width, const int height) : WIDTH(width), HEIGHT(height)
{
    const int TOTAL_CELLS = width * height;

    _horizontals = QVector<bool>(width * (height+1));
    _verticals = QVector<bool>((width+1) * height);

    _horizontals.fill(true);
    _verticals.fill(true);

    QSet<QPoint> visited;
    visited.insert(QPoint(0,0));

    // grab unvisited point next to visited point
    while (visited.size() < TOTAL_CELLS) {
        QPoint randomVisited = randomInSet(visited);

        // figure out possible directions
        QPoint left(randomVisited.x()-1, randomVisited.y());
        QPoint right(randomVisited.x()+1, randomVisited.y());
        QPoint up(randomVisited.x(), randomVisited.y()+1);
        QPoint down(randomVisited.x(), randomVisited.y()-1);

        QSet<QPoint> possibleNexts;
        if (left.x() >= 0 && !visited.contains(left))
            possibleNexts.insert(left);
        else if (right.x() < width && !visited.contains(right))
            possibleNexts.insert(right);
        else if (up.y() < height && !visited.contains(up))
            possibleNexts.insert(up);
        else if (down.y() >= 0 && !visited.contains(down))
            possibleNexts.insert(down);

        if (possibleNexts.size() > 0) {
            QPoint next = randomInSet(possibleNexts);

            // knock down walls between to points
            if (randomVisited.x() - next.x() != 0) { // horizontal move
                const int x = std::min(randomVisited.x(), next.x());
                _verticals[randomVisited.y()*(width+1) + x+1] = false;
            } else { // vertical move
                const int y = std::min(randomVisited.y(), next.y());
                _horizontals[y+1 + randomVisited.x()*(height+1)] = false;
            }

            visited.insert(next);
        }
    }
}

Cell Maze::cell(int x, int y)
{
    Cell cell;

    cell.left = _verticals[y*(WIDTH+1) + x];
    cell.right = _verticals[y*(WIDTH+1) + x+1];
    cell.up = _horizontals[y+1 + x*(HEIGHT+1)];
    cell.down = _horizontals[y + x*(HEIGHT+1)];

    return cell;
}
