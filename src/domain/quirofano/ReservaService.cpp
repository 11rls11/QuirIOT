#include "ReservaService.h"
#include "../../infra/exceptions/ValidacionException.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <algorithm>

ReservaService::ReservaService(QSqlDatabase& db, QuirofanoRepository& quirofanoRepo,
                               UsuarioRepository& usuarioRepo)
    : database(db), quirofanoRepository(quirofanoRepo), usuarioRepository(usuarioRepo) {}

Reserva* ReservaService::crearReserva(const DatosReserva& datos) {
    QString mensajeError;

    if (!validarReserva(datos, mensajeError)) {
        throw ValidacionException(mensajeError);
    }

    Reserva* reserva = new Reserva(
        datos.idUsuario,
        datos.idQuirofano,
        datos.fechaInicio,
        datos.fechaFin
        );
    reserva->setMotivoCirugia(datos.motivoCirugia);

    if (!guardarReserva(*reserva)) {
        delete reserva;
        throw ValidacionException("Error al guardar la reserva en la base de datos");
    }

    quirofanoRepository.actualizarEstado(datos.idQuirofano, EstadoQuirofano::OCUPADO);

    qInfo() << "[OK] Reserva creada con ID:" << reserva->getId();
    return reserva;
}

bool ReservaService::cancelarReserva(int idReserva) {
    QSqlQuery query(database);

    query.prepare("UPDATE reservan SET estado_reserva = 'CANCELADA' WHERE id_reserva = ?");
    query.addBindValue(idReserva);

    if (!query.exec()) {
        qCritical() << "[ERROR] Al cancelar reserva:" << query.lastError().text();
        return false;
    }

    QSqlQuery queryQuirofano(database);
    queryQuirofano.prepare("SELECT id_quirofano FROM reservan WHERE id_reserva = ?");
    queryQuirofano.addBindValue(idReserva);

    if (queryQuirofano.exec() && queryQuirofano.next()) {
        int idQuirofano = queryQuirofano.value(0).toInt();
        quirofanoRepository.actualizarEstado(idQuirofano, EstadoQuirofano::DISPONIBLE);
    }

    qInfo() << "[OK] Reserva" << idReserva << "cancelada";
    return true;
}

Reserva* ReservaService::obtenerReserva(int idReserva) {
    QSqlQuery query(database);

    query.prepare("SELECT * FROM reservan WHERE id_reserva = ?");
    query.addBindValue(idReserva);

    if (query.exec() && query.next()) {
        return mapearReserva(query);
    }

    return nullptr;
}

QVector<Reserva*> ReservaService::listarReservasDelDia(const QDate& fecha) {
    QVector<Reserva*> reservas;
    QSqlQuery query(database);

    QDateTime inicioDia(fecha, QTime(0, 0, 0));
    QDateTime finDia(fecha, QTime(23, 59, 59));

    query.prepare(
        "SELECT * FROM reservan "
        "WHERE fecha_inicio BETWEEN ? AND ? AND estado_reserva != 'CANCELADA' "
        "ORDER BY fecha_inicio"
        );
    query.addBindValue(inicioDia);
    query.addBindValue(finDia);

    if (query.exec()) {
        while (query.next()) {
            reservas.append(mapearReserva(query));
        }
    }

    return reservas;
}

QVector<Reserva*> ReservaService::listarReservasPorQuirofano(int idQuirofano) {
    QVector<Reserva*> reservas;
    QSqlQuery query(database);

    query.prepare(
        "SELECT * FROM reservan WHERE id_quirofano = ? "
        "AND estado_reserva != 'CANCELADA' ORDER BY fecha_inicio"
        );
    query.addBindValue(idQuirofano);

    if (query.exec()) {
        while (query.next()) {
            reservas.append(mapearReserva(query));
        }
    }

    return reservas;
}

QVector<HorarioDisponible> ReservaService::obtenerHorariosDisponibles(int idQuirofano,
                                                                      const QDate& fecha) {
    QVector<HorarioDisponible> horariosDisponibles;

    QDateTime inicioJornada(fecha, QTime(7, 0, 0));
    QDateTime finJornada(fecha, QTime(19, 0, 0));

    auto reservas = listarReservasPorQuirofano(idQuirofano);

    if (reservas.isEmpty()) {
        HorarioDisponible horario;
        horario.inicio = inicioJornada;
        horario.fin = finJornada;
        horario.duracionMinutos = inicioJornada.secsTo(finJornada) / 60;
        horariosDisponibles.append(horario);
        return horariosDisponibles;
    }

    std::sort(reservas.begin(), reservas.end(), [](Reserva* a, Reserva* b) {
        return a->getFechaInicio() < b->getFechaInicio();
    });

    QDateTime ultimoFin = inicioJornada;

    for (Reserva* reserva : reservas) {
        if (reserva->getFechaInicio().date() != fecha) continue;
        if (reserva->getEstado() == EstadoReserva::CANCELADA) continue;

        if (ultimoFin < reserva->getFechaInicio()) {
            HorarioDisponible horario;
            horario.inicio = ultimoFin;
            horario.fin = reserva->getFechaInicio();
            horario.duracionMinutos = ultimoFin.secsTo(reserva->getFechaInicio()) / 60;
            horariosDisponibles.append(horario);
        }

        ultimoFin = qMax(ultimoFin, reserva->getFechaFin());
    }

    if (ultimoFin < finJornada) {
        HorarioDisponible horario;
        horario.inicio = ultimoFin;
        horario.fin = finJornada;
        horario.duracionMinutos = ultimoFin.secsTo(finJornada) / 60;
        horariosDisponibles.append(horario);
    }

    qDeleteAll(reservas);

    return horariosDisponibles;
}

