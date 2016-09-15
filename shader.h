#ifndef SHADERFACTORY_H
#define SHADERFACTORY_H

#include <QGLShaderProgram>

class ShaderFactory
{
public:
    static QGLShaderProgram* wallShader(QGLContext * context);
};

#endif // SHADERFACTORY_H
