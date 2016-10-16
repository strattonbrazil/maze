#ifndef CONSOLE_H
#define CONSOLE_H

#include <QObject>
#include <QDebug>
#include <QScriptValue>

#include <iostream>

class JSConsole : public QObject
{
    Q_OBJECT
public:
    JSConsole(QObject *parent = 0) : QObject(parent) {}

signals:

public slots:
    void log(QString msg) {
        qDebug() << "jsConsole: "<< msg;
    }
};

void errorOutput(QScriptValue v);

#endif // CONSOLE_H

