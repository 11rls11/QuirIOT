#include "SugerenciaAgenda.h"
#include "Reserva.h"
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
    
    // Verificar si hay conflicto directo con otra reserva
    for (const Reserva* reserva : reservasExistentes) {
        if (reserva->getEstado() == EstadoReserva::CANCELADA) {
            continue;
        }
        
        // Conflicto directo: se solapan los horarios
        if (reserva->tieneConflicto(inicioDeseado, finDeseado)) {
            resultado.tipo = TipoSugerencia::NO_DISPONIBLE;
            resultado.mensaje = "El horario seleccionado se solapa con otra cirugia existente";
            resultado.requiereAjuste = true;
            
            // Sugerir siguiente horario disponible
            QDateTime proximoInicio = reserva->getFechaFin().addSecs(MINUTOS_SANITIZACION * 60);
            resultado.inicioSugerido = proximoInicio;
            resultado.finSugerido = proximoInicio.addSecs(inicioDeseado.secsTo(finDeseado));
            
            return resultado;
        }
    }
    
    // Verificar conflictos de sanitización (30 minutos)
    int minutosAntes = 0;
    int minutosDespues = 0;
    
    if (tieneConflictoSanitizacion(inicioDeseado, finDeseado, reservasExistentes, 
                                   minutosAntes, minutosDespues)) {
        resultado.minutosAntesAnterior = minutosAntes;
        resultado.minutosDespuesSiguiente = minutosDespues;
        resultado.requiereAjuste = true;
        
        // Determinar tipo de advertencia
        if (minutosAntes > 0 && minutosAntes < MINUTOS_SANITIZACION && 
            minutosDespues > 0 && minutosDespues < MINUTOS_SANITIZACION) {
            resultado.tipo = TipoSugerencia::ADVERTENCIA_AMBOS;
        } else if (minutosAntes > 0 && minutosAntes < MINUTOS_SANITIZACION) {
            resultado.tipo = TipoSugerencia::ADVERTENCIA_INICIO;
        } else if (minutosDespues > 0 && minutosDespues < MINUTOS_SANITIZACION) {
            resultado.tipo = TipoSugerencia::ADVERTENCIA_FIN;
        }
        
        resultado.mensaje = generarMensajeSugerencia(resultado.tipo, minutosAntes, minutosDespues);
        
        // Calcular horarios sugeridos ajustados
        Reserva* reservaAnterior = nullptr;
        Reserva* reservaSiguiente = nullptr;
        
        for (Reserva* reserva : reservasExistentes) {
            if (reserva->getEstado() == EstadoReserva::CANCELADA) continue;
            
            if (reserva->getFechaFin() <= inicioDeseado) {
                if (!reservaAnterior || reserva->getFechaFin() > reservaAnterior->getFechaFin()) {
                    reservaAnterior = reserva;
                }
            }
            
            if (reserva->getFechaInicio() >= finDeseado) {
                if (!reservaSiguiente || reserva->getFechaInicio() < reservaSiguiente->getFechaInicio()) {
                    reservaSiguiente = reserva;
                }
            }
        }
        
        // Ajustar horarios según sea necesario
        if (reservaAnterior && minutosAntes < MINUTOS_SANITIZACION) {
            resultado.inicioSugerido = ajustarHorarioInicio(inicioDeseado, reservaAnterior);
        } else {
            resultado.inicioSugerido = inicioDeseado;
        }
        
        if (reservaSiguiente && minutosDespues < MINUTOS_SANITIZACION) {
            resultado.finSugerido = ajustarHorarioFin(finDeseado, reservaSiguiente);
        } else {
            resultado.finSugerido = finDeseado;
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
    
    for (const Reserva* reserva : reservas) {
        if (reserva->getEstado() == EstadoReserva::CANCELADA) {
            continue;
        }
        
        // Calcular minutos entre fin de reserva anterior y inicio deseado
        if (reserva->getFechaFin() <= inicio) {
            int minutos = reserva->getFechaFin().secsTo(inicio) / 60;
            if (minutos < minutosAntesAnterior) {
                minutosAntesAnterior = minutos;
            }
            if (minutos < MINUTOS_SANITIZACION) {
                hayConflicto = true;
            }
        }
        
        // Calcular minutos entre fin deseado e inicio de siguiente reserva
        if (reserva->getFechaInicio() >= fin) {
            int minutos = fin.secsTo(reserva->getFechaInicio()) / 60;
            if (minutos < minutosDespuesSiguiente) {
                minutosDespuesSiguiente = minutos;
            }
            if (minutos < MINUTOS_SANITIZACION) {
                hayConflicto = true;
            }
        }
    }
    
    // Si no hay reservas cercanas, no hay conflicto
    if (minutosAntesAnterior == 999999) minutosAntesAnterior = 0;
    if (minutosDespuesSiguiente == 999999) minutosDespuesSiguiente = 0;
    
    return hayConflicto;
}

QString SugerenciaAgenda::generarMensajeSugerencia(
    TipoSugerencia tipo,
    int minutosAntes,
    int minutosDespues)
{
    switch (tipo) {
        case TipoSugerencia::ADVERTENCIA_INICIO:
            return QString("ADVERTENCIA: Solo hay %1 minutos entre la cirugia anterior y esta. "
                          "Se requieren minimo 30 minutos para sanitizacion. "
                          "Se sugiere un horario alternativo.")
                   .arg(minutosAntes);
                   
        case TipoSugerencia::ADVERTENCIA_FIN:
            return QString("ADVERTENCIA: Solo hay %1 minutos entre esta cirugia y la siguiente. "
                          "Se requieren minimo 30 minutos para sanitizacion. "
                          "Se sugiere un horario alternativo.")
                   .arg(minutosDespues);
                   
        case TipoSugerencia::ADVERTENCIA_AMBOS:
            return QString("ADVERTENCIA: Tiempo insuficiente antes (%1 min) y despues (%2 min) "
                          "de esta cirugia. Se requieren 30 minutos de sanitizacion. "
                          "Se sugiere un horario alternativo.")
                   .arg(minutosAntes).arg(minutosDespues);
                   
        case TipoSugerencia::NO_DISPONIBLE:
            return "El horario seleccionado NO esta disponible (se solapa con otra cirugia)";
            
        default:
            return "Horario disponible";
    }
}

QDateTime SugerenciaAgenda::ajustarHorarioInicio(
    const QDateTime& inicioDeseado,
    const Reserva* reservaAnterior)
{
    if (!reservaAnterior) {
        return inicioDeseado;
    }
    
    // Sugerir 30 minutos después del fin de la reserva anterior
    QDateTime inicioSugerido = reservaAnterior->getFechaFin().addSecs(MINUTOS_SANITIZACION * 60);
    
    return inicioSugerido;
}

QDateTime SugerenciaAgenda::ajustarHorarioFin(
    const QDateTime& finDeseado,
    const Reserva* reservaSiguiente)
{
    if (!reservaSiguiente) {
        return finDeseado;
    }
    
    // Sugerir terminar 30 minutos antes del inicio de la siguiente reserva
    QDateTime finSugerido = reservaSiguiente->getFechaInicio().addSecs(-MINUTOS_SANITIZACION * 60);
    
    return finSugerido;
}

HorarioSugerido SugerenciaAgenda::encontrarProximoDisponible(
    int idQuirofano,
    const QDateTime& inicioDeseado,
    int duracionMinutos,
    const QVector<Reserva*>& reservasExistentes)
{
    Q_UNUSED(idQuirofano);
    
    // Ordenar reservas por fecha de inicio
    QVector<Reserva*> reservasOrdenadas = reservasExistentes;
    std::sort(reservasOrdenadas.begin(), reservasOrdenadas.end(),
              [](const Reserva* a, const Reserva* b) {
                  return a->getFechaInicio() < b->getFechaInicio();
              });
    
    QDateTime candidato = inicioDeseado;
    QDateTime finCandidato = candidato.addSecs(duracionMinutos * 60);
    
    // Buscar primer slot disponible
    for (int intento = 0; intento < 20; intento++) {  // Máximo 20 intentos
        HorarioSugerido validacion = validarYSugerir(
            idQuirofano, candidato, finCandidato, reservasOrdenadas
        );
        
        if (validacion.tipo == TipoSugerencia::DISPONIBLE) {
            return validacion;
        }
        
        // Mover al siguiente slot
        if (validacion.inicioSugerido > candidato) {
            candidato = validacion.inicioSugerido;
            finCandidato = candidato.addSecs(duracionMinutos * 60);
        } else {
            // Avanzar 30 minutos
            candidato = candidato.addSecs(30 * 60);
            finCandidato = candidato.addSecs(duracionMinutos * 60);
        }
    }
    
    // Si no encontró nada, retornar el último intento
    return validarYSugerir(idQuirofano, candidato, finCandidato, reservasOrdenadas);
}
