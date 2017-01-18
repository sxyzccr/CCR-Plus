#ifndef GENERALCONFIGUREWIDGET_H
#define GENERALCONFIGUREWIDGET_H

#include "configure/configuretabwidget.h"

namespace Ui
{
class GeneralTabWidget;
}

class GeneralTabWidget : public ConfigureTabWidget
{
    Q_OBJECT
public:
    explicit GeneralTabWidget(QWidget* parent = nullptr);
    ~GeneralTabWidget();

    /// 载入 problem，显示具体配置
    void ShowProblemConfiguration(Problem* problem) override;
    /// 重置为默认
    void Reset() override;

private:
    Ui::GeneralTabWidget* ui;

private slots:
    void on_radioButton_builtin_clicked();
    void on_radioButton_custom_clicked();
    void on_pushButton_resetSubmit_clicked();
    void on_pushButton_resetRun_clicked();
    void on_pushButton_resetChecker_clicked();
};

#endif // GENERALCONFIGUREWIDGET_H
