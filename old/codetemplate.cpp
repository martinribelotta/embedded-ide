#include "codetemplate.h"

#include <QNetworkAccessManager>
#include <QTextStream>

QString CodeTemplate::templateText() const
{
    if (!isLocal())
        return QString();
    QFile f(m_path.absoluteFilePath());
    if (!f.open(QFile::ReadOnly))
        return QString();
    return QTextStream(&f).readAll();
}

uint qHash(const CodeTemplate &self)
{
    return self.url().isValid()?
                qHash(QFileInfo(self.url().fileName()).baseName()) :
                qHash(self.path().baseName());
}
