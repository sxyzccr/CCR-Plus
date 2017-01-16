#ifndef GENERALCONFIGUREWIDGET_H
#define GENERALCONFIGUREWIDGET_H

#include <QWidget>

#include "common/problem.h"

namespace Ui
{
class GeneralConfigureWidget;
}

class GeneralConfigureWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GeneralConfigureWidget(QWidget* parent = nullptr);
    ~GeneralConfigureWidget();

    void LoadFromProblem(Problem* problem);

private:
    Ui::GeneralConfigureWidget* ui;
    Problem* current_problem;

private slots:
    void on_radioButton_internal_clicked();
    void on_radioButton_custom_clicked();
    void on_pushButton_resetSubmit_clicked();
    void on_pushButton_resetRun_clicked();
    void on_pushButton_resetChecker_clicked();
};

#endif // GENERALCONFIGUREWIDGET_H
