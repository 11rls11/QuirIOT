#include "Usuario.h"

Usuario::Usuario() : id(0) {}

Usuario::Usuario(int id, const QString& email, const QString& nombre)
    : id(id), email(email), nombre(nombre), fechaCreacion(QDateTime::currentDateTime()) {}

int Usuario::getId() const { return id; }
QString Usuario::getEmail() const { return email; }
QString Usuario::getNombre() const { return nombre; }
QVector<RolUsuario> Usuario::getRoles() const { return roles; }
QDateTime Usuario::getFechaCreacion() const { return fechaCreacion; }

void Usuario::setId(int id) { this->id = id; }
void Usuario::setEmail(const QString& email) { this->email = email; }
void Usuario::setNombre(const QString& nombre) { this->nombre = nombre; }
void Usuario::setFechaCreacion(const QDateTime& fecha) { this->fechaCreacion = fecha; }

void Usuario::agregarRol(RolUsuario rol) {
    if (!tieneRol(rol)) {
        roles.append(rol);
    }
}

bool Usuario::tieneRol(RolUsuario rol) const {
    return roles.contains(rol);
}

QString Usuario::rolesComoString() const {
    QStringList rolesStr;
    for (const auto& rol : roles) {
        rolesStr.append(rolToString(rol));
    }
    return rolesStr.join(", ");
}

QString Usuario::rolToString(RolUsuario rol) {
    switch (rol) {
    case RolUsuario::ENFERMERO: return "ENFERMERO";
    case RolUsuario::MEDICO: return "MEDICO";
    case RolUsuario::ANESTESIOLOGO: return "ANESTESIOLOGO";
    default: return "DESCONOCIDO";
    }
}

RolUsuario Usuario::stringToRol(const QString& str) {
    if (str == "ENFERMERO") return RolUsuario::ENFERMERO;
    if (str == "MEDICO") return RolUsuario::MEDICO;
    if (str == "ANESTESIOLOGO") return RolUsuario::ANESTESIOLOGO;
    return RolUsuario::ENFERMERO;
}
