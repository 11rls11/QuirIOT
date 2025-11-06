#include "ConsoleView.h"
#include "../infra/exceptions/ValidacionException.h"
#include "qsqlquery.h"
#include <QDateTime>
#include <QDebug>

ConsoleView::ConsoleView(AutenticacionController& authCtrl, QuirofanoController& quirofanoCtrl,
                         IoTController& iotCtrl)
    : autenticacionController(authCtrl), quirofanoController(quirofanoCtrl),
    iotController(iotCtrl), in(stdin), out(stdout) {}

void ConsoleView::ejecutar() {
    limpiarPantalla();

    out << "============================================================\n";
    out << "           QUIRIOT - Sistema de Quirofanos IoT             \n";
    out << "              Gestion Inteligente de Quirofanos            \n";
    out << "============================================================\n\n";
    out.flush();

    // US 1: Proceso de autenticacion
    if (!procesarLogin()) {
        mostrarError("No se pudo autenticar. Saliendo...");
        return;
    }

    while (autenticacionController.estaAutenticado()) {
        mostrarMenuPrincipal();
    }

    out << "\nHasta pronto!\n";
    out.flush();
}

void ConsoleView::mostrarMenuPrincipal() {
    limpiarPantalla();

    out << "\n============================================================\n";
    out << "                      MENU PRINCIPAL                        \n";
    out << "============================================================\n";
    out << " Usuario: " << autenticacionController.getNombreUsuarioActual() << "\n";
    out << "============================================================\n";
    out << " 1. Gestion de Quirofanos y Reservas                       \n";
    out << " 2. Monitoreo IoT - Sensores [Proximamente]                \n";
    out << " 3. Control de Actuadores [Proximamente]                   \n";
    out << " 4. Ver Reservas del Dia                                   \n";
    out << " 5. Cerrar Sesion                                          \n";
    out << " 0. Salir                                                  \n";
    out << "============================================================\n";
    out.flush();

    int opcion = leerEntero("\nSeleccione una opcion", 0, 5);

    switch (opcion) {
    case 1:
        mostrarMenuQuirofanos();
        break;
    case 2:
        mostrarMenuIoT();
        break;
    case 3:
        mostrarInfo("Funcionalidad en desarrollo (US 8-9)");
        pausa();
        break;
    case 4:
        listarReservasDelDia();
        break;
    case 5:
        procesarLogout();
        break;
    case 0:
        procesarLogout();
        break;
    default:
        mostrarError("Opcion no valida");
        pausa();
    }
}

void ConsoleView::mostrarMenuQuirofanos() {
    bool volver = false;

    while (!volver) {
        limpiarPantalla();

        out << "\n============================================================\n";
        out << "              GESTION DE QUIROFANOS Y RESERVAS             \n";
        out << "                                                           \n";
        out << "============================================================\n";
        out << " 1. Listar Todos los Quirofanos                            \n";
        out << " 2. Consultar Horarios Disponibles                         \n";
        out << " 3. Agendar Nueva Cirugia                                  \n";
        out << " 4. Buscar Proximo Horario Disponible                      \n";
        out << " 5. Ver Reservas del Dia                                   \n";
        out << " 6. Cancelar Reserva                                       \n";
        out << " 7. Validar Horario (sin agendar)                          \n";
        out << " 0. Volver al Menu Principal                               \n";
        out << "============================================================\n";
        out.flush();

        int opcion = leerEntero("\nSeleccione una opcion", 0, 7);

        switch (opcion) {
        case 1: listarQuirofanos(); break;
        case 2: consultarHorariosDisponibles(); break;
        case 3: agendarCirugiaConSugerencias(); break;
        case 4: buscarProximoHorarioDisponible(); break;
        case 5: listarReservasDelDia(); break;
        case 6: cancelarReserva(); break;
        case 7: mostrarSugerenciaHorario(); break;
        case 0: volver = true; break;
        default:
            mostrarError("Opcion no valida");
            pausa();
        }
    }
}

void ConsoleView::mostrarMenuIoT() {
    bool volver = false;

    while (!volver) {
        limpiarPantalla();

        out << "\n============================================================\n";
        out << "          CONTROL IoT - SISTEMA DE LIMPIEZA (US 8)        \n";
        out << "============================================================\n";
        out << " 1. Activar/Desactivar Sistema de Limpieza                 \n";
        out << " 2. Consultar Estado del Sistema                           \n";
        out << " 3. Ver Historial de Acciones del Sistema                  \n";
        out << " 0. Volver al Menu Principal                               \n";
        out << "============================================================\n";
        out.flush();

        int opcion = leerEntero("\nSeleccione una opcion", 0, 3);

        switch (opcion) {
        case 1: activarDesactivarSistemaLimpieza(); break;
        case 2: consultarEstadoSistemaLimpieza(); break;
        case 3: verHistorialSistemaLimpieza(); break;
        case 0: volver = true; break;
        default:
            mostrarError("Opcion no valida");
            pausa();
        }
    }
}

