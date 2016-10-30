#ifndef ETAGS_H
#define ETAGS_H

#include <QMultiHash>
#include <QMetaType>
#include <QString>
#include <QStringList>

class QIODevice;

class ETags
{
public:
    struct Tag {
        QString file;
        QString decl;
        int line;
        int offset;

        Tag() {}

        bool isEmpty() {
            return file.isEmpty();
        }
    };
    typedef QMultiHash<QString, Tag> TagMap;

    ETags() {}

    bool isValid() { return !tagList().isEmpty(); }

    bool parse(QIODevice *in);

    QStringList tagList() const {
        return map.keys();
    }

    QList<Tag> find(const QString& ident) const {
        return map.contains(ident)? map.values(ident) : QList<Tag>();
    }

    const QString& tagFile() const { return m_path; }

private:
    TagMap map;
    QString m_path;
};

Q_DECLARE_METATYPE(ETags::Tag)

#endif // ETAGS_H
