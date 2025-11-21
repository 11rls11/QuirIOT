#include "DemoController.h"
#include <QDebug>

DemoController::DemoController() 
    : modoDemo(false), citaDemo(nullptr), pruebaEnCurso(nullptr), pasoActual(0) {}

DemoController::~DemoController() {
    resetearDatosDemo();
}

bool DemoController::iniciarModoDemo() {
    qInfo() << "[DEMO] Iniciando modo demo de capacitación";
    modoDemo = true;
    inicioSesionDemo = QDateTime::currentDateTime();
    resultados.clear();
    resetearDatosDemo();
    
    qInfo() << "[DEMO] Modo demo activado exitosamente";
    return true;
}

void DemoController::finalizarModoDemo() {
    qInfo() << "[DEMO] Finalizando modo demo";
    
    // Finalizar prueba en curso si existe
    if (pruebaEnCurso) {
        finalizarPruebaActual(false);
    }
    
    modoDemo = false;
    QString reporte = generarReporteCapacitacion();
    qInfo() << "[DEMO] Reporte generado:\n" << reporte;
}

bool DemoController::estaEnModoDemo() const {
    return modoDemo;
}

// ============================================================
//                  PRUEBA 1: NAVEGACIÓN
// ============================================================

bool DemoController::iniciarPruebaNavegacion() {
    qInfo() << "[DEMO] Iniciando prueba de navegación";
    
    if (pruebaEnCurso) {
        finalizarPruebaActual(false);
    }
    
    pruebaEnCurso = new ResultadoPrueba();
    pruebaEnCurso->tipo = TipoPrueba::NAVEGACION_MENU;
    pruebaEnCurso->fechaInicio = QDateTime::currentDateTime();
    pruebaEnCurso->completada = false;
    pruebaEnCurso->intentos = 1;
    pruebaEnCurso->totalPasos = 3;
    pruebaEnCurso->pasosExitosos = 0;
    
    pruebaActual = TipoPrueba::NAVEGACION_MENU;
    pasoActual = 0;
    
    return true;
}

bool DemoController::registrarPasoNavegacion(int opcionSeleccionada) {
    if (!pruebaEnCurso || pruebaEnCurso->tipo != TipoPrueba::NAVEGACION_MENU) {
        return false;
    }
    
    pasoActual++;
    bool pasoExitoso = false;
    
    switch (pasoActual) {
        case 1: // Debe seleccionar opción 1 (Gestión de Quirófanos)
            pasoExitoso = (opcionSeleccionada == 1);
            if (pasoExitoso) {
                qInfo() << "[DEMO] Paso 1 completado: Accedió a Gestión de Quirófanos";
            }
            break;
            
        case 2: // Debe seleccionar opción 1 (Listar Quirófanos)
            pasoExitoso = (opcionSeleccionada == 1);
            if (pasoExitoso) {
                qInfo() << "[DEMO] Paso 2 completado: Listó quirófanos";
            }
            break;
            
        case 3: // Debe regresar (opción 0)
            pasoExitoso = (opcionSeleccionada == 0);
            if (pasoExitoso) {
                qInfo() << "[DEMO] Paso 3 completado: Regresó al menú principal";
            }
            break;
    }
    
    if (pasoExitoso) {
        pruebaEnCurso->pasosExitosos++;
        registrarPaso(TipoPrueba::NAVEGACION_MENU, static_cast<PasoPrueba>(pasoActual), true);
    } else {
        pruebaEnCurso->intentos++;
    }
    
    return pasoExitoso;
}

bool DemoController::verificarNavegacionCompleta() {
    if (!pruebaEnCurso || pruebaEnCurso->tipo != TipoPrueba::NAVEGACION_MENU) {
        return false;
    }
    
    bool completada = (pruebaEnCurso->pasosExitosos >= 3);
    finalizarPruebaActual(completada);
    
    return completada;
}

// ============================================================
//                  PRUEBA 2: AGENDAR CITA
// ============================================================

bool DemoController::iniciarPruebaAgendarCita() {
    qInfo() << "[DEMO] Iniciando prueba de agendar cita";
    
    if (pruebaEnCurso) {
        finalizarPruebaActual(false);
    }
    
    pruebaEnCurso = new ResultadoPrueba();
    pruebaEnCurso->tipo = TipoPrueba::AGENDAR_CIRUGIA;
    pruebaEnCurso->fechaInicio = QDateTime::currentDateTime();
    pruebaEnCurso->completada = false;
    pruebaEnCurso->intentos = 1;
    pruebaEnCurso->totalPasos = 4;
    pruebaEnCurso->pasosExitosos = 0;
    
    pruebaActual = TipoPrueba::AGENDAR_CIRUGIA;
    pasoActual = 0;
    
    // Crear datos de prueba
    citaDemo = new AgendaDemoCita();
    citaDemo->id = 9999;
    citaDemo->motivoPrueba = "Cirugia de prueba - Modo Demo";
    citaDemo->agendadaCorrectamente = false;
    
    return true;
}

