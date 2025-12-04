#include "ReservaFirestoreRepository.h"
#include <QDebug>

ReservaFirestoreRepository::ReservaFirestoreRepository()
    : firebase(FirebaseConfig::getInstance()) {}

void ReservaFirestoreRepository::guardar(const Reserva& reserva, std::function<void(bool, QString)> callback) {
    QJsonObject json;
    
    json["id_sql"] = reserva.getId();
    json["quirofano_id"] = reserva.getIdQuirofano();
    json["fecha"] = reserva.getFechaInicio().date().toString("yyyy-MM-dd");
    json["inicio"] = reserva.getFechaInicio().time().toString("HH:mm");
    json["fin"] = reserva.getFechaFin().time().toString("HH:mm");
    json["motivo"] = reserva.getMotivoCirugia(); 
    json["timestamp_respaldo"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QString docId = QString("reserva_%1_%2").arg(reserva.getIdQuirofano()).arg(reserva.getId());

    firebase.guardarDocumento("reservas", docId, json, [callback](bool exito, const QJsonValue&, const QString& msg) {
        if(exito) qInfo() << "[FIREBASE] Respaldo de cirugia guardado en nube.";
        else qWarning() << "[FIREBASE] Error respaldando cirugia:" << msg;
        
        callback(exito, msg);
    });
}
