#ifndef CREATEFILEDIALOG_H
#define CREATEFILEDIALOG_H

#include "player.h"
#include "problem.h"

#include <QDialog>
#include <QListWidget>

namespace Ui
{
class CreateFileDialog;
}

class CreateFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateFileDialog(QWidget* parent = 0, Player* ply = 0, Problem* prob = 0);
    ~CreateFileDialog();

    QString selectedFile;

private slots:
    void on_listWidget_itemDoubleClicked(QListWidgetItem* item);

private:
    Ui::CreateFileDialog* ui;
    Player* player;
    Problem* problem;
    void accept();
};

#endif // CREATEFILEDIALOG_H
