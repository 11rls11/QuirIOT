#ifndef SUGERENCIAAGENDA_H
#define SUGERENCIAAGENDA_H

#include <QDateTime>
#include <QString>
#include <QVector>

enum class TipoSugerencia {
    DISPONIBLE,           // Horario completamente libre
    ADVERTENCIA_INICIO,   // Menos de 30 min después de la anterior
    ADVERTENCIA_FIN,      // Menos de 30 min antes de la siguiente
    ADVERTENCIA_AMBOS,    // Problemas en inicio y fin
    NO_DISPONIBLE        // Horario ocupado
};

struct HorarioSugerido {
    QDateTime inicio;
    QDateTime fin;
    TipoSugerencia tipo;
    QString mensaje;
    QDateTime inicioSugerido;
    QDateTime finSugerido;
    int minutosAntesAnterior;
    int minutosDespuesSiguiente;
    bool requiereAjuste;
};

class SugerenciaAgenda {
public:
    SugerenciaAgenda();
    
    HorarioSugerido validarYSugerir(
        int idQuirofano,
        const QDateTime& inicioDeseado,
        const QDateTime& finDeseado,
        const QVector<class Reserva*>& reservasExistentes
    );
    
    HorarioSugerido encontrarProximoDisponible(
        int idQuirofano,
        const QDateTime& inicioDeseado,
        int duracionMinutos,
        const QVector<Reserva*>& reservasExistentes
    );
    
    static const int MINUTOS_SANITIZACION = 30;
    static const int HORA_INICIO_JORNADA = 7;
    static const int HORA_FIN_JORNADA = 19;
    
private:
    bool tieneConflictoSanitizacion(
        const QDateTime& inicio,
        const QDateTime& fin,
        const QVector<Reserva*>& reservas,
        int& minutosAntesAnterior,
        int& minutosDespuesSiguiente
    );
    
    QString generarMensajeSugerencia(TipoSugerencia tipo, int minutosAntes, int minutosDespues);
    
    QDateTime ajustarHorarioInicio(
        const QDateTime& inicioDeseado,
        const Reserva* reservaAnterior
    );
    
    QDateTime ajustarHorarioFin(
        const QDateTime& finDeseado,
        const Reserva* reservaSiguiente
    );
};

#endif // SUGERENCIAAGENDA_H
