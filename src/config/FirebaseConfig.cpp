#include "FirebaseConfig.h"
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QUrl>
#include <QDebug>

FirebaseConfig::FirebaseConfig()
    : QObject(nullptr),
    networkManager(nullptr),
    apiKey(""),
    projectId(""),
    authToken(""),
    userId(""),
    configurado(false),
    autenticado(false)
{
    networkManager = new QNetworkAccessManager(this);
    cargarConfiguracionPorDefecto();
}

FirebaseConfig::~FirebaseConfig() {}

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
        qWarning() << "[WARN] Archivo" << rutaEnv << "no encontrado.";
        return;
    }
    if (!envFile.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&envFile);
    while (!in.atEnd()) {
        QString linea = in.readLine().trimmed();
        if (linea.isEmpty() || linea.startsWith("#")) continue;
        int sep = linea.indexOf('=');
        if (sep == -1) continue;
        QString key = linea.left(sep).trimmed();
        QString val = linea.mid(sep + 1).trimmed();
        if (val.startsWith('"') && val.endsWith('"')) val = val.mid(1, val.length() - 2);
        
        if (key == "FIREBASE_API_KEY") apiKey = val;
        else if (key == "FIREBASE_PROJECT_ID") projectId = val;
    }
    envFile.close();
    configurado = !apiKey.isEmpty() && !projectId.isEmpty();
}

QString FirebaseConfig::construirUrlAuth() const {
    return QString("https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=%1").arg(apiKey);
}

QString FirebaseConfig::construirUrlFirestore(const QString& path, const QString& queryParams) const {
    QString url = QString("https://firestore.googleapis.com/v1/projects/%1/databases/(default)/documents/%2")
                  .arg(projectId, path);
    if (!queryParams.isEmpty()) {
        url += "?" + queryParams;
    }
    return url;
}

void FirebaseConfig::autenticarUsuario(const QString& email, const QString& password, std::function<void(bool, const QString&)> callback) {
    if (!configurado) { callback(false, "No configurado"); return; }
    
    QJsonObject json;
    json["email"] = email;
    json["password"] = password;
    json["returnSecureToken"] = true;

    QNetworkRequest req((QUrl(construirUrlAuth())));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* reply = networkManager->post(req, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply, callback](){
        if(reply->error() == QNetworkReply::NoError){
            auto obj = QJsonDocument::fromJson(reply->readAll()).object();
            authToken = obj["idToken"].toString();
            userId = obj["localId"].toString();
            autenticado = true;
            callback(true, "Login OK");
        } else {
            callback(false, reply->errorString());
        }
        reply->deleteLater();
    });
}

void FirebaseConfig::guardarDocumento(const QString& coleccion, const QString& docId, const QJsonObject& data, FirebaseCallback callback) {
    if (!autenticado) { callback(false, QJsonValue(), "No autenticado"); return; }

    QString urlStr = construirUrlFirestore(coleccion + "/" + docId);
    QNetworkRequest request((QUrl(urlStr)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + authToken).toUtf8());

    QJsonObject body = convertirAFormatoFirestore(data);
    QNetworkReply* reply = networkManager->put(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [reply, callback]() {
        if (reply->error() == QNetworkReply::NoError) {
            callback(true, QJsonObject(), "Guardado OK");
        } else {
            callback(false, QJsonValue(), reply->errorString());
        }
        reply->deleteLater();
    });
}

void FirebaseConfig::leerDocumento(const QString& coleccion, const QString& docId, FirebaseCallback callback) {
    if (!autenticado) { callback(false, QJsonValue(), "No autenticado"); return; }

    QString urlStr = construirUrlFirestore(coleccion + "/" + docId);
    QNetworkRequest request((QUrl(urlStr)));
    request.setRawHeader("Authorization", ("Bearer " + authToken).toUtf8());

    QNetworkReply* reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonObject doc = QJsonDocument::fromJson(reply->readAll()).object();
            callback(true, convertirDesdeFormatoFirestore(doc), "Leido OK");
        } else {
            callback(false, QJsonValue(), reply->errorString());
        }
        reply->deleteLater();
    });
}

void FirebaseConfig::listarColeccion(const QString& coleccion, FirebaseCallback callback) {
    if (!autenticado) { callback(false, QJsonValue(), "No autenticado"); return; }

    QString urlStr = construirUrlFirestore(coleccion);
    QNetworkRequest request((QUrl(urlStr)));
    request.setRawHeader("Authorization", ("Bearer " + authToken).toUtf8());

    QNetworkReply* reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonObject doc = QJsonDocument::fromJson(reply->readAll()).object();
            QJsonArray raw = doc["documents"].toArray();
            QJsonArray clean;
            for(const auto& val : raw) {
                 clean.append(convertirDesdeFormatoFirestore(val.toObject()));
            }
            callback(true, clean, "Listado OK");
        } else {
            callback(false, QJsonValue(), reply->errorString());
        }
        reply->deleteLater();
    });
}

void FirebaseConfig::eliminarDocumento(const QString& coleccion, const QString& docId, FirebaseCallback callback) {
    if (!autenticado) { callback(false, QJsonValue(), "No autenticado"); return; }
    
    QString urlStr = construirUrlFirestore(coleccion + "/" + docId);
    QNetworkRequest request((QUrl(urlStr)));
    request.setRawHeader("Authorization", ("Bearer " + authToken).toUtf8());
    
    QNetworkReply* reply = networkManager->deleteResource(request);
    connect(reply, &QNetworkReply::finished, this, [reply, callback](){
        if(reply->error() == QNetworkReply::NoError) callback(true, QJsonObject(), "Eliminado OK");
        else callback(false, QJsonValue(), reply->errorString());
        reply->deleteLater();
    });
}

void FirebaseConfig::actualizarDocumento(const QString& coleccion, const QString& docId, const QJsonObject& data, FirebaseCallback callback) {
    guardarDocumento(coleccion, docId, data, callback); 
}

QJsonObject FirebaseConfig::convertirAFormatoFirestore(const QJsonObject& datos) {
    QJsonObject firestoreDoc, fields;
    for (auto it = datos.begin(); it != datos.end(); ++it) {
        QJsonObject field;
        if (it.value().isString()) field["stringValue"] = it.value().toString();
        else if (it.value().isDouble()) field["integerValue"] = QString::number(it.value().toInt()); // Simplificado a int
        else if (it.value().isBool()) field["booleanValue"] = it.value().toBool();
        fields[it.key()] = field;
    }
    firestoreDoc["fields"] = fields;
    return firestoreDoc;
}

QJsonObject FirebaseConfig::convertirDesdeFormatoFirestore(const QJsonObject& firestoreDoc) {
    QJsonObject res;
    if(!firestoreDoc.contains("fields")) return res;
    QJsonObject fields = firestoreDoc["fields"].toObject();
    for(auto it = fields.begin(); it != fields.end(); ++it) {
        res[it.key()] = convertirValorDesdeFirestore(it.value().toObject());
    }
    return res;
}

QJsonValue FirebaseConfig::convertirValorDesdeFirestore(const QJsonObject& field) {
    if(field.contains("stringValue")) return field["stringValue"].toString();
    if(field.contains("integerValue")) return field["integerValue"].toString().toInt();
    if(field.contains("booleanValue")) return field["booleanValue"].toBool();
    return QJsonValue();
}

bool FirebaseConfig::estaConfigurado() const { return configurado; }
bool FirebaseConfig::estaAutenticado() const { return autenticado; }
QString FirebaseConfig::getToken() const { return authToken; }
QString FirebaseConfig::getUserId() const { return userId; }