void ConsoleView::activarDesactivarSistemaLimpieza() {
    limpiarPantalla();
    mostrarSeparador();
    out << colorCyan("ACTIVAR/DESACTIVAR SISTEMA DE LIMPIEZA") << "\n";
    mostrarSeparador();

    int idQuirofano = leerEntero("ID del Quirofano", 1, 100);

    // Consultar estado actual
    bool estadoActual = iotController.consultarEstadoSistemaLimpieza(idQuirofano);

    out << "\n[INFO] Estado actual del sistema en Quirofano " << idQuirofano << ": ";
    if (estadoActual) {
        out << colorVerde("ACTIVO") << "\n";
    } else {
        out << colorRojo("DESACTIVADO") << "\n";
    }

    mostrarSeparador();
    out << "\nQue desea hacer?\n";
    out << " 1. Activar sistema de limpieza\n";
    out << " 2. Desactivar sistema de limpieza\n";
    out << " 0. Cancelar\n";

    int opcion = leerEntero("\nSeleccione una opcion", 0, 2);

    if (opcion == 0) {
        mostrarInfo("Operacion cancelada");
        pausa();
        return;
    }

    try {
        ResultadoOperacionSistema resultado;

        if (opcion == 1) {
            // ACTIVAR SISTEMA
            out << "\n" << colorVerde("=== ACTIVAR SISTEMA DE LIMPIEZA ===") << "\n";
            out << "[INFO] Para activar el sistema se requiere su contrasena\n\n";

            QString password = leerLineaSegura("Ingrese su contrasena");

            resultado = iotController.activarSistemaLimpieza(
                idQuirofano,
                autenticacionController.getUsuarioIdActual(),
                password
                );

        } else if (opcion == 2) {
            // DESACTIVAR SISTEMA (requiere confirmación)
            out << "\n" << colorRojo("=== DESACTIVAR SISTEMA DE LIMPIEZA ===") << "\n";
            out << colorAmarillo("[ADVERTENCIA] Esta a punto de desactivar el sistema de limpieza") << "\n";
            out << colorAmarillo("[ADVERTENCIA] Esto detendra todos los protocolos automaticos") << "\n\n";

            out << "Acepta la responsabilidad de desactivar el sistema? (s/n): ";
            out.flush();
            QString confirmacion = leerLinea();

            if (confirmacion.toLower() != "s") {
                mostrarInfo("Operacion cancelada");
                pausa();
                return;
            }

            QString razon = leerLinea("\nRazon para desactivar (opcional)");
            QString password = leerLineaSegura("Ingrese su contrasena para confirmar");

            resultado = iotController.desactivarSistemaLimpieza(
                idQuirofano,
                autenticacionController.getUsuarioIdActual(),
                password,
                razon
                );
        }

        // Mostrar resultado
        mostrarSeparador();
        if (resultado.exito) {
            out << "\n" << colorVerde("============================================================") << "\n";
            out << colorVerde("              OPERACION EXITOSA") << "\n";
            out << colorVerde("============================================================") << "\n";
            mostrarExito(resultado.mensaje);
            out << "\nQuirofano:       " << idQuirofano << "\n";
            out << "Usuario:         " << resultado.nombreUsuario << "\n";
            out << "Accion:          " << resultado.accion << "\n";
            out << "Fecha/Hora:      " << QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss") << "\n";
            out << "\n[INFO] La accion ha sido registrada en el historial del sistema\n";
            out << colorVerde("============================================================") << "\n";
        } else {
            mostrarError(resultado.mensaje);
        }

    } catch (const std::exception& e) {
        mostrarError(QString("Error: %1").arg(e.what()));
    }

    pausa();
}

