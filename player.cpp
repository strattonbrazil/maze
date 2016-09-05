#include "player.h"

#include <math.h>
#include <algorithm>
#include <iostream>

const float MAX_FORWARD_VELOCITY = 5.0f;
const float MAX_BACKWARD_VELOCITY = -2.0f;
const float MAX_STRAFE_VELOCITY = 2.0f;
const float ACCELERATION = 10.2f; // m/s^2
const float TURN_SPEED = 0.8f; // radians/s

Player::Player()
{
    _zRot = 0;
    forwardVelocity = 0;
    strafeVelocity = 0;
    _pos = QVector3D(.5f,.5f,0.2f);
}

QVector3D Player::lookDir()
{
    return QVector3D(cos(_zRot), sin(_zRot), 0).normalized();
}

void Player::turnLeft(int elapsed)
{
    float seconds = elapsed * 0.001f;

    _zRot += TURN_SPEED * seconds;
}

void Player::turnRight(int elapsed)
{
    float seconds = elapsed * 0.001f;

    _zRot -= TURN_SPEED * seconds;
}

void Player::speedForward(int elapsed)
{
    float seconds = elapsed * 0.001f;

    forwardVelocity = std::min(MAX_FORWARD_VELOCITY, forwardVelocity + seconds * ACCELERATION);
}

void Player::speedBack(int elapsed)
{
    float seconds = elapsed * 0.001f;

    forwardVelocity = std::max(MAX_BACKWARD_VELOCITY, forwardVelocity - seconds * ACCELERATION);
}

void Player::speedSideways(int elapsed, int dir)
{
    float seconds = elapsed * 0.001f;

    if (dir == STRAFE_LEFT) {
        strafeVelocity = std::max(-MAX_STRAFE_VELOCITY, strafeVelocity - seconds * ACCELERATION);
    } else {
        strafeVelocity = std::min(MAX_STRAFE_VELOCITY, strafeVelocity + seconds * ACCELERATION);
    }
}

void Player::slowDown(int elapsed)
{
    float seconds = elapsed * 0.001f;

    if (forwardVelocity > 0)
        forwardVelocity = std::max(0.0f, forwardVelocity - seconds * ACCELERATION);
    else
        forwardVelocity = std::min(0.0f, forwardVelocity + seconds * ACCELERATION);
}

void Player::sidewaysDown(int elapsed)
{
    float seconds = elapsed * 0.001f;

    if (strafeVelocity > 0)
        strafeVelocity = std::max(0.0f, strafeVelocity - seconds * ACCELERATION);
    else
        strafeVelocity = std::min(0.0f, strafeVelocity + seconds * ACCELERATION);
}

void Player::update(int elapsed)
{
    float seconds = elapsed * 0.001f;

    QVector3D look = lookDir();
    QVector3D sideDir = QVector3D::crossProduct(look, QVector3D(0,0,1));

    QVector3D move(look.x() * seconds * forwardVelocity,
                   look.y() * seconds * forwardVelocity,
                   look.z() * seconds * forwardVelocity);

    // add strafing
    move += sideDir * strafeVelocity * seconds;

    _pos = _pos + move;
}


