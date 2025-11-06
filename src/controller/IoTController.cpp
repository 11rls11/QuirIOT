#include "IoTController.h"
#include "../infra/security/SecurityManager.h"
#include "../domain/usuario/UsuarioRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

IoTController::IoTController(SensorRepository& sensorRepo, ActuadorRepository& actuadorRepo,
                             QSqlDatabase& db)
    : sensorRepository(sensorRepo), actuadorRepository(actuadorRepo), database(db) {
    qInfo() << "[IoTController] Inicializado";
}

ResultadoOperacionSistema IoTController::activarSistemaLimpieza(
    int idQuirofano,
    int idUsuario,
    const QString& password)
{
    ResultadoOperacionSistema resultado;
    resultado.exito = false;
    resultado.accion = "ACTIVAR";

    qInfo() << "Solicitud de activacion del sistema de limpieza";
    qInfo() << "       Quirofano:" << idQuirofano << "Usuario:" << idUsuario;

    // Validar contraseña
    if (!validarPassword(idUsuario, password)) {
        resultado.mensaje = "Contrasena incorrecta. Operacion cancelada.";
        qWarning() << "Contrasena incorrecta para usuario" << idUsuario;
        return resultado;
    }

    // Verificar si ya está activo
    if (consultarEstadoSistemaLimpieza(idQuirofano)) {
        resultado.mensaje = "El sistema de limpieza ya esta activo en este quirofano";
        qInfo() << "Sistema ya estaba activo";
        return resultado;
    }

    // Activar sistema
    if (!actualizarEstadoSistema(idQuirofano, true)) {
        resultado.mensaje = "Error al activar el sistema en la base de datos";
        qCritical() << "Error al actualizar estado del sistema";
        return resultado;
    }

    // Registrar acción en historial
    if (!registrarAccionSistema(idQuirofano, idUsuario, "ACTIVAR",
                                "Activacion manual del sistema")) {
        qWarning() << "No se pudo registrar en historial";
    }

    // Obtener nombre del usuario para el mensaje
    UsuarioRepository usuarioRepo(database);
    auto usuario = usuarioRepo.buscarPorId(idUsuario);
    resultado.nombreUsuario = usuario ? usuario->getNombre() : "Desconocido";
    delete usuario;

    resultado.exito = true;
    resultado.mensaje = "Sistema de limpieza ACTIVADO exitosamente";

    qInfo() << "Sistema de limpieza activado en quirofano" << idQuirofano;
    qInfo() << "          Por:" << resultado.nombreUsuario;

    return resultado;
}

ResultadoOperacionSistema IoTController::desactivarSistemaLimpieza(
    int idQuirofano,
    int idUsuario,
    const QString& password,
    const QString& razon)
{
    ResultadoOperacionSistema resultado;
    resultado.exito = false;
    resultado.accion = "DESACTIVAR";

    qInfo() << "Solicitud de desactivacion del sistema de limpieza";
    qInfo() << "       Quirofano:" << idQuirofano << "Usuario:" << idUsuario;
    qInfo() << "       Razon:" << (razon.isEmpty() ? "No especificada" : razon);

    // Validar contraseña
    if (!validarPassword(idUsuario, password)) {
        resultado.mensaje = "Contrasena incorrecta. Operacion cancelada.";
        qWarning() << "[US 8] Contrasena incorrecta para usuario" << idUsuario;
        return resultado;
    }

    // Verificar si ya está desactivado
    if (!consultarEstadoSistemaLimpieza(idQuirofano)) {
        resultado.mensaje = "El sistema de limpieza ya esta desactivado en este quirofano";
        qInfo() << "Sistema ya estaba desactivado";
        return resultado;
    }

    // Desactivar sistema
    if (!actualizarEstadoSistema(idQuirofano, false)) {
        resultado.mensaje = "Error al desactivar el sistema en la base de datos";
        qCritical() << "Error al actualizar estado del sistema";
        return resultado;
    }

    // Registrar acción en historial
    QString razonFinal = razon.isEmpty() ? "Desactivacion manual del sistema" : razon;
    if (!registrarAccionSistema(idQuirofano, idUsuario, "DESACTIVAR", razonFinal)) {
        qWarning() << "No se pudo registrar en historial";
    }

    // Obtener nombre del usuario
    UsuarioRepository usuarioRepo(database);
    auto usuario = usuarioRepo.buscarPorId(idUsuario);
    resultado.nombreUsuario = usuario ? usuario->getNombre() : "Desconocido";
    delete usuario;

    resultado.exito = true;
    resultado.mensaje = "Sistema de limpieza DESACTIVADO exitosamente";

    qWarning() << "Sistema de limpieza desactivado en quirofano" << idQuirofano;
    qWarning() << "Por: " << resultado.nombreUsuario;
    qWarning() << "Razon: " << razonFinal;

    return resultado;
}

