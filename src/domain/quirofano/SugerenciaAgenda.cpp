#include "SugerenciaAgenda.h"
#include "src/domain/quirofano/Reserva.h"
#include <algorithm>
#include <QDebug>

SugerenciaAgenda::SugerenciaAgenda() {}

HorarioSugerido SugerenciaAgenda::validarYSugerir(
    int idQuirofano,
    const QDateTime& inicioDeseado,
    const QDateTime& finDeseado,
    const QVector<Reserva*>& reservasExistentes)
{
    Q_UNUSED(idQuirofano);

    HorarioSugerido resultado;
    resultado.inicio = inicioDeseado;
    resultado.fin = finDeseado;
    resultado.tipo = TipoSugerencia::DISPONIBLE;
    resultado.requiereAjuste = false;
    resultado.minutosAntesAnterior = 0;
    resultado.minutosDespuesSiguiente = 0;

    for (const Reserva* reserva : reservasExistentes) {
        if (reserva->getEstado() == EstadoReserva::CANCELADA) {
            continue;
        }

        if (reserva->tieneConflicto(inicioDeseado, finDeseado)) {
            resultado.tipo = TipoSugerencia::NO_DISPONIBLE;
            resultado.mensaje = "El horario seleccionado se solapa con otra cirugia existente";
            resultado.requiereAjuste = true;

            // Sugerir siguiente horario (30 min después del fin de la conflictiva)
            QDateTime proximoInicio = reserva->getFechaFin().addSecs(MINUTOS_SANITIZACION * 60);
            resultado.inicioSugerido = proximoInicio;
            resultado.finSugerido = proximoInicio.addSecs(inicioDeseado.secsTo(finDeseado));

            return resultado;
        }
    }

    int minutosAntes = 0;
    int minutosDespues = 0;

    if (tieneConflictoSanitizacion(inicioDeseado, finDeseado, reservasExistentes,
                                   minutosAntes, minutosDespues)) {
        resultado.minutosAntesAnterior = minutosAntes;
        resultado.minutosDespuesSiguiente = minutosDespues;
        resultado.requiereAjuste = true;

        bool problemaInicio = (minutosAntes >= 0 && minutosAntes < MINUTOS_SANITIZACION);
        bool problemaFin = (minutosDespues >= 0 && minutosDespues < MINUTOS_SANITIZACION);

        if (problemaInicio && problemaFin) {
            resultado.tipo = TipoSugerencia::ADVERTENCIA_AMBOS;
        } else if (problemaInicio) {
            resultado.tipo = TipoSugerencia::ADVERTENCIA_INICIO;
        } else if (problemaFin) {
            resultado.tipo = TipoSugerencia::ADVERTENCIA_FIN;
        }

        resultado.mensaje = generarMensajeSugerencia(resultado.tipo, minutosAntes, minutosDespues);

        Reserva* reservaAnterior = nullptr;
        Reserva* reservaSiguiente = nullptr;

        int minDiffAntes = 999999;
        int minDiffDespues = 999999;

        for (Reserva* reserva : reservasExistentes) {
            if (reserva->getEstado() == EstadoReserva::CANCELADA) continue;

            if (reserva->getFechaFin() <= inicioDeseado) {
                int diff = reserva->getFechaFin().secsTo(inicioDeseado);
                if (diff < minDiffAntes) {
                    minDiffAntes = diff;
                    reservaAnterior = reserva;
                }
            }

            if (reserva->getFechaInicio() >= finDeseado) {
                int diff = finDeseado.secsTo(reserva->getFechaInicio());
                if (diff < minDiffDespues) {
                    minDiffDespues = diff;
                    reservaSiguiente = reserva;
                }
            }
        }

        if (reservaAnterior && problemaInicio) {
            resultado.inicioSugerido = ajustarHorarioInicio(inicioDeseado, reservaAnterior);
        } else {
            resultado.inicioSugerido = inicioDeseado;
        }

        if (reservaSiguiente && problemaFin) {
            resultado.finSugerido = ajustarHorarioFin(finDeseado, reservaSiguiente);
        } else {
            if (resultado.inicioSugerido != inicioDeseado) {
                qint64 duracion = inicioDeseado.secsTo(finDeseado);
                resultado.finSugerido = resultado.inicioSugerido.addSecs(duracion);
            } else {
                resultado.finSugerido = finDeseado;
            }
        }

    } else {
        resultado.mensaje = "Horario disponible sin conflictos";
        resultado.inicioSugerido = inicioDeseado;
        resultado.finSugerido = finDeseado;
    }

    return resultado;
}

