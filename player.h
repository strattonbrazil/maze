#ifndef PLAYER_H
#define PLAYER_H

#include <QVector3D>

enum { STRAFE_LEFT, STRAFE_RIGHT };

class Player
{
public:
    Player();
    QVector3D lookDir();
    QVector3D pos() { return _pos; }
    void turnLeft(int elapsed);
    void turnRight(int elapsed);
    void speedForward(int elapsed);
    void speedBack(int elapsed);
    void speedSideways(int elapsed, int dir);
    void slowDown(int elapsed);
    void sidewaysDown(int elapsed);
    void update(int elapsed);
private:
    QVector3D _pos;
    float _zRot;
    float forwardVelocity;
    float strafeVelocity;
};

#endif // PLAYER_H
