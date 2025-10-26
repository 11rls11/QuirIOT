#include "UsuarioRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

UsuarioRepository::UsuarioRepository(QSqlDatabase& db) : database(db) {}

bool UsuarioRepository::guardar(Usuario& usuario, const QString& passwordHash) {
    QSqlQuery query(database);

    query.prepare("INSERT INTO usuarios (email, contrasena, nombre) VALUES (?, ?, ?)");
    query.addBindValue(usuario.getEmail());
    query.addBindValue(passwordHash);
    query.addBindValue(usuario.getNombre());

    if (!query.exec()) {
        qCritical() << "[ERROR] Al guardar usuario:" << query.lastError().text();
        return false;
    }

    usuario.setId(query.lastInsertId().toInt());

    for (const auto& rol : usuario.getRoles()) {
        QSqlQuery rolQuery(database);
        rolQuery.prepare("INSERT INTO rol_usuario (rol, id_usuario) VALUES (?, ?)");
        rolQuery.addBindValue(Usuario::rolToString(rol));
        rolQuery.addBindValue(usuario.getId());

        if (!rolQuery.exec()) {
            qWarning() << "[WARN] Error al guardar rol:" << rolQuery.lastError().text();
        }
    }

    qInfo() << "[OK] Usuario guardado con ID:" << usuario.getId();
    return true;
}

Usuario* UsuarioRepository::buscarPorId(int id) {
    QSqlQuery query(database);

    query.prepare("SELECT * FROM usuarios WHERE id_usuario = ?");
    query.addBindValue(id);

    if (!query.exec()) {
        qCritical() << "[ERROR] Al buscar usuario por ID:" << query.lastError().text();
        return nullptr;
    }

    if (query.next()) {
        Usuario* usuario = mapearUsuario(query);
        cargarRoles(usuario);
        return usuario;
    }

    return nullptr;
}

Usuario* UsuarioRepository::buscarPorEmail(const QString& email) {
    QSqlQuery query(database);

    query.prepare("SELECT * FROM usuarios WHERE email = ?");
    query.addBindValue(email);

    if (!query.exec()) {
        qCritical() << "[ERROR] Al buscar usuario por email:" << query.lastError().text();
        return nullptr;
    }

    if (query.next()) {
        Usuario* usuario = mapearUsuario(query);
        cargarRoles(usuario);
        return usuario;
    }

    return nullptr;
}

QVector<Usuario*> UsuarioRepository::listarTodos() {
    QVector<Usuario*> usuarios;
    QSqlQuery query(database);

    if (!query.exec("SELECT * FROM usuarios ORDER BY nombre")) {
        qCritical() << "[ERROR] Al listar usuarios:" << query.lastError().text();
        return usuarios;
    }

    while (query.next()) {
        Usuario* usuario = mapearUsuario(query);
        cargarRoles(usuario);
        usuarios.append(usuario);
    }

    return usuarios;
}

QString UsuarioRepository::obtenerPasswordHash(const QString& email) {
    QSqlQuery query(database);

    query.prepare("SELECT contrasena FROM usuarios WHERE email = ?");
    query.addBindValue(email);

    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }

    return QString();
}

bool UsuarioRepository::existeEmail(const QString& email) {
    QSqlQuery query(database);

    query.prepare("SELECT COUNT(*) FROM usuarios WHERE email = ?");
    query.addBindValue(email);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

Usuario* UsuarioRepository::mapearUsuario(const QSqlQuery& query) {
    Usuario* usuario = new Usuario(
        query.value("id_usuario").toInt(),
        query.value("email").toString(),
        query.value("nombre").toString()
        );

    usuario->setFechaCreacion(query.value("fecha_creacion").toDateTime());

    return usuario;
}

void UsuarioRepository::cargarRoles(Usuario* usuario) {
    QSqlQuery query(database);

    query.prepare("SELECT rol FROM rol_usuario WHERE id_usuario = ?");
    query.addBindValue(usuario->getId());

    if (query.exec()) {
        while (query.next()) {
            usuario->agregarRol(Usuario::stringToRol(query.value(0).toString()));
        }
    }
}
