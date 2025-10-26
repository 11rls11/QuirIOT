#include "AutenticacionController.h"
#include <QDebug>

AutenticacionController::AutenticacionController(AutenticacionService& authService)
    : autenticacionService(authService), autenticado(false), usuarioIdActual(0) {}

RespuestaAutenticacion AutenticacionController::login(const QString& email, const QString& password) {
    DatosAutenticacion datos;
    datos.email = email;
    datos.password = password;

    RespuestaAutenticacion respuesta = autenticacionService.autenticar(datos);

    if (respuesta.exito) {
        autenticado = true;
        usuarioIdActual = respuesta.usuarioId;
        nombreUsuarioActual = respuesta.nombreUsuario;
        tokenActual = respuesta.token;

        qInfo() << "[OK] Login exitoso para:" << nombreUsuarioActual;
    } else {
        qWarning() << "[WARN] Login fallido:" << respuesta.mensaje;
    }

    return respuesta;
}

void AutenticacionController::logout() {
    autenticado = false;
    usuarioIdActual = 0;
    nombreUsuarioActual.clear();
    tokenActual.clear();

    qInfo() << "[INFO] Sesion cerrada";
}

bool AutenticacionController::estaAutenticado() const {
    return autenticado;
}

int AutenticacionController::getUsuarioIdActual() const {
    return usuarioIdActual;
}

QString AutenticacionController::getNombreUsuarioActual() const {
    return nombreUsuarioActual;
}