bool ReservaService::validarReserva(const DatosReserva& datos, QString& mensajeError) {
    Usuario* usuario = usuarioRepository.buscarPorId(datos.idUsuario);
    if (!usuario) {
        mensajeError = "Usuario no encontrado";
        return false;
    }
    delete usuario;

    Quirofano* quirofano = quirofanoRepository.buscarPorId(datos.idQuirofano);
    if (!quirofano) {
        mensajeError = "Quirofano no encontrado";
        return false;
    }
    delete quirofano;

    if (datos.fechaInicio >= datos.fechaFin) {
        mensajeError = "La fecha de inicio debe ser anterior a la fecha de fin";
        return false;
    }

    if (datos.fechaInicio < QDateTime::currentDateTime()) {
        mensajeError = "La fecha de inicio no puede ser en el pasado";
        return false;
    }

    int duracionMinutos = datos.fechaInicio.secsTo(datos.fechaFin) / 60;
    if (duracionMinutos < 30) {
        mensajeError = "La duracion minima de una reserva es 30 minutos";
        return false;
    }
    if (duracionMinutos > 480) {
        mensajeError = "La duracion maxima de una reserva es 8 horas";
        return false;
    }

    if (existeConflicto(datos.idQuirofano, datos.fechaInicio, datos.fechaFin)) {
        mensajeError = "El horario solicitado tiene conflictos con otras reservas";
        return false;
    }

    return true;
}

bool ReservaService::existeConflicto(int idQuirofano, const QDateTime& inicio,
                                     const QDateTime& fin) {
    return !quirofanoRepository.estaDisponible(idQuirofano, inicio, fin);
}

HorarioSugerido ReservaService::validarYSugerirHorario(
    int idQuirofano,
    const QDateTime& inicio,
    const QDateTime& fin)
{
    QVector<Reserva*> reservas = listarReservasPorQuirofano(idQuirofano);
    
    HorarioSugerido resultado = sugerenciaAgenda.validarYSugerir(
        idQuirofano, inicio, fin, reservas
    );
    
    qDeleteAll(reservas);
    
    return resultado;
}

HorarioSugerido ReservaService::encontrarProximoHorarioDisponible(
    int idQuirofano,
    const QDateTime& inicioDeseado,
    int duracionMinutos)
{
    QVector<Reserva*> reservas = listarReservasPorQuirofano(idQuirofano);
    
    HorarioSugerido resultado = sugerenciaAgenda.encontrarProximoDisponible(
        idQuirofano, inicioDeseado, duracionMinutos, reservas
    );
    
    qDeleteAll(reservas);
    
    return resultado;
}

QVector<Reserva*> ReservaService::listarReservasPorQuirofanoYFecha(int idQuirofano, const QDate& fecha) {
    QVector<Reserva*> reservas;
    QSqlQuery query(database);
    
    QDateTime inicioDia(fecha, QTime(0, 0, 0));
    QDateTime finDia(fecha, QTime(23, 59, 59));
    
    query.prepare(
        "SELECT * FROM reservan "
        "WHERE id_quirofano = ? AND fecha_inicio BETWEEN ? AND ? "
        "AND estado_reserva != 'CANCELADA' "
        "ORDER BY fecha_inicio"
    );
    query.addBindValue(idQuirofano);
    query.addBindValue(inicioDia);
    query.addBindValue(finDia);
    
    if (query.exec()) {
        while (query.next()) {
            reservas.append(mapearReserva(query));
        }
    }
    
    return reservas;
}

Reserva* ReservaService::mapearReserva(const QSqlQuery& query) {
    Reserva* reserva = new Reserva(
        query.value("id_usuario").toInt(),
        query.value("id_quirofano").toInt(),
        query.value("fecha_inicio").toDateTime(),
        query.value("fecha_fin").toDateTime()
        );

    reserva->setId(query.value("id_reserva").toInt());
    reserva->setMotivoCirugia(query.value("motivo_cirugia").toString());
    reserva->setEstado(Reserva::stringToEstado(query.value("estado_reserva").toString()));

    return reserva;
}

bool ReservaService::guardarReserva(Reserva& reserva) {
    QSqlQuery query(database);

    query.prepare(
        "INSERT INTO reservan (id_usuario, id_quirofano, fecha_inicio, fecha_fin, "
        "motivo_cirugia, estado_reserva) VALUES (?, ?, ?, ?, ?, ?)"
        );
    query.addBindValue(reserva.getIdUsuario());
    query.addBindValue(reserva.getIdQuirofano());
    query.addBindValue(reserva.getFechaInicio());
    query.addBindValue(reserva.getFechaFin());
    query.addBindValue(reserva.getMotivoCirugia());
    query.addBindValue(Reserva::estadoToString(reserva.getEstado()));

    if (!query.exec()) {
        qCritical() << "[ERROR] Al guardar reserva:" << query.lastError().text();
        return false;
    }

    reserva.setId(query.lastInsertId().toInt());
    return true;
}
