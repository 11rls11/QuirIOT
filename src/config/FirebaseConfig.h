#ifndef FIREBASECONFIG_H
#define FIREBASECONFIG_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonObject>
#include <functional>

class FirebaseConfig : public QObject {
    Q_OBJECT

public:
    static FirebaseConfig& getInstance();

    void cargarDesdeEnv(const QString& rutaEnv = ".env");
    void autenticarUsuario(const QString& email, const QString& password,
                           std::function<void(bool, const QString&)> callback);
    void guardarDocumento(const QString& coleccion, const QString& documento,
                          const QJsonObject& datos,
                          std::function<void(bool, const QString&)> callback);

    QString getToken() const;
    bool estaAutenticado() const;
    bool estaConfigurado() const;

private:
    FirebaseConfig();
    ~FirebaseConfig() override;

    FirebaseConfig(const FirebaseConfig&) = delete;
    FirebaseConfig& operator=(const FirebaseConfig&) = delete;

    QNetworkAccessManager* networkManager;
    QString apiKey;
    QString projectId;
    QString authToken;
    QString userId;
    bool autenticado;
    bool configurado;

    void cargarConfiguracionPorDefecto();
    QString construirUrlAuth() const;
    QString construirUrlFirestore(const QString& path) const;
    QJsonObject convertirAFormatoFirestore(const QJsonObject& datos);
};

#endif // FIREBASECONFIG_H
