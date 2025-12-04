#include "SensorRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QJsonObject>

SensorRepository::SensorRepository(QSqlDatabase& db) 
    : database(db), firebase(FirebaseConfig::getInstance()) {}

int SensorRepository::obtenerIdSensor(int idQuirofano, const QString& tipo) {
    QSqlQuery query(database);
    query.prepare("SELECT id_sensor FROM sensores WHERE id_quirofano = ? AND tipo_sensor = ?");
    query.addBindValue(idQuirofano);
    query.addBindValue(tipo);
    
    if(query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

void SensorRepository::registrarLectura(const LecturaSensor& lectura) {
    QSqlQuery query(database);
    query.prepare("INSERT INTO mediciones (id_sensor, valor, fecha_hora) VALUES (?, ?, ?)");
    query.addBindValue(lectura.idSensor);
    query.addBindValue(lectura.valor);

    QString fechaFormateada = lectura.timestamp.toLocalTime().toString("yyyy-MM-dd HH:mm:ss");
    query.addBindValue(fechaFormateada);

    if(!query.exec()) {
        qWarning() << "[SQL] Error guardando lectura:" << query.lastError().text();
    }

    if(firebase.estaConfigurado()) {
        QJsonObject json;
        json["id_sensor"] = lectura.idSensor;
        json["tipo"] = lectura.tipo;
        json["valor"] = lectura.valor;

        json["timestamp"] = lectura.timestamp.toString(Qt::ISODate);

        QString docId = QString("%1_%2").arg(lectura.tipo).arg(lectura.timestamp.toMSecsSinceEpoch());

        firebase.guardarDocumento("historial_sensores", docId, json, [](bool ok, const QJsonValue&, const QString& msg){
        });
    }
}
