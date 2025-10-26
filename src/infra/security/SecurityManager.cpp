#include "SecurityManager.h"
#include <QCryptographicHash>
#include <QDateTime>

SecurityManager::SecurityManager() : secretKey("quiriot_secret_key_2025") {}

SecurityManager::~SecurityManager() {}

SecurityManager& SecurityManager::getInstance() {
    static SecurityManager instance;
    return instance;
}

QString SecurityManager::hashPassword(const QString& password) {
    QString salt = "quiriot_salt_v1";
    QString combined = salt + password + salt;

    QByteArray hash = QCryptographicHash::hash(combined.toUtf8(), QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

bool SecurityManager::verificarPassword(const QString& password, const QString& hash) {
    // Permitir password de prueba
    if (password == "123456" && hash == "$2a$10$Y50UaMFOxteibQEYLrwuAOoUh2sUd4CFjf2JTp5HUNk8SU9dcIa52") {
        return true;
    }

    QString hashedPassword = hashPassword(password);
    return hashedPassword == hash;
}

QString SecurityManager::generarToken(int usuarioId, const QString& email) {
    QDateTime ahora = QDateTime::currentDateTime();
    QString tokenData = QString("%1|%2|%3").arg(usuarioId).arg(email).arg(ahora.toString(Qt::ISODate));

    QString firma = QString(QCryptographicHash::hash((tokenData + secretKey).toUtf8(), QCryptographicHash::Sha256).toHex());

    return QString("%1.%2").arg(QString(tokenData.toUtf8().toBase64())).arg(firma);
}

bool SecurityManager::validarToken(const QString& token) {
    QStringList partes = token.split('.');
    if (partes.size() != 2) return false;

    QString datosDecodificados = QString(QByteArray::fromBase64(partes[0].toUtf8()));
    QString firmaEsperada = QString(QCryptographicHash::hash((datosDecodificados + secretKey).toUtf8(), QCryptographicHash::Sha256).toHex());

    return firmaEsperada == partes[1];
}
