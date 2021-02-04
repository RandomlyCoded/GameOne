#ifndef GAMEONE_IMAGEPROVIDER_H
#define GAMEONE_IMAGEPROVIDER_H

#include <QMutex>
#include <QQuickImageProvider>

namespace GameOne {

class ImageProvider : public QQuickImageProvider
{
public:
    ImageProvider() : QQuickImageProvider{Image} {}
    QImage requestImage(const QString &id, QSize *size, const QSize& requestedSize) override;

private:
    QMap<std::tuple<QString, int, int>, QImage> m_cache;
    QMutex m_cacheMutex;
};

} // namespace GameOne

#endif // GAMEONE_IMAGEPROVIDER_H
