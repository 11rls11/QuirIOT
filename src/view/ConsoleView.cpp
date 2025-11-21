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

    while (true) {  // Loop infinito
        // Si no está autenticado, mostrar login
        if (!autenticacionController.estaAutenticado()) {
            bool loginExitoso = procesarLogin();

            if (!loginExitoso) {
                mostrarError("No se pudo iniciar sesion. Intente nuevamente.");
                pausa();
                continue;
            }
        }

        // Si está autenticado, mostrar menú principal
        mostrarMenuPrincipal();

        // Después de logout desde el menú principal, vuelve al login
    }
}

void ConsoleView::mostrarMenuPrincipal() {
    int opcion = 0;
    do {
        limpiarPantalla();
        mostrarSeparador();
        out << "QUIRIOT - SISTEMA DE GESTION DE QUIROFANOS\n";
        mostrarSeparador();
        out << "Usuario: " << colorCyan(autenticacionController.getNombreUsuarioActual()) << "\n";
        mostrarSeparador();
        out << " 1. Gestion de Quirofano\n";
        out << " 2. Control IoT\n";
        out << " 3. Cerrar sesion\n";
        mostrarSeparador();

        opcion = leerEntero("Seleccione una opcion", 1, 3);

        switch (opcion) {
        case 1: mostrarMenuQuirofanos(); break;
        case 2: mostrarMenuIoT(); break;
        case 3:
            procesarLogout();
            return; // Volver al login
        default: mostrarError("Opcion invalida");
        }
    } while (true);
}

void ConsoleView::mostrarMenuQuirofanos() {
    int opcion = 0;
    do {
        limpiarPantalla();
        mostrarSeparador();
        out << "MENU - GESTION DE QUIROFANO\n";
        mostrarSeparador();
        out << " 1. Ver informacion del quirofano\n";
        out << " 2. Consultar horarios disponibles\n";
        out << " 3. Agendar cirugia\n";
        out << " 4. Ver reservas\n";  // Cambio aquí
        out << " 5. Cancelar reserva\n";
        out << " 6. Validar horario (sin agendar)\n";
        out << " 0. Volver al menu anterior\n";
        mostrarSeparador();

        opcion = leerEntero("Seleccione una opcion", 0, 6);

        switch (opcion) {
        case 1: verInformacionQuirofano(); break;
        case 2: consultarHorariosDisponibles(); break;
        case 3: agendarCirugiaConSugerencias(); break;
        case 4: verReservasFuturas(); break;  // Cambio aquí
        case 5: cancelarReserva(); break;
        case 6: mostrarSugerenciaHorario(); break;
        case 0: break;
        default: mostrarError("Opcion invalida");
        }
    } while (opcion != 0);
}

void ConsoleView::verReservasFuturas() {
    limpiarPantalla();
    mostrarSeparador();
    out << "RESERVAS FUTURAS\n";
    mostrarSeparador();

    const int ID_QUIROFANO = 1;
    QDateTime ahora = QDateTime::currentDateTime();

    // Obtener todas las reservas del quirófano
    auto todasReservas = quirofanoController.listarReservasPorQuirofano(ID_QUIROFANO);

    // Filtrar solo las futuras
    QVector<Reserva*> reservasFuturas;
    for (Reserva* reserva : todasReservas) {
        if (reserva->getFechaInicio() >= ahora &&
            reserva->getEstado() != EstadoReserva::CANCELADA) {
            reservasFuturas.append(reserva);
        } else {
            delete reserva;  // Liberar las que no usaremos
        }
    }

    if (reservasFuturas.isEmpty()) {
        mostrarInfo("No hay reservas futuras");
    } else {
        out << "\n[INFO] Total de reservas futuras: " << reservasFuturas.size() << "\n\n";

        for (Reserva* reserva : reservasFuturas) {
            out << "----------------------------------------------------\n";
            out << "ID: " << reserva->getId() << "\n";
            out << "Fecha: " << reserva->getFechaInicio().toString("dd/MM/yyyy") << "\n";
            out << "Horario: " << reserva->getFechaInicio().toString("hh:mm")
                << " - " << reserva->getFechaFin().toString("hh:mm") << "\n";
            out << "Duracion: " << reserva->getDuracionMinutos() << " minutos\n";
            out << "Motivo: " << reserva->getMotivoCirugia() << "\n";
            out << "Estado: " << Reserva::estadoToString(reserva->getEstado()) << "\n";
            delete reserva;
        }
        out << "----------------------------------------------------\n";
    }

    pausa();
}

