#ifndef USUARIOREPOSITORY_H
#define USUARIOREPOSITORY_H

#include "Usuario.h"
#include <QSqlDatabase>
#include <QVector>
#include <QString>

class UsuarioRepository {
public:
    explicit UsuarioRepository(QSqlDatabase& db);

    bool guardar(Usuario& usuario, const QString& passwordHash);
    Usuario* buscarPorId(int id);
    Usuario* buscarPorEmail(const QString& email);
    QVector<Usuario*> listarTodos();
    QString obtenerPasswordHash(const QString& email);
    bool existeEmail(const QString& email);

private:
    QSqlDatabase& database;

    Usuario* mapearUsuario(const class QSqlQuery& query);
    void cargarRoles(Usuario* usuario);
};

#endif
