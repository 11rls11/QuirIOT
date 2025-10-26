#include "FirebaseConfig.h"
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDebug>

FirebaseConfig::FirebaseConfig()
    : QObject(nullptr),
    networkManager(nullptr),
    autenticado(false),
    configurado(false)
{
    networkManager = new QNetworkAccessManager(this);
    cargarConfiguracionPorDefecto();
}

FirebaseConfig::~FirebaseConfig() {
    // networkManager se destruye automaticamente (es hijo de QObject)
}

FirebaseConfig& FirebaseConfig::getInstance() {
    static FirebaseConfig instance;
    return instance;
}

void FirebaseConfig::cargarConfiguracionPorDefecto() {
    apiKey = "";
    projectId = "";
    authToken = "";
    userId = "";
    configurado = false;
    autenticado = false;
}

void FirebaseConfig::cargarDesdeEnv(const QString& rutaEnv) {
    QFile envFile(rutaEnv);

    if (!envFile.exists()) {
        qWarning() << "[WARN] Archivo" << rutaEnv << "no encontrado. Firebase no configurado.";
        return;
    }

    if (!envFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[WARN] No se pudo abrir" << rutaEnv << "para Firebase";
        return;
    }

    QTextStream in(&envFile);
    qInfo() << "[INFO] Cargando configuracion de Firebase desde" << rutaEnv;

    while (!in.atEnd()) {
        QString linea = in.readLine().trimmed();

        if (linea.isEmpty() || linea.startsWith("#")) {
            continue;
        }

        int separador = linea.indexOf('=');
        if (separador == -1) continue;

        QString clave = linea.left(separador).trimmed();
        QString valor = linea.mid(separador + 1).trimmed();

        if (valor.startsWith('"') && valor.endsWith('"')) {
            valor = valor.mid(1, valor.length() - 2);
        }

        if (clave == "FIREBASE_API_KEY") {
            apiKey = valor;
        } else if (clave == "FIREBASE_PROJECT_ID") {
            projectId = valor;
        }
    }

    envFile.close();

    configurado = !apiKey.isEmpty() && !projectId.isEmpty();

    if (configurado) {
        qInfo() << "[OK] Firebase configurado correctamente";
    } else {
        qWarning() << "[WARN] Firebase no configurado (opcional para modo local)";
    }
}

QString FirebaseConfig::construirUrlAuth() const {
    return QString("https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=%1")
    .arg(apiKey);
}

QString FirebaseConfig::construirUrlFirestore(const QString& path) const {
    return QString("https://firestore.googleapis.com/v1/projects/%1/databases/(default)/documents/%2")
    .arg(projectId, path);
}

void FirebaseConfig::autenticarUsuario(const QString& email,
                                       const QString& password,
                                       std::function<void(bool, const QString&)> callback) {
    if (!configurado) {
        qWarning() << "[WARN] Firebase no configurado. Saltando autenticacion en la nube.";
        callback(false, "Firebase no configurado");
        return;
    }

    QJsonObject requestData;
    requestData["email"] = email;
    requestData["password"] = password;
    requestData["returnSecureToken"] = true;

    QUrl url(construirUrlAuth());
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument doc(requestData);
    QByteArray data = doc.toJson();

    QNetworkReply* reply = networkManager->post(request, data);

    connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(responseData);
            QJsonObject obj = doc.object();

            if (obj.contains("idToken")) {
                authToken = obj["idToken"].toString();
                userId = obj["localId"].toString();
                autenticado = true;

                qInfo() << "[OK] Autenticacion exitosa con Firebase";
                callback(true, "Autenticacion exitosa");
            } else {
                callback(false, "Respuesta invalida de Firebase");
            }
        } else {
            QString errorMsg = reply->errorString();
            qWarning() << "[ERROR] Error de autenticacion Firebase:" << errorMsg;
            callback(false, errorMsg);
        }

        reply->deleteLater();
    });
}

void FirebaseConfig::guardarDocumento(const QString& coleccion,
                                      const QString& documento,
                                      const QJsonObject& datos,
                                      std::function<void(bool, const QString&)> callback) {
    if (!configurado || !autenticado) {
        callback(false, "Firebase no configurado o no autenticado");
        return;
    }

    QString urlStr = construirUrlFirestore(coleccion + "/" + documento);
    QUrl url(urlStr);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + authToken).toUtf8());

    QJsonObject firestoreDoc = convertirAFormatoFirestore(datos);

    QJsonDocument doc(firestoreDoc);
    QByteArray data = doc.toJson();

    QNetworkReply* reply = networkManager->put(request, data);

    connect(reply, &QNetworkReply::finished, this, [reply, callback]() {
        if (reply->error() == QNetworkReply::NoError) {
            callback(true, "Documento guardado en Firebase");
        } else {
            callback(false, reply->errorString());
        }
        reply->deleteLater();
    });
}

QJsonObject FirebaseConfig::convertirAFormatoFirestore(const QJsonObject& datos) {
    QJsonObject firestoreDoc;
    QJsonObject fields;

    QJsonObject::const_iterator it;
    for (it = datos.constBegin(); it != datos.constEnd(); ++it) {
        QJsonObject field;
        QJsonValue value = it.value();

        if (value.isString()) {
            field["stringValue"] = value.toString();
        } else if (value.isDouble()) {
            field["integerValue"] = QString::number(value.toInt());
        } else if (value.isBool()) {
            field["booleanValue"] = value.toBool();
        }

        fields[it.key()] = field;
    }

    firestoreDoc["fields"] = fields;
    return firestoreDoc;
}

QString FirebaseConfig::getToken() const {
    return authToken;
}

bool FirebaseConfig::estaAutenticado() const {
    return autenticado;
}

bool FirebaseConfig::estaConfigurado() const {
    return configurado;
}
