#include "Reserva.h"

Reserva::Reserva() : id(0), idUsuario(0), idQuirofano(0), estado(EstadoReserva::PROGRAMADA) {}

Reserva::Reserva(int idUsuario, int idQuirofano, const QDateTime& inicio, const QDateTime& fin)
    : id(0), idUsuario(idUsuario), idQuirofano(idQuirofano),
    fechaInicio(inicio), fechaFin(fin), estado(EstadoReserva::PROGRAMADA),
    fechaCreacion(QDateTime::currentDateTime()) {}

int Reserva::getId() const { return id; }
int Reserva::getIdUsuario() const { return idUsuario; }
int Reserva::getIdQuirofano() const { return idQuirofano; }
QDateTime Reserva::getFechaInicio() const { return fechaInicio; }
QDateTime Reserva::getFechaFin() const { return fechaFin; }
QString Reserva::getMotivoCirugia() const { return motivoCirugia; }
EstadoReserva Reserva::getEstado() const { return estado; }
QDateTime Reserva::getFechaCreacion() const { return fechaCreacion; }

void Reserva::setId(int id) { this->id = id; }
void Reserva::setMotivoCirugia(const QString& motivo) { this->motivoCirugia = motivo; }
void Reserva::setEstado(EstadoReserva estado) { this->estado = estado; }

bool Reserva::tieneConflicto(const QDateTime& inicio, const QDateTime& fin) const {
    return !(fechaFin <= inicio || fechaInicio >= fin);
}

int Reserva::getDuracionMinutos() const {
    return fechaInicio.secsTo(fechaFin) / 60;
}

bool Reserva::esValida() const {
    return idUsuario > 0 && idQuirofano > 0 && fechaInicio.isValid() &&
           fechaFin.isValid() && fechaInicio < fechaFin;
}

QString Reserva::estadoToString(EstadoReserva estado) {
    switch (estado) {
    case EstadoReserva::PROGRAMADA: return "PROGRAMADA";
    case EstadoReserva::EN_CURSO: return "EN_CURSO";
    case EstadoReserva::COMPLETADA: return "COMPLETADA";
    case EstadoReserva::CANCELADA: return "CANCELADA";
    default: return "DESCONOCIDO";
    }
}

EstadoReserva Reserva::stringToEstado(const QString& str) {
    if (str == "PROGRAMADA") return EstadoReserva::PROGRAMADA;
    if (str == "EN_CURSO") return EstadoReserva::EN_CURSO;
    if (str == "COMPLETADA") return EstadoReserva::COMPLETADA;
    if (str == "CANCELADA") return EstadoReserva::CANCELADA;
    return EstadoReserva::PROGRAMADA;
}
