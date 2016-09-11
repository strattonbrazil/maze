#ifndef PLAYER_H
#define PLAYER_H

#include <QVector3D>

enum { STRAFE_LEFT, STRAFE_RIGHT };

const float MAX_FORWARD_VELOCITY = 5.0f;
const float MAX_BACKWARD_VELOCITY = -2.0f;
const float MAX_STRAFE_VELOCITY = 2.0f;
const float ACCELERATION = 10.2f; // m/s^2
const float TURN_ACCELERATION = 20.8f; // radians/s^2
const float MAX_TURN_VELOCITY = 4.0f; // radians/s



/*
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
*/

#endif // PLAYER_H