void ConsoleView::consultarEstadoSistemaLimpieza() {
    limpiarPantalla();
    mostrarSeparador();
    out << "CONSULTAR ESTADO DEL SISTEMA DE LIMPIEZA\n";
    mostrarSeparador();

    int idQuirofano = leerEntero("ID del Quirofano", 1, 100);

    bool estadoActivo = iotController.consultarEstadoSistemaLimpieza(idQuirofano);

    mostrarSeparador();
    out << "\n[INFO] Quirofano: " << idQuirofano << "\n";
    out << "[INFO] Sistema de Limpieza: ";

    if (estadoActivo) {
        out << colorVerde("ACTIVO") << "\n";
        out << "\n[OK] El sistema de limpieza esta funcionando normalmente\n";
        out << "[OK] Los protocolos automaticos estan activos\n";
    } else {
        out << colorRojo("DESACTIVADO") << "\n";
        out << "\n" << colorAmarillo("[ADVERTENCIA] El sistema de limpieza esta desactivado") << "\n";
        out << colorAmarillo("[ADVERTENCIA] Los protocolos automaticos estan inactivos") << "\n";
        out << "\n[INFO] Para activarlo, use la opcion 1 del menu IoT\n";
    }

    pausa();
}

void ConsoleView::verHistorialSistemaLimpieza() {
    limpiarPantalla();
    mostrarSeparador();
    out << "HISTORIAL DEL SISTEMA DE LIMPIEZA\n";
    mostrarSeparador();

    int idQuirofano = leerEntero("ID del Quirofano (0 para todos)", 0, 100);

    // Consultar historial desde la base de datos
    QSqlDatabase& db = iotController.obtenerDatabase();
    QSqlQuery query(db);

    if (idQuirofano == 0) {
        query.prepare(
            "SELECT h.id_historial, h.id_quirofano, u.nombre, h.accion, "
            "h.fecha_accion, h.razon "
            "FROM historial_sistema_limpieza h "
            "JOIN usuarios u ON h.id_usuario = u.id_usuario "
            "ORDER BY h.fecha_accion DESC "
            "LIMIT 20"
            );
    } else {
        query.prepare(
            "SELECT h.id_historial, h.id_quirofano, u.nombre, h.accion, "
            "h.fecha_accion, h.razon "
            "FROM historial_sistema_limpieza h "
            "JOIN usuarios u ON h.id_usuario = u.id_usuario "
            "WHERE h.id_quirofano = ? "
            "ORDER BY h.fecha_accion DESC "
            "LIMIT 20"
            );
        query.addBindValue(idQuirofano);
    }

    if (!query.exec()) {
        mostrarError("No se pudo consultar el historial");
        pausa();
        return;
    }

    int count = 0;
    mostrarSeparador();
    out << "\n[HISTORIAL DE ACCIONES DEL SISTEMA]\n\n";

    while (query.next()) {
        count++;
        int idReg = query.value(0).toInt();
        int quirofano = query.value(1).toInt();
        QString usuario = query.value(2).toString();
        QString accion = query.value(3).toString();
        QDateTime fecha = query.value(4).toDateTime();
        QString razon = query.value(5).toString();

        out << "--- Registro #" << idReg << " ---\n";
        out << "Quirofano:  " << quirofano << "\n";
        out << "Usuario:    " << usuario << "\n";

        if (accion == "ACTIVAR") {
            out << "Accion:     " << colorVerde(accion) << "\n";
        } else {
            out << "Accion:     " << colorRojo(accion) << "\n";
        }

        out << "Fecha:      " << fecha.toString("dd/MM/yyyy hh:mm:ss") << "\n";
        if (!razon.isEmpty()) {
            out << "Razon:      " << razon << "\n";
        }
        out << "\n";
    }

    if (count == 0) {
        mostrarInfo("No hay registros en el historial");
    } else {
        out << "[INFO] Total de registros mostrados: " << count << "\n";
    }

    pausa();
}

// ============================================================
//                       AUTENTICACION
// ============================================================

bool ConsoleView::procesarLogin() {
    mostrarSeparador();
    out << "INICIO DE SESION\n";
    mostrarSeparador();

    QString email = leerLinea("Email");
    QString password = leerLinea("Contrasena");

    RespuestaAutenticacion respuesta = autenticacionController.login(email, password);

    if (respuesta.exito) {
        mostrarExito("Autenticacion exitosa!");
        mostrarInfo("Bienvenido, " + respuesta.nombreUsuario);
        pausa();
        return true;
    } else {
        mostrarError(respuesta.mensaje);
        pausa();
        return false;
    }
}

void ConsoleView::procesarLogout() {
    autenticacionController.logout();
    mostrarInfo("Sesion cerrada correctamente");
}

// =============================================================
//                       AGENDAR CIRUGIA
// =============================================================

