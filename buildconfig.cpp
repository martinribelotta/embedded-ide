#include "buildconfig.h"

#include <QtGui>
#include <QtCore>

#if QT_VERSION > 0x050000
#include <QtWidget>
#endif

#include "programmsettings.h"

struct BuildConfig::Priv_t {
    QLineEdit *compilerPrefix;
    QTableView *opSimpleView;
    QStandardItemModel *opSimpleModel;
};

static QString runAndCaptureStdout(const QString& command) {
    QProcess p;
    p.start(command);
    p.waitForFinished();
    return p.readAllStandardOutput();
}

static QHash<QString, QString> getSimpleFlag(const QString& out) {
    QHash<QString, QString> list;
    QStringList result = out.split("\n");
    QRegExp re(QString("\\s{2}(\\-{1,2}[\\w\\-]*)\\s{3,}(.*)"));
    int nline = 0;
    while(nline < result.size()) {
        if (re.indexIn(result.at(nline)) != -1) {
            QString flag = re.cap(1);
            QString help = re.cap(2).trimmed();
            nline++;
            if (result.at(nline).startsWith("    "))
                help += QChar(' ') + result.at(nline).trimmed();
            if (flag.contains("help"))
                continue;
            if (help.contains("This switch lacks documentation"))
                continue;
            list.insert(flag, help);
        } else
            nline++;
    }
    return list;
}

BuildConfig::BuildConfig(QWidget *parent) :
    QWidget(parent), priv(new Priv_t)
{
    ProgrammSettings s;
    QFormLayout *l = new QFormLayout(this);
    l->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    l->addRow(new QLabel(tr("Compiler prefix")), priv->compilerPrefix = new QLineEdit(this));
    l->addRow(priv->opSimpleView = new QTableView(this));
    priv->opSimpleModel = new QStandardItemModel(0, 2, this);
    priv->opSimpleView->setModel(priv->opSimpleModel);
    priv->opSimpleView->setSelectionBehavior(QAbstractItemView::SelectRows);

    QString out = runAndCaptureStdout(QString("arm-none-eabi-gcc "
                                              "--help=common "
                                              "--help=optimizers "
                                              "--help=params "
                                              "--help=target "
                                              "--help=warnings"));
    QHash<QString, QString> h = getSimpleFlag(out);
    QStringList keys = h.keys();
    keys.sort();
    foreach(QString k, keys) {
        QStandardItem* flag = new QStandardItem(k);
        QStandardItem* help = new QStandardItem(h.value(k));
        flag->setCheckable(true);
        flag->setCheckState(Qt::Unchecked);
        flag->setEditable(false);
        help->setEditable(false);
        priv->opSimpleModel->appendRow(QList<QStandardItem*>() << flag << help);
    }
    priv->opSimpleView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

    l->addItem(new QSpacerItem(0, 120, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

BuildConfig::~BuildConfig()
{
    delete priv;
}

void BuildConfig::save()
{

}

void BuildConfig::load()
{

}
