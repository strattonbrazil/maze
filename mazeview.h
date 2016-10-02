#ifndef MAZEVIEW_H
#define MAZEVIEW_H

#include "maze.h"
#include "player.h"

#include <QWidget>
#include <QGLWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <QGLShaderProgram>

#include <btBulletDynamicsCommon.h>
#include <Box2D/Box2D.h>

const float PLAYER_RADIUS = 0.5f;

class MazeView : public QGLWidget
{
    Q_OBJECT
public:
    explicit MazeView(QWidget *parent = 0);
    ~MazeView();
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
signals:

public slots:
private:
    void drawWall(QPoint p, QVector2D basis, bool w1, bool w2, bool w3, bool w4, bool w5, bool w6);
    void drawMazeOverlay(QPainter &painter);
    Maze* maze;
    //Player player;
    QTimer* updateTimer;

    QElapsedTimer elapsedTimer;
    int lastTime;

    QPoint lastMouseP;
    QPoint lastMouseDiff;
    float upDownAngle;

    bool playerForward;
    bool playerBack;
    bool playerLeft;
    bool playerRight;
    bool playerStrafeLeft;
    bool playerStrafeRight;

    // bullet physics
    btBroadphaseInterface* broadphase;// = new btDbvtBroadphase();
    btDefaultCollisionConfiguration* collisionConfiguration;
    btCollisionDispatcher* dispatcher;
    btSequentialImpulseConstraintSolver* solver;
    btDiscreteDynamicsWorld* dynamicsWorld;
    btCollisionShape* groundShape;
    btCollisionShape* fallShape;
    btDefaultMotionState* groundMotionState;
    btRigidBody* groundRigidBody;
    btDefaultMotionState* fallMotionState;
    btRigidBody* fallRigidBody;

    b2World* world;
    b2Body* mazeBody;
    b2Body* groundBody;
    b2Body* body;
    b2Body* playerBody;

    QPoint goal;

    QGLShaderProgram* wallShader;
};

#endif // MAZEVIEW_H
