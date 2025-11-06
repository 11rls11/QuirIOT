#ifndef IOTCONTROLLER_H
#define IOTCONTROLLER_H

#include "../domain/sensor/SensorRepository.h"
#include "../domain/actuador/ActuadorRepository.h"
#include <QSqlDatabase>
#include <QString>

struct ResultadoOperacionSistema {
    bool exito;
    QString mensaje;
    QString nombreUsuario;
    QString accion;
};

class IoTController {
public:
    IoTController(SensorRepository& sensorRepo, ActuadorRepository& actuadorRepo,
                  QSqlDatabase& db);

    ResultadoOperacionSistema activarSistemaLimpieza(
        int idQuirofano,
        int idUsuario,
        const QString& password
        );

    ResultadoOperacionSistema desactivarSistemaLimpieza(
        int idQuirofano,
        int idUsuario,
        const QString& password,
        const QString& razon = ""
        );

    QSqlDatabase& obtenerDatabase() { return database; }

    bool consultarEstadoSistemaLimpieza(int idQuirofano);

    bool obtenerEstadoQuirofano(int idQuirofano);
    bool registrarAccionEmergencia(int idQuirofano);

private:
    SensorRepository& sensorRepository;
    ActuadorRepository& actuadorRepository;
    QSqlDatabase& database;

    bool validarPassword(int idUsuario, const QString& password);
    bool registrarAccionSistema(int idQuirofano, int idUsuario,
                                const QString& accion, const QString& razon);
    bool actualizarEstadoSistema(int idQuirofano, bool activo);
};

#endif // IOTCONTROLLER_H
