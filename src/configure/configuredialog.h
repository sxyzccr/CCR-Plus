#ifndef CONFIGUREDIALOG_H
#define CONFIGUREDIALOG_H

#include <QDialog>

#include "configure/configuretable.h"

namespace Ui
{
class ConfigureDialog;
}

class ConfigureDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ConfigureDialog(const QStringList& list, QWidget* parent = nullptr);
    ~ConfigureDialog();

public slots:
    virtual void accept() override;

private:
    Ui::ConfigureDialog* ui;
    ConfigureTable* configure_table;
    QList<Problem*> problems;

private slots:
    void on_pushButton_adv_clicked();
};

#endif // CONFIGUREDIALOG_H
