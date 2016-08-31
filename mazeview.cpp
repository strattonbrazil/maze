#include "mazeview.h"

#include <QMatrix4x4>
#include <QKeyEvent>

#include <iostream>


MazeView::MazeView(QWidget *parent) : QGLWidget(parent), lastTime(0)
{
    maze = new Maze(50, 50);

    setFocusPolicy(Qt::ClickFocus);

    updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(update()));
    updateTimer->setInterval(10);
    updateTimer->start();

    elapsedTimer.start();
}

void MazeView::initializeGL()
{
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

}

void MazeView::resizeGL(int w, int h)
{

}

void MazeView::paintGL()
{
    int newTime = elapsedTimer.elapsed();
    int elapsed = newTime - lastTime;
    lastTime = newTime;

    // TODO: make an update function
    if (playerForward) {
        player.speedForward(elapsed);
    } else if (playerBack) {
        player.speedBack(elapsed);
    } else {
        player.slowDown(elapsed);
    }
    player.update(elapsed);

    QPainter painter(this);
    //painter.begin(this);

    painter.beginNativePainting();

    glEnable(GL_DEPTH_TEST);

    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float aspect = width() / (float)height();
    QMatrix4x4 proj;
    proj.perspective(30, aspect, 0.3, 100);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glLoadMatrixf(proj.data());

    QMatrix4x4 camera;
    QVector3D playerPos = player.pos();
    camera.lookAt(playerPos,
                  playerPos + player.lookDir(),
                  QVector3D(0, 0, 1));

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLoadMatrixf(camera.data());

    glBegin(GL_QUADS);
    {
        for (int row = 0; row < maze->height(); row++) {
            for (int column = 0; column < maze->width(); column++) {
                Cell cell = maze->cell(column, row);

                const int x = column;
                const int y = row;

                glColor3f(1,0,0); //red
                if (cell.up) {
                    glVertex3f(x,y+1,0);
                    glVertex3f(x+1,y+1,0);
                    glVertex3f(x+1,y+1,1);
                    glVertex3f(x,y+1,1);
                }

                glColor3f(1,1,0); // yellow
                if (cell.down) {
                    glVertex3f(x+1,y,0);
                    glVertex3f(x,y,0);
                    glVertex3f(x,y,1);
                    glVertex3f(x+1,y,1);
                }

                glColor3f(1,0,1); // purple
                if (cell.left) {
                    glVertex3f(x,y,0);
                    glVertex3f(x,y+1,0);
                    glVertex3f(x,y+1,1);
                    glVertex3f(x,y,1);
                }

                glColor3f(0,1,1); // cyan
                if (cell.right) {
                    glVertex3f(x+1,y+1,0);
                    glVertex3f(x+1,y,0);
                    glVertex3f(x+1,y,1);
                    glVertex3f(x+1,y+1,1);
                }
            }
        }

        glColor3f(1,1,1);
        glVertex2f(0, 0);
        glVertex2f(1, 0);
        glVertex2f(1, 1);
        glVertex2f(0, 1);
    }
    glEnd();

    painter.endNativePainting();

    drawMazeOverlay(painter);

    painter.end();
}

void MazeView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_W && !playerBack) {
        playerForward = true;
    } else if (event->key() == Qt::Key_S && !playerForward) {
        playerBack = true;
    }
}

void MazeView::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_W) {
        playerForward = false;
    } else if (event->key() == Qt::Key_S) {
        playerBack = false;
    }
}

void MazeView::drawMazeOverlay(QPainter &painter)
{
    QPen penHText(QColor("#00e0fc"));
    painter.setPen(penHText);
    //painter.setBrush(Qt::white);

    QMatrix prevMatrix = painter.matrix();

    QMatrix flipMatrix;
    flipMatrix.translate(0, height());
    flipMatrix.scale(1,-1);
    painter.setMatrix(flipMatrix);

    for (int row = 0; row < maze->height(); row++) {
        for (int column = 0; column < maze->width(); column++) {
            Cell cell = maze->cell(column, row);

            const int x = column * 20 + 20;
            const int y = row * 20 + 20;

            if (cell.up)
                painter.drawLine(x, y+18, x+18, y+18);
            if (cell.down)
                painter.drawLine(x, y, x+18, y);
            if (cell.left)
                painter.drawLine(x, y, x, (y+18));
            if (cell.right)
                painter.drawLine(x+18, y, x+18, (y+18));
        }
    }

    // draw the player
    QVector3D playerPos = player.pos();
    painter.drawRect(20*playerPos.x() - 1 + 20, 20*playerPos.y() - 1 + 20, 2, 2);

    painter.setMatrix(prevMatrix);
}
