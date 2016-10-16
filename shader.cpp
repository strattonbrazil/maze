#include "shader.h"

const char* wallVertexShader = \
"varying vec4 color;\n" \
"void main()\n" \
"{\n" \
"  color = gl_Color * gl_Vertex.z;\n" \
"  gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;" \
"}\n";

const char* wallFragShader = \
"varying vec4 color;\n" \
"void main(void)\n" \
"{\n" \
"  gl_FragColor = color * (1-gl_FragCoord.z);\n" \
"}\n";

QGLShaderProgram* ShaderFactory::wallShader(QGLContext * context)
{
    QGLShader* vertShader = new QGLShader(QGLShader::Vertex);
    vertShader->compileSourceCode(wallVertexShader);

    QGLShader* fragShader = new QGLShader(QGLShader::Fragment);
    fragShader->compileSourceCode(wallFragShader);

    QGLShaderProgram* program = new QGLShaderProgram(context);
    program->addShader(vertShader);
    program->addShader(fragShader);

    program->link();
    program->bind();

    return program;
}

