#ifndef PLAYER_H
#define PLAYER_H

#include <QVector3D>

class Player
{
public:
    Player();
    QVector3D lookDir();
    QVector3D pos() { return _pos; }
    void speedForward(int elapsed);
    void speedBack(int elapsed);
    void slowDown(int elapsed);
    void update(int elapsed);
private:
    QVector3D _pos;
    float _zRot;
    float velocity;
};

#endif // PLAYER_H
