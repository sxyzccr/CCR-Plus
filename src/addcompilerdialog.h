#ifndef ADDCOMPILERDIALOG_H
#define ADDCOMPILERDIALOG_H

#include "problem.h"

#include <QDialog>

namespace Ui
{
class AddCompilerDialog;
}

class AddCompilerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddCompilerDialog(Problem* problem, QWidget *parent = 0);
    ~AddCompilerDialog();

    QString Cmd() const { return cmd; }
    QString SourceFile() const { return file; }
    int TimeLimit() const { return time_lim; }

protected:
    void accept() override;

private:
    Ui::AddCompilerDialog *ui;
    Problem* problem;
    QString cmd, file;
    int time_lim;

private slots:
    void onChangeCmd();

    void on_comboBox_type_currentIndexChanged(int index);
    void on_lineEdit_file_textChanged(const QString& arg1);
    void on_lineEdit_args_textChanged(const QString& arg1);
};

#endif // ADDCOMPILERDIALOG_H
