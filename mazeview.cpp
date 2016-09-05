#include "mazeview.h"

#include <QMatrix4x4>
#include <QKeyEvent>
#include <QCache>

#include <iostream>


MazeView::MazeView(QWidget *parent) : QGLWidget(parent), lastTime(0)
{
    maze = new Maze(10, 10);

    setFocusPolicy(Qt::ClickFocus);

    updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(update()));
    updateTimer->setInterval(10);
    updateTimer->start();

    elapsedTimer.start();

    playerLeft = false;
    playerRight = false;
    playerForward = false;
    playerBack = false;
    playerStrafeLeft = false;
    playerStrafeRight = false;
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

    if (playerStrafeLeft) {
        player.speedSideways(elapsed, STRAFE_LEFT);
    } else if (playerStrafeRight) {
        player.speedSideways(elapsed, STRAFE_RIGHT);
    } else {
        player.sidewaysDown(elapsed);
    }

    if (playerLeft)
        player.turnLeft(elapsed);
    if (playerRight)
        player.turnRight(elapsed);

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

    const int THICK = 0.05f;

    glBegin(GL_QUADS);
    {
        for (int row = 0; row < maze->height(); row++) {
            for (int column = 0; column < maze->width(); column++) {
                Cell cell = maze->cell(column, row);

                const int x = column;
                const int y = row;

                // TODO: toss this in a cache
                // cells around current cell (row-ordered top to bottom)
                Cell c1 = maze->cell(column-1, row+1);
                Cell c2 = maze->cell(column, row+1);
                Cell c3 = maze->cell(column+1, row+1);
                Cell c4 = maze->cell(column-1, row);
                Cell c5 = maze->cell(column, row); // drawing this one
                Cell c6 = maze->cell(column + 1, row);
                Cell c7 = maze->cell(column - 1, row - 1);
                Cell c8 = maze->cell(column, row - 1);
                Cell c9 = maze->cell(column + 1, row - 1);

                glColor3f(1,0,0); //red
                if (cell.up)
                    drawWall(QPoint(x, y+1), QVector2D(1,0), c1.right, c1.down, c4.right, c5.right, c6.up, c3.left);

                glColor3f(1,1,0); // yellow
                if (cell.down) {
                    drawWall(QPoint(x+1, y), QVector2D(-1,0), c8.right, c6.down, c5.right, c4.right, c4.down, c7.right);
                }

                glColor3f(1,0,1); // purple
                if (cell.left) {
                    drawWall(QPoint(x,y), QVector2D(0,1), c7.up, c7.right, c5.down, c5.up, c2.left, c1.down);
                    /*
                    glVertex3f(x,y,0);
                    glVertex3f(x,y+1,0);
                    glVertex3f(x,y+1,1);
                    glVertex3f(x,y,1);
                    */
                }

                glColor3f(0,1,1); // cyan
                if (cell.right) {
                    drawWall(QPoint(x+1,y+1), QVector2D(0,-1), c3.down, c3.left, c5.up, c5.down, c8.right, c9.up);
                    /*
                    glVertex3f(x+1,y+1,0);
                    glVertex3f(x+1,y,0);
                    glVertex3f(x+1,y,1);
                    glVertex3f(x+1,y+1,1);
                    */
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

    // draw ground grid
    glBegin(GL_LINES);
    {
        for (int row = 0; row <= maze->height(); row++) {
            glVertex2f(0, row);
            glVertex2f(width(), row);
        }

        for (int column = 0; column <= maze->width(); column++) {
            glVertex2f(column, 0);
            glVertex2f(column, height());
        }
    }
    glEnd();

    glDisable(GL_DEPTH_TEST);

    painter.endNativePainting();

    drawMazeOverlay(painter);

    painter.end();
}

void MazeView::keyPressEvent(QKeyEvent *event)
{
    // forward and back
    if (event->key() == Qt::Key_W && !playerBack) {
        playerForward = true;
    } else if (event->key() == Qt::Key_S && !playerForward) {
        playerBack = true;
    }

    // turning
    if (event->key() == Qt::Key_Q) {
        playerLeft = true;
    } else if (event->key() == Qt::Key_E) {
        playerRight = true;
    }

    // stafing
    if (event->key() == Qt::Key_A) {
        playerStrafeLeft = true;
    } else if (event->key() == Qt::Key_D) {
        playerStrafeRight = true;
    }
}

void MazeView::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_W) {
        playerForward = false;
    } else if (event->key() == Qt::Key_S) {
        playerBack = false;
    } else if (event->key() == Qt::Key_Q) {
        playerLeft = false;
    } else if (event->key() == Qt::Key_E) {
        playerRight = false;
    } else if (event->key() == Qt::Key_A) {
        playerStrafeLeft = false;
    } else if (event->key() == Qt::Key_D) {
        playerStrafeRight = false;
    }
}

std::string toStr(QVector3D v) {
    return QString("(%1, %2, %3)").arg(v.x()).arg(v.y()).arg(v.z()).toStdString();
}

void MazeView::drawWall(QPoint pXY, QVector2D basisBottom, bool w1, bool w2, bool w3, bool w4, bool w5, bool w6)
{
    const float OFFSET = 0.1f;

    QVector3D start(pXY);
    QVector3D basisTop(0, 0, 1);
    QVector3D basisOut = QVector3D::crossProduct(basisBottom, basisTop);

    float wallLength = 1.0f;
    QVector3D cornerA;
    if (w3) {
        cornerA = start + (basisBottom * OFFSET) + (basisOut * OFFSET);
        wallLength -= OFFSET;
    } else if (w2) {
        cornerA = start + basisOut * OFFSET;
    } else {
        cornerA = start + (basisOut * OFFSET) + (basisBottom * -OFFSET);
        wallLength += OFFSET;
    }

    if (w4) {
        wallLength -= OFFSET;
    } else if (w6) {
        wallLength += OFFSET;
    } else if (!w5) {
        wallLength += OFFSET;
    }

    QVector3D cornerB = cornerA + basisBottom * wallLength;
    QVector3D cornerC = cornerB + basisTop;
    QVector3D cornerD = cornerA + basisTop;

    glVertex3f(cornerA.x(), cornerA.y(), cornerA.z());
    glVertex3f(cornerB.x(), cornerB.y(), cornerB.z());
    glVertex3f(cornerC.x(), cornerC.y(), cornerC.z());
    glVertex3f(cornerD.x(), cornerD.y(), cornerD.z());
}

void MazeView::drawMazeOverlay(QPainter &painter)
{
    QPen penHText(QColor("#00ff00"));
    painter.setPen(penHText);
    //painter.setBrush(Qt::white);

    QMatrix prevMatrix = painter.matrix();

    QMatrix flipMatrix;
    flipMatrix.translate(0, height());
    flipMatrix.scale(1,-1);
    painter.setMatrix(flipMatrix);

    painter.fillRect(0, 0, 250, 250, Qt::gray);

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
