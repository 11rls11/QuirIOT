#ifndef DATABASECONFIG_H
#define DATABASECONFIG_H

#include <QSqlDatabase>
#include <QString>

class DatabaseConfig {
public:
    static DatabaseConfig& getInstance();

    bool conectarLocal();
    QSqlDatabase& getDatabase();
    void cerrarConexion();
    bool estaConectado() const;
    void cargarDesdeEnv(const QString& rutaEnv = ".env");

private:
    DatabaseConfig();
    ~DatabaseConfig();

    DatabaseConfig(const DatabaseConfig&) = delete;
    DatabaseConfig& operator=(const DatabaseConfig&) = delete;

    QSqlDatabase db;
    QString host;
    QString database;
    QString username;
    QString password;
    int port;
    bool conectado;

    void cargarConfiguracionPorDefecto();
};

#endif
