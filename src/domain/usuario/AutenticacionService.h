#ifndef AUTENTICACIONSERVICE_H
#define AUTENTICACIONSERVICE_H

#include "Usuario.h"
#include "UsuarioRepository.h"
#include "DatosAutenticacion.h"
#include <QSqlDatabase>

class AutenticacionService {
public:
    explicit AutenticacionService(QSqlDatabase& db, UsuarioRepository& usuarioRepo);

    RespuestaAutenticacion autenticar(const DatosAutenticacion& datos);
    bool validarPassword(const QString& password, const QString& hash);

private:
    QSqlDatabase& database;
    UsuarioRepository& usuarioRepository;
    
    QString generarToken(const Usuario& usuario);
};

#endif // AUTENTICACIONSERVICE_H