void ConsoleView::listarQuirofanos() {
    limpiarPantalla();
    mostrarSeparador();
    out << "LISTA DE QUIROFANOS\n";
    mostrarSeparador();

    auto quirofanos = quirofanoController.listarQuirofanos();

    if (quirofanos.isEmpty()) {
        mostrarInfo("No hay quirofanos registrados");
    } else {
        out << QString("ID").leftJustified(5) << " | "
            << QString("Nombre").leftJustified(30) << " | "
            << QString("Estado").leftJustified(15) << " | "
            << QString("Capacidad").leftJustified(10) << "\n";
        mostrarSeparador();

        for (Quirofano* quirofano : quirofanos) {
            out << QString::number(quirofano->getId()).leftJustified(5) << " | "
                << quirofano->getNombre().leftJustified(30) << " | "
                << Quirofano::estadoToString(quirofano->getEstado()).leftJustified(15) << " | "
                << QString::number(quirofano->getCapacidad()).leftJustified(10) << "\n";
            delete quirofano;
        }
    }

    pausa();
}

void ConsoleView::consultarHorariosDisponibles() {
    limpiarPantalla();
    mostrarSeparador();
    out << "CONSULTAR HORARIOS DISPONIBLES\n";
    mostrarSeparador();

    int idQuirofano = leerEntero("ID del Quirofano", 1, 100);

    out << "Fecha (formato: YYYY-MM-DD, ejemplo: 2025-10-22): ";
    out.flush();
    QString fechaStr = leerLinea();
    QDate fecha = QDate::fromString(fechaStr, "yyyy-MM-dd");

    if (!fecha.isValid()) {
        mostrarError("Fecha invalida");
        pausa();
        return;
    }

    auto horarios = quirofanoController.consultarHorariosDisponibles(idQuirofano, fecha);

    if (horarios.isEmpty()) {
        mostrarInfo("No hay horarios disponibles para esta fecha");
    } else {
        out << "\n[OK] Horarios disponibles encontrados:\n";
        mostrarSeparador();

        for (const auto& horario : horarios) {
            out << " Desde: " << horario.inicio.toString("hh:mm")
            << " Hasta: " << horario.fin.toString("hh:mm")
            << " (Duracion: " << horario.duracionMinutos << " minutos)\n";
        }

        out << "[INFO] Se requieren minimo 30 minutos entre cirugias para sanitizacion\n";
    }

    pausa();
}

void ConsoleView::agendarCirugia() {
    limpiarPantalla();
    mostrarSeparador();
    out << "AGENDAR NUEVA CIRUGIA\n";
    mostrarSeparador();

    try {
        int idQuirofano = leerEntero("ID del Quirofano", 1, 100);

        out << "Fecha y hora de inicio (formato: YYYY-MM-DD HH:MM): ";
        out.flush();
        QString inicioStr = leerLinea();
        QDateTime inicio = QDateTime::fromString(inicioStr, "yyyy-MM-dd hh:mm");

        out << "Fecha y hora de fin (formato: YYYY-MM-DD HH:MM): ";
        out.flush();
        QString finStr = leerLinea();
        QDateTime fin = QDateTime::fromString(finStr, "yyyy-MM-dd hh:mm");

        QString motivo = leerLinea("Motivo de la cirugia");

        if (!inicio.isValid() || !fin.isValid()) {
            mostrarError("Fechas invalidas");
            pausa();
            return;
        }

        out << "\n[INFO] Validando disponibilidad...\n";
        out.flush();

        Reserva* reserva = quirofanoController.agendarCirugia(
            autenticacionController.getUsuarioIdActual(),
            idQuirofano,
            inicio,
            fin,
            motivo
            );

        if (reserva) {
            mostrarExito("Cirugia agendada exitosamente!");
            mostrarInfo("ID de reserva: " + QString::number(reserva->getId()));
            mostrarInfo("Quirofano: " + QString::number(idQuirofano));
            mostrarInfo("Horario: " + inicio.toString("dd/MM/yyyy hh:mm") +
                        " - " + fin.toString("hh:mm"));
            mostrarInfo("La informacion se guardo correctamente en la base de datos");
            delete reserva;
        }

    } catch (const ValidacionException& e) {
        mostrarError(e.getMensaje());
    }

    pausa();
}

