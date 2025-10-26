#include "Actuador.h"

Actuador::Actuador() : id(0), idQuirofano(0), tipo(TipoActuador::VENTILADOR), encendido(false) {}

Actuador::Actuador(int idQuirofano, const QString& nombre, TipoActuador tipo)
    : id(0), idQuirofano(idQuirofano), nombre(nombre), tipo(tipo), encendido(false) {}

int Actuador::getId() const { return id; }
int Actuador::getIdQuirofano() const { return idQuirofano; }
QString Actuador::getNombre() const { return nombre; }
TipoActuador Actuador::getTipo() const { return tipo; }
bool Actuador::estaEncendido() const { return encendido; }

void Actuador::setId(int id) { this->id = id; }
void Actuador::setEncendido(bool encendido) { this->encendido = encendido; }

QString Actuador::tipoToString(TipoActuador tipo) {
    switch (tipo) {
    case TipoActuador::LUZ_UV: return "LUZ_UV";
    case TipoActuador::VENTILADOR: return "VENTILADOR";
    default: return "DESCONOCIDO";
    }
}
