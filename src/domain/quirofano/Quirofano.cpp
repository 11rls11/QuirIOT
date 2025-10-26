#include "Quirofano.h"

Quirofano::Quirofano() : id(0), capacidad(10), estado(EstadoQuirofano::DISPONIBLE) {}

Quirofano::Quirofano(int id, const QString& nombre)
    : id(id), nombre(nombre), capacidad(10), estado(EstadoQuirofano::DISPONIBLE) {}

int Quirofano::getId() const { return id; }
QString Quirofano::getNombre() const { return nombre; }
int Quirofano::getCapacidad() const { return capacidad; }
EstadoQuirofano Quirofano::getEstado() const { return estado; }
QString Quirofano::getDisponibilidad() const { return disponibilidad; }

void Quirofano::setId(int id) { this->id = id; }
void Quirofano::setNombre(const QString& nombre) { this->nombre = nombre; }
void Quirofano::setCapacidad(int capacidad) { this->capacidad = capacidad; }
void Quirofano::setEstado(EstadoQuirofano estado) { this->estado = estado; }
void Quirofano::setDisponibilidad(const QString& disp) { this->disponibilidad = disp; }

QString Quirofano::estadoToString(EstadoQuirofano estado) {
    switch (estado) {
    case EstadoQuirofano::DISPONIBLE: return "DISPONIBLE";
    case EstadoQuirofano::OCUPADO: return "OCUPADO";
    case EstadoQuirofano::MANTENIMIENTO: return "MANTENIMIENTO";
    case EstadoQuirofano::SANITIZANDO: return "SANITIZANDO";
    default: return "DESCONOCIDO";
    }
}

EstadoQuirofano Quirofano::stringToEstado(const QString& str) {
    if (str == "DISPONIBLE") return EstadoQuirofano::DISPONIBLE;
    if (str == "OCUPADO") return EstadoQuirofano::OCUPADO;
    if (str == "MANTENIMIENTO") return EstadoQuirofano::MANTENIMIENTO;
    if (str == "SANITIZANDO") return EstadoQuirofano::SANITIZANDO;
    return EstadoQuirofano::DISPONIBLE;
}

bool Quirofano::estaDisponible() const {
    return estado == EstadoQuirofano::DISPONIBLE;
}
