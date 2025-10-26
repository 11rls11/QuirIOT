#ifndef IOTCONTROLLER_H
#define IOTCONTROLLER_H

#include "../domain/sensor/Sensor.h"
#include "../domain/sensor/SensorRepository.h"
#include "../domain/actuador/Actuador.h"
#include "../domain/actuador/ActuadorRepository.h"
#include <QVector>

// Esqueleto preparado para User Stories 3, 8, 9, 10, 11
class IoTController {
public:
    IoTController(SensorRepository& sensorRepo, ActuadorRepository& actuadorRepo);

    // US 3: Visualizar estado del quirofano
    // TODO: Implementar en proxima iteracion
    bool obtenerEstadoQuirofano(int idQuirofano);

    // US 8: Activar/Desactivar sistema
    // TODO: Implementar control de actuadores
    bool activarSistemaLimpieza(int idQuirofano, int idUsuario, const QString& password);
    bool desactivarSistemaLimpieza(int idQuirofano, int idUsuario, const QString& password);

    // US 9: Boton fisico de emergencia
    // TODO: Implementar integracion con Raspberry Pi
    bool registrarAccionEmergencia(int idQuirofano);

    // US 10: Monitoreo en tiempo real
    // TODO: Implementar lectura de sensores
    QVector<Sensor*> listarSensoresActivos(int idQuirofano);

    // US 11: Historial de condiciones
    // TODO: Implementar consulta de mediciones historicas
    bool obtenerHistorialCondiciones(int idQuirofano, const QDateTime& inicio, const QDateTime& fin);

private:
    SensorRepository& sensorRepository;
    ActuadorRepository& actuadorRepository;
};

#endif // IOTCONTROLLER_H
