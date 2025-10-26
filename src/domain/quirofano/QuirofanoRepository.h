#ifndef QUIROFANOREPOSITORY_H
#define QUIROFANOREPOSITORY_H

#include "Quirofano.h"
#include <QSqlDatabase>
#include <QVector>

class QuirofanoRepository {
public:
    explicit QuirofanoRepository(QSqlDatabase& db);

    bool guardar(Quirofano& quirofano);
    Quirofano* buscarPorId(int id);
    QVector<Quirofano*> listarTodos();
    QVector<Quirofano*> listarDisponibles();
    bool actualizarEstado(int id, EstadoQuirofano estado);
    bool estaDisponible(int id, const QDateTime& inicio, const QDateTime& fin);

private:
    QSqlDatabase& database;
    Quirofano* mapearQuirofano(const class QSqlQuery& query);
};

#endif
