QT = core sql network
QT -= gui

CONFIG += c++17 cmdline console

# ============================================
# CONFIGURACION DE MySQL/MariaDB POR PLATAFORMA
# ============================================

unix:!macx {
    LIBS += -lmariadb
    INCLUDEPATH += /usr/include/mariadb
}

macx {
    LIBS += -L/usr/local/mysql/lib -lmysqlclient
    INCLUDEPATH += /usr/local/mysql/include
}

win32 {
    # Usa la ruta sin espacios donde copiaste MySQL Connector C
    LIBS += -L"C:/MySQL/ConnectorC/lib" -llibmysql
    INCLUDEPATH += "C:/MySQL/ConnectorC/include"
}

DEFINES += QT_DEPRECATED_WARNINGS

# ============================================
# HEADERS
# ============================================
HEADERS += \
    src/config/DatabaseConfig.h \
    src/config/FirebaseConfig.h \
    src/domain/usuario/Usuario.h \
    src/domain/usuario/UsuarioRepository.h \
    src/domain/usuario/DatosAutenticacion.h \
    src/domain/usuario/AutenticacionService.h \
    src/domain/quirofano/Quirofano.h \
    src/domain/quirofano/QuirofanoRepository.h \
    src/domain/quirofano/Reserva.h \
    src/domain/quirofano/ReservaService.h \
    src/domain/quirofano/SugerenciaAgenda.h \
    src/domain/sensor/Sensor.h \
    src/domain/sensor/SensorRepository.h \
    src/domain/actuador/Actuador.h \
    src/domain/actuador/ActuadorRepository.h \
    src/controller/AutenticacionController.h \
    src/controller/QuirofanoController.h \
    src/controller/IoTController.h \
    src/infra/security/SecurityManager.h \
    src/infra/exceptions/ValidacionException.h \
    src/view/ConsoleView.h \
    src/controller/DemoController.h \
    src/domain/usuario/UsuarioFirestoreRepository.h \
    src/domain/quirofano/ReservaFirestoreRepository.h \
    src/domain/sensor/LecturaSensor.h


# ============================================
# SOURCES
# ============================================
SOURCES += \
    main.cpp \
    src/config/DatabaseConfig.cpp \
    src/config/FirebaseConfig.cpp \
    src/domain/usuario/Usuario.cpp \
    src/domain/usuario/UsuarioRepository.cpp \
    src/domain/usuario/AutenticacionService.cpp \
    src/domain/quirofano/Quirofano.cpp \
    src/domain/quirofano/QuirofanoRepository.cpp \
    src/domain/quirofano/Reserva.cpp \
    src/domain/quirofano/ReservaService.cpp \
    src/domain/quirofano/SugerenciaAgenda.cpp \
    src/domain/sensor/Sensor.cpp \
    src/domain/sensor/SensorRepository.cpp \
    src/domain/actuador/Actuador.cpp \
    src/domain/actuador/ActuadorRepository.cpp \
    src/controller/AutenticacionController.cpp \
    src/controller/QuirofanoController.cpp \
    src/controller/IoTController.cpp \
    src/infra/security/SecurityManager.cpp \
    src/infra/exceptions/ValidacionException.cpp \
    src/controller/DemoController.cpp \
    src/view/ConsoleView.cpp \
    src/domain/usuario/UsuarioFirestoreRepository.cpp \
    src/domain/quirofano/ReservaFirestoreRepository.cpp


# ============================================
# OTROS ARCHIVOS (visibles en Qt Creator)
# ============================================
OTHER_FILES += \
    resources/schema.sql \
    .env.example \
    .env \
    README.md \
    .gitignore

# ============================================
# COPIAR .env AL DIRECTORIO DE BUILD (MULTIPLATAFORMA)
# ============================================

# Define rutas de origen y destino
ENV_SRC = $$PWD/.env
ENV_DEST = $$OUT_PWD/.env

# Solo copiar si el archivo existe
exists($$PWD/.env) {
    win32 {
        ENV_SRC ~= s,/,\\,g
        ENV_DEST ~= s,/,\\,g
        QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$ENV_SRC) $$quote($$ENV_DEST) $$escape_expand(\\n\\t)
    }

    unix {
        QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$ENV_SRC) $$quote($$ENV_DEST) $$escape_expand(\\n\\t)
    }
}

# ============================================
# CONFIGURACION
# ============================================
INCLUDEPATH += src

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

TARGET = QuirIOT
VERSION = 1.0.0
