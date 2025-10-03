#include "imageprovider.h"

#include <QFile>
#include <QImage>
#include <QLoggingCategory>
#include <QPainter>
#include <QStringLiteral>
#include <QRegularExpression>
#include <QSvgRenderer>
#include <QUrlQuery>

namespace GameOne {

namespace {

Q_LOGGING_CATEGORY(lcImages, "GameOne.images");

struct Layer
{
    QString layerId;
    QString xmlId;
};

auto resolveLayers(const QByteArray &data)
{
    QList<Layer> layers;

    QXmlStreamReader svg{data};
    while (!svg.atEnd()) {
        if (!svg.readNextStartElement())
            continue;

       if (svg.name() == u"g") {
            const auto attrs = svg.attributes();
            if (attrs.value("inkscape:groupmode") != u"layer")
                continue;

            layers += {
                attrs.value("inkscape:label").toString(),
                attrs.value("id").toString()
            };
        } else if (svg.name() != u"svg") {
            svg.skipCurrentElement();
        }
    }

    return layers;
}

auto makeRegularExpression(QStringList wildcards)
{
    QStringList expressions;

    for (const auto &pattern: wildcards) {
        const auto wildcardExpression = QRegularExpression::wildcardToRegularExpression(pattern);
        expressions += QRegularExpression::anchoredPattern(wildcardExpression);
    }

    return QRegularExpression(expressions.join("|"));
}

} // namespace

QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    const auto key = std::make_tuple(id, requestedSize.width(), requestedSize.height());

    if (QMutexLocker lock{&m_cacheMutex}; true)
        if (const auto it = m_cache.find(key); it != m_cache.end())
            return *it;

    const auto url = QUrl{id};
    const auto query = QUrlQuery{url.query()};
    const auto hide = query.queryItemValue("hide").split(',', Qt::SkipEmptyParts);
    const auto show = query.queryItemValue("show").split(',', Qt::SkipEmptyParts);
    const auto debug = query.hasQueryItem("debug");

    auto file = QFile{":/GameOne/assets/" + url.path()};
    if (!file.open(QFile::ReadOnly)) {
        qCWarning(lcImages, "%ls: Could not read %ls: %ls",
                  qUtf16Printable(id), qUtf16Printable(file.fileName()),
                  qUtf16Printable(file.errorString()));
        return {};
    }

    const auto data = file.readAll();
    auto svg = QSvgRenderer{data};
    if (!svg.isValid()) {
        qCWarning(lcImages, "%ls: Not a valid SVG image: %ls",
                  qUtf16Printable(id), qUtf16Printable(file.fileName()));
        return {};
    }

    const auto imageSize = requestedSize.isValid() ? requestedSize : svg.defaultSize();
    if (imageSize.isNull())
        return {};

    auto image = QImage{imageSize, QImage::Format_ARGB32};
    image.fill(0);

    QPainter painter;

    if (!painter.begin(&image)) {
        qCWarning(lcImages, "%ls: Could not start painting", qUtf16Printable(id));
        return {};
    }

    if (debug)
        qCInfo(lcImages, "%ls: %dx%d", qUtf16Printable(id), imageSize.width(), imageSize.height());

    if (hide.isEmpty() && show.isEmpty()) {
        svg.render(&painter);
    } else {
        const auto layers = resolveLayers(data);

        if (debug)
            qCInfo(lcImages, "- #layers=%d", static_cast<int>(layers.count()));

        const auto hidePattern = makeRegularExpression(hide);
        const auto showPattern = makeRegularExpression(show);

        for (const auto &l: layers) {
            if (!hide.isEmpty() && hidePattern.match(l.layerId).hasMatch())
                continue;
            if (!show.isEmpty() && !showPattern.match(l.layerId).hasMatch())
                continue;

            if (debug) {
                qCInfo(lcImages, "- rendering layer \"%ls\" (aka. \"%ls\")",
                       qUtf16Printable(l.layerId), qUtf16Printable(l.xmlId));
            }

            const auto viewBox = svg.viewBoxF().size();
            auto sx = static_cast<qreal>(imageSize.width()) / viewBox.width();
            auto sy = static_cast<qreal>(imageSize.height()) / viewBox.height();
            const auto bounds = QTransform{}.scale(sx, sy).mapRect(svg.boundsOnElement(l.xmlId));
            svg.render(&painter, l.xmlId, bounds);
        }
    }

    painter.end();

    if (QMutexLocker lock{&m_cacheMutex}; true)
        m_cache.insert(key, image);

    if (size)
        *size = image.size();

    return image;
}

} // namespace GameOne
