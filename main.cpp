#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include "src/config/DatabaseConfig.h"
#include "src/domain/usuario/UsuarioRepository.h"
#include "src/domain/usuario/AutenticacionService.h"
#include "src/domain/quirofano/QuirofanoRepository.h"
#include "src/domain/quirofano/ReservaService.h"
#include "src/domain/sensor/SensorRepository.h"
#include "src/domain/actuador/ActuadorRepository.h"
#include "src/controller/AutenticacionController.h"
#include "src/controller/QuirofanoController.h"
#include "src/controller/IoTController.h"
#include "src/infra/security/SecurityManager.h"
#include "src/view/ConsoleView.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCoreApplication::setApplicationName("QuirIOT");
    QCoreApplication::setApplicationVersion("1.0.0 - MVP");
    QCoreApplication::setOrganizationName("TecDeMonterrey");

    QCoreApplication::addLibraryPath("/usr/lib/qt6/plugins");
    qDebug() << "[DEBUG] Plugin paths:" << QCoreApplication::libraryPaths();
    qDebug() << "[DEBUG] Available drivers:" << QSqlDatabase::drivers();

    qInfo() << "=============================================================";
    qInfo() << "             QuirIOT - Sistema de Quirofanos IoT             ";
    qInfo() << "                        Version 1.0.0                        ";
    qInfo() << "============================================================";

    try {
        // Verificar archivo .env
        if (!QFile::exists(".env")) {
            qCritical() << "\n[ERROR] No se encontro el archivo .env";
            qCritical() << "\n[SOLUCION]";
            qCritical() << "   1. Copia .env.example a .env";
            qCritical() << "   2. Edita .env con tus credenciales";
            return 1;
        }

        // Cargar configuracion
        qInfo() << "\n[INFO] Cargando configuracion desde .env...";
        DatabaseConfig& dbConfig = DatabaseConfig::getInstance();
        dbConfig.cargarDesdeEnv(".env");

        SecurityManager& securityManager = SecurityManager::getInstance();
        Q_UNUSED(SecurityManager::getInstance());
        qInfo() << "[OK] Security Manager inicializado";

        // Conectar a base de datos
        qInfo() << "\n[INFO] Conectando a la base de datos local...";
        if (!dbConfig.conectarLocal()) {
            qCritical() << "\n[ERROR] No se pudo conectar a la base de datos";
            qCritical() << "\n[SOLUCION] Verifica:";
            qCritical() << "   1. MariaDB corriendo: sudo systemctl status mariadb";
            qCritical() << "   2. Credenciales correctas en .env";
            qCritical() << "   3. Base de datos existe: mariadb -u root -p -e 'SHOW DATABASES;'";
            return 1;
        }

        QSqlDatabase& db = dbConfig.getDatabase();

        // Inicializar repositorios (solo los necesarios para US 1 y 2)
        qInfo() << "[INFO] Inicializando repositorios...";
        UsuarioRepository usuarioRepo(db);
        QuirofanoRepository quirofanoRepo(db);
        SensorRepository sensorRepo(db);        // Esqueleto para US futuras
        ActuadorRepository actuadorRepo(db);    // Esqueleto para US futuras

        // Inicializar servicios
        qInfo() << "[INFO] Inicializando servicios...";
        AutenticacionService authService(db, usuarioRepo);
        ReservaService reservaService(db, quirofanoRepo, usuarioRepo);

        // Inicializar controladores
        qInfo() << "[INFO] Inicializando controladores...";
        AutenticacionController authController(authService);
        QuirofanoController quirofanoController(quirofanoRepo, reservaService);
        IoTController iotController(sensorRepo, actuadorRepo);  // Esqueleto

        qInfo() << "\n[OK] Sistema inicializado correctamente";

        // Iniciar aplicacion
        ConsoleView vista(authController, quirofanoController, iotController);
        vista.ejecutar();

        // Cerrar conexion
        qInfo() << "\n[INFO] Cerrando conexion a la base de datos...";
        dbConfig.cerrarConexion();

        qInfo() << "[OK] Aplicacion finalizada correctamente";

        return 0;

    } catch (const std::exception& e) {
        qCritical() << "[ERROR FATAL]" << e.what();
        return 1;
    }
}
