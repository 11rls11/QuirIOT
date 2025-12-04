#ifndef RESERVAFIRESTOREREPOSITORY_H
#define RESERVAFIRESTOREREPOSITORY_H

#include "src/config/FirebaseConfig.h"
#include "Reserva.h"
#include <functional>

class ReservaFirestoreRepository
{
public:
    ReservaFirestoreRepository();
    void guardar(const Reserva& reserva, std::function<void(bool, QString)> callback);

private:
    FirebaseConfig& firebase;
};

#endif // RESERVAFIRESTOREREPOSITORY_H
