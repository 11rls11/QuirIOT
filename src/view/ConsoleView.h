#ifndef CONSOLEVIEW_H
#define CONSOLEVIEW_H

#include "../controller/AutenticacionController.h"
#include "../controller/QuirofanoController.h"
#include "../controller/IoTController.h"
#include <QTextStream>
#include <QString>
#include "../controller/DemoController.h"


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
    
    // Métodos modo demo
    void iniciarModoDemo();
    void menuModoDemo();
    void ejecutarPruebaDemo(TipoPrueba tipo);
    void mostrarTutorialInteractivo();
    void mostrarProgresoDetallado();
    void finalizarYMostrarReporte();
    
    // Pruebas interactivas
    bool ejecutarPruebaNavegacionInteractiva();
    bool ejecutarPruebaAgendarInteractiva();
    bool ejecutarPruebaSistemaLimpiezaInteractiva();

    // Menus
    void mostrarMenuPrincipal();
    void mostrarMenuQuirofanos();
    void mostrarMenuIoT();

    // US 1: Autenticacion
    bool procesarLogin();
    void procesarLogout();

    // US 2: Agendar cirugia
    void verInformacionQuirofano();
    void consultarHorariosDisponibles();
    void agendarCirugia();
    void listarReservasDelDia();
    void cancelarReserva();

    void agendarCirugiaConSugerencias();
    void verReservasFuturas();
    void mostrarSugerenciaHorario();
    void buscarProximoHorarioDisponible();
    void validarYConfirmarReserva(int idQuirofano, const QDateTime& inicio, 
                                    const QDateTime& fin, const QString& motivo);

    void activarDesactivarSistemaLimpieza();
    void consultarEstadoSistemaLimpieza();
    void verHistorialSistemaLimpieza();

    void visualizarEstadoQuirofano();      // US 3
    void verRegistroDisponibilidad();      // US 4
    void monitorearTiempoReal();           // US 10
    void verHistorialCondiciones();        // US 11
    void definirMantenimiento();           // US de desarrolladores

    // Utilidades
    QString leerLinea(const QString& prompt = "");
    QString leerLineaSegura(const QString& prompt);
    int leerEntero(const QString& prompt, int min = 0, int max = 999999);
    void pausa();
    void limpiarPantalla();
    void mostrarSeparador();
    void mostrarError(const QString& mensaje);
    void mostrarExito(const QString& mensaje);
    void mostrarInfo(const QString& mensaje);
    void mostrarAdvertencia(const QString& mensaje);

    // Utilidades de color (pa' la terminal)
    QString colorAmarillo(const QString& texto);
    QString colorVerde(const QString& texto);
    QString colorRojo(const QString& texto);
    QString colorCyan(const QString& texto);
    QString colorMagenta(const QString& texto);
    QString resetColor();
};

#endif // CONSOLEVIEW_H
