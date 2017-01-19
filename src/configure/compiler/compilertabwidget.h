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
    virtual void ShowProblemConfiguration(Problem* problem) override;
    /// 重置为默认
    virtual void Reset() override;

private:
    Ui::CompilerTabWidget* ui;

private slots:
    void on_tableWidget_doubleClicked(const QModelIndex& index);
    void on_tableWidget_itemSelectionChanged();
    void on_pushButton_add_clicked();
    void on_pushButton_delete_clicked();
    void on_pushButton_reset_clicked();
};

#endif // COMPILERCONFIGUREWIDGET_H
