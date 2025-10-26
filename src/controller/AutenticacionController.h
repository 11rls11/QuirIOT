#ifndef AUTENTICACIONCONTROLLER_H
#define AUTENTICACIONCONTROLLER_H

#include "../domain/usuario/AutenticacionService.h"
#include "../domain/usuario/DatosAutenticacion.h"

class AutenticacionController {
public:
    explicit AutenticacionController(AutenticacionService& authService);

    RespuestaAutenticacion login(const QString& email, const QString& password);
    void logout();
    bool estaAutenticado() const;
    int getUsuarioIdActual() const;
    QString getNombreUsuarioActual() const;

private:
    AutenticacionService& autenticacionService;
    bool autenticado;
    int usuarioIdActual;
    QString nombreUsuarioActual;
    QString tokenActual;
};

#endif