void ConsoleView::mostrarMenuIoT() {
    int opcion = 0;
    do {
        limpiarPantalla();
        mostrarSeparador();
        out << "MENU - CONTROL IoT\n";
        mostrarSeparador();
        out << " 1. Sistema de limpieza (Activar/Desactivar)\n";
        out << " 2. Consultar estado del sistema\n";
        out << " 3. Ver historial del sistema\n";
        out << " 0. Volver al menu anterior\n";
        mostrarSeparador();

        opcion = leerEntero("Seleccione una opcion", 0, 3);

        switch (opcion) {
        case 1: activarDesactivarSistemaLimpieza(); break;
        case 2: consultarEstadoSistemaLimpieza(); break;
        case 3: verHistorialSistemaLimpieza(); break;
        case 0: break;
        default: mostrarError("Opcion invalida");
        }
    } while (opcion != 0);
}

void ConsoleView::activarDesactivarSistemaLimpieza() {
    limpiarPantalla();
    mostrarSeparador();
    out << "SISTEMA DE LIMPIEZA\n";
    mostrarSeparador();

    const int ID_QUIROFANO = 1;
    bool estadoActual = iotController.consultarEstadoSistemaLimpieza(ID_QUIROFANO);

    // Mostrar estado actual
    out << "\nEstado actual del sistema: ";
    if (estadoActual) {
        out << colorVerde("ACTIVADO") << "\n\n";
        mostrarSeparador();
        out << " 1. Desactivar sistema de limpieza\n";
        out << " 0. Volver al menu anterior\n";
        mostrarSeparador();

        int opcion = leerEntero("Seleccione una opcion", 0, 1);

        if (opcion == 1) {
            // Desactivar
            QString password = leerLineaSegura("Ingrese su contrasena");
            QString razon = leerLinea("Razon de la desactivacion");

            auto resultado = iotController.desactivarSistemaLimpieza(
                ID_QUIROFANO,
                autenticacionController.getUsuarioIdActual(),
                password,
                razon
                );

            if (resultado.exito) {
                mostrarExito(resultado.mensaje);
            } else {
                mostrarError(resultado.mensaje);
            }
            pausa();
        }
    } else {
        out << colorRojo("DESACTIVADO") << "\n\n";
        mostrarSeparador();
        out << " 1. Activar sistema de limpieza\n";
        out << " 0. Volver al menu anterior\n";
        mostrarSeparador();

        int opcion = leerEntero("Seleccione una opcion", 0, 1);

        if (opcion == 1) {
            // Activar
            QString password = leerLineaSegura("Ingrese su contrasena");

            auto resultado = iotController.activarSistemaLimpieza(
                ID_QUIROFANO,
                autenticacionController.getUsuarioIdActual(),
                password
                );

            if (resultado.exito) {
                mostrarExito(resultado.mensaje);
            } else {
                mostrarError(resultado.mensaje);
            }
            pausa();
        }
    }
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

void ConsoleView::verInformacionQuirofano() {
    limpiarPantalla();
    mostrarSeparador();
    out << "INFORMACION DEL QUIROFANO\n";
    mostrarSeparador();

    // ID fijo del quirófano = 1
    const int ID_QUIROFANO = 1;
    Quirofano* quirofano = quirofanoController.obtenerQuirofano(ID_QUIROFANO);

    if (!quirofano) {
        mostrarError("No se pudo obtener informacion del quirofano");
        pausa();
        return;
    }

    out << "\n--- Detalles del Quirofano ---\n";
    out << "ID:             " << quirofano->getId() << "\n";
    out << "Nombre:         " << quirofano->getNombre() << "\n";
    out << "Capacidad:      " << quirofano->getCapacidad() << " personas\n";
    out << "Estado actual:  " << colorVerde(Quirofano::estadoToString(quirofano->getEstado())) << "\n";
    out << "Disponibilidad: " << quirofano->getDisponibilidad() << "\n";

    // Verificar estado del sistema de limpieza
    bool sistemaActivo = iotController.consultarEstadoSistemaLimpieza(ID_QUIROFANO);
    out << "Sistema limpieza: "
        << (sistemaActivo ? colorVerde("ACTIVO") : colorRojo("DESACTIVADO")) << "\n";

    mostrarSeparador();
    delete quirofano;
    pausa();
}

void ConsoleView::consultarHorariosDisponibles() {
    limpiarPantalla();
    mostrarSeparador();
    out << "CONSULTAR HORARIOS DISPONIBLES\n";
    mostrarSeparador();

    // ID fijo del quirófano = 1
    const int ID_QUIROFANO = 1;

    // Obtener fecha actual por defecto
    QDate fecha = QDate::currentDate();

    out << "\nConsultando horarios disponibles para: "
        << fecha.toString("dd/MM/yyyy") << "\n\n";

    auto horarios = quirofanoController.consultarHorariosDisponibles(ID_QUIROFANO, fecha);

    if (horarios.isEmpty()) {
        mostrarAdvertencia("No hay horarios disponibles para hoy");
    } else {
        out << colorVerde("VENTANAS DE TIEMPO DISPONIBLES") << "\n";
        mostrarSeparador();

        int contador = 1;
        for (const auto& horario : horarios) {
            out << "Ventana " << contador++ << ":\n";
            out << "  Desde: " << horario.inicio.toString("hh:mm") << "\n";
            out << "  Hasta: " << horario.fin.toString("hh:mm") << "\n";
            out << "  Duracion disponible: " << horario.duracionMinutos << " minutos\n";
            out << "----------------------------------------------------\n";
        }
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

    const int ID_QUIROFANO = 1; // Quirófano fijo

    // Solicitar fecha con opción de cancelar
    QDate fecha;
    while (true) {
        out << "\nFecha de la cirugia (dd/mm/aaaa) [0 para volver]: ";
        out.flush();
        QString fechaStr = leerLinea();

        // Permitir regresar
        if (fechaStr == "0") {
            mostrarInfo("Operacion cancelada");
            pausa();
            return;
        }

        fecha = QDate::fromString(fechaStr, "dd/MM/yyyy");

        if (fecha.isValid()) {
            break;
        } else {
            mostrarError("Fecha invalida. Use el formato dd/mm/aaaa (ejemplo: 25/12/2025)");
        }
    }

    // Solicitar hora de inicio con opción de cancelar
    QTime horaInicio;
    while (true) {
        out << "Hora de inicio (hh:mm) [0 para volver]: ";
        out.flush();
        QString horaStr = leerLinea();

        // Permitir regresar
        if (horaStr == "0") {
            mostrarInfo("Operacion cancelada");
            pausa();
            return;
        }

        horaInicio = QTime::fromString(horaStr, "hh:mm");

        if (horaInicio.isValid()) {
            break;
        } else {
            mostrarError("Hora invalida. Use el formato hh:mm (ejemplo: 09:30)");
        }
    }

    // Solicitar hora de fin con opción de cancelar
    QTime horaFin;
    while (true) {
        out << "Hora de fin (hh:mm) [0 para volver]: ";
        out.flush();
        QString horaStr = leerLinea();

        // Permitir regresar
        if (horaStr == "0") {
            mostrarInfo("Operacion cancelada");
            pausa();
            return;
        }

        horaFin = QTime::fromString(horaStr, "hh:mm");

        if (horaFin.isValid() && horaFin > horaInicio) {
            break;
        } else if (!horaFin.isValid()) {
            mostrarError("Hora invalida. Use el formato hh:mm (ejemplo: 11:30)");
        } else {
            mostrarError("La hora de fin debe ser posterior a la hora de inicio");
        }
    }

    QDateTime inicio(fecha, horaInicio);
    QDateTime fin(fecha, horaFin);

    QString motivo = leerLinea("\nMotivo de la cirugia [0 para volver]");

    // Permitir regresar
    if (motivo == "0") {
        mostrarInfo("Operacion cancelada");
        pausa();
        return;
    }

    // Validar con sistema de sugerencias
    out << "\n[INFO] Validando horario y generando sugerencias...\n";
    out.flush();

    HorarioSugerido sugerencia = quirofanoController.validarYSugerirHorario(
        ID_QUIROFANO, inicio, fin
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
        validarYConfirmarReserva(ID_QUIROFANO, inicio, fin, motivo);
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
            validarYConfirmarReserva(ID_QUIROFANO, sugerencia.inicioSugerido,
                                     sugerencia.finSugerido, motivo);
        } else if (opcion == 2) {
            out << colorRojo("\n[ADVERTENCIA] Estas agendando sin cumplir el tiempo de sanitizacion\n");
            out << "Confirmas bajo tu responsabilidad? (s/n): ";
            out.flush();
            QString confirmacion = leerLinea();

            if (confirmacion.toLower() == "s") {
                validarYConfirmarReserva(ID_QUIROFANO, inicio, fin, motivo);
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
            validarYConfirmarReserva(ID_QUIROFANO, sugerencia.inicioSugerido,
                                     sugerencia.finSugerido, motivo);
        } else {
            mostrarInfo("Operacion cancelada");
        }
        break;
    }
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

    const int ID_QUIROFANO = 1;

    // Solicitar fecha con opción de cancelar
    QDate fecha;
    while (true) {
        out << "\nFecha (dd/mm/aaaa) [0 para volver]: ";
        out.flush();
        QString fechaStr = leerLinea();

        // Permitir regresar
        if (fechaStr == "0") {
            mostrarInfo("Operacion cancelada");
            pausa();
            return;
        }

        fecha = QDate::fromString(fechaStr, "dd/MM/yyyy");

        if (fecha.isValid()) {
            break;
        } else {
            mostrarError("Fecha invalida. Use el formato dd/mm/aaaa");
        }
    }

    // Solicitar hora de inicio con opción de cancelar
    QTime horaInicio;
    while (true) {
        out << "Hora de inicio (hh:mm) [0 para volver]: ";
        out.flush();
        QString horaStr = leerLinea();

        // Permitir regresar
        if (horaStr == "0") {
            mostrarInfo("Operacion cancelada");
            pausa();
            return;
        }

        horaInicio = QTime::fromString(horaStr, "hh:mm");

        if (horaInicio.isValid()) {
            break;
        } else {
            mostrarError("Hora invalida. Use el formato hh:mm");
        }
    }

    // Solicitar hora de fin con opción de cancelar
    QTime horaFin;
    while (true) {
        out << "Hora de fin (hh:mm) [0 para volver]: ";
        out.flush();
        QString horaStr = leerLinea();

        // Permitir regresar
        if (horaStr == "0") {
            mostrarInfo("Operacion cancelada");
            pausa();
            return;
        }

        horaFin = QTime::fromString(horaStr, "hh:mm");

        if (horaFin.isValid() && horaFin > horaInicio) {
            break;
        } else if (!horaFin.isValid()) {
            mostrarError("Hora invalida. Use el formato hh:mm");
        } else {
            mostrarError("La hora de fin debe ser posterior a la hora de inicio");
        }
    }

    QDateTime inicio(fecha, horaInicio);
    QDateTime fin(fecha, horaFin);

    HorarioSugerido sugerencia = quirofanoController.validarYSugerirHorario(
        ID_QUIROFANO, inicio, fin
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

void ConsoleView::listarReservasDelDia() {
    limpiarPantalla();
    mostrarSeparador();
    out << "RESERVAS DEL DIA\n";
    mostrarSeparador();

    QDate fecha;
    while (true) {
        out << "\nFecha (dd/mm/aaaa) [Enter para hoy]: ";
        out.flush();
        QString fechaStr = leerLinea();

        if (fechaStr.isEmpty()) {
            fecha = QDate::currentDate();
            break;
        }

        fecha = QDate::fromString(fechaStr, "dd/MM/yyyy");

        if (fecha.isValid()) {
            break;
        } else {
            mostrarError("Fecha invalida. Use el formato dd/mm/aaaa (ejemplo: 25/12/2024)");
        }
    }

    auto reservas = quirofanoController.listarReservasDelDia(fecha);

    if (reservas.isEmpty()) {
        mostrarInfo("No hay reservas para esta fecha");
    } else {
        out << "\n[INFO] Total de reservas: " << reservas.size() << "\n\n";

        for (Reserva* reserva : reservas) {
            out << "----------------------------------------------------\n";
            out << "ID: " << reserva->getId() << "\n";
            out << "Quirofano: " << reserva->getIdQuirofano() << "\n";
            out << "Fecha: " << reserva->getFechaInicio().toString("dd/MM/yyyy") << "\n";
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

    const int ID_QUIROFANO = 1;
    QDateTime ahora = QDateTime::currentDateTime();

    // Obtener todas las reservas del quirófano
    auto todasReservas = quirofanoController.listarReservasPorQuirofano(ID_QUIROFANO);

    // Filtrar solo las futuras
    QVector<Reserva*> reservasFuturas;
    for (Reserva* reserva : todasReservas) {
        if (reserva->getFechaInicio() >= ahora &&
            reserva->getEstado() != EstadoReserva::CANCELADA) {
            reservasFuturas.append(reserva);
        } else {
            delete reserva;  // Liberar las que no usaremos
        }
    }

    if (reservasFuturas.isEmpty()) {
        mostrarInfo("No hay reservas futuras para cancelar");
        pausa();
        return;
    }

    // Mostrar reservas futuras
    out << "\n[INFO] Reservas futuras disponibles para cancelar:\n\n";

    for (Reserva* reserva : reservasFuturas) {
        out << "----------------------------------------------------\n";
        out << colorCyan(QString("ID: %1").arg(reserva->getId())) << "\n";
        out << "Fecha: " << reserva->getFechaInicio().toString("dd/MM/yyyy") << "\n";
        out << "Horario: " << reserva->getFechaInicio().toString("hh:mm")
            << " - " << reserva->getFechaFin().toString("hh:mm") << "\n";
        out << "Motivo: " << reserva->getMotivoCirugia() << "\n";
        out << "Estado: " << Reserva::estadoToString(reserva->getEstado()) << "\n";
    }
    out << "----------------------------------------------------\n\n";

    // Solicitar ID de reserva a cancelar
    int idReserva = leerEntero("ID de la reserva a cancelar [0 para volver]", 0, 999999);

    if (idReserva == 0) {
        mostrarInfo("Operacion cancelada");
        qDeleteAll(reservasFuturas);
        pausa();
        return;
    }

    // Verificar que el ID existe en las futuras
    bool encontrada = false;
    for (Reserva* reserva : reservasFuturas) {
        if (reserva->getId() == idReserva) {
            encontrada = true;
            break;
        }
    }

    if (!encontrada) {
        mostrarError("ID de reserva no valido o no es una reserva futura");
        qDeleteAll(reservasFuturas);
        pausa();
        return;
    }

    // Confirmar cancelación
    out << "\n" << colorAmarillo("Esta seguro de cancelar esta reserva?") << " (s/n): ";
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

    qDeleteAll(reservasFuturas);
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