void ConsoleView::agendarCirugiaConSugerencias() {
    limpiarPantalla();
    mostrarSeparador();
    out << "AGENDAR CIRUGIA\n";
    mostrarSeparador();
    
    try {
        int idQuirofano = leerEntero("ID del Quirofano", 1, 100);
        
        out << "\nFecha y hora de inicio (formato: YYYY-MM-DD HH:MM): ";
        out.flush();
        QString inicioStr = leerLinea();
        QDateTime inicio = QDateTime::fromString(inicioStr, "yyyy-MM-dd hh:mm");
        
        out << "Fecha y hora de fin (formato: YYYY-MM-DD HH:MM): ";
        out.flush();
        QString finStr = leerLinea();
        QDateTime fin = QDateTime::fromString(finStr, "yyyy-MM-dd hh:mm");
        
        QString motivo = leerLinea("Motivo de la cirugia");
        
        if (!inicio.isValid() || !fin.isValid()) {
            mostrarError("Fechas invalidas");
            pausa();
            return;
        }
        
        // Validar con sistema de sugerencias
        out << "\n[INFO] Validando horario y generando sugerencias...\n";
        out.flush();
        
        HorarioSugerido sugerencia = quirofanoController.validarYSugerirHorario(
            idQuirofano, inicio, fin
        );
        
        mostrarSeparador();
        
        switch (sugerencia.tipo) {
            case TipoSugerencia::DISPONIBLE:
                out << colorVerde("[OK] HORARIO DISPONIBLE") << "\n";
                mostrarExito(sugerencia.mensaje);
                out << "\nInicio: " << inicio.toString("dd/MM/yyyy hh:mm") << "\n";
                out << "Fin:    " << fin.toString("dd/MM/yyyy hh:mm") << "\n";
                out << "Duracion: " << (inicio.secsTo(fin) / 60) << " minutos\n";
                
                mostrarSeparador();
                validarYConfirmarReserva(idQuirofano, inicio, fin, motivo);
                break;
                
            case TipoSugerencia::ADVERTENCIA_INICIO:
            case TipoSugerencia::ADVERTENCIA_FIN:
            case TipoSugerencia::ADVERTENCIA_AMBOS: {
                out << colorAmarillo("[ADVERTENCIA] CONFLICTO DE SANITIZACION") << "\n";
                mostrarAdvertencia(sugerencia.mensaje);
                
                out << "\n--- Horario solicitado ---\n";
                out << "Inicio: " << inicio.toString("dd/MM/yyyy hh:mm") << "\n";
                out << "Fin:    " << fin.toString("dd/MM/yyyy hh:mm") << "\n";
                
                if (sugerencia.minutosAntesAnterior > 0 && sugerencia.minutosAntesAnterior < 30) {
                    out << colorAmarillo(QString("Tiempo desde cirugia anterior: %1 minutos (minimo: 30)")
                                        .arg(sugerencia.minutosAntesAnterior)) << "\n";
                }
                
                if (sugerencia.minutosDespuesSiguiente > 0 && sugerencia.minutosDespuesSiguiente < 30) {
                    out << colorAmarillo(QString("Tiempo hasta siguiente cirugia: %1 minutos (minimo: 30)")
                                        .arg(sugerencia.minutosDespuesSiguiente)) << "\n";
                }
                
                out << "\n" << colorVerde("--- Horario sugerido (con 30 min de sanitizacion) ---") << "\n";
                out << "Inicio: " << sugerencia.inicioSugerido.toString("dd/MM/yyyy hh:mm") << "\n";
                out << "Fin:    " << sugerencia.finSugerido.toString("dd/MM/yyyy hh:mm") << "\n";
                
                mostrarSeparador();
                out << "\nOpciones:\n";
                out << " 1. Agendar con horario sugerido (recomendado)\n";
                out << " 2. Agendar con horario original (bajo tu responsabilidad)\n";
                out << " 0. Cancelar\n";
                
                int opcion = leerEntero("\nSeleccione una opcion", 0, 2);
                
                if (opcion == 1) {
                    validarYConfirmarReserva(idQuirofano, sugerencia.inicioSugerido, 
                                            sugerencia.finSugerido, motivo);
                } else if (opcion == 2) {
                    out << colorRojo("\n[ADVERTENCIA] Estas agendando sin cumplir el tiempo de sanitizacion\n");
                    out << "Confirmas bajo tu responsabilidad? (s/n): ";
                    out.flush();
                    QString confirmacion = leerLinea();
                    
                    if (confirmacion.toLower() == "s") {
                        validarYConfirmarReserva(idQuirofano, inicio, fin, motivo);
                    } else {
                        mostrarInfo("Operacion cancelada");
                    }
                } else {
                    mostrarInfo("Operacion cancelada");
                }
                break;
            }
            case TipoSugerencia::NO_DISPONIBLE: {
                out << colorRojo("[ERROR] HORARIO NO DISPONIBLE") << "\n";
                mostrarError(sugerencia.mensaje);
                
                out << "\nHorario solicitado se solapa con otra cirugia:\n";
                out << "Inicio solicitado: " << inicio.toString("dd/MM/yyyy hh:mm") << "\n";
                out << "Fin solicitado:    " << fin.toString("dd/MM/yyyy hh:mm") << "\n";
                
                out << "\n" << colorVerde("Horario alternativo disponible:") << "\n";
                out << "Inicio: " << sugerencia.inicioSugerido.toString("dd/MM/yyyy hh:mm") << "\n";
                out << "Fin:    " << sugerencia.finSugerido.toString("dd/MM/yyyy hh:mm") << "\n";
                
                mostrarSeparador();
                out << "Deseas agendar con el horario alternativo? (s/n): ";
                out.flush();
                QString respuesta = leerLinea();
                
                if (respuesta.toLower() == "s") {
                    validarYConfirmarReserva(idQuirofano, sugerencia.inicioSugerido,
                                            sugerencia.finSugerido, motivo);
                } else {
                    mostrarInfo("Operacion cancelada");
                }
                break;
            }
        }
    } catch (const ValidacionException& e) {
        mostrarError(e.getMensaje());
    }
    
    pausa();
}

