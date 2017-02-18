#ifndef ADDCOMPILERDIALOG_H
#define ADDCOMPILERDIALOG_H

#include <QDialog>

#include "common/problem.h"

namespace Ui
{
class AddCompilerDialog;
}

class AddCompilerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddCompilerDialog(const Problem* problem, const Compiler* compiler, int focusRow, QWidget* parent = nullptr);
    ~AddCompilerDialog();

    // Getter member functions
    Compiler GetCompiler() const { return *compiler; }

protected:
    virtual void accept() override;

private:
    Ui::AddCompilerDialog* ui;
    const Problem* problem;
    const Compiler* compiler;
    QString cmd, file;
    int time_lim;

    void initCompiler(const Compiler* compiler);

private slots:
    void onChangeCmd();

    void on_comboBox_type_currentIndexChanged(int index);
    void on_lineEdit_file_textChanged(const QString& text);
    void on_lineEdit_args_textChanged(const QString& text);
};

#endif // ADDCOMPILERDIALOG_H
