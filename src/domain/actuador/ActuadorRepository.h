#ifndef ACTUADORREPOSITORY_H
#define ACTUADORREPOSITORY_H

#include "Actuador.h"
#include <QSqlDatabase>
#include <QVector>

class ActuadorRepository {
public:
    explicit ActuadorRepository(QSqlDatabase& db);

    QVector<Actuador*> listarPorQuirofano(int idQuirofano);

private:
    QSqlDatabase& database;
};

#endif
