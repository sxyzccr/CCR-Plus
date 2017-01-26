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
        AddIOTM,    // Add with I/O file, time/memory limit;            1st constructor
        AddSIOTM,   // Add with score, I/O file, time/memory limit;     1st constructor
        EditS,      // Edit with score;                                 1st constructor
        EditTM,     // Edit with time/memory limit;                     2nd constructor
        EditSTM,    // Edit with score, time/memory limit;              2nd constructor
        EditIOTM,   // Edit with I/O file, time/memory limit;           1st constructor
        EditSIOTM,  // Edit with score, I/O file, time/memory limit;    1st constructor
    };

    explicit AddTestCaseDialog(const Problem* problem, TestCaseType type,
                               const TestCase* point, int focusRow, QWidget* parent = nullptr, int score = 10);
    explicit AddTestCaseDialog(const Problem* problem, TestCaseType type,
                               double minTime, double maxTime, double minMemory, double maxMemory,
                               int minScore, int maxScore, QWidget* parent = nullptr);
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
    void on_spinBox_score_valueChanged(int);
    void on_spinBox_timeLim_valueChanged(double);
    void on_spinBox_memLim_valueChanged(double);

private:
    Ui::AddTestCaseDialog *ui;
    const Problem* problem;
    const TestCase* point;
    int score;
    TestCaseType type;
    bool load_finished;
};

#endif // ADDTESTCASEDIALOG_H
