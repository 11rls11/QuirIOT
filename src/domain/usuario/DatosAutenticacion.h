#ifndef DATOSAUTENTICACION_H
#define DATOSAUTENTICACION_H

#include <QString>

struct DatosAutenticacion {
    QString email;
    QString password;

    bool esValido() const {
        return !email.isEmpty() && !password.isEmpty();
    }
};

struct RespuestaAutenticacion {
    bool exito;
    QString mensaje;
    QString token;
    int usuarioId;
    QString nombreUsuario;
};

#endif