void ConsoleView::validarYConfirmarReserva(int idQuirofano, const QDateTime& inicio,
                                           const QDateTime& fin, const QString& motivo) {
    try {
        Reserva* reserva = quirofanoController.agendarCirugia(
            autenticacionController.getUsuarioIdActual(),
            idQuirofano,
            inicio,
            fin,
            motivo
        );
        
        if (reserva) {
            out << "\n" 
                << colorVerde("=============================================================") << "\n";
            out << colorVerde("                CIRUGIA AGENDADA EXITOSAMENTE                ") << "\n";
            out << colorVerde("=============================================================") << "\n";
            mostrarExito("La reserva se guardo correctamente en la base de datos");
            out << "\n--- Detalles de la reserva ---\n";
            out << "ID de reserva: " << reserva->getId() << "\n";
            out << "Quirofano:     " << idQuirofano << "\n";
            out << "Fecha inicio:  " << inicio.toString("dd/MM/yyyy hh:mm") << "\n";
            out << "Fecha fin:     " << fin.toString("hh:mm") << "\n";
            out << "Duracion:      " << (inicio.secsTo(fin) / 60) << " minutos\n";
            out << "Motivo:        " << motivo << "\n";
            out << "Estado:        " << Reserva::estadoToString(reserva->getEstado()) << "\n";
            out << colorVerde("============================================================") << "\n";
            
            delete reserva;
        }
        
    } catch (const ValidacionException& e) {
        mostrarError(QString("Error al agendar: %1").arg(e.getMensaje()));
    }
}

void ConsoleView::mostrarSugerenciaHorario() {
    limpiarPantalla();
    mostrarSeparador();
    out << "VALIDAR HORARIO (sin agendar)\n";
    mostrarSeparador();
    
    int idQuirofano = leerEntero("ID del Quirofano", 1, 100);
    
    out << "\nFecha y hora de inicio (formato: YYYY-MM-DD HH:MM): ";
    out.flush();
    QString inicioStr = leerLinea();
    QDateTime inicio = QDateTime::fromString(inicioStr, "yyyy-MM-dd hh:mm");
    
    out << "Fecha y hora de fin (formato: YYYY-MM-DD HH:MM): ";
    out.flush();
    QString finStr = leerLinea();
    QDateTime fin = QDateTime::fromString(finStr, "yyyy-MM-dd hh:mm");
    
    if (!inicio.isValid() || !fin.isValid()) {
        mostrarError("Fechas invalidas");
        pausa();
        return;
    }
    
    HorarioSugerido sugerencia = quirofanoController.validarYSugerirHorario(
        idQuirofano, inicio, fin
    );
    
    mostrarSeparador();
    out << "RESULTADO DE VALIDACION\n";
    mostrarSeparador();
    
    switch (sugerencia.tipo) {
        case TipoSugerencia::DISPONIBLE:
            out << colorVerde("Estado: DISPONIBLE") << "\n";
            out << sugerencia.mensaje << "\n";
            break;
            
        case TipoSugerencia::ADVERTENCIA_INICIO:
        case TipoSugerencia::ADVERTENCIA_FIN:
        case TipoSugerencia::ADVERTENCIA_AMBOS:
            out << colorAmarillo("Estado: ADVERTENCIA DE SANITIZACION") << "\n";
            out << sugerencia.mensaje << "\n\n";
            out << "Horario sugerido:\n";
            out << "  Inicio: " << sugerencia.inicioSugerido.toString("dd/MM/yyyy hh:mm") << "\n";
            out << "  Fin:    " << sugerencia.finSugerido.toString("dd/MM/yyyy hh:mm") << "\n";
            break;
            
        case TipoSugerencia::NO_DISPONIBLE:
            out << colorRojo("Estado: NO DISPONIBLE") << "\n";
            out << sugerencia.mensaje << "\n\n";
            out << "Horario alternativo sugerido:\n";
            out << "  Inicio: " << sugerencia.inicioSugerido.toString("dd/MM/yyyy hh:mm") << "\n";
            out << "  Fin:    " << sugerencia.finSugerido.toString("dd/MM/yyyy hh:mm") << "\n";
            break;
    }
    
    pausa();
}

