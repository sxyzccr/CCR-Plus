#ifndef ADVANCEDCONFIGUREDIALOG_H
#define ADVANCEDCONFIGUREDIALOG_H

#include <QDialog>
#include <QListWidget>

#include "common/problem.h"

namespace Ui
{
class AdvancedConfigureDialog;
}

class AdvancedConfigureDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AdvancedConfigureDialog(const QList<Problem*>& problems, QWidget* parent = nullptr);
    ~AdvancedConfigureDialog();

    QList<Problem*> Problems() const { return problems; }

public slots:
    virtual void accept() override;

private:
    Ui::AdvancedConfigureDialog* ui;
    Problem* current_problem;
    QList<Problem*> old_problems, problems;
    bool load_finished;

    void loadFromProblem(Problem* problem);

private slots:
    void onListWidgetCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);

    void on_pushButton_reset_clicked();
    void on_comboBox_type_currentIndexChanged(int index);
};

#endif // ADVANCEDCONFIGUREDIALOG_H
