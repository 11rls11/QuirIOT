#include "QuirofanoRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

QuirofanoRepository::QuirofanoRepository(QSqlDatabase& db) : database(db) {}

bool QuirofanoRepository::guardar(Quirofano& quirofano) {
    QSqlQuery query(database);

    query.prepare("INSERT INTO quirofanos (disponibilidad, nombre, capacidad, estado) VALUES (?, ?, ?, ?)");
    query.addBindValue(quirofano.getDisponibilidad());
    query.addBindValue(quirofano.getNombre());
    query.addBindValue(quirofano.getCapacidad());
    query.addBindValue(Quirofano::estadoToString(quirofano.getEstado()));

    if (!query.exec()) {
        qCritical() << "[ERROR] Al guardar quirofano:" << query.lastError().text();
        return false;
    }

    quirofano.setId(query.lastInsertId().toInt());
    qInfo() << "[OK] Quirofano guardado con ID:" << quirofano.getId();
    return true;
}

Quirofano* QuirofanoRepository::buscarPorId(int id) {
    QSqlQuery query(database);

    query.prepare("SELECT * FROM quirofanos WHERE id_quirofano = ?");
    query.addBindValue(id);

    if (!query.exec()) {
        qCritical() << "[ERROR] Al buscar quirofano:" << query.lastError().text();
        return nullptr;
    }

    if (query.next()) {
        return mapearQuirofano(query);
    }

    return nullptr;
}

QVector<Quirofano*> QuirofanoRepository::listarTodos() {
    QVector<Quirofano*> quirofanos;
    QSqlQuery query(database);

    if (!query.exec("SELECT * FROM quirofanos ORDER BY nombre")) {
        qCritical() << "[ERROR] Al listar quirofanos:" << query.lastError().text();
        return quirofanos;
    }

    while (query.next()) {
        quirofanos.append(mapearQuirofano(query));
    }

    return quirofanos;
}

QVector<Quirofano*> QuirofanoRepository::listarDisponibles() {
    QVector<Quirofano*> quirofanos;
    QSqlQuery query(database);

    query.prepare("SELECT * FROM quirofanos WHERE estado = ? ORDER BY nombre");
    query.addBindValue("DISPONIBLE");

    if (query.exec()) {
        while (query.next()) {
            quirofanos.append(mapearQuirofano(query));
        }
    }

    return quirofanos;
}

bool QuirofanoRepository::actualizarEstado(int id, EstadoQuirofano estado) {
    QSqlQuery query(database);

    query.prepare("UPDATE quirofanos SET estado = ? WHERE id_quirofano = ?");
    query.addBindValue(Quirofano::estadoToString(estado));
    query.addBindValue(id);

    if (!query.exec()) {
        qCritical() << "[ERROR] Al actualizar estado:" << query.lastError().text();
        return false;
    }

    return true;
}

bool QuirofanoRepository::estaDisponible(int id, const QDateTime& inicio, const QDateTime& fin) {
    QSqlQuery query(database);

    query.prepare(
        "SELECT COUNT(*) FROM reservan "
        "WHERE id_quirofano = ? AND estado_reserva != 'CANCELADA' "
        "AND NOT (fecha_fin <= ? OR fecha_inicio >= ?)"
        );
    query.addBindValue(id);
    query.addBindValue(inicio);
    query.addBindValue(fin);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() == 0;
    }

    return false;
}

Quirofano* QuirofanoRepository::mapearQuirofano(const QSqlQuery& query) {
    Quirofano* quirofano = new Quirofano(
        query.value("id_quirofano").toInt(),
        query.value("nombre").toString()
        );

    quirofano->setCapacidad(query.value("capacidad").toInt());
    quirofano->setEstado(Quirofano::stringToEstado(query.value("estado").toString()));
    quirofano->setDisponibilidad(query.value("disponibilidad").toString());

    return quirofano;
}
