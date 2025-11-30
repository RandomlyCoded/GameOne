#ifndef GAMEONE_APPLICATION_H
#define GAMEONE_APPLICATION_H

#include <QGuiApplication>
#include <QUrl>

namespace GameOne {

class Application : public QGuiApplication
{
    Q_OBJECT

public:
    using QGuiApplication::QGuiApplication;

    int run(const QUrl &qmlRoot = {});
};

} // namespace GameOne

#endif // GAMEONE_APPLICATION_H
