#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QTimeZone>
#include <QEventLoop>
#include <QTimer>
#include <QThread>

#include "src/config/DatabaseConfig.h"
#include "src/config/FirebaseConfig.h"
#include "src/domain/usuario/UsuarioRepository.h"
#include "src/domain/usuario/UsuarioFirestoreRepository.h"
#include "src/domain/usuario/Usuario.h"
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

    QTimeZone mexicoTimezone("America/Mexico_City");
    if (mexicoTimezone.isValid()) {
        qInfo() << "[OK] Zona horaria detectada: America/Mexico_City (UTC-6)";
    }
    qputenv("TZ", "America/Mexico_City");
    QCoreApplication::addLibraryPath("/usr/lib/qt6/plugins");

    qInfo() << "=============================================================";
    qInfo() << "             QuirIOT - Sistema de Quirofanos IoT             ";
    qInfo() << "                        Version 1.0.0                        ";
    qInfo() << "============================================================";

    try {
        if (!QFile::exists(".env")) {
            qCritical() << "\n[ERROR] No se encontro el archivo .env";
            return 1;
        }

        DatabaseConfig& dbConfig = DatabaseConfig::getInstance();
        dbConfig.cargarDesdeEnv(".env");

        FirebaseConfig& firebaseConfig = FirebaseConfig::getInstance();
        firebaseConfig.cargarDesdeEnv(".env");

        if (firebaseConfig.estaConfigurado()) {
            qInfo() << "------------------------------------------------------------";
            qInfo() << "[BOOT] Inicializando conexion a Nube (Firebase)...";
            QEventLoop bootLoop;
            bool conectado = false;
            QString sysEmail = "admin@quiriot.com";
            QString sysPass = "123456";

            firebaseConfig.autenticarUsuario(sysEmail, sysPass, [&](bool ok, const QString& msg){
                conectado = ok;
                if(ok) qInfo() << "[BOOT] Nube Conectada. Token de sistema listo.";
                else qWarning() << "[BOOT] Fallo conexion Nube:" << msg;
                bootLoop.quit();
            });
            QTimer::singleShot(5000, &bootLoop, &QEventLoop::quit);
            bootLoop.exec();
            qInfo() << "------------------------------------------------------------\n";
        }

        qInfo() << "[INFO] Conectando a MariaDB Local...";
        if (!dbConfig.conectarLocal()) {
            qCritical() << "[ERROR] Fallo conexion a MariaDB.";
            return 1;
        }
        QSqlDatabase& db = dbConfig.getDatabase();

        UsuarioRepository usuarioRepo(db);
        QuirofanoRepository quirofanoRepo(db);
        SensorRepository sensorRepo(db);
        ActuadorRepository actuadorRepo(db);

        AutenticacionService authService(db, usuarioRepo);
        ReservaService reservaService(db, quirofanoRepo, usuarioRepo);

        AutenticacionController authController(authService);

        QThread* iotThread = new QThread(&app);

        IoTController* iotController = new IoTController(sensorRepo, actuadorRepo, db);

        QuirofanoController quirofanoController(quirofanoRepo, reservaService, *iotController);

        iotController->moveToThread(iotThread);

        QString ipNodeMCU = "10.229.201.120";

        QObject::connect(iotThread, &QThread::started, [iotController, ipNodeMCU]() {
            iotController->iniciarMonitoreo(ipNodeMCU, 5000);
        });

        QObject::connect(&app, &QCoreApplication::aboutToQuit, iotThread, &QThread::quit);
        QObject::connect(iotThread, &QThread::finished, iotThread, &QThread::deleteLater);
        QObject::connect(iotThread, &QThread::finished, iotController, &QObject::deleteLater);

        iotThread->start();

        qInfo() << "[SISTEMA] Servicio de monitoreo IoT iniciado en segundo plano (" << ipNodeMCU << ")";
        qInfo() << "Iniciando Interfaz...";

        ConsoleView vista(authController, quirofanoController, *iotController);
        vista.ejecutar();

        dbConfig.cerrarConexion();
        qInfo() << "[OK] Aplicacion finalizada.";

        return 0;

    } catch (const std::exception& e) {
        qCritical() << "[ERROR FATAL]" << e.what();
        return 1;
    }
}
