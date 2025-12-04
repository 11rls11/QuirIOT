#include "QuirofanoController.h"
#include "../infra/exceptions/ValidacionException.h"
#include <QDebug>

QuirofanoController::QuirofanoController(QuirofanoRepository& quirofanoRepo,
                                         ReservaService& reservaServ,
                                         IoTController& iotCtrl)
    : quirofanoRepository(quirofanoRepo),
      reservaService(reservaServ),
      iotController(iotCtrl)
{}

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

        qInfo() << "[OK] Cirugia agendada exitosamente. ID:" << reserva->getId();

        QDateTime horaLimpieza = inicio.addSecs(-15 * 60);
        QDateTime ahora = QDateTime::currentDateTime();

        qint64 msecsParaLimpieza = ahora.msecsTo(horaLimpieza);

        const qint64 MAX_TIMER_MS = 1800000000;

        if (msecsParaLimpieza <= 0) {
            qInfo() << "[ALERTA] Tiempo insuficiente para protocolo completo.";
            qInfo() << "[AUTO] Iniciando limpieza de emergencia AHORA MISMO.";

            iotController.activarSistemaLimpieza(idQuirofano, 0, "SISTEMA_AUTO");

        } else if (msecsParaLimpieza > MAX_TIMER_MS) {
            qInfo() << "[INFO] La cirugia es muy lejana (" << (msecsParaLimpieza/1000/60/60/24) << " dias).";
            qInfo() << "       NO se programara la limpieza automatica en memoria para evitar errores.";
            qInfo() << "       (En un sistema real, esto se guardaria en BD y un cron job lo ejecutaria).";

        } else {
            qInfo() << "--------------------------------------------------";
            qInfo() << "[AUTO] Protocolo de Limpieza Programado";
            qInfo() << "       Hora Limpieza:" << horaLimpieza.toString("hh:mm:ss");
            qInfo() << "       Tiempo espera:" << (msecsParaLimpieza / 1000 / 60) << "minutos";
            qInfo() << "--------------------------------------------------";

            QTimer::singleShot(msecsParaLimpieza, &iotController, [this, idQuirofano]() {
                qInfo() << "\n>>> [AUTO-TRIGGER] HORA DE LIMPIEZA ALCANZADA <<<";
                iotController.activarSistemaLimpieza(idQuirofano, 0, "SISTEMA_AUTO");
            });
        }

        return reserva;

    } catch (const ValidacionException& e) {
        qCritical() << "[ERROR] No se pudo agendar:" << e.getMensaje();
        throw;
    }
}

bool QuirofanoController::cancelarReserva(int idReserva) {
    return reservaService.cancelarReserva(idReserva);
}

QVector<Reserva*> QuirofanoController::listarReservasDelDia(const QDate& fecha) {
    return reservaService.listarReservasDelDia(fecha);
}

QVector<Reserva*> QuirofanoController::listarReservasPorQuirofano(int idQuirofano) {
    return reservaService.listarReservasPorQuirofano(idQuirofano);
}

QVector<HorarioDisponible> QuirofanoController::consultarHorariosDisponibles(int idQuirofano,
                                                                             const QDate& fecha) {
    return reservaService.obtenerHorariosDisponibles(idQuirofano, fecha);
}

HorarioSugerido QuirofanoController::validarYSugerirHorario(
    int idQuirofano,
    const QDateTime& inicio,
    const QDateTime& fin)
{
    return reservaService.validarYSugerirHorario(idQuirofano, inicio, fin);
}

HorarioSugerido QuirofanoController::encontrarProximoHorarioDisponible(
    int idQuirofano,
    const QDateTime& inicioDeseado,
    int duracionMinutos)
{
    return reservaService.encontrarProximoHorarioDisponible(
        idQuirofano, inicioDeseado, duracionMinutos
    );
}
