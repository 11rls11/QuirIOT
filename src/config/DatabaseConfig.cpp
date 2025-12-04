#include "DatabaseConfig.h"
#include <QSqlError>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QSqlQuery>

DatabaseConfig::DatabaseConfig() : port(3306), conectado(false) {
    cargarConfiguracionPorDefecto();
}

DatabaseConfig::~DatabaseConfig() {
    cerrarConexion();
}

DatabaseConfig& DatabaseConfig::getInstance() {
    static DatabaseConfig instance;
    return instance;
}

void DatabaseConfig::cargarConfiguracionPorDefecto() {
    host = "localhost";
    database = "quiriot_db";
    username = "root";
    password = "";
    port = 3306;
}

void DatabaseConfig::cargarDesdeEnv(const QString& rutaEnv) {
    QFile envFile(rutaEnv);

    if (!envFile.exists()) {
        qWarning() << "[WARN] Archivo" << rutaEnv << "no encontrado. Usando configuracion por defecto.";
        return;
    }

    if (!envFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[WARN] No se pudo abrir" << rutaEnv;
        return;
    }

    QTextStream in(&envFile);
    qInfo() << "[INFO] Cargando configuracion desde" << rutaEnv;

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

        if (clave == "DB_HOST") {
            host = valor;
        } else if (clave == "DB_PORT") {
            port = valor.toInt();
        } else if (clave == "DB_DATABASE") {
            database = valor;
        } else if (clave == "DB_USERNAME") {
            username = valor;
        } else if (clave == "DB_PASSWORD") {
            password = valor;
        }
    }

    envFile.close();
    qInfo() << "[OK] Configuracion de base de datos cargada";
}

bool DatabaseConfig::conectarLocal() {
    if (conectado && db.isOpen()) {
        qInfo() << "[OK] Ya existe una conexion activa";
        return true;
    }

    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(host);
    db.setDatabaseName(database);
    db.setUserName(username);
    db.setPassword(password);
    db.setPort(port);

    qInfo() << "[INFO] Conectando a:" << username << "@" << host << ":" << port << "/" << database;

    db.setConnectOptions("MYSQL_OPT_SSL_MODE=SSL_MODE_DISABLED");

    if (!db.open()) {
        qCritical() << "[ERROR] Error al conectar con MySQL/MariaDB:";
        qCritical() << "        " << db.lastError().text();
        conectado = false;
        return false;
    }

    conectado = true;
    qInfo() << "[OK] Conexion exitosa a MySQL/MariaDB";
    return true;
}

QSqlDatabase& DatabaseConfig::getDatabase() {
    return db;
}

void DatabaseConfig::cerrarConexion() {
    if (db.isOpen()) {
        db.close();
        conectado = false;
        qInfo() << "[INFO] Conexion a base de datos cerrada";
    }
}

bool DatabaseConfig::estaConectado() const {
    return conectado && db.isOpen();
}
