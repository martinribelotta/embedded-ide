#include "childprocess.h"

QProcess *ChildProcess::safeStop(QProcess *p)
{
    p->blockSignals(true);
    if (p->state() == QProcess::Running) {
        p->terminate();
        p->waitForFinished(100);
        if (p->state() == QProcess::Running) {
            p->kill();
            p->waitForFinished(100);
        }
    }
    p->blockSignals(false);
    return p;
}

ChildProcess::~ChildProcess() {
    safeStop(this);
}
