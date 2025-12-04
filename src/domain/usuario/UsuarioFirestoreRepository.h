#ifndef USUARIOFIRESTOREREPOSITORY_H
#define USUARIOFIRESTOREREPOSITORY_H

#include "src/config/FirebaseConfig.h"
#include "Usuario.h"
#include <functional>
#include <QObject>

class UsuarioFirestoreRepository
{
public:
    UsuarioFirestoreRepository();

    void guardar(const Usuario& usuario, std::function<void(bool, QString)> callback);

    void buscarPorEmail(const QString& email, std::function<void(Usuario*, QString)> callback);

    void eliminar(const QString& email, std::function<void(bool, QString)> callback);

private:
    FirebaseConfig& firebase;
};

#endif // USUARIOFIRESTOREREPOSITORY_H
