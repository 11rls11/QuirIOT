#ifndef QUIROFANO_H
#define QUIROFANO_H

#include <QString>
#include <QDateTime>

enum class EstadoQuirofano {
    DISPONIBLE,
    OCUPADO,
    MANTENIMIENTO,
    SANITIZANDO
};

class Quirofano {
public:
    Quirofano();
    Quirofano(int id, const QString& nombre);

    int getId() const;
    QString getNombre() const;
    int getCapacidad() const;
    EstadoQuirofano getEstado() const;
    QString getDisponibilidad() const;

    void setId(int id);
    void setNombre(const QString& nombre);
    void setCapacidad(int capacidad);
    void setEstado(EstadoQuirofano estado);
    void setDisponibilidad(const QString& disp);

    static QString estadoToString(EstadoQuirofano estado);
    static EstadoQuirofano stringToEstado(const QString& str);
    bool estaDisponible() const;

private:
    int id;
    QString nombre;
    int capacidad;
    EstadoQuirofano estado;
    QString disponibilidad;
};

#endif
