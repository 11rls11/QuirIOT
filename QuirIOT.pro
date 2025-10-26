QT = core sql network
QT -= gui

CONFIG += c++17 cmdline console

# Configuracion de MariaDB/MySQL para Arch Linux
unix:!macx {
    LIBS += -lmariadb
    INCLUDEPATH += /usr/include/mariadb
}

macx {
    LIBS += -L/usr/local/mysql/lib -lmysqlclient
    INCLUDEPATH += /usr/local/mysql/include
}

win32 {
    LIBS += -L"C:/Program Files/MySQL/MySQL Server 8.0/lib" -lmysqlclient
    INCLUDEPATH += "C:/Program Files/MySQL/MySQL Server 8.0/include"
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
    src/domain/sensor/Sensor.h \
    src/domain/sensor/SensorRepository.h \
    src/domain/actuador/Actuador.h \
    src/domain/actuador/ActuadorRepository.h \
    src/controller/AutenticacionController.h \
    src/controller/QuirofanoController.h \
    src/controller/IoTController.h \
    src/infra/security/SecurityManager.h \
    src/infra/exceptions/ValidacionException.h \
    src/view/ConsoleView.h

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
    src/domain/sensor/Sensor.cpp \
    src/domain/sensor/SensorRepository.cpp \
    src/domain/actuador/Actuador.cpp \
    src/domain/actuador/ActuadorRepository.cpp \
    src/controller/AutenticacionController.cpp \
    src/controller/QuirofanoController.cpp \
    src/controller/IoTController.cpp \
    src/infra/security/SecurityManager.cpp \
    src/infra/exceptions/ValidacionException.cpp \
    src/view/ConsoleView.cpp

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
# COPIAR .env AL DIRECTORIO DE BUILD
# ============================================
QMAKE_POST_LINK += $$quote(cp -f $$PWD/.env $$OUT_PWD/.env$$escape_expand(\\n\\t))

# ============================================
# CONFIGURACION
# ============================================
INCLUDEPATH += src

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

TARGET = QuirIOT
VERSION = 1.0.0
