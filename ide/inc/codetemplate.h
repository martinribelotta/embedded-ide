#ifndef CODETEMPLATE_H
#define CODETEMPLATE_H

#include <QFileInfo>
#include <QUrl>

class CodeTemplate
{
public:
    CodeTemplate() {}
    CodeTemplate(const QFileInfo& _path) : m_path(_path) {}
    CodeTemplate(const QUrl& _url, const QFileInfo& _path=QFileInfo()) : m_url(_url), m_path(_path) {}

    bool isNull() const { return m_path.absoluteFilePath().isEmpty() && m_url.isEmpty(); }
    bool isLocal() const { return m_path.exists(); }
    QFileInfo path() const { return m_path; }
    QUrl url() const { return m_url; }
    QString templateText() const;

    void setUrl(const QUrl& url) { m_url = url; }
    void setFilePath(const QFileInfo& path) { m_path = path; }

    bool operator ==(const CodeTemplate& o) const { return o.m_path == m_path || o.m_url == m_url; }

private:
    QFileInfo m_path;
    QUrl m_url;
};

uint qHash(const CodeTemplate&);

Q_DECLARE_METATYPE(CodeTemplate)

#endif // CODETEMPLATE_H
