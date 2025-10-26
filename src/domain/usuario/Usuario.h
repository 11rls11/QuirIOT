#ifndef USUARIO_H
#define USUARIO_H

#include <QString>
#include <QDateTime>
#include <QVector>

enum class RolUsuario {
    ENFERMERO,
    MEDICO,
    ANESTESIOLOGO
};

class Usuario {
public:
    Usuario();
    Usuario(int id, const QString& email, const QString& nombre);

    int getId() const;
    QString getEmail() const;
    QString getNombre() const;
    QVector<RolUsuario> getRoles() const;
    QDateTime getFechaCreacion() const;

    void setId(int id);
    void setEmail(const QString& email);
    void setNombre(const QString& nombre);
    void agregarRol(RolUsuario rol);
    void setFechaCreacion(const QDateTime& fecha);

    bool tieneRol(RolUsuario rol) const;
    QString rolesComoString() const;

    static QString rolToString(RolUsuario rol);
    static RolUsuario stringToRol(const QString& str);

private:
    int id;
    QString email;
    QString nombre;
    QVector<RolUsuario> roles;
    QDateTime fechaCreacion;
};

#endif
