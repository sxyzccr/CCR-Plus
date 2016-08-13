#ifndef CONFIGUREDIALOG_H
#define CONFIGUREDIALOG_H

#include "configuretable.h"

#include <QDialog>

namespace Ui
{
class ConfigureDialog;
}

class ConfigureDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ConfigureDialog(const QStringList& list, QWidget* parent = 0);
    ~ConfigureDialog();

public slots:
    void accept() override;

private:
    Ui::ConfigureDialog* ui;
    ConfigureTable* configure_table;
    QStringList problem_list;

private slots:
    void on_pushButton_clicked();
};

#endif // CONFIGUREDIALOG_H
