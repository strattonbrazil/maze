#ifndef MAZEVIEW_H
#define MAZEVIEW_H

#include "maze.h"
#include "player.h"

#include <QWidget>
#include <QGLWidget>
#include <QTimer>
#include <QElapsedTimer>

class MazeView : public QGLWidget
{
    Q_OBJECT
public:
    explicit MazeView(QWidget *parent = 0);
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
signals:

public slots:
private:
    void drawMazeOverlay(QPainter &painter);
    Maze* maze;
    Player player;
    QTimer* updateTimer;

    QElapsedTimer elapsedTimer;
    int lastTime;

    bool playerForward;
    bool playerBack;
};

#endif // MAZEVIEW_H
