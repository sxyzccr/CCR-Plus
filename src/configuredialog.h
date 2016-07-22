#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include "itemdelegate.h"

#include <QDialog>
#include <QStandardItemModel>

namespace Ui
{
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget* parent = 0);
    virtual ~ConfigDialog();

private slots:
    void dataChangedEvent(const QModelIndex& tl, const QModelIndex& br);
    void on_pushButton_clicked();

private:
    Ui::ConfigDialog* ui;

    QStringList problemList;
    QStandardItemModel model;
    ItemDelegate delegate;
    bool alreadyChangingData;
    int num;

    void loadProblems();
    void setModelData(int c);
    void accept();
};

#endif // CONFIGDIALOG_H