bool SugerenciaAgenda::tieneConflictoSanitizacion(
    const QDateTime& inicio,
    const QDateTime& fin,
    const QVector<Reserva*>& reservas,
    int& minutosAntesAnterior,
    int& minutosDespuesSiguiente)
{
    minutosAntesAnterior = 999999;
    minutosDespuesSiguiente = 999999;
    bool hayConflicto = false;

    qDebug() << "\n--- INICIO DEBUG CONFLICTO SANITIZACION ---";
    qDebug() << "Buscando conflicto para Inicio:" << inicio.toString("dd/MM/yyyy HH:mm:ss");

    for (const Reserva* reserva : reservas) {
        if (reserva->getEstado() == EstadoReserva::CANCELADA) {
            continue;
        }

        if (reserva->getFechaFin() <= inicio) {

            qDebug() << "--------------------------------------------------";
            qDebug() << "Encontrada Reserva Anterior ID:" << reserva->getId();
            qDebug() << "Fin Reserva:" << reserva->getFechaFin().toString("dd/MM/yyyy HH:mm:ss");
            qDebug() << "Inicio Deseado:" << inicio.toString("dd/MM/yyyy HH:mm:ss");

            int minutos = reserva->getFechaFin().secsTo(inicio) / 60;
            qDebug() << "Diferencia calculada (minutos):" << minutos;

            if (minutos < minutosAntesAnterior) {
                minutosAntesAnterior = minutos;
                qDebug() << "-> Nuevo Minimo 'Antes' encontrado:" << minutosAntesAnterior;
            }
        }

        if (reserva->getFechaInicio() >= fin) {
            int minutos = fin.secsTo(reserva->getFechaInicio()) / 60;
            if (minutos < minutosDespuesSiguiente) {
                minutosDespuesSiguiente = minutos;
            }
        }
    }

    qDebug() << "--- RESULTADO FINAL ---";
    qDebug() << "Minimo Antes:" << minutosAntesAnterior;
    qDebug() << "Limite Sanitizacion:" << MINUTOS_SANITIZACION;

    if (minutosAntesAnterior < MINUTOS_SANITIZACION) {
        qDebug() << "-> CONFLICTO DETECTADO (Antes)";
        hayConflicto = true;
    }
    if (minutosDespuesSiguiente < MINUTOS_SANITIZACION) {
        hayConflicto = true;
    }
    qDebug() << "--------------------------------------------------\n";

    return hayConflicto;
}

QString SugerenciaAgenda::generarMensajeSugerencia(
    TipoSugerencia tipo,
    int minutosAntes,
    int minutosDespues)
{
    switch (tipo) {
    case TipoSugerencia::ADVERTENCIA_INICIO:
        return QString("ADVERTENCIA: Solo hay %1 minutos desde la anterior. Minimo 30.")
            .arg(minutosAntes);

    case TipoSugerencia::ADVERTENCIA_FIN:
        return QString("ADVERTENCIA: Solo hay %1 minutos hasta la siguiente. Minimo 30.")
            .arg(minutosDespues);

    case TipoSugerencia::ADVERTENCIA_AMBOS:
        return QString("ADVERTENCIA: Tiempo insuficiente antes (%1 min) y despues (%2 min).")
            .arg(minutosAntes).arg(minutosDespues);

    case TipoSugerencia::NO_DISPONIBLE:
        return "Horario no disponible (solapamiento).";

    default:
        return "Horario disponible";
    }
}

QDateTime SugerenciaAgenda::ajustarHorarioInicio(
    const QDateTime& inicioDeseado,
    const Reserva* reservaAnterior)
{
    if (!reservaAnterior) return inicioDeseado;
    return reservaAnterior->getFechaFin().addSecs(MINUTOS_SANITIZACION * 60);
}

QDateTime SugerenciaAgenda::ajustarHorarioFin(
    const QDateTime& finDeseado,
    const Reserva* reservaSiguiente)
{
    if (!reservaSiguiente) return finDeseado;
    return reservaSiguiente->getFechaInicio().addSecs(-MINUTOS_SANITIZACION * 60);
}

HorarioSugerido SugerenciaAgenda::encontrarProximoDisponible(
    int idQuirofano,
    const QDateTime& inicioDeseado,
    int duracionMinutos,
    const QVector<Reserva*>& reservasExistentes)
{
    Q_UNUSED(idQuirofano);

    QVector<Reserva*> reservasOrdenadas = reservasExistentes;
    std::sort(reservasOrdenadas.begin(), reservasOrdenadas.end(),
              [](const Reserva* a, const Reserva* b) {
                  return a->getFechaInicio() < b->getFechaInicio();
              });

    QDateTime candidato = inicioDeseado;
    if (candidato < QDateTime::currentDateTime()) {
        candidato = QDateTime::currentDateTime().addSecs(60);
    }

    QDateTime finCandidato = candidato.addSecs(duracionMinutos * 60);

    for (int i = 0; i < 50; i++) {
        HorarioSugerido val = validarYSugerir(idQuirofano, candidato, finCandidato, reservasOrdenadas);

        if (val.tipo == TipoSugerencia::DISPONIBLE) {
            return val;
        }

        if (val.inicioSugerido > candidato) {
            candidato = val.inicioSugerido;
        } else {
            candidato = candidato.addSecs(30 * 60);
        }
        finCandidato = candidato.addSecs(duracionMinutos * 60);
    }

    return validarYSugerir(idQuirofano, candidato, finCandidato, reservasOrdenadas);
}
