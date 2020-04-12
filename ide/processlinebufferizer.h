#ifndef PROCESSLINEBUFFERIZER_H
#define PROCESSLINEBUFFERIZER_H

#include <QObject>

class QProcess;

class ProcessLineBufferizer : public QObject
{
    Q_OBJECT
public:
    enum Channel {
        StdoutChannel, StderrChannel
    };

    explicit ProcessLineBufferizer(Channel ch, QProcess *parent = nullptr);

    const QString& buffer() const { return m_buffer; }

public slots:
    void pushData(const QString &text);
    void flush();

signals:
    void haveLine(const QString& line);

private:
    QString m_buffer;
};

#endif // PROCESSLINEBUFFERIZER_H
