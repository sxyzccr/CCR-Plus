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
    virtual ~CreateFileDialog();

    QString GetSelectedFile() const { return selected_file; }

public slots:
    void accept() override;

private:
    Ui::CreateFileDialog* ui;
    Player* player;
    Problem* problem;
    QString selected_file;

private slots:
    void on_listWidget_itemDoubleClicked(QListWidgetItem*);
};

#endif // CREATEFILEDIALOG_H
