#ifndef CREATEFILEDIALOG_H
#define CREATEFILEDIALOG_H

#include <QDialog>

class QListWidgetItem;
class Player;
class Problem;

namespace Ui
{
class CreateFileDialog;
}

class CreateFileDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CreateFileDialog(const Player* player, const Problem* problem, QWidget* parent = nullptr);
    ~CreateFileDialog();

    QString GetSelectedFile() const { return selected_file; }

public slots:
    virtual void accept() override;

private:
    Ui::CreateFileDialog* ui;
    const Player* player;
    const Problem* problem;
    QString selected_file;

private slots:
    void on_listWidget_itemDoubleClicked(QListWidgetItem*);
};

#endif // CREATEFILEDIALOG_H
