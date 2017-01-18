#ifndef COMPILERCONFIGUREWIDGET_H
#define COMPILERCONFIGUREWIDGET_H

#include "configure/configuretabwidget.h"

namespace Ui
{
class CompilerTabWidget;
}

class CompilerTabWidget : public ConfigureTabWidget
{
    Q_OBJECT
public:
    explicit CompilerTabWidget(QWidget* parent = nullptr);
    ~CompilerTabWidget();

    /// 载入 problem，显示具体配置
    void ShowProblemConfiguration(Problem* problem) override;
    /// 重置为默认
    void Reset() override;

private:
    Ui::CompilerTabWidget* ui;

private slots:
    void on_tableWidget_compiler_doubleClicked(const QModelIndex& index);
    void on_tableWidget_compiler_itemSelectionChanged();
    void on_pushButton_addCompiler_clicked();
    void on_pushButton_removeCompiler_clicked();
    void on_pushButton_resetCompiler_clicked();
};

#endif // COMPILERCONFIGUREWIDGET_H
