#ifndef CREATEFILEDIALOG_H
#define CREATEFILEDIALOG_H

#include <QDialog>
#include <QListWidget>

#include "common/player.h"
#include "common/problem.h"

namespace Ui
{
class CreateFileDialog;
}

class CreateFileDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CreateFileDialog(Player* player, Problem* problem, QWidget* parent = nullptr);
    ~CreateFileDialog();

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