void ConsoleView::buscarProximoHorarioDisponible() {
    limpiarPantalla();
    mostrarSeparador();
    out << "BUSCAR PROXIMO HORARIO DISPONIBLE\n";
    mostrarSeparador();
    
    int idQuirofano = leerEntero("ID del Quirofano", 1, 100);
    
    out << "\nFecha y hora deseada (formato: YYYY-MM-DD HH:MM): ";
    out.flush();
    QString inicioStr = leerLinea();
    QDateTime inicioDeseado = QDateTime::fromString(inicioStr, "yyyy-MM-dd hh:mm");
    
    int duracion = leerEntero("Duracion estimada en minutos", 30, 480);
    
    if (!inicioDeseado.isValid()) {
        mostrarError("Fecha invalida");
        pausa();
        return;
    }
    
    out << "\n[INFO] Buscando proximo horario disponible...\n";
    out.flush();
    
    HorarioSugerido sugerencia = quirofanoController.encontrarProximoHorarioDisponible(
        idQuirofano, inicioDeseado, duracion
    );
    
    mostrarSeparador();
    out << colorVerde("PROXIMO HORARIO DISPONIBLE ENCONTRADO") << "\n";
    mostrarSeparador();
    out << "Inicio: " << sugerencia.inicioSugerido.toString("dd/MM/yyyy hh:mm") << "\n";
    out << "Fin:    " << sugerencia.finSugerido.toString("dd/MM/yyyy hh:mm") << "\n";
    out << "Duracion: " << duracion << " minutos\n";
    out << "\n" << sugerencia.mensaje << "\n";
    
    pausa();
}

void ConsoleView::listarReservasDelDia() {
    limpiarPantalla();
    mostrarSeparador();
    out << "RESERVAS DEL DIA\n";
    mostrarSeparador();

    out << "Fecha (YYYY-MM-DD) [Enter para hoy]: ";
    out.flush();
    QString fechaStr = leerLinea();

    QDate fecha = fechaStr.isEmpty() ? QDate::currentDate() : QDate::fromString(fechaStr, "yyyy-MM-dd");

    auto reservas = quirofanoController.listarReservasDelDia(fecha);

    if (reservas.isEmpty()) {
        mostrarInfo("No hay reservas para esta fecha");
    } else {
        out << "\n[INFO] Total de reservas: " << reservas.size() << "\n\n";

        for (Reserva* reserva : reservas) {
            out << "----------------------------------------------------\n";
            out << "ID: " << reserva->getId() << "\n";
            out << "Quirofano: " << reserva->getIdQuirofano() << "\n";
            out << "Horario: " << reserva->getFechaInicio().toString("hh:mm")
                << " - " << reserva->getFechaFin().toString("hh:mm") << "\n";
            out << "Motivo: " << reserva->getMotivoCirugia() << "\n";
            out << "Estado: " << Reserva::estadoToString(reserva->getEstado()) << "\n";
            delete reserva;
        }
        out << "----------------------------------------------------\n";
    }

    pausa();
}

void ConsoleView::cancelarReserva() {
    limpiarPantalla();
    mostrarSeparador();
    out << "CANCELAR RESERVA\n";
    mostrarSeparador();

    int idReserva = leerEntero("ID de la Reserva a cancelar", 1, 999999);

    out << "Esta seguro de cancelar esta reserva? (s/n): ";
    out.flush();
    QString confirmacion = leerLinea();

    if (confirmacion.toLower() == "s") {
        if (quirofanoController.cancelarReserva(idReserva)) {
            mostrarExito("Reserva cancelada exitosamente");
        } else {
            mostrarError("No se pudo cancelar la reserva");
        }
    } else {
        mostrarInfo("Operacion cancelada");
    }

    pausa();
}

