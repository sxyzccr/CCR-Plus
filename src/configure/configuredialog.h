#ifndef CONFIGUREDIALOG_H
#define CONFIGUREDIALOG_H

#include <QDialog>
#include <QAbstractButton>

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
    bool apply();
    virtual void accept() override;

private:
    Ui::ConfigureDialog* ui;
    ConfigureTable* configure_table;
    QList<Problem*> problems;

private slots:
    void on_pushButton_adv_clicked();
    void on_buttonBox_clicked(QAbstractButton* button);

signals:
    void applied();
};

#endif // CONFIGUREDIALOG_H
