#include "mazeview.h"
#include "shader.h"

#include <QMatrix4x4>
#include <QKeyEvent>
#include <QCache>


#include <math.h>

#include <iostream>

const float WALL_HEIGHT = 2.0f;
const float CELL_WIDTH = 2.0f;

b2Vec2 dir(float angle)
{
    return b2Vec2(cos(angle), sin(angle));
}

QVector3D to3D(b2Vec2 v) {
    return QVector3D((float)(v.x), (float)(v.y), 0.0f);
}

MazeView::MazeView(QWidget *parent) : QGLWidget(parent), lastTime(0)
{
    maze = new Maze(10, 10);

    setFocusPolicy(Qt::ClickFocus);
    setMouseTracking(true);

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
                b2Vec2 v1(CELL_WIDTH * column, CELL_WIDTH * (row+1));
                b2Vec2 v2(CELL_WIDTH * (column+1), CELL_WIDTH * (row+1));
                b2EdgeShape edge;
                edge.Set(v1, v2);

                mazeBody->CreateFixture(&edge, 0.0f);
            }
            if (cell.left) {
                b2Vec2 v1(CELL_WIDTH * column, CELL_WIDTH * row);
                b2Vec2 v2(CELL_WIDTH * column, CELL_WIDTH * (row+1));
                b2EdgeShape edge;
                edge.Set(v1, v2);

                mazeBody->CreateFixture(&edge, 0.0f);
            }
            if (cell.right && column == maze->width() - 1) {
                b2Vec2 v1(CELL_WIDTH * (column+1), CELL_WIDTH * row);
                b2Vec2 v2(CELL_WIDTH * (column+1), CELL_WIDTH * (row+1));
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
    upDownAngle = 0.0f;
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

    wallShader = ShaderFactory::wallShader(context());
}

