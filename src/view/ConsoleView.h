#ifndef CONSOLEVIEW_H
#define CONSOLEVIEW_H

#include "../controller/AutenticacionController.h"
#include "../controller/QuirofanoController.h"
#include "../controller/IoTController.h"
#include <QTextStream>
#include <QString>

class ConsoleView {
public:
    ConsoleView(AutenticacionController& authCtrl, QuirofanoController& quirofanoCtrl,
                IoTController& iotCtrl);

    void ejecutar();

private:
    AutenticacionController& autenticacionController;
    QuirofanoController& quirofanoController;
    IoTController& iotController;

    QTextStream in;
    QTextStream out;

    // Menus
    void mostrarMenuPrincipal();
    void mostrarMenuQuirofanos();
    void mostrarMenuIoT();

    // US 1: Autenticacion
    bool procesarLogin();
    void procesarLogout();

    // US 2: Agendar cirugia
    void listarQuirofanos();
    void consultarHorariosDisponibles();
    void agendarCirugia();
    void listarReservasDelDia();
    void cancelarReserva();

    void visualizarEstadoQuirofano();      // US 3
    void verRegistroDisponibilidad();      // US 4
    void sugerirHorarioAlternativo();      // US 6
    void activarDesactivarSistema();       // US 8
    void monitorearTiempoReal();           // US 10
    void verHistorialCondiciones();        // US 11
    void definirMantenimiento();           // US de desarrolladores

    QString leerLinea(const QString& prompt = "");
    int leerEntero(const QString& prompt, int min = 0, int max = 999999);
    void pausa();
    void limpiarPantalla();
    void mostrarSeparador();
    void mostrarError(const QString& mensaje);
    void mostrarExito(const QString& mensaje);
    void mostrarInfo(const QString& mensaje);
};

#endif // CONSOLEVIEW_H
