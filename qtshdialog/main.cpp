/*
 * This file is part of qtshdialog, utility of Embedded-IDE
 *
 * Copyright 2019 Martin Ribelotta <martinribelotta@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <QApplication>
#include <QDialog>
#include <QFile>
#include <QFileInfo>
#include <QJSEngine>
#include <QJsonObject>
#include <QLabel>
#include <QLayout>
#include <QMetaProperty>
#include <QRegularExpression>
#include <QTextStream>
#include <QTimer>
#include <QUiLoader>
#include <QWidget>

static bool noEmpty = false;
static bool noInternal = false;
static bool noLayout = false;
static bool noLabel = false;
static bool noAll = false;

static QStringList propertyList;

static QTextStream& out() {
    static QTextStream sout(stdout);
    return sout;
}

QStringList parentsOf(QObject *obj) {
    QStringList parentList;
    while (obj) {
        auto parent = obj->objectName();
        if (parent.isEmpty())
            parent = "<noname>";
        parentList.push_front(parent);
        obj = obj->parent();
    }
    return parentList;
}

void printProperty(QObject *obj, const QString& name, const QString& type, const QVariant& value) {
    if (noInternal && name.startsWith("_q_"))
        return;
    auto fullName = (parentsOf(obj) << name).join(".");
    if (noAll && !propertyList.contains(fullName))
        return;
    endl(out() << "property:" << fullName << "[" << type << "]:" << value.toString());
}

void printProperty(QObject *obj, const QMetaProperty& prop) {
    auto name = QString{ prop.name() };
    auto type = QString{ prop.typeName() };
    auto value = prop.read(obj);
    printProperty(obj, name, type, value);
}

void dump(QObject *obj) {
    auto objectName = obj->objectName();
    if (noEmpty && objectName.isEmpty())
        return;
    if (noInternal && objectName.startsWith("_q_"))
        return;
    if (noLayout && qobject_cast<QLayout*>(obj))
        return;
    if (noLabel && qobject_cast<QLabel*>(obj))
        return;
    auto meta = obj->metaObject();
    for(int i=meta->propertyOffset(); i<meta->propertyCount(); i++) {
        printProperty(obj, meta->property(i));
    }
    for(auto& n: obj->dynamicPropertyNames()) {
        auto v = obj->property(n.data());
        auto t = QString(v.typeName());
        printProperty(obj, n, t, v);
    }
    for(auto *c: obj->findChildren<QObject*>())
        dump(c);
}

void publishToJs(QJSEngine& js, QObject *obj, QJSValue parent) {
    auto jsObj = js.newQObject(obj);
    auto name = obj->objectName();
    if (name.isEmpty())
        return;
    parent.setProperty(name, jsObj);
    for(auto *child: obj->children())
        publishToJs(js, child, jsObj);
}

static constexpr auto USAGE_HELP = R"(usage:
   %1 [options] filename.ui [filename.js]
Options:
  --no-label             Not show QLabel and derived
  --no-empty             Not show unnamed objects
  --no-layout            Not show QLayout and derived
  --no-internal          Not show internal properties
  --no-all               Not show any properties except specified with --show
  --show=object.property With --no-all, show sepcified property
  --exec=expression      Execute script before show ui
Files:
    filename.ui:         Required ui file for gui
    filename.js:         Optional javascript, execured after all --exec=
)";

void usage() {
    auto name = QFileInfo(QApplication::instance()->applicationFilePath()).fileName();
    flush(out() << QString(USAGE_HELP).arg(name));
}

static QString readAll(const QString& path, bool *ok=nullptr) {
    QFile f{path};
    if (!f.open(QFile::ReadOnly)) {
        if (ok)
            *ok = false;
        return f.errorString();
    }
    if (ok)
        *ok = true;
    return QTextStream{&f}.readAll();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QStringList exprList;
    QString uiFileName;
    QString jsFileName;
    QRegularExpression execRe(R"(^\-\-exec\=(.*)$)", QRegularExpression::MultilineOption);
    for (int i=1; i<argc; i++) {
        auto p = QString{argv[i]};
        if (p == "--no-label")
            noLabel = true;
        if (p == "--no-empty")
            noEmpty = true;
        if (p == "--no-layout")
            noLayout = true;
        if (p == "--no-internal")
            noInternal = true;
        if (p == "--no-all")
            noAll = true;

        if (QFileInfo{p}.suffix() == "ui")
            uiFileName = p;

        if (QFileInfo{p}.suffix() == "js")
            jsFileName = p;

        if (p.startsWith("--show=")) {
            auto name = p.split("=").at(1);
            propertyList.append(name);
        }

        auto m = execRe.match(p);
        if (m.hasMatch()) {
            auto expr = m.captured(1);
            exprList.append(expr);
        }
    }

    if (uiFileName.isEmpty()) {
        usage();
        return -1;
    }

    QFile f{argv[1]};
    if (!f.open(QFile::ReadOnly)) {
        endl(out() << "error open file " << argv[1] << ": " << f.errorString());
        return -1;
    }
    QUiLoader loader;
    auto *w = loader.load(&f);
    f.close();
    if (!w) {
        endl(out() << "error loading ui: " << argv[1] << ": " << loader.errorString());
        return -1;
    }

    QJSEngine js;
    js.installExtensions(QJSEngine::ConsoleExtension);
    publishToJs(js, w, js.globalObject());

    QTimer::singleShot(0, [w, &js, &exprList, &jsFileName]() {
        for(const auto& e: exprList) {
            auto r = js.evaluate(e);
            if (r.isError()) {
                endl(out() << "Uncaught exception at --exec"
                           << r.property("lineNumber").toInt()
                           << ":" << r.toString());
            }
        }
        auto script = w->property("script").toString();
        if (!script.isEmpty()) {
            auto result = js.evaluate(script, QString("%1.script").arg(w->objectName()));
            if (result.isError())
                endl(out() << "Uncaught exception at line script:"
                           << result.property("lineNumber").toInt()
                           << ":" << result.toString());
        }
        if (!jsFileName.isEmpty()) {
            bool ok = false;
            script = readAll(jsFileName, &ok);
            if (!ok) {
                endl(out() << "Error loading file " << jsFileName << ": " << script);
            } else {
                auto result = js.evaluate(script, jsFileName);
                if (result.isError())
                    endl(out() << "Uncaught exception at " << jsFileName << ":"
                               << result.property("lineNumber").toInt()
                               << ":" << result.toString());
            }
        }
    });

    QDialog *d = qobject_cast<QDialog*>(w);
    if (d) {
        a.connect(d, &QDialog::finished, [d](int r) {
            d->setProperty("isAccepted", bool(r == QDialog::Accepted));
        });
    }
    w->show();
    auto r = a.exec();
    w->setProperty("script", {});
    dump(w);
    return r;
}
