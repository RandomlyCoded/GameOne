#include "backend.h"
#include "inventorymodel.h"
#include "levelmodel.h"
#include "mapmodel.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

namespace GameOne {

class Application : public QGuiApplication
{
    Q_OBJECT

public:
    using QGuiApplication::QGuiApplication;

    int run();
};

int Application::run()
{
    QQmlApplicationEngine qml;

    qmlRegisterType<InventoryModel>("GameOne", 1, 0, "InventoryModel");
    qmlRegisterType<LevelModel>("GameOne", 1, 0, "LevelModel");
    qmlRegisterType<MapModel>("GameOne", 1, 0, "MapModel");

    const auto backend = new Backend{this};
    backend->load(arguments().count() > 1 ? arguments().at(1) : "level1.json");
    qml.rootContext()->setContextProperty("backend", backend);

    qml.load(QUrl{"qrc:/GameOne/qml/main.qml"});
    if (qml.rootObjects().isEmpty())
        return EXIT_FAILURE;

    return exec();
}

} // namespace GameOne

int main(int argc, char *argv[])
{
    return GameOne::Application{argc, argv}.run();
}

#include "main.moc"
