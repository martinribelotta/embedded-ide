#include "processlinebufferizer.h"

#include <QProcess>

ProcessLineBufferizer::ProcessLineBufferizer(Channel ch, QProcess *parent) : QObject(parent)
{
    if (ch == StderrChannel) {
        connect(parent, &QProcess::readyReadStandardError, this, [this, parent]() {
            pushData(parent->readAllStandardError());
        });
    } else if (ch == StdoutChannel) {
        connect(parent, &QProcess::readyReadStandardOutput, this, [this, parent]() {
            pushData(parent->readAllStandardOutput());
        });
    } else if (ch == MergedChannel) {
        parent->setProcessChannelMode(QProcess::MergedChannels);
        connect(parent, &QProcess::readyRead, this, [this, parent]() {
            pushData(parent->readAll());
        });
    }
}

void ProcessLineBufferizer::pushData(const QString &text)
{
    m_buffer.append(text);
    int idx = m_buffer.lastIndexOf('\n');
    if (idx != -1) {
        auto line = m_buffer.mid(0, idx + 1);
        emit haveLine(line);
        m_buffer.remove(0, idx + 1);
    }
}

void ProcessLineBufferizer::flush()
{
    emit haveLine(m_buffer);
    m_buffer.clear();
}
