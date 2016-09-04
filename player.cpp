#include "player.h"

#include <math.h>
#include <algorithm>
#include <iostream>

const float MAX_FORWARD_VELOCITY = 5.0f;
const float MAX_BACKWARD_VELOCITY = -2.0f;
const float ACCELERATION = 10.2f; // m/s^2
const float TURN_SPEED = 0.8f; // radians/s

Player::Player()
{
    _zRot = 0;
    velocity = 0;
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

    velocity = std::min(MAX_FORWARD_VELOCITY, velocity + seconds * ACCELERATION);
}

void Player::speedBack(int elapsed)
{
    float seconds = elapsed * 0.001f;

    velocity = std::max(MAX_BACKWARD_VELOCITY, velocity - seconds * ACCELERATION);
}

void Player::slowDown(int elapsed)
{
    float seconds = elapsed * 0.001f;

    if (velocity > 0)
        velocity = std::max(0.0f, velocity - seconds * ACCELERATION);
    else
        velocity = std::min(0.0f, velocity + seconds * ACCELERATION);
}

void Player::update(int elapsed)
{
    float seconds = elapsed * 0.001f;

    QVector3D look = lookDir();
    QVector3D move(look.x() * seconds * velocity,
                   look.y() * seconds * velocity,
                   look.z() * seconds * velocity);

    _pos = _pos + move;
}


