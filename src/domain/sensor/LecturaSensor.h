#ifndef LECTURASENSOR_H
#define LECTURASENSOR_H

#include <QDateTime>
#include <QString>

struct LecturaSensor {
    int idSensor;
    double valor;
    QString tipo;
    QDateTime timestamp;
    
    LecturaSensor(int id, double v, const QString& t) 
        : idSensor(id), valor(v), tipo(t), timestamp(QDateTime::currentDateTime()) {}
};

#endif // LECTURASENSOR_H
