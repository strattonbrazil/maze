#ifndef MAZEVIEW_H
#define MAZEVIEW_H

#include "maze.h"

#include <QWidget>
#include <QGLWidget>

class MazeView : public QGLWidget
{
    Q_OBJECT
public:
    explicit MazeView(QWidget *parent = 0);
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
signals:

public slots:
private:
    void drawMazeOverlay(QPainter &painter);
    Maze* maze;
};

#endif // MAZEVIEW_H
