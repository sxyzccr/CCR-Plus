#ifndef COMPILERCONFIGUREWIDGET_H
#define COMPILERCONFIGUREWIDGET_H

#include <QWidget>

#include "common/problem.h"

namespace Ui
{
class CompilerConfigureWidget;
}

class CompilerConfigureWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CompilerConfigureWidget(QWidget* parent = nullptr);
    ~CompilerConfigureWidget();

    void LoadFromProblem(Problem* problem);

private:
    Ui::CompilerConfigureWidget* ui;
    Problem* current_problem;

private slots:
    void on_tableWidget_compiler_doubleClicked(const QModelIndex& index);
    void on_tableWidget_compiler_itemSelectionChanged();
    void on_pushButton_addCompiler_clicked();
    void on_pushButton_removeCompiler_clicked();
    void on_pushButton_resetCompiler_clicked();
};

#endif // COMPILERCONFIGUREWIDGET_H
