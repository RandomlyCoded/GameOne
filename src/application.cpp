#include "application.h"

#include "backend.h"
#include "imageprovider.h"
#include "inventorymodel.h"
#include "levelmodel.h"
#include "mapmodel.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

static void initResources()
{
    Q_INIT_RESOURCE(assets);
    Q_INIT_RESOURCE(data);
    Q_INIT_RESOURCE(qml);
}

namespace GameOne {

int Application::run(const QUrl &qmlRoot)
{
    if (qmlRoot.isEmpty())
        return run(QUrl{"qrc:/GameOne/qml/main.qml"});

    initResources();

    qmlRegisterUncreatableType<Actor>("GameOne", 1, 0, "Actor", "Cannot construct abstract base class");
    qmlRegisterUncreatableType<Player>("GameOne", 1, 0, "Player", "Managed and created by Backend");
    qmlRegisterUncreatableType<Chest>("GameOne", 1, 0, "Chest", "Managed and created by Backend");
    qmlRegisterUncreatableType<Enemy>("GameOne", 1, 0, "Enemy", "Managed and created by Backend");
    qmlRegisterUncreatableType<Ladder>("GameOne", 1, 0, "Ladder", "Managed and created by Backend");

    qmlRegisterType<InventoryModel>("GameOne", 1, 0, "InventoryModel");
    qmlRegisterType<LevelModel>("GameOne", 1, 0, "LevelModel");
    qmlRegisterType<MapModel>("GameOne", 1, 0, "MapModel");

    auto *const backend = new Backend{this};
    qmlRegisterSingletonInstance<Backend>("GameOne", 1, 0, "Backend", backend);

    const auto levelFileName = arguments().count() > 1
                                   ? arguments().at(1)
                                   : LevelModel::levelFileName(LevelModel::DEFAULT_LEVEL);

    backend->load(levelFileName);

    auto qml = QQmlApplicationEngine{};
    qml.addImageProvider("assets", new ImageProvider);
    qml.load(qmlRoot);

    if (qml.rootObjects().isEmpty())
        return EXIT_FAILURE;

    return exec();
}

} // namespace GameOne

#include "moc_application.cpp"