bool IoTController::validarPassword(int idUsuario, const QString& password) {
    UsuarioRepository usuarioRepo(database);

    // Obtener hash de la contraseña del usuario
    QSqlQuery query(database);
    query.prepare("SELECT contrasena FROM usuarios WHERE id_usuario = ?");
    query.addBindValue(idUsuario);

    if (!query.exec() || !query.next()) {
        qCritical() << "No se pudo obtener contraseña del usuario" << idUsuario;
        return false;
    }

    QString hashAlmacenado = query.value(0).toString();

    // Verificar contraseña usando SecurityManager
    SecurityManager& securityManager = SecurityManager::getInstance();
    return securityManager.verificarPassword(password, hashAlmacenado);
}

bool IoTController::registrarAccionSistema(int idQuirofano, int idUsuario,
                                           const QString& accion, const QString& razon) {
    QSqlQuery query(database);

    query.prepare(
        "INSERT INTO historial_sistema_limpieza "
        "(id_quirofano, id_usuario, accion, razon) "
        "VALUES (?, ?, ?, ?)"
        );
    query.addBindValue(idQuirofano);
    query.addBindValue(idUsuario);
    query.addBindValue(accion);
    query.addBindValue(razon);

    if (!query.exec()) {
        qCritical() << "[ERROR] No se pudo registrar accion en historial:"
                    << query.lastError().text();
        return false;
    }

    qInfo() << "[OK] Accion registrada en historial_sistema_limpieza";
    return true;
}

bool IoTController::actualizarEstadoSistema(int idQuirofano, bool activo) {
    QSqlQuery query(database);

    query.prepare(
        "UPDATE quirofanos SET sistema_limpieza_activo = ? WHERE id_quirofano = ?"
        );
    query.addBindValue(activo);
    query.addBindValue(idQuirofano);

    if (!query.exec()) {
        qCritical() << "[ERROR] No se pudo actualizar estado del sistema:"
                    << query.lastError().text();
        return false;
    }

    qInfo() << "[OK] Estado del sistema actualizado:" << (activo ? "ACTIVO" : "DESACTIVADO");
    return true;
}

bool IoTController::consultarEstadoSistemaLimpieza(int idQuirofano) {
    QSqlQuery query(database);

    query.prepare(
        "SELECT sistema_limpieza_activo FROM quirofanos WHERE id_quirofano = ?"
        );
    query.addBindValue(idQuirofano);

    if (!query.exec() || !query.next()) {
        qWarning() << "[WARN] No se pudo consultar estado del sistema para quirofano"
                   << idQuirofano;
        return true; // Por defecto asumimos activo
    }

    return query.value(0).toBool();
}

bool IoTController::obtenerEstadoQuirofano(int idQuirofano) {
    Q_UNUSED(idQuirofano);
    qInfo() << "[TODO US 3] Obtener estado del quirofano";
    return false;
}

bool IoTController::registrarAccionEmergencia(int idQuirofano) {
    Q_UNUSED(idQuirofano);
    qInfo() << "[TODO US 9] Registrar accion de emergencia";
    return false;
}
