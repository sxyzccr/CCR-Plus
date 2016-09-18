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
    explicit AddCompilerDialog(Problem* problem, Compiler* compiler, QWidget* parent = nullptr);
    ~AddCompilerDialog();

    // Getter member functions
    QString Cmd() const { return cmd; }
    QString SourceFile() const { return file; }
    int TimeLimit() const { return time_lim; }

protected:
    void accept() override;

private:
    Ui::AddCompilerDialog *ui;
    Problem* problem;
    Compiler* compiler;
    QString cmd, file;
    int time_lim;

    void initCompiler(Compiler *compiler);

private slots:
    void onChangeCmd();

    void on_comboBox_type_currentIndexChanged(int index);
    void on_lineEdit_file_textChanged(const QString& arg1);
    void on_lineEdit_args_textChanged(const QString& arg1);
};

#endif // ADDCOMPILERDIALOG_H
