#include "QuirofanoController.h"
#include "../infra/exceptions/ValidacionException.h"
#include <QDebug>

QuirofanoController::QuirofanoController(QuirofanoRepository& quirofanoRepo, ReservaService& reservaServ)
    : quirofanoRepository(quirofanoRepo), reservaService(reservaServ) {}

QVector<Quirofano*> QuirofanoController::listarQuirofanos() {
    return quirofanoRepository.listarTodos();
}

QVector<Quirofano*> QuirofanoController::listarDisponibles() {
    return quirofanoRepository.listarDisponibles();
}

Quirofano* QuirofanoController::obtenerQuirofano(int id) {
    return quirofanoRepository.buscarPorId(id);
}

Reserva* QuirofanoController::agendarCirugia(int idUsuario, int idQuirofano,
                                             const QDateTime& inicio, const QDateTime& fin,
                                             const QString& motivo) {
    try {
        DatosReserva datos;
        datos.idUsuario = idUsuario;
        datos.idQuirofano = idQuirofano;
        datos.fechaInicio = inicio;
        datos.fechaFin = fin;
        datos.motivoCirugia = motivo;

        Reserva* reserva = reservaService.crearReserva(datos);

        qInfo() << "[OK] Cirugia agendada. ID:" << reserva->getId();
        return reserva;

    } catch (const ValidacionException& e) {
        qCritical() << "[ERROR] Al agendar cirugia:" << e.getMensaje();
        throw;
    }
}

bool QuirofanoController::cancelarReserva(int idReserva) {
    return reservaService.cancelarReserva(idReserva);
}

QVector<Reserva*> QuirofanoController::listarReservasDelDia(const QDate& fecha) {
    return reservaService.listarReservasDelDia(fecha);
}

QVector<HorarioDisponible> QuirofanoController::consultarHorariosDisponibles(int idQuirofano,
                                                                             const QDate& fecha) {
    return reservaService.obtenerHorariosDisponibles(idQuirofano, fecha);
}
