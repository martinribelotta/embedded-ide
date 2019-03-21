#include "filepropertiesdialog.h"
#include "ui_filepropertiesdialog.h"

#include <QtDebug>

#ifndef Q_OS_WIN
#include <unistd.h>
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>

static uint getCurrentUID() {
    return static_cast<uint>(::getuid());
}

#else

static uint getCurrentUID() {
    return static_cast<uint>(-2);
}

#endif

FilePropertiesDialog::FilePropertiesDialog(const QFileInfo &info, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FilePropertiesDialog),
    thisFile(info)
{
    ui->setupUi(this);
    ui->fileName->setText(thisFile.fileName());
    ui->fileOwner->setText(thisFile.group());
    ui->fileGroup->setText(thisFile.group());
    bool canModify = thisFile.permission(QFile::WriteUser) || thisFile.ownerId() == getCurrentUID();
    ui->fileName->setReadOnly(!canModify);
    // FIX: cannot modify owner:group
    ui->fileOwner->setReadOnly(true);
    ui->fileGroup->setReadOnly(true);
    ui->ownerPerms->setEnabled(canModify);
    ui->groupPerms->setEnabled(canModify);
    ui->otherPerms->setEnabled(canModify);

#define _(a, b) \
    ui->a##b->setChecked(thisFile.permission(QFile::b##a))
    _(Owner, Read);
    _(Owner, Write);
    _(Owner, Exe);
    _(Group, Read);
    _(Group, Write);
    _(Group, Exe);
    _(Other, Read);
    _(Other, Write);
    _(Other, Exe);
#undef _

#ifdef Q_OS_WIN
    ui->labelOwner->hide();
    ui->labelGroup->hide();
    ui->fileOwner->hide();
    ui->fileGroup->hide();
    ui->groupPerms->hide();
    ui->otherPerms->hide();
    ui->ownerPerms->setTitle(tr("Permissions"));
    ui->OwnerExe->hide();
#endif

    adjustSize();
}

FilePropertiesDialog::~FilePropertiesDialog()
{
    delete ui;
}

void FilePropertiesDialog::on_FilePropertiesDialog_accepted()
{
    QFlags<QFile::Permission> perms;
#define _(a, b) do { \
        if (ui->a##b->isChecked()) perms |= (QFile::b##a); else perms &= ~(QFile::b##a); \
    } while(0)
    _(Owner, Read);
    _(Owner, Write);
    _(Owner, Exe);
    _(Group, Read);
    _(Group, Write);
    _(Group, Exe);
    _(Other, Read);
    _(Other, Write);
    _(Other, Exe);
#undef _
    QFile f(thisFile.absoluteFilePath());
    if (perms != f.permissions())
        f.setPermissions(perms);
    if (ui->fileName->text() != thisFile.fileName())
        f.rename(ui->fileName->text());
#if 0
    QString userNameText = ui->fileOwner->text();
    QString groupNameText = ui->fileGroup->text();
    if ((userNameText != thisFile.owner()) || (groupNameText != thisFile.group())) {
        char *fileName = f.fileName().toLocal8Bit().data();
        char *ownerName = userNameText.toLocal8Bit().data();
        char *groupName = groupNameText.toLocal8Bit().data();
        struct passwd *o = getpwnam(ownerName);
        struct group *g = getgrnam(groupName);
        if (g && o) {
            ::chown(fileName, o->pw_uid, g->gr_gid);
        } else
            qDebug() << "g" << ownerName << ((void*)g) << "o" << groupName << ((void*)o);
    }
#endif
}
