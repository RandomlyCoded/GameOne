#include "imageprovider.h"

#include <QFile>
#include <QImage>
#include <QLoggingCategory>
#include <QPainter>
#include <QRegularExpression>
#include <QStringLiteral>
#include <QSvgRenderer>
#include <QUrlQuery>

using namespace Qt::StringLiterals;

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

auto makeRegularExpression(const QStringList &wildcards)
{
    QStringList expressions;

    for (const auto &pattern: wildcards) {
        const auto wildcardExpression = QRegularExpression::wildcardToRegularExpression(pattern);
        expressions += QRegularExpression::anchoredPattern(wildcardExpression);
    }

    return QRegularExpression(expressions.join("|"));
}

struct LayerOptions
{
    static LayerOptions fromId(const QString &id);

    QString     id;
    QString     filePath;
    QStringList hide;
    QStringList show;
    bool        debug;
};

LayerOptions LayerOptions::fromId(const QString &id)
{
    const auto url   = QUrl{id};
    const auto query = QUrlQuery{url.query()};

    return {
        .id       = id,
        .filePath = u":/GameOne/assets/"_s + url.path(),
        .hide     = query.queryItemValue("hide").split(',', Qt::SkipEmptyParts),
        .show     = query.queryItemValue("show").split(',', Qt::SkipEmptyParts),
        .debug    = query.hasQueryItem("debug"),
    };
}

void renderLayers(QPainter &painter, QSvgRenderer &svg, const QList<Layer> &layerList,
                  const LayerOptions &options, const QSize &imageSize)
{
    if (options.debug)
        qCInfo(lcImages, "- #layers=%d", static_cast<int>(layerList.count()));

    const auto hidePattern = makeRegularExpression(options.hide);
    const auto showPattern = makeRegularExpression(options.show);

    for (const auto &layer: layerList) {
        if (!options.hide.isEmpty() && hidePattern.match(layer.layerId).hasMatch())
            continue;
        if (!options.show.isEmpty() && !showPattern.match(layer.layerId).hasMatch())
            continue;

        if (options.debug) {
            qCInfo(lcImages, "- rendering layer \"%ls\" (aka. \"%ls\")",
                   qUtf16Printable(layer.layerId), qUtf16Printable(layer.xmlId));
        }

        const auto viewBox = svg.viewBoxF().size();
        auto sx = static_cast<qreal>(imageSize.width()) / viewBox.width();
        auto sy = static_cast<qreal>(imageSize.height()) / viewBox.height();
        const auto bounds = QTransform{}.scale(sx, sy).mapRect(svg.boundsOnElement(layer.xmlId));
        svg.render(&painter, layer.xmlId, bounds);
    }
}

QImage renderImage(const QByteArray &data, const LayerOptions &options, const QSize &requestedSize)
{
    auto svg = QSvgRenderer{data};

    if (!svg.isValid()) {
        qCWarning(lcImages, "%ls: Not a valid SVG image: %ls",
                  qUtf16Printable(options.id),
                  qUtf16Printable(options.filePath));

        return {};
    }

    const auto imageSize = requestedSize.isValid() ? requestedSize : svg.defaultSize();

    if (imageSize.isNull())
        return {};

    auto image = QImage{imageSize, QImage::Format_ARGB32};
    image.fill(0);

    auto painter = QPainter{};

    if (!painter.begin(&image)) {
        qCWarning(lcImages, "%ls: Could not start painting", qUtf16Printable(options.id));
        return {};
    }

    if (options.debug)
        qCInfo(lcImages, "%ls: %dx%d", qUtf16Printable(options.id), imageSize.width(), imageSize.height());

    if (options.hide.isEmpty() && options.show.isEmpty()) {
        svg.render(&painter);
    } else {
        renderLayers(painter, svg, resolveLayers(data), options, imageSize);
    }

    painter.end();

    return image;
}

} // namespace

QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    const auto key = std::make_tuple(id, requestedSize.width(), requestedSize.height());

    if (QMutexLocker lock{&m_cacheMutex}; true) {
        if (const auto it = m_cache.find(key); it != m_cache.end())
            return *it;
    }

    const auto options = LayerOptions::fromId(id);
    auto file = QFile{options.filePath};

    if (!file.open(QFile::ReadOnly)) {
        qCWarning(lcImages, "%ls: Could not read %ls: %ls",
                  qUtf16Printable(id), qUtf16Printable(file.fileName()),
                  qUtf16Printable(file.errorString()));
        return {};
    }

    auto image = renderImage(file.readAll(), options, requestedSize);

    if (QMutexLocker lock{&m_cacheMutex}; true)
        m_cache.insert(key, image);

    if (size != nullptr)
        *size = image.size();

    return image;
}

} // namespace GameOne
