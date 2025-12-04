#ifndef IOTCONTROLLER_H
#define IOTCONTROLLER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QSqlDatabase> // Necesario para obtenerDatabase()
#include "../domain/sensor/SensorRepository.h"
#include "../domain/actuador/ActuadorRepository.h"

struct ResultadoAccion {
    bool exito;
    QString mensaje;
};

class IoTController : public QObject {
    Q_OBJECT
public:
    IoTController(SensorRepository& sRepo, ActuadorRepository& aRepo, QSqlDatabase& db, QObject* parent = nullptr);
    
    void iniciarMonitoreo(const QString& ipNodeMCU, int intervaloMs = 5000);
    void detenerMonitoreo();
    void forzarLectura();
    QString getResumenEstado() const;

    bool consultarEstadoSistemaLimpieza(int idQuirofano);
    
    ResultadoAccion activarSistemaLimpieza(int idQuirofano, int idUsuario, const QString& password);
    
    ResultadoAccion desactivarSistemaLimpieza(int idQuirofano, int idUsuario, const QString& password, const QString& razon);
    
    QSqlDatabase& obtenerDatabase(); 

private slots:
    void alRecibirDatos(QNetworkReply* reply);
    void solicitarDatos();

private:
    SensorRepository& sensorRepo;
    ActuadorRepository& actuadorRepo;
    QSqlDatabase& database;
    QNetworkAccessManager* networkManager;
    QTimer* timerMuestreo;
    QString urlApi;
    QString urlMantenimiento;
    
    double lastTemp;
    double lastHum;
    int lastAire;
    bool ventiladorOn;
    bool ledOn;
    bool enMantenimiento;
};

#endif // IOTCONTROLLER_H
