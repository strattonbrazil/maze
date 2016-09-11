#include "mazeview.h"

#include <QMatrix4x4>
#include <QKeyEvent>
#include <QCache>

#include <math.h>

#include <iostream>

b2Vec2 dir(float angle)
{
    return b2Vec2(cos(angle), sin(angle));
}

MazeView::MazeView(QWidget *parent) : QGLWidget(parent), lastTime(0)
{
    maze = new Maze(10, 10);

    setFocusPolicy(Qt::ClickFocus);

    updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(update()));
    updateTimer->setInterval(10);
    updateTimer->start();

    elapsedTimer.start();

    // http://www.bulletphysics.org/mediawiki-1.5.8/index.php/Hello_World
    broadphase = new btDbvtBroadphase();
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    solver = new btSequentialImpulseConstraintSolver;
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0, 0, -10));
    groundShape = new btStaticPlaneShape(btVector3(0, 0, 1), 1);
    fallShape = new btSphereShape(0.3f);

    groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, -1)));
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
    groundRigidBody = new btRigidBody(groundRigidBodyCI);
    dynamicsWorld->addRigidBody(groundRigidBody);

    fallMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 50)));
    btScalar mass = 1;
    btVector3 fallInertia(0, 0, 0);
    fallShape->calculateLocalInertia(mass, fallInertia);
    btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass, fallMotionState, fallShape, fallInertia);
    fallRigidBody = new btRigidBody(fallRigidBodyCI);
    dynamicsWorld->addRigidBody(fallRigidBody);

    world = new b2World(b2Vec2(0.0f, 0.0f)); // no gravity

    // create the ground
    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(0.0f, -10.0f);
    groundBody = world->CreateBody(&groundBodyDef);
    b2PolygonShape groundBox;
    groundBox.SetAsBox(50.0f, 10.0f);
    groundBody->CreateFixture(&groundBox, 0.0f);

    // create the maze body
    b2BodyDef mazeBodyDef;
    mazeBody = world->CreateBody(&mazeBodyDef);
    for (int row = 0; row < maze->height(); row++) {
        for (int column = 0; column < maze->width(); column++) {
            Cell cell = maze->cell(column, row);
            if (cell.up) {
                b2Vec2 v1(column, row+1);
                b2Vec2 v2(column+1, row+1);
                b2EdgeShape edge;
                edge.Set(v1, v2);

                mazeBody->CreateFixture(&edge, 0.0f);
            }
            if (cell.left) {
                b2Vec2 v1(column, row);
                b2Vec2 v2(column, row+1);
                b2EdgeShape edge;
                edge.Set(v1, v2);

                mazeBody->CreateFixture(&edge, 0.0f);
            }
        }
    }

    // create the body
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody; // can move
    bodyDef.position.Set(0.0f, 40.0f);
    body = world->CreateBody(&bodyDef);
    b2PolygonShape dynamicBox;
    dynamicBox.SetAsBox(1.0f, 1.0f);
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &dynamicBox; // is this a bug waiting to happen?
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    body->CreateFixture(&fixtureDef);

    // create the player
    b2BodyDef playerDef;
    playerDef.type = b2_dynamicBody;
    playerDef.position.Set(0.5f, 0.5f);
    playerBody = world->CreateBody(&playerDef);
    b2CircleShape circle;
    circle.m_radius = PLAYER_RADIUS;
    b2FixtureDef playerFixtureDef;
    playerFixtureDef.shape = &circle; // is this a bug waiting to happen?
    playerFixtureDef.density = 1.0f;
    playerFixtureDef.friction = 0.0f;
    playerBody->CreateFixture(&playerFixtureDef);
    playerBody->SetLinearVelocity(b2Vec2(0,0));


    playerLeft = false;
    playerRight = false;
    playerForward = false;
    playerBack = false;
    playerStrafeLeft = false;
    playerStrafeRight = false;
}

MazeView::~MazeView()
{
    delete dynamicsWorld;
    delete solver;
    delete dispatcher;
    delete collisionConfiguration;
    delete broadphase;
    delete groundShape;
    delete fallShape;
    delete groundMotionState;
    delete groundRigidBody;

    world->DestroyBody(groundBody);
    delete world;
}

void MazeView::initializeGL()
{
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

}