// ============================================================
// ESQUELETOS PARA USER STORIES FUTURAS
// ============================================================

void ConsoleView::visualizarEstadoQuirofano() {
    limpiarPantalla();
    mostrarSeparador();
    out << "VISUALIZAR ESTADO DEL QUIROFANO\n";
    mostrarSeparador();
    out << "\n[INFO] Funcionalidad en desarrollo\n";
    out << "\nEsta funcion mostrara:\n";
    out << " - Porcentaje de limpieza actual\n";
    out << " - Calidad del aire (CO2, particulas)\n";
    out << " - Nivel de humedad\n";
    out << " - Advertencias si los valores estan fuera de rango\n";
    out << " - Opcion de proceder o reprogramar cirugia\n";
    mostrarSeparador();
    pausa();
}

void ConsoleView::verRegistroDisponibilidad() {
    limpiarPantalla();
    mostrarSeparador();
    out << "REGISTRO DE DISPONIBILIDAD\n";
    mostrarSeparador();
    out << "\n[INFO] Funcionalidad planeada\n";
    out << "\nMostrara horarios disponibles con codigo de colores:\n";
    out << " - Verde: Disponible\n";
    out << " - Rojo: Ocupado (con detalles de la reserva)\n";
    out << " - Amarillo: Advertencia (menos de 30 min entre cirugias)\n";
    mostrarSeparador();
    pausa();
}

void ConsoleView::monitorearTiempoReal() {
    mostrarInfo("User Story 10 - Monitoreo en tiempo real - En desarrollo");
    pausa();
}

void ConsoleView::verHistorialCondiciones() {
    mostrarInfo("User Story 11 - Historial de condiciones - En desarrollo");
    pausa();
}

void ConsoleView::definirMantenimiento() {
    mostrarInfo("User Stories de desarrolladores - Planeadas para futuras iteraciones");
    pausa();
}

// ============================================================
// UTILIDADES
// ============================================================

QString ConsoleView::leerLinea(const QString& prompt) {
    if (!prompt.isEmpty()) {
        out << prompt << ": ";
        out.flush();
    }
    return in.readLine().trimmed();
}

int ConsoleView::leerEntero(const QString& prompt, int min, int max) {
    while (true) {
        out << prompt << " [" << min << "-" << max << "]: ";
        out.flush();

        QString input = in.readLine().trimmed();
        bool ok;
        int valor = input.toInt(&ok);

        if (ok && valor >= min && valor <= max) {
            return valor;
        }

        out << "[ERROR] Entrada invalida. Intente de nuevo.\n";
    }
}

void ConsoleView::pausa() {
    out << "\n[Presione Enter para continuar...]";
    out.flush();
    in.readLine();
}

void ConsoleView::limpiarPantalla() {
#ifdef Q_OS_WIN
    system("cls");
#else
    system("clear");
#endif
}

void ConsoleView::mostrarSeparador() {
    out << "============================================================\n";
    out.flush();
}

void ConsoleView::mostrarError(const QString& mensaje) {
    out << "\n[ERROR] " << mensaje << "\n";
    out.flush();
}

void ConsoleView::mostrarExito(const QString& mensaje) {
    out << "\n[OK] " << mensaje << "\n";
    out.flush();
}

void ConsoleView::mostrarInfo(const QString& mensaje) {
    out << "[INFO] " << mensaje << "\n";
    out.flush();
}

QString ConsoleView::colorAmarillo(const QString& texto) {
    return "\033[1;33m" + texto + "\033[0m";
}

QString ConsoleView::colorVerde(const QString& texto) {
    return "\033[1;32m" + texto + "\033[0m";
}

QString ConsoleView::colorRojo(const QString& texto) {
    return "\033[1;31m" + texto + "\033[0m";
}

QString ConsoleView::leerLineaSegura(const QString& prompt) {
    if (!prompt.isEmpty()) {
        out << prompt << ": ";
        out.flush();
    }

    // En terminal real, esto ocultaría el input
    // Para la versión básica, simplemente lee normalmente
    QString password = in.readLine().trimmed();

    return password;
}

QString ConsoleView::colorCyan(const QString& texto) {
    return "\033[1;36m" + texto + "\033[0m";
}

QString ConsoleView::colorMagenta(const QString& texto) {
    return "\033[1;35m" + texto + "\033[0m";
}

QString ConsoleView::resetColor() {
    return "\033[0m";
}

void ConsoleView::mostrarAdvertencia(const QString& mensaje) {
    out << colorAmarillo("[ADVERTENCIA] ") << mensaje << "\n";
    out.flush();
}
