#ifndef ACTUADOR_H
#define ACTUADOR_H

#include <QString>

enum class TipoActuador {
    LUZ_UV,
    VENTILADOR
};

class Actuador {
public:
    Actuador();
    Actuador(int idQuirofano, const QString& nombre, TipoActuador tipo);

    int getId() const;
    int getIdQuirofano() const;
    QString getNombre() const;
    TipoActuador getTipo() const;
    bool estaEncendido() const;

    void setId(int id);
    void setEncendido(bool encendido);

    static QString tipoToString(TipoActuador tipo);

private:
    int id;
    int idQuirofano;
    QString nombre;
    TipoActuador tipo;
    bool encendido;
};

#endif
