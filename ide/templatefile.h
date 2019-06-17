#ifndef TEMPLATEFILE_H
#define TEMPLATEFILE_H

#include <QString>
#include <QFileInfo>
#include <QHash>

struct DiffParameter {
    QString name;
    QString type;
    QString params;
};

struct DiffFile {
    QString name;
    QString manifest;
    QString diffText;
    QString headerText;
    QList<DiffParameter> parameters;

    DiffFile(const QString& path);
};

class TemplateFile
{
public:
    enum class Type {
        Unknown,
        DiffFile,
        TarGzFile,
        TarGzWithMetaFile,
    };

    using Metadata = QHash<QString, QByteArray>;

    TemplateFile(const QFileInfo& path);

    Type type() const;
    Metadata meta() const { return infometa; }

    static Metadata extractMeta(const QString& path);
private:
    QFileInfo info;
    Metadata infometa;
};

#endif // TEMPLATEFILE_H
