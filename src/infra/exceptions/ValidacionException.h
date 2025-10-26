#ifndef VALIDACIONEXCEPTION_H
#define VALIDACIONEXCEPTION_H

#include <QString>
#include <exception>

class ValidacionException : public std::exception {
public:
    explicit ValidacionException(const QString& mensaje);
    const char* what() const noexcept override;
    QString getMensaje() const;

private:
    QString mensaje;
    mutable QByteArray mensajeBytes;
};

#endif
