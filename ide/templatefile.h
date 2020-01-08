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

struct TarGzFile {
    QString metadataSuffix;
    QString metadata;

    TarGzFile(const QString& path);
};

class TemplateFile
{
public:
    enum class Type {
        Unknown,
        DiffFile,
        TarGzFile,
    };

    static const QString TEMPLATE_FILEDIALOG_FILTER;

    TemplateFile(const QFileInfo& path);

    Type type() const;

private:
    QFileInfo info;
};

#endif // TEMPLATEFILE_H
