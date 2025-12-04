#ifndef QUIROFANOCONTROLLER_H
#define QUIROFANOCONTROLLER_H

#include "../domain/quirofano/Quirofano.h"
#include "../domain/quirofano/QuirofanoRepository.h"
#include "../domain/quirofano/ReservaService.h"
#include "IoTController.h"
#include <QVector>
#include <QTimer>

class QuirofanoController {
public:
    QuirofanoController(QuirofanoRepository& quirofanoRepo,
                        ReservaService& reservaServ,
                        IoTController& iotCtrl);

    QVector<Quirofano*> listarQuirofanos();
    QVector<Quirofano*> listarDisponibles();
    Quirofano* obtenerQuirofano(int id);

    Reserva* agendarCirugia(int idUsuario, int idQuirofano, const QDateTime& inicio,
                            const QDateTime& fin, const QString& motivo);
    bool cancelarReserva(int idReserva);
    QVector<Reserva*> listarReservasDelDia(const QDate& fecha);
    QVector<Reserva*> listarReservasPorQuirofano(int idQuirofano);
    QVector<HorarioDisponible> consultarHorariosDisponibles(int idQuirofano, const QDate& fecha);

    HorarioSugerido validarYSugerirHorario(int idQuirofano, const QDateTime& inicio, 
                                           const QDateTime& fin);
    HorarioSugerido encontrarProximoHorarioDisponible(int idQuirofano, 
                                                       const QDateTime& inicioDeseado,
                                                       int duracionMinutos);

private:
    QuirofanoRepository& quirofanoRepository;
    ReservaService& reservaService;
    IoTController& iotController;
};

#endif
