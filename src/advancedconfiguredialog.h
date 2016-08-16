#ifndef ADVANCEDCONFIGUREDIALOG_H
#define ADVANCEDCONFIGUREDIALOG_H

#include "problem.h"
#include "testcasetable.h"

#include <QDialog>
#include <QListWidget>
#include <QTableWidget>
#include <QDragLeaveEvent>

namespace Ui
{
class AdvancedConfigureDialog;
}

class AdvancedConfigureDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AdvancedConfigureDialog(const QStringList& list, QWidget *parent = 0);
    ~AdvancedConfigureDialog();

public slots:
    void accept() override;

private:
    Ui::AdvancedConfigureDialog *ui;
    TestCaseTable* test_case_table;
    QStringList problem_list;

    void loadFromProblem(Problem *problem);

private slots:
    void onListWidgetCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void onSetPushButtonsEnable();

    void on_pushButton_merge_clicked();
    void on_pushButton_split_clicked();
};

#endif // ADVANCEDCONFIGUREDIALOG_H
