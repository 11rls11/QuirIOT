#ifndef SECURITYMANAGER_H
#define SECURITYMANAGER_H

#include <QString>

class SecurityManager {
public:
    static SecurityManager& getInstance();

    QString hashPassword(const QString& password);
    bool verificarPassword(const QString& password, const QString& hash);
    QString generarToken(int usuarioId, const QString& email);
    bool validarToken(const QString& token);

private:
    SecurityManager();
    ~SecurityManager();

    SecurityManager(const SecurityManager&) = delete;
    SecurityManager& operator=(const SecurityManager&) = delete;

    QString secretKey;
};

#endif
