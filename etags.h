#ifndef ETAGS_H
#define ETAGS_H

#include <QMultiHash>
#include <QMetaType>

#include <functional>

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

    bool parse(const QString& path);

    QStringList tagList() const {
        return map.keys();
    }

    QList<Tag> find(const QString& ident) const {
        return map.contains(ident)? map.values(ident) : QList<Tag>();
    }

    const QString& tagFile() const { return m_path; }

    static void etagsStart(const QString &workDir, std::function<void (ETags&)> callback);

    ETags& operator=(const ETags& other) {
        m_path = other.m_path;
        map = other.map;
        map.detach();
        return *this;
    }

private:
    TagMap map;
    QString m_path;
};

Q_DECLARE_METATYPE(ETags::Tag);

#endif // ETAGS_H
