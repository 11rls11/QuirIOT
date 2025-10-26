#include "IoTController.h"
#include <QDebug>

IoTController::IoTController(SensorRepository& sensorRepo, ActuadorRepository& actuadorRepo)
    : sensorRepository(sensorRepo), actuadorRepository(actuadorRepo) {
    qInfo() << "[IoTController] Inicializado - Esqueleto preparado para US 3, 8-11";
}

bool IoTController::obtenerEstadoQuirofano(int idQuirofano) {
    Q_UNUSED(idQuirofano);
    qInfo() << "[TODO US 3] Obtener estado del quirofano";
    return false;
}

bool IoTController::activarSistemaLimpieza(int idQuirofano, int idUsuario, const QString& password) {
    Q_UNUSED(idQuirofano);
    Q_UNUSED(idUsuario);
    Q_UNUSED(password);
    qInfo() << "[TODO US 8] Activar sistema de limpieza";
    return false;
}

bool IoTController::desactivarSistemaLimpieza(int idQuirofano, int idUsuario, const QString& password) {
    Q_UNUSED(idQuirofano);
    Q_UNUSED(idUsuario);
    Q_UNUSED(password);
    qInfo() << "[TODO US 8] Desactivar sistema de limpieza";
    return false;
}

bool IoTController::registrarAccionEmergencia(int idQuirofano) {
    Q_UNUSED(idQuirofano);
    qInfo() << "[TODO US 9] Registrar accion de boton fisico de emergencia";
    return false;
}

QVector<Sensor*> IoTController::listarSensoresActivos(int idQuirofano) {
    Q_UNUSED(idQuirofano);
    qInfo() << "[TODO US 10] Listar sensores activos";
    return QVector<Sensor*>();
}

bool IoTController::obtenerHistorialCondiciones(int idQuirofano, const QDateTime& inicio, const QDateTime& fin) {
    Q_UNUSED(idQuirofano);
    Q_UNUSED(inicio);
    Q_UNUSED(fin);
    qInfo() << "[TODO US 11] Obtener historial de condiciones";
    return false;
}
