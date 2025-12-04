#include "IoTController.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QEventLoop>
#include <QDebug>
#include "../domain/sensor/LecturaSensor.h"

IoTController::IoTController(SensorRepository& sRepo, ActuadorRepository& aRepo, QSqlDatabase& db, QObject* parent)
    : QObject(parent),
    sensorRepo(sRepo),
    actuadorRepo(aRepo),
    database(db),
    lastTemp(0.0), lastHum(0.0), lastAire(0),
    ventiladorOn(false), ledOn(false), enMantenimiento(false)
{
    networkManager = nullptr;
    timerMuestreo = nullptr;
}

void IoTController::iniciarMonitoreo(const QString& ip, int intervaloMs) {
    if (!networkManager) {
        networkManager = new QNetworkAccessManager(this);
        connect(networkManager, &QNetworkAccessManager::finished, this, &IoTController::alRecibirDatos);
    }

    if (!timerMuestreo) {
        timerMuestreo = new QTimer(this);
        connect(timerMuestreo, &QTimer::timeout, this, &IoTController::solicitarDatos);
    }

    urlApi = "http://" + ip + "/datos";
    urlMantenimiento = "http://" + ip + "/mantenimiento";

    // qInfo() << "[IoT] Iniciando monitoreo en:" << urlApi;

    solicitarDatos();
    timerMuestreo->start(intervaloMs);
}

void IoTController::detenerMonitoreo() {
    if (timerMuestreo && timerMuestreo->isActive()) {
        timerMuestreo->stop();
    }
}

void IoTController::forzarLectura() {
    solicitarDatos();
}

void IoTController::solicitarDatos() {
    if (urlApi.isEmpty() || !networkManager) return;
    QNetworkRequest request((QUrl(urlApi)));
    networkManager->get(request);
}

void IoTController::alRecibirDatos(QNetworkReply* reply) {
    QString urlString = reply->url().toString();
    if (!urlString.endsWith("/datos")) {
        reply->deleteLater();
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        reply->deleteLater();
        return;
    }

    if (doc.isObject()) {
        QJsonObject obj = doc.object();

        lastTemp = obj["temperatura"].toDouble();
        lastHum = obj["humedad"].toDouble();
        lastAire = obj["calidadAirePPM"].toInt();
        ventiladorOn = obj["ventiladorEncendido"].toBool();
        ledOn = obj["ledEncendido"].toBool();
        enMantenimiento = obj["enMantenimiento"].toBool();

        int idQ = 1;

        int idTemp = sensorRepo.obtenerIdSensor(idQ, "TEMPERATURA");
        if(idTemp > 0) sensorRepo.registrarLectura(LecturaSensor(idTemp, lastTemp, "TEMPERATURA"));

        int idHum = sensorRepo.obtenerIdSensor(idQ, "HUMEDAD");
        if(idHum > 0) sensorRepo.registrarLectura(LecturaSensor(idHum, lastHum, "HUMEDAD"));

        int idAire = sensorRepo.obtenerIdSensor(idQ, "CALIDAD_AIRE");
        if(idAire > 0) sensorRepo.registrarLectura(LecturaSensor(idAire, lastAire, "CALIDAD_AIRE"));

        //qDebug() << "--- DEBUG TRIGGER ---";
        //qDebug() << "Temp Leida:" << lastTemp << " (Rango 18-26)";
        //qDebug() << "Hum Leida:" << lastHum << " (Rango 45-55)";

        bool tempEnRango = (lastTemp >= 18.0 && lastTemp <= 26.0);
        bool humEnRango  = (lastHum >= 45.0 && lastHum <= 55.0);

        bool zonaSegura  = (tempEnRango || humEnRango);

        //qDebug() << "Temp OK?" << tempEnRango;
        //qDebug() << "Hum OK?" << humEnRango;
        //qDebug() << "Zona Segura?" << zonaSegura;

        QString cmdVentilador = zonaSegura ? "off" : "on";

        bool alertaPPM = (lastAire > 400);
        QString cmdLed = alertaPPM ? "on" : "off";

        //qDebug() << "Comando a enviar -> Ventilador:" << cmdVentilador << " LED:" << cmdLed;

        QString urlTrigger = urlApi;
        urlTrigger.replace("/datos", QString("/control?ventilador=%1&led=%2").arg(cmdVentilador).arg(cmdLed));

        //qDebug() << "URL Generada:" << urlTrigger;

        QNetworkRequest requestTrigger((QUrl(urlTrigger)));
        networkManager->get(requestTrigger);
    }

    reply->deleteLater();
}

QString IoTController::getResumenEstado() const {
    if (urlApi.isEmpty()) return "Monitor no iniciado.";

    return QString(
               "--- ESTADO EN TIEMPO REAL ---\n"
               "Temperatura:  %1 °C\n"
               "Humedad:      %2 %\n"
               "Calidad Aire: %3 PPM\n"
               "Ventilador:   %4\n"
               "LED Alerta:   %5\n"
               "Limpieza:     %6\n"
               "-----------------------------"
               ).arg(lastTemp).arg(lastHum).arg(lastAire)
        .arg(ventiladorOn ? "ENCENDIDO (ON)" : "APAGADO (OFF)")
        .arg(ledOn ? "ENCENDIDO (ON)" : "APAGADO (OFF)")
        .arg(enMantenimiento ? "EN PROCESO" : "INACTIVO");
}

QSqlDatabase& IoTController::obtenerDatabase() {
    return database;
}

bool IoTController::consultarEstadoSistemaLimpieza(int idQuirofano) {
    Q_UNUSED(idQuirofano);
    return enMantenimiento;
}

ResultadoAccion IoTController::activarSistemaLimpieza(int idQuirofano, int idUsuario, const QString& password) {
    Q_UNUSED(idQuirofano);
    Q_UNUSED(idUsuario);
    Q_UNUSED(password);

    ResultadoAccion resultado;
    resultado.exito = false;

    if (urlMantenimiento.isEmpty()) {
        resultado.mensaje = "Error: No hay conexión con el Hardware (IP desconocida).";
        return resultado;
    }

    QNetworkRequest request((QUrl(urlMantenimiento)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QEventLoop loop;
    QNetworkReply* reply = networkManager->post(request, "{}");

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    loop.exec();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray resp = reply->readAll();

        if (resp.contains("OK") || resp.contains("true")) {
            resultado.exito = true;
            resultado.mensaje = "Sistema de limpieza ACTIVADO (30 segundos).";
            enMantenimiento = true;

            QTimer::singleShot(500, this, &IoTController::forzarLectura);
        } else {
            resultado.mensaje = "Hardware rechazó la orden (posiblemente ya activo).";
        }
    } else {
        resultado.mensaje = "Fallo de red: " + reply->errorString();
    }

    reply->deleteLater();
    return resultado;
}

ResultadoAccion IoTController::desactivarSistemaLimpieza(int idQuirofano, int idUsuario, const QString& password, const QString& razon) {
    Q_UNUSED(idQuirofano);
    Q_UNUSED(idUsuario);
    Q_UNUSED(password);
    Q_UNUSED(razon);

    ResultadoAccion res;
    res.exito = false;
    res.mensaje = "El sistema de limpieza es automatico y dura 30s. No se puede cancelar manualmente.";
    return res;
}
