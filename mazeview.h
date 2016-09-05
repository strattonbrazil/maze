#ifndef MAZEVIEW_H
#define MAZEVIEW_H

#include "maze.h"
#include "player.h"

#include <QWidget>
#include <QGLWidget>
#include <QTimer>
#include <QElapsedTimer>

//#include <btBulletDynamicsCommon.h>

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
    void drawWall(QPoint p, QVector2D basis, bool w1, bool w2, bool w3, bool w4, bool w5, bool w6);
    void drawMazeOverlay(QPainter &painter);
    Maze* maze;
    Player player;
    QTimer* updateTimer;

    QElapsedTimer elapsedTimer;
    int lastTime;

    bool playerForward;
    bool playerBack;
    bool playerLeft;
    bool playerRight;
    bool playerStrafeLeft;
    bool playerStrafeRight;
};

#endif // MAZEVIEW_H
