#ifndef DEMOCONTROLLER_H
#define DEMOCONTROLLER_H

#include <QString>
#include <QVector>
#include <QDateTime>
#include <QMap>

enum class TipoPrueba {
    NAVEGACION_MENU,
    REGISTRO_USUARIO,
    AGENDAR_CIRUGIA,
    SISTEMA_LIMPIEZA,
    CONSULTAR_HORARIOS
};

enum class PasoPrueba {
    PASO_1_INICIO,
    PASO_2_SELECCION,
    PASO_3_DATOS,
    PASO_4_CONFIRMACION,
    PASO_5_VERIFICACION
};

struct ResultadoPrueba {
    TipoPrueba tipo;
    bool completada;
    int intentos;
    QDateTime fechaInicio;
    QDateTime fechaFin;
    int duracionSegundos;
    QString feedback;
    QMap<PasoPrueba, bool> pasosCompletados;
    int totalPasos;
    int pasosExitosos;
};

struct AgendaDemoCita {
    int id;
    QString motivoPrueba;
    QDateTime inicioDemo;
    QDateTime finDemo;
    bool agendadaCorrectamente;
};

class DemoController {
public:
    DemoController();
    ~DemoController();
    
    // Métodos principales
    bool iniciarModoDemo();
    void finalizarModoDemo();
    bool estaEnModoDemo() const;
    
    // Pruebas específicas (criterios de aceptación)
    bool iniciarPruebaNavegacion();
    bool registrarPasoNavegacion(int opcionSeleccionada);
    bool verificarNavegacionCompleta();
    
    bool iniciarPruebaAgendarCita();
    bool validarDatosAgenda(int idQuirofano, const QString& fechaInicio, const QString& fechaFin);
    bool confirmarAgendaDemo();
    
    bool iniciarPruebaSistemaLimpieza();
    bool validarActivacionSistema(int idQuirofano);
    bool confirmarEntendimientoSistema();
    
    // Registro y evaluación
    void registrarAccionDemo(TipoPrueba tipo, bool exito);
    void registrarPaso(TipoPrueba tipo, PasoPrueba paso, bool exito);
    QVector<ResultadoPrueba> obtenerResultados() const;
    int calcularPuntajeTotal() const;
    QString generarReporteCapacitacion() const;
    
    // Datos de prueba
    AgendaDemoCita* obtenerCitaDemo() const;
    ResultadoPrueba* obtenerPruebaActual();
    void resetearDatosDemo();
    
    // Estado de prueba actual
    TipoPrueba getPruebaActual() const;
    bool hayPruebaEnCurso() const;
    
private:
    bool modoDemo;
    QVector<ResultadoPrueba> resultados;
    AgendaDemoCita* citaDemo;
    QDateTime inicioSesionDemo;
    
    // Control de prueba actual
    ResultadoPrueba* pruebaEnCurso;
    TipoPrueba pruebaActual;
    int pasoActual;
    
    QString generarFeedback(TipoPrueba tipo, bool exito, int pasosCompletados, int totalPasos) const;
    int obtenerPuntajePrueba(TipoPrueba tipo, int pasosCompletados, int totalPasos) const;
    void finalizarPruebaActual(bool exito);
};

#endif // DEMOCONTROLLER_H
