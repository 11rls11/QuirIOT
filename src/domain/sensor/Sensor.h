#ifndef SENSOR_H
#define SENSOR_H

#include <QString>

enum class TipoSensor {
    HUM_TEMP,
    CONTAMINACION
};

class Sensor {
public:
    Sensor();
    Sensor(int idQuirofano, const QString& nombre, TipoSensor tipo);

    int getId() const;
    int getIdQuirofano() const;
    QString getNombre() const;
    TipoSensor getTipo() const;
    bool estaActivo() const;

    void setId(int id);
    void setActivo(bool activo);

    static QString tipoToString(TipoSensor tipo);

private:
    int id;
    int idQuirofano;
    QString nombre;
    TipoSensor tipo;
    bool activo;
};

#endif
