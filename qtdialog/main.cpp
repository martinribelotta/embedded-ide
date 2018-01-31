#include <QApplication>
#include <QCommandLineParser>
#include <QtWidgets>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.connect(&a, &QApplication::lastWindowClosed, &a, &QApplication::quit);
    QCommandLineParser opt;
    opt.addHelpOption();
    opt.addVersionOption();
    opt.addOptions({
                       { "msgbox", "Show message box", "text" },
                       { "error", "Show error text", "text" },
                       { "yesno", "Show [yes] [no] msbox", "text" }
                       // TODO add more commands like kdialog/zenity/xdialog/dialog
                   });
    opt.process(a);
    if (opt.isSet("msgbox")) {
        QTimer::singleShot(0, [&opt]() {
            QMessageBox::information(nullptr, qAppName(), opt.value("msgbox"), QMessageBox::Close);
            QApplication::instance()->exit(0);
        });
    } else if (opt.isSet("error")) {
        QTimer::singleShot(0, [&opt]() {
            QMessageBox::critical(nullptr, qAppName(), opt.value("msgbox"), QMessageBox::Close);
            QApplication::instance()->exit(0);
        });
    } else if (opt.isSet("yesno")) {
        QTimer::singleShot(0, [&opt]() {
            auto a = QApplication::instance();
            switch(QMessageBox::question(nullptr, qAppName(), opt.value("yesno"), QMessageBox::Yes | QMessageBox::No)) {
            case QMessageBox::Yes: a->exit(1); break;
            case QMessageBox::No: a->exit(0); break;
            default: a->exit(-1); break;
            }
        });
    } else {
        opt.showHelp(0);
    }
    return a.exec();
}
