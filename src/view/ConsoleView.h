#ifndef CONSOLEVIEW_H
#define CONSOLEVIEW_H

#include "../controller/AutenticacionController.h"
#include "../controller/QuirofanoController.h"
#include "../controller/IoTController.h"
#include "../controller/DemoController.h"
#include <QTextStream>
#include <QDate>

class ConsoleView {
public:
    ConsoleView(AutenticacionController& authCtrl, QuirofanoController& quirofanoCtrl,
                IoTController& iotCtrl);
    void ejecutar();

private:
    AutenticacionController& autenticacionController;
    QuirofanoController& quirofanoController;
    IoTController& iotController;
    DemoController demoController;

    QTextStream in;
    QTextStream out;

    void mostrarMenuPrincipal();
    bool procesarLogin();
    void procesarLogout();

    void mostrarMenuQuirofanos();
    void mostrarMenuIoT();

    void verInformacionQuirofano();
    void consultarHorariosDisponibles();
    void agendarCirugia();
    void agendarCirugiaConSugerencias();
    void verReservasFuturas();
    void cancelarReserva();
    void mostrarSugerenciaHorario();
    void listarReservasDelDia();
    void validarYConfirmarReserva(int idQuirofano, const QDateTime& inicio, const QDateTime& fin, const QString& motivo); // Faltaba

    void visualizarEstadoQuirofano();
    void verRegistroDisponibilidad();
    void monitorearTiempoReal();
    void verHistorialCondiciones();
    void definirMantenimiento();

    void activarDesactivarSistemaLimpieza();
    void consultarEstadoSistemaLimpieza();
    void verHistorialSistemaLimpieza();
    void verEstadoSensores();

    void iniciarModoDemo();
    void menuModoDemo();
    void ejecutarPruebaDemo(TipoPrueba tipo);
    bool ejecutarPruebaNavegacionInteractiva();
    bool ejecutarPruebaReservaInteractiva();
    bool ejecutarPruebaAgendarInteractiva();
    bool ejecutarPruebaSistemaLimpiezaInteractiva();
    void mostrarTutorialInteractivo();
    void mostrarProgresoDetallado();
    void finalizarYMostrarReporte();

    void limpiarPantalla();
    void mostrarSeparador();
    void pausa();
    void mostrarError(const QString& mensaje);
    void mostrarExito(const QString& mensaje);
    void mostrarInfo(const QString& mensaje);
    void mostrarAdvertencia(const QString& mensaje);

    int leerEntero(const QString& mensaje, int min, int max);

    QString leerLinea(const QString& mensaje = QString());

    QString leerLineaSegura(const QString& mensaje);

    QString colorRojo(const QString& texto);
    QString colorVerde(const QString& texto);
    QString colorAmarillo(const QString& texto);
    QString colorCyan(const QString& texto);
    QString colorMagenta(const QString& texto);
    QString resetColor();
};

#endif // CONSOLEVIEW_H