bool DemoController::validarDatosAgenda(int idQuirofano, const QString& fechaInicio, const QString& fechaFin) {
    if (!pruebaEnCurso || pruebaEnCurso->tipo != TipoPrueba::AGENDAR_CIRUGIA) {
        return false;
    }
    
    pasoActual++;
    bool datosValidos = false;
    
    // Validar quirófano (debe ser entre 1 y 10)
    if (pasoActual == 1) {
        datosValidos = (idQuirofano >= 1 && idQuirofano <= 10);
        if (datosValidos) {
            qInfo() << "[DEMO] Paso 1: Quirófano válido seleccionado";
            pruebaEnCurso->pasosExitosos++;
            registrarPaso(TipoPrueba::AGENDAR_CIRUGIA, PasoPrueba::PASO_1_INICIO, true);
        }
    }
    
    // Validar formato de fechas
    if (pasoActual == 2) {
        QDateTime inicio = QDateTime::fromString(fechaInicio, "yyyy-MM-dd hh:mm");
        QDateTime fin = QDateTime::fromString(fechaFin, "yyyy-MM-dd hh:mm");
        
        datosValidos = inicio.isValid() && fin.isValid() && inicio < fin;
        
        if (datosValidos) {
            qInfo() << "[DEMO] Paso 2: Fechas válidas ingresadas";
            pruebaEnCurso->pasosExitosos++;
            registrarPaso(TipoPrueba::AGENDAR_CIRUGIA, PasoPrueba::PASO_2_SELECCION, true);
            
            citaDemo->inicioDemo = inicio;
            citaDemo->finDemo = fin;
        }
    }
    
    if (!datosValidos) {
        pruebaEnCurso->intentos++;
    }
    
    return datosValidos;
}

bool DemoController::confirmarAgendaDemo() {
    if (!pruebaEnCurso || pruebaEnCurso->tipo != TipoPrueba::AGENDAR_CIRUGIA) {
        return false;
    }
    
    // Paso 3: Ingresar motivo
    pruebaEnCurso->pasosExitosos++;
    registrarPaso(TipoPrueba::AGENDAR_CIRUGIA, PasoPrueba::PASO_3_DATOS, true);
    
    // Paso 4: Confirmar
    pruebaEnCurso->pasosExitosos++;
    registrarPaso(TipoPrueba::AGENDAR_CIRUGIA, PasoPrueba::PASO_4_CONFIRMACION, true);
    
    citaDemo->agendadaCorrectamente = true;
    
    bool completada = (pruebaEnCurso->pasosExitosos >= 4);
    finalizarPruebaActual(completada);
    
    return completada;
}

// ============================================================
//                  PRUEBA 3: SISTEMA LIMPIEZA
// ============================================================

bool DemoController::iniciarPruebaSistemaLimpieza() {
    qInfo() << "[DEMO] Iniciando prueba de sistema de limpieza";
    
    if (pruebaEnCurso) {
        finalizarPruebaActual(false);
    }
    
    pruebaEnCurso = new ResultadoPrueba();
    pruebaEnCurso->tipo = TipoPrueba::SISTEMA_LIMPIEZA;
    pruebaEnCurso->fechaInicio = QDateTime::currentDateTime();
    pruebaEnCurso->completada = false;
    pruebaEnCurso->intentos = 1;
    pruebaEnCurso->totalPasos = 3;
    pruebaEnCurso->pasosExitosos = 0;
    
    pruebaActual = TipoPrueba::SISTEMA_LIMPIEZA;
    pasoActual = 0;
    
    return true;
}

bool DemoController::validarActivacionSistema(int idQuirofano) {
    if (!pruebaEnCurso || pruebaEnCurso->tipo != TipoPrueba::SISTEMA_LIMPIEZA) {
        return false;
    }
    
    pasoActual++;
    bool valido = (idQuirofano >= 1 && idQuirofano <= 10);
    
    if (valido) {
        qInfo() << "[DEMO] Paso 1: Quirófano seleccionado correctamente";
        pruebaEnCurso->pasosExitosos++;
        registrarPaso(TipoPrueba::SISTEMA_LIMPIEZA, PasoPrueba::PASO_1_INICIO, true);
    } else {
        pruebaEnCurso->intentos++;
    }
    
    return valido;
}

