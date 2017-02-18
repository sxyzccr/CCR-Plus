#ifndef ADVANCEDCONFIGUREDIALOG_H
#define ADVANCEDCONFIGUREDIALOG_H

#include <QDialog>

class QAbstractButton;
class Problem;

namespace Ui
{
class AdvancedConfigureDialog;
}

class AdvancedConfigureDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AdvancedConfigureDialog(const QList<const Problem*>& problems, QWidget* parent = nullptr, const QString& currentProblem = "");
    ~AdvancedConfigureDialog();

    QList<Problem*> Problems() const { return problems; }

public slots:
    bool apply();
    virtual void accept() override;

private:
    Ui::AdvancedConfigureDialog* ui;
    Problem* current_problem;
    QList<Problem*> problems;
    bool load_finished;

    void loadFromProblem(Problem* problem);

private slots:
    void on_listWidget_currentRowChanged(int currentRow);
    void on_comboBox_type_currentIndexChanged(int index);
    void on_pushButton_reset_clicked();
    void on_buttonBox_clicked(QAbstractButton* button);

signals:
    void applied();
};

#endif // ADVANCEDCONFIGUREDIALOG_H
