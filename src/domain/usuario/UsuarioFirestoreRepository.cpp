#include "UsuarioFirestoreRepository.h"
#include <QDebug>

UsuarioFirestoreRepository::UsuarioFirestoreRepository()
    : firebase(FirebaseConfig::getInstance()) {}

void UsuarioFirestoreRepository::guardar(const Usuario& usuario, std::function<void(bool, QString)> callback) {
    QJsonObject json;
    json["nombre"] = usuario.getNombre();
    json["email"] = usuario.getEmail();

    QString docId = usuario.getEmail();

    firebase.guardarDocumento("usuarios", docId, json, 
        [callback](bool exito, const QJsonValue& /*unused*/, const QString& msg) {
            callback(exito, msg);
    });
}

void UsuarioFirestoreRepository::buscarPorEmail(const QString& email, std::function<void(Usuario*, QString)> callback) {
    QString docId = email;

    firebase.leerDocumento("usuarios", docId, 
        [callback](bool exito, const QJsonValue& data, const QString& msg) {
            if (!exito) {
                callback(nullptr, msg);
                return;
            }

            QJsonObject json = data.toObject();
            Usuario* u = new Usuario();
            if(json.contains("nombre")) u->setNombre(json["nombre"].toString());
            if(json.contains("email")) u->setEmail(json["email"].toString());
            
            callback(u, "Encontrado");
    });
}

void UsuarioFirestoreRepository::eliminar(const QString& email, std::function<void(bool, QString)> callback) {
    QString docId = email;
    
    firebase.eliminarDocumento("usuarios", docId, 
        [callback](bool exito, const QJsonValue& /*unused*/, const QString& msg) {
            callback(exito, msg);
    });
}
