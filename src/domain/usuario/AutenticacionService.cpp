#include "AutenticacionService.h"
#include "../../infra/security/SecurityManager.h"
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>

AutenticacionService::AutenticacionService(QSqlDatabase& db, UsuarioRepository& usuarioRepo)
    : database(db), usuarioRepository(usuarioRepo) {}

RespuestaAutenticacion AutenticacionService::autenticar(const DatosAutenticacion& datos) {
    RespuestaAutenticacion respuesta;
    respuesta.exito = false;

    if (!datos.esValido()) {
        respuesta.mensaje = "Email y contrasena son requeridos";
        return respuesta;
    }

    Usuario* usuario = usuarioRepository.buscarPorEmail(datos.email);
    if (!usuario) {
        respuesta.mensaje = "Credenciales invalidas";
        qWarning() << "[WARN] Intento de login con email inexistente:" << datos.email;
        return respuesta;
    }

    QString hashAlmacenado = usuarioRepository.obtenerPasswordHash(datos.email);

    SecurityManager& securityManager = SecurityManager::getInstance();
    if (hashAlmacenado.isEmpty() || !securityManager.verificarPassword(datos.password, hashAlmacenado)) {
        respuesta.mensaje = "Credenciales invalidas";
        qWarning() << "[WARN] Contrasena incorrecta para usuario:" << datos.email;
        delete usuario;
        return respuesta;
    }

    respuesta.exito = true;
    respuesta.mensaje = "Autenticacion exitosa";
    respuesta.usuarioId = usuario->getId();
    respuesta.nombreUsuario = usuario->getNombre();
    respuesta.token = generarToken(*usuario);

    qInfo() << "[OK] Usuario autenticado:" << usuario->getEmail();

    delete usuario;
    return respuesta;
}

bool AutenticacionService::validarPassword(const QString& password, const QString& hash) {
    SecurityManager& securityManager = SecurityManager::getInstance();
    return securityManager.verificarPassword(password, hash);
}

QString AutenticacionService::generarToken(const Usuario& usuario) {
    SecurityManager& securityManager = SecurityManager::getInstance();
    return securityManager.generarToken(usuario.getId(), usuario.getEmail());
}
