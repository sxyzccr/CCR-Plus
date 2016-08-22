#ifndef ADDTESTCASEDIALOG_H
#define ADDTESTCASEDIALOG_H

#include "problem.h"

#include <QDialog>

namespace Ui
{
class AddTestCaseDialog;
}

class AddTestCaseDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddTestCaseDialog(Problem* problem, TestCase* point, bool isScoreShow, QWidget *parent = 0);
    ~AddTestCaseDialog();

    int GetScore() const { return score;}
    TestCase* GetTestCase() const { return point; }

protected:
    void accept() override;

private slots:
    void on_lineEdit_inFile_textChanged(const QString &arg1);

    void on_lineEdit_outFile_textChanged(const QString &arg1);

    void on_pushButton_browseInFile_clicked();

    void on_pushButton_browseOutFile_clicked();

private:
    Ui::AddTestCaseDialog *ui;
    Problem* problem;
    TestCase* point;
    int score;
};

#endif // ADDTESTCASEDIALOG_H
