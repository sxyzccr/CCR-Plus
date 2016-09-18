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
    enum TestCaseType
    {
        AddTestCase,
        AddSubTestCase,
        EditScore,
        EditSubTestCase
    };

    explicit AddTestCaseDialog(Problem* problem, TestCase* point, TestCaseType type, QWidget* parent = nullptr, int score = 10);
    ~AddTestCaseDialog();

    // Getter member functions
    int GetScore() const { return score;}
    TestCase* GetTestCase() const { return point; }

protected:
    void accept() override;

private slots:
    void on_lineEdit_inFile_textChanged(const QString& arg1);
    void on_lineEdit_outFile_textChanged(const QString& arg1);
    void on_pushButton_browseInFile_clicked();
    void on_pushButton_browseOutFile_clicked();

private:
    Ui::AddTestCaseDialog *ui;
    Problem* problem;
    TestCase* point;
    int score;
    TestCaseType type;
};

#endif // ADDTESTCASEDIALOG_H
