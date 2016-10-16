#include "console.h"

void errorOutput(QScriptValue v) {
    qDebug() << "js error: " << v.property("lineNumber").toInteger() << ":" << v.toString();
}
