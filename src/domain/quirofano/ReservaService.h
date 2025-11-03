#ifndef RESERVASERVICE_H
#define RESERVASERVICE_H

#include "Reserva.h"
#include "QuirofanoRepository.h"
#include "../usuario/UsuarioRepository.h"
#include "domain/quirofano/SugerenciaAgenda.h"
#include <QSqlDatabase>
#include <QVector>

struct DatosReserva {
    int idUsuario;
    int idQuirofano;
    QDateTime fechaInicio;
    QDateTime fechaFin;
    QString motivoCirugia;
};

struct HorarioDisponible {
    QDateTime inicio;
    QDateTime fin;
    int duracionMinutos;
};

class ReservaService {
public:
    ReservaService(QSqlDatabase& db, QuirofanoRepository& quirofanoRepo,
                   UsuarioRepository& usuarioRepo);

    Reserva* crearReserva(const DatosReserva& datos);
    bool cancelarReserva(int idReserva);
    Reserva* obtenerReserva(int idReserva);
    QVector<Reserva*> listarReservasDelDia(const QDate& fecha);
    QVector<HorarioDisponible> obtenerHorariosDisponibles(int idQuirofano, const QDate& fecha);

    bool validarReserva(const DatosReserva& datos, QString& mensajeError);
    bool existeConflicto(int idQuirofano, const QDateTime& inicio, const QDateTime& fin);
    
    HorarioSugerido validarYSugerirHorario(
        int idQuirofano,
        const QDateTime& inicio,
        const QDateTime& fin
    );
    
    HorarioSugerido encontrarProximoHorarioDisponible(
        int idQuirofano,
        const QDateTime& inicioDeseado,
        int duracionMinutos
    );
    
    QVector<Reserva*> listarReservasPorQuirofano(int idQuirofano);
    QVector<Reserva*> listarReservasPorQuirofanoYFecha(int idQuirofano, const QDate& fecha);

private:
    QSqlDatabase& database;
    QuirofanoRepository& quirofanoRepository;
    UsuarioRepository& usuarioRepository;
    SugerenciaAgenda sugerenciaAgenda;

    Reserva* mapearReserva(const class QSqlQuery& query);
    bool guardarReserva(Reserva& reserva);
};

#endif
