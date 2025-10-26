#include "ValidacionException.h"

ValidacionException::ValidacionException(const QString& mensaje) : mensaje(mensaje) {}

const char* ValidacionException::what() const noexcept {
    mensajeBytes = mensaje.toUtf8();
    return mensajeBytes.constData();
}

QString ValidacionException::getMensaje() const {
    return mensaje;
}
