#ifndef ADDTESTCASEDIALOG_H
#define ADDTESTCASEDIALOG_H

#include <QDialog>

#include "common/problem.h"

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

    explicit AddTestCaseDialog(const Problem* problem, const TestCase* point, TestCaseType type, int focusRow, QWidget* parent = nullptr, int score = 10);
    ~AddTestCaseDialog();

    // Getter member functions
    int GetScore() const { return score;}
    TestCase GetTestCase() const { return *point; }

protected:
    virtual void accept() override;

private slots:
    void on_lineEdit_inFile_textChanged(const QString& text);
    void on_lineEdit_outFile_textChanged(const QString& text);
    void on_lineEdit_submitFile_textChanged(const QString& text);
    void on_pushButton_browseInFile_clicked();
    void on_pushButton_browseOutFile_clicked();

private:
    Ui::AddTestCaseDialog *ui;
    const Problem* problem;
    const TestCase* point;
    int score;
    TestCaseType type;
};

#endif // ADDTESTCASEDIALOG_H
