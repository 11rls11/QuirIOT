#ifndef RESERVA_H
#define RESERVA_H

#include <QString>
#include <QDateTime>

enum class EstadoReserva {
    PROGRAMADA,
    EN_CURSO,
    COMPLETADA,
    CANCELADA
};

class Reserva {
public:
    Reserva();
    Reserva(int idUsuario, int idQuirofano, const QDateTime& inicio, const QDateTime& fin);

    int getId() const;
    int getIdUsuario() const;
    int getIdQuirofano() const;
    QDateTime getFechaInicio() const;
    QDateTime getFechaFin() const;
    QString getMotivoCirugia() const;
    EstadoReserva getEstado() const;
    QDateTime getFechaCreacion() const;

    void setId(int id);
    void setMotivoCirugia(const QString& motivo);
    void setEstado(EstadoReserva estado);

    bool tieneConflicto(const QDateTime& inicio, const QDateTime& fin) const;
    int getDuracionMinutos() const;
    bool esValida() const;

    static QString estadoToString(EstadoReserva estado);
    static EstadoReserva stringToEstado(const QString& str);

private:
    int id;
    int idUsuario;
    int idQuirofano;
    QDateTime fechaInicio;
    QDateTime fechaFin;
    QString motivoCirugia;
    EstadoReserva estado;
    QDateTime fechaCreacion;
};

#endif