void MazeView::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void MazeView::paintGL()
{
    int newTime = elapsedTimer.elapsed();
    int elapsed = newTime - lastTime;
    lastTime = newTime;

    float elapsedSeconds = elapsed * 0.001f;
    dynamicsWorld->stepSimulation(elapsedSeconds, 10);
    btTransform trans;
    fallRigidBody->getMotionState()->getWorldTransform(trans);

    world->Step(elapsedSeconds, 6, 2);
    b2Vec2 position = body->GetPosition();

    float currentAngle = playerBody->GetAngle();

    // TODO: make an update function
    b2Vec2 lookDir = dir(currentAngle);
    if (playerForward) {
        b2Vec2 v = playerBody->GetLinearVelocity();
        b2Vec2 newV = ACCELERATION * elapsedSeconds * lookDir + v;
        float l = std::min(MAX_FORWARD_VELOCITY, newV.Length());
        newV.Normalize();
        newV *= l;
        playerBody->SetLinearVelocity(newV);
    } else if (playerBack) {
        b2Vec2 v = playerBody->GetLinearVelocity();
        b2Vec2 newV = -ACCELERATION * elapsedSeconds * lookDir + v;
        float l = v.Length();
        newV.Normalize();
        newV *= l;
        playerBody->SetLinearVelocity(newV);
    } else {
        b2Vec2 v = playerBody->GetLinearVelocity();

        // adjust velocity to be where player is facing
        float l = v.Length();
        if (l > 0) {
            l -= elapsedSeconds * ACCELERATION;
            l = std::max(0.0f, l);
        } else {
            l += elapsedSeconds * ACCELERATION;
            l = std::min(0.0f, l);
        }
        v.Normalize();
        v *= l;
        playerBody->SetLinearVelocity(v);
    }

    /*
    if (playerStrafeLeft) {
        player.speedSideways(elapsed, STRAFE_LEFT);
    } else if (playerStrafeRight) {
        player.speedSideways(elapsed, STRAFE_RIGHT);
    } else {
        player.sidewaysDown(elapsed);
    }

    */
    if (playerLeft && !playerRight) {
        float v = playerBody->GetAngularVelocity();
        v += elapsedSeconds * TURN_ACCELERATION;
        v = std::min(MAX_TURN_VELOCITY, v);
        playerBody->SetAngularVelocity(v);
    } else if (playerRight && !playerLeft) {
        float v = playerBody->GetAngularVelocity();
        v -= elapsedSeconds * TURN_ACCELERATION;
        v = std::max(-MAX_TURN_VELOCITY, v);
        playerBody->SetAngularVelocity(v);
    } else { // slow down
        float v = playerBody->GetAngularVelocity();
        if (v > 0) {
            v -= elapsedSeconds * TURN_ACCELERATION;
            v = std::max(0.0f, v);
            playerBody->SetAngularVelocity(v);
        } else {
            v += elapsedSeconds * TURN_ACCELERATION;
            v = std::min(0.0f, v);
            playerBody->SetAngularVelocity(v);
        }
    }


    QPainter painter(this);

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
    QVector3D playerPos((float)(playerBody->GetPosition().x), (float)(playerBody->GetPosition().y), 0.5f);
    camera.lookAt(playerPos,
                  playerPos + QVector3D((float)(lookDir.x), (float)(lookDir.y), 0.0f),
                  QVector3D(0, 0, 1));
    /*
    camera.lookAt(QVector3D(maze->width() / 2, maze->height() / 2, 30),
                  QVector3D(maze->width() / 2, maze->height() / 2, 0),
                  QVector3D(0,1,0));
                  */

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
                }

                glColor3f(0,1,1); // cyan
                if (cell.right) {
                    drawWall(QPoint(x+1,y+1), QVector2D(0,-1), c3.down, c3.left, c5.up, c5.down, c8.right, c9.up);
                }
            }
        }

        float x = position.x;
        float y = position.y;
        glColor3f(1,1,1);
        glVertex2f(x-1, y);
        glVertex2f(x, y);
        glVertex2f(x, y+1);
        glVertex2f(x-1, y+1);
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

    // draw player
    glColor3f(1,1,1);
    b2Vec2 playerP = playerBody->GetPosition();
    glTranslatef(playerP.x, playerP.y, 0);
    glBegin(GL_TRIANGLE_FAN);
    {
        const int NUM_SIDES = 32;
        for (int i = 0; i <= NUM_SIDES; i++) {
            float angle = 2.0f * M_PI * i / NUM_SIDES;
            float x = PLAYER_RADIUS * cos(angle);
            float y = PLAYER_RADIUS * sin(angle);
            glVertex2f(x, y);
        }
    }
    glEnd();
    float x = PLAYER_RADIUS * cos(currentAngle);
    float y = PLAYER_RADIUS * sin(currentAngle);
    glTranslatef(x, y, 0);
    glColor3f(0,0,255);
    glBegin(GL_QUADS);
    {
        glVertex2f(-0.1,-0.1);
        glVertex2f(0.1,-0.1);
        glVertex2f(0.1,0.1);
        glVertex2f(-0.1,0.1);
    }
    glEnd();
    glTranslatef(-x, -y, 0);
    glTranslatef(-playerP.x, -playerP.y, 0);

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
    b2Vec2 p = playerBody->GetPosition();
    QVector3D playerPos(p.x, p.y, 0);
    painter.drawRect(20*playerPos.x() - 1 + 20, 20*playerPos.y() - 1 + 20, 2, 2);

    painter.setMatrix(prevMatrix);
}
