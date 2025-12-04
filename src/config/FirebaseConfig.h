#ifndef FIREBASECONFIG_H
#define FIREBASECONFIG_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <functional>

using FirebaseCallback = std::function<void(bool, const QJsonValue&, const QString&)>;

class FirebaseConfig : public QObject
{
    Q_OBJECT

public:
    static FirebaseConfig& getInstance();

    void cargarDesdeEnv(const QString& rutaEnv = ".env");
    bool estaConfigurado() const;
    bool estaAutenticado() const;
    QString getToken() const;
    QString getUserId() const;

    void autenticarUsuario(const QString& email, const QString& password, 
                           std::function<void(bool, const QString&)> callback);

    void guardarDocumento(const QString& coleccion, const QString& docId, const QJsonObject& data, FirebaseCallback callback);
    void leerDocumento(const QString& coleccion, const QString& docId, FirebaseCallback callback);
    void actualizarDocumento(const QString& coleccion, const QString& docId, const QJsonObject& data, FirebaseCallback callback);
    void eliminarDocumento(const QString& coleccion, const QString& docId, FirebaseCallback callback);
    void listarColeccion(const QString& coleccion, FirebaseCallback callback);

private:
    explicit FirebaseConfig();
    ~FirebaseConfig();
    
    FirebaseConfig(const FirebaseConfig&) = delete;
    FirebaseConfig& operator=(const FirebaseConfig&) = delete;

    void cargarConfiguracionPorDefecto();
    QString construirUrlAuth() const;
    
    QString construirUrlFirestore(const QString& path, const QString& queryParams = "") const;

    QJsonObject convertirAFormatoFirestore(const QJsonObject& datos);
    QJsonObject convertirDesdeFormatoFirestore(const QJsonObject& firestoreDoc);
    QJsonValue convertirValorDesdeFirestore(const QJsonObject& field);

    QNetworkAccessManager* networkManager;
    QString apiKey;
    QString projectId;
    QString authToken;
    QString userId;
    
    bool configurado;
    bool autenticado;
};

#endif // FIREBASECONFIG_H