bool DemoController::confirmarEntendimientoSistema() {
    if (!pruebaEnCurso || pruebaEnCurso->tipo != TipoPrueba::SISTEMA_LIMPIEZA) {
        return false;
    }
    
    // Paso 2: Entendió el proceso
    pruebaEnCurso->pasosExitosos++;
    registrarPaso(TipoPrueba::SISTEMA_LIMPIEZA, PasoPrueba::PASO_2_SELECCION, true);
    
    // Paso 3: Confirmó conocimiento
    pruebaEnCurso->pasosExitosos++;
    registrarPaso(TipoPrueba::SISTEMA_LIMPIEZA, PasoPrueba::PASO_3_DATOS, true);
    
    bool completada = (pruebaEnCurso->pasosExitosos >= 3);
    finalizarPruebaActual(completada);
    
    return completada;
}

// ============================================================
//                  MÉTODOS AUXILIARES
// ============================================================

void DemoController::registrarAccionDemo(TipoPrueba tipo, bool exito) {
    ResultadoPrueba resultado;
    resultado.tipo = tipo;
    resultado.completada = exito;
    resultado.intentos = 1;
    resultado.fechaInicio = QDateTime::currentDateTime();
    resultado.fechaFin = QDateTime::currentDateTime();
    resultado.duracionSegundos = 0;
    resultado.totalPasos = 1;
    resultado.pasosExitosos = exito ? 1 : 0;
    resultado.feedback = generarFeedback(tipo, exito, resultado.pasosExitosos, resultado.totalPasos);
    
    resultados.append(resultado);
}

void DemoController::registrarPaso(TipoPrueba tipo, PasoPrueba paso, bool exito) {
    if (pruebaEnCurso) {
        pruebaEnCurso->pasosCompletados[paso] = exito;
    }
}

void DemoController::finalizarPruebaActual(bool exito) {
    if (!pruebaEnCurso) return;
    
    pruebaEnCurso->completada = exito;
    pruebaEnCurso->fechaFin = QDateTime::currentDateTime();
    pruebaEnCurso->duracionSegundos = pruebaEnCurso->fechaInicio.secsTo(pruebaEnCurso->fechaFin);
    pruebaEnCurso->feedback = generarFeedback(
        pruebaEnCurso->tipo, 
        exito, 
        pruebaEnCurso->pasosExitosos,
        pruebaEnCurso->totalPasos
    );
    
    resultados.append(*pruebaEnCurso);
    
    delete pruebaEnCurso;
    pruebaEnCurso = nullptr;
    pasoActual = 0;
}

QVector<ResultadoPrueba> DemoController::obtenerResultados() const {
    return resultados;
}

int DemoController::calcularPuntajeTotal() const {
    int puntajeTotal = 0;
    
    for (const ResultadoPrueba& resultado : resultados) {
        puntajeTotal += obtenerPuntajePrueba(resultado.tipo, resultado.pasosExitosos, resultado.totalPasos);
    }
    
    return puntajeTotal;
}

