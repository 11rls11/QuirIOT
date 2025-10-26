#include "Sensor.h"

Sensor::Sensor() : id(0), idQuirofano(0), tipo(TipoSensor::HUM_TEMP), activo(true) {}

Sensor::Sensor(int idQuirofano, const QString& nombre, TipoSensor tipo)
    : id(0), idQuirofano(idQuirofano), nombre(nombre), tipo(tipo), activo(true) {}

int Sensor::getId() const { return id; }
int Sensor::getIdQuirofano() const { return idQuirofano; }
QString Sensor::getNombre() const { return nombre; }
TipoSensor Sensor::getTipo() const { return tipo; }
bool Sensor::estaActivo() const { return activo; }

void Sensor::setId(int id) { this->id = id; }
void Sensor::setActivo(bool activo) { this->activo = activo; }

QString Sensor::tipoToString(TipoSensor tipo) {
    switch (tipo) {
    case TipoSensor::HUM_TEMP: return "HUM_TEMP";
    case TipoSensor::CONTAMINACION: return "CONTAMINACION";
    default: return "DESCONOCIDO";
    }
}