void MazeView::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);

    update();
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
        float l = std::min(MAX_BACKWARD_VELOCITY, newV.Length());
        newV.Normalize();
        newV *= l;
        playerBody->SetLinearVelocity(newV);
    } else { // slow down
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

    if (playerStrafeLeft) {
        QVector3D lookDir3D = to3D(lookDir);
        QVector3D leftDir = QVector3D::crossProduct(lookDir3D, QVector3D(0,0,-1));
        leftDir.normalize();

        b2Vec2 v = playerBody->GetLinearVelocity();
        float currentLeftV = QVector3D::dotProduct(leftDir, to3D(v)) / v.Length();
        if (v.Length() < 0.001f || currentLeftV < MAX_STRAFE_VELOCITY) { // can strafe left
            b2Vec2 newV = ACCELERATION * elapsedSeconds * b2Vec2(leftDir.x(), leftDir.y()) + v;
            playerBody->SetLinearVelocity(newV);
        }
    } else if (playerStrafeRight) {
        QVector3D lookDir3D = to3D(lookDir);
        QVector3D rightDir = QVector3D::crossProduct(lookDir3D, QVector3D(0,0,1));
        rightDir.normalize();

        b2Vec2 v = playerBody->GetLinearVelocity();
        float currentRightV = QVector3D::dotProduct(rightDir, to3D(v)) / v.Length();
        if (v.Length() < 0.001f || currentRightV < MAX_STRAFE_VELOCITY) { // can strafe right
            b2Vec2 newV = ACCELERATION * elapsedSeconds * b2Vec2(rightDir.x(), rightDir.y()) + v;
            playerBody->SetLinearVelocity(newV);
        }
    } else {
        //player.sidewaysDown(elapsed);
    }

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

    if (lastMouseDiff.x() != 0) {
        float newAngle = playerBody->GetAngle() + lastMouseDiff.x() * -0.0005f;
        playerBody->SetTransform(playerBody->GetPosition(), newAngle);
        //std::cout << newAngle << std::endl;
    }
    if (lastMouseDiff.y() != 0) {
        upDownAngle += lastMouseDiff.y() * 0.0005f;
    }
    lastMouseDiff = QPoint(0,0);

    QPainter painter(this);

    painter.beginNativePainting();

    glEnable(GL_DEPTH_TEST);

    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float aspect = width() / (float)height();
    QMatrix4x4 proj;
    proj.perspective(45, aspect, 0.2, 100);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glLoadMatrixf(proj.data());

    QMatrix4x4 camera;

#if 0
    QVector3D playerPos((float)(playerBody->GetPosition().x), (float)(playerBody->GetPosition().y), 1.0f);
    QVector3D lookDir3D = QVector3D((float)(lookDir.x), (float)(lookDir.y), tan(upDownAngle));

    camera.lookAt(playerPos,
                  playerPos + lookDir3D,
                  QVector3D(0, 0, 1));
#else
    camera.lookAt(QVector3D(CELL_WIDTH * maze->width() / 2, CELL_WIDTH * maze->height() / 2, 30),
                  QVector3D(CELL_WIDTH * maze->width() / 2, CELL_WIDTH * maze->height() / 2, 0),
                  QVector3D(0,1,0));
#endif

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glLoadMatrixf(camera.data());

    wallShader->bind();


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
                    drawWall(QPoint(CELL_WIDTH*x, CELL_WIDTH*(y+1)), QVector2D(1,0), c1.right, c1.down, c4.right, c5.right, c6.up, c3.left);

                glColor3f(1,1,0); // yellow
                if (cell.down) {
                    drawWall(QPoint(CELL_WIDTH*(x+1), CELL_WIDTH*y), QVector2D(-1,0), c8.right, c6.down, c5.right, c4.right, c4.down, c7.right);
                }

                glColor3f(1,0,1); // purple
                if (cell.left) {
                    drawWall(QPoint(CELL_WIDTH*x,CELL_WIDTH*y), QVector2D(0,1), c7.up, c7.right, c5.down, c5.up, c2.left, c1.down);
                }

                glColor3f(0,1,1); // cyan
                if (cell.right) {
                    drawWall(QPoint(CELL_WIDTH*(x+1),CELL_WIDTH*(y+1)), QVector2D(0,-1), c3.down, c3.left, c5.up, c5.down, c8.right, c9.up);
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

    wallShader->release();

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

void MazeView::mousePressEvent(QMouseEvent *event)
{
    grabMouse();
    setCursor(Qt::BlankCursor);
}

void MazeView::mouseMoveEvent(QMouseEvent *event)
{
    if (cursor().shape() == Qt::BlankCursor) {
        lastMouseDiff = lastMouseP - event->pos();
        lastMouseP = event->pos();

        QPoint centerOfScreen = mapToGlobal(QPoint(width() / 2, height() / 2));
        QCursor::setPos(centerOfScreen);
    }
}


void MazeView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        releaseMouse();
        setCursor(Qt::ArrowCursor);
    }

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
    QVector3D basisIn = -1 * basisOut;

    float wallLength = CELL_WIDTH; //1.0f;
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
    QVector3D cornerC = cornerB + basisTop*WALL_HEIGHT;
    QVector3D cornerD = cornerA + basisTop*WALL_HEIGHT;

    // draw wall
    glVertex3f(cornerA.x(), cornerA.y(), cornerA.z());
    glVertex3f(cornerB.x(), cornerB.y(), cornerB.z());
    glVertex3f(cornerC.x(), cornerC.y(), cornerC.z());
    glVertex3f(cornerD.x(), cornerD.y(), cornerD.z());

    // draw caps
    if (!w1 && !w2) {
        QVector3D in = basisIn * OFFSET;
        QVector3D top = cornerD + in;
        QVector3D bottom = cornerA + in;

        glVertex3f(cornerA.x(), cornerA.y(), cornerA.z());
        glVertex3f(cornerD.x(), cornerD.y(), cornerD.z());
        glVertex3f(top.x(), top.y(), top.z());
        glVertex3f(bottom.x(), bottom.y(), bottom.z());
    }
    if (!w5 && !w6) {
        QVector3D in = basisIn * OFFSET;
        QVector3D top = cornerC + in;
        QVector3D bottom = cornerB + in;

        glVertex3f(cornerB.x(), cornerB.y(), cornerB.z());
        glVertex3f(bottom.x(), bottom.y(), bottom.z());
        glVertex3f(top.x(), top.y(), top.z());
        glVertex3f(cornerC.x(), cornerC.y(), cornerC.z());
    }
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
    QVector3D playerPos(p.x / CELL_WIDTH, p.y / CELL_WIDTH, 0);
    painter.drawRect(20*playerPos.x() - 1 + 20, 20*playerPos.y() - 1 + 20, 2, 2);

    painter.setMatrix(prevMatrix);
}