QString DemoController::generarReporteCapacitacion() const {
    QString reporte = "\n";
    reporte += "========================================================\n";
    reporte += "       REPORTE DE CAPACITACION - MODO DEMO\n";
    reporte += "========================================================\n\n";
    
    reporte += "Duracion total: ";
    if (!resultados.isEmpty()) {
        int segundosTotales = inicioSesionDemo.secsTo(QDateTime::currentDateTime());
        reporte += QString("%1 minutos %2 segundos\n").arg(segundosTotales / 60).arg(segundosTotales % 60);
    }
    
    reporte += QString("\nPruebas realizadas: %1\n").arg(resultados.size());
    int pruebasExitosas = 0;
    for (const auto& r : resultados) {
        if (r.completada) pruebasExitosas++;
    }
    reporte += QString("Pruebas exitosas: %1\n\n").arg(pruebasExitosas);
    
    reporte += "--- Detalle de pruebas ---\n";
    for (const ResultadoPrueba& resultado : resultados) {
        QString tipoPrueba;
        switch (resultado.tipo) {
            case TipoPrueba::NAVEGACION_MENU:
                tipoPrueba = "Navegacion de Menu";
                break;
            case TipoPrueba::AGENDAR_CIRUGIA:
                tipoPrueba = "Agendar Cirugia";
                break;
            case TipoPrueba::SISTEMA_LIMPIEZA:
                tipoPrueba = "Sistema de Limpieza";
                break;
            case TipoPrueba::CONSULTAR_HORARIOS:
                tipoPrueba = "Consultar Horarios";
                break;
            default:
                tipoPrueba = "Desconocido";
        }
        
        reporte += QString("\n%1: %2\n")
                    .arg(tipoPrueba)
                    .arg(resultado.completada ? "[EXITOSA]" : "[INCOMPLETA]");
        reporte += QString("  Pasos completados: %1/%2\n")
                    .arg(resultado.pasosExitosos)
                    .arg(resultado.totalPasos);
        reporte += QString("  Intentos: %1\n").arg(resultado.intentos);
        reporte += QString("  Duracion: %1 segundos\n").arg(resultado.duracionSegundos);
        reporte += QString("  Feedback: %1\n").arg(resultado.feedback);
    }
    
    int puntaje = calcularPuntajeTotal();
    reporte += QString("\n\nPUNTAJE TOTAL: %1/10 puntos\n").arg(puntaje);
    
    if (puntaje >= 8) {
        reporte += "\n[EXCELENTE] Has demostrado dominio del sistema.\n";
        reporte += "Estas listo para usar el sistema en produccion.\n";
    } else if (puntaje >= 6) {
        reporte += "\n[BUENO] Comprension adecuada del sistema.\n";
        reporte += "Practica un poco mas para mejorar tu desempeno.\n";
    } else if (puntaje >= 4) {
        reporte += "\n[REGULAR] Conocimiento basico del sistema.\n";
        reporte += "Se recomienda repetir algunas pruebas.\n";
    } else {
        reporte += "\n[REQUIERE PRACTICA] Necesitas mas familiarizacion.\n";
        reporte += "Por favor, repite la capacitacion completa.\n";
    }
    
    reporte += "========================================================\n";
    
    return reporte;
}

AgendaDemoCita* DemoController::obtenerCitaDemo() const {
    return citaDemo;
}

ResultadoPrueba* DemoController::obtenerPruebaActual() {
    return pruebaEnCurso;
}

void DemoController::resetearDatosDemo() {
    if (citaDemo) {
        delete citaDemo;
        citaDemo = nullptr;
    }
    
    if (pruebaEnCurso) {
        delete pruebaEnCurso;
        pruebaEnCurso = nullptr;
    }
    
    pasoActual = 0;
}

TipoPrueba DemoController::getPruebaActual() const {
    return pruebaActual;
}

bool DemoController::hayPruebaEnCurso() const {
    return (pruebaEnCurso != nullptr);
}

QString DemoController::generarFeedback(TipoPrueba tipo, bool exito, int pasosCompletados, int totalPasos) const {
    if (!exito) {
        return QString("Completaste %1 de %2 pasos. Intenta nuevamente.").arg(pasosCompletados).arg(totalPasos);
    }
    
    switch (tipo) {
        case TipoPrueba::NAVEGACION_MENU:
            return QString("Excelente! Navegaste correctamente por el sistema (%1/%2 pasos).").arg(pasosCompletados).arg(totalPasos);
        case TipoPrueba::AGENDAR_CIRUGIA:
            return QString("Bien hecho! Agendaste una cita correctamente (%1/%2 pasos).").arg(pasosCompletados).arg(totalPasos);
        case TipoPrueba::SISTEMA_LIMPIEZA:
            return QString("Perfecto! Comprendes el sistema de limpieza (%1/%2 pasos).").arg(pasosCompletados).arg(totalPasos);
        case TipoPrueba::CONSULTAR_HORARIOS:
            return QString("Correcto! Sabes consultar horarios (%1/%2 pasos).").arg(pasosCompletados).arg(totalPasos);
        default:
            return QString("Prueba completada (%1/%2 pasos).").arg(pasosCompletados).arg(totalPasos);
    }
}

int DemoController::obtenerPuntajePrueba(TipoPrueba tipo, int pasosCompletados, int totalPasos) const {
    int puntajeMaximo = 0;
    
    switch (tipo) {
        case TipoPrueba::NAVEGACION_MENU:
            puntajeMaximo = 2;
            break;
        case TipoPrueba::AGENDAR_CIRUGIA:
            puntajeMaximo = 4;
            break;
        case TipoPrueba::SISTEMA_LIMPIEZA:
            puntajeMaximo = 3;
            break;
        case TipoPrueba::CONSULTAR_HORARIOS:
            puntajeMaximo = 1;
            break;
        default:
            puntajeMaximo = 1;
    }
    
    // Calcular puntaje proporcional a los pasos completados
    if (totalPasos > 0) {
        return (puntajeMaximo * pasosCompletados) / totalPasos;
    }
    
    return 0;
}
