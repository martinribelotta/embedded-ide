#ifndef PROGRAMMSETTINGS_H
#define PROGRAMMSETTINGS_H

#include <QSettings>

class ProgrammSettings : public QSettings {
public:
    ProgrammSettings() : QSettings("org.nonprofit.nonsense.void", "eIDE") {}

    const QString buildCommand() const {
        return value("build/command").toString();
    }

    const QString compilerPrefix() const {
        return value("build/compiler/prefix", "arm-none-eabi-").toString();
    }

    void setCompilerPrefix(const QString& v) {
        setValue("build/compiler/prefix", v);
    }

    void setBuildCommand(const QString& v) {
        setValue("build/command", v);
    }

    const QStringList memLayout() const {
        return value("mem/layout").toStringList();
    }

    void setMemLayout(const QStringList &config) {
        setValue("mem/layout", config);
    }
};

#endif // PROGRAMMSETTINGS_H
