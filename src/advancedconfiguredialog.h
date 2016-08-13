#ifndef ADVANCEDCONFIGUREDIALOG_H
#define ADVANCEDCONFIGUREDIALOG_H

#include "problem.h"

#include <QDialog>
#include <QListWidget>
#include <QTableWidget>

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
    QStringList problem_list;
    std::vector<int> row_span_top, row_span_bottom;

    void loadFromProblem(Problem *problem);

private slots:
    void on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_tableWidget_testCase_itemSelectionChanged();
    void on_tableWidget_testCase_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);
    void on_pushButton_merge_clicked();
};

#endif // ADVANCEDCONFIGUREDIALOG_H
