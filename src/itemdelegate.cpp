#include "global.h"
#include "itemdelegate.h"

#include <QComboBox>
#include <QListView>
#include <QDoubleSpinBox>
#include <QStandardItemModel>
#include <QApplication>
#include <QMouseEvent>

ItemDelegate::ItemDelegate(QObject* parent) : QStyledItemDelegate(parent)
{

}

ItemDelegate::~ItemDelegate()
{

}

QRect ItemDelegate::CheckBoxRect(const QStyleOptionViewItem& viewItemStyleOptions) const
{
    QStyleOptionButton checkBoxStyleOption;
    QRect checkBoxRect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkBoxStyleOption);
    QPoint checkBoxPoint(viewItemStyleOptions.rect.x() + viewItemStyleOptions.rect.width() / 2 - checkBoxRect.width() / 2,
                         viewItemStyleOptions.rect.y() + viewItemStyleOptions.rect.height() / 2 - checkBoxRect.height() / 2);
    return QRect(checkBoxPoint, checkBoxRect.size());
}

void ItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.row() != 4) QStyledItemDelegate::paint(painter, option, index);
    else
    {
        int checked = index.model()->data(index, Qt::DisplayRole).toInt();
        QStyleOptionButton* checkBoxOption = new QStyleOptionButton();
        if (checked != 2) checkBoxOption->state |= QStyle::State_Enabled;
        if (checked) checkBoxOption->state |= QStyle::State_On; else checkBoxOption->state |= QStyle::State_Off;
        checkBoxOption->rect = CheckBoxRect(option);
        QApplication::style()->drawControl(QStyle::CE_CheckBox, checkBoxOption, painter);
    }
}

bool ItemDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (index.row() != 4) return QStyledItemDelegate::editorEvent(event, model, option, index);
    if ((event->type() == QEvent::MouseButtonRelease) || (event->type() == QEvent::MouseButtonDblClick))
    {
        QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
        if (mouse_event->button() != Qt::LeftButton || !CheckBoxRect(option).contains(mouse_event->pos())) return false;
        if (event->type() == QEvent::MouseButtonDblClick) return true;
    }
    else if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
        if (key_event->key() != Qt::Key_Space && key_event->key() != Qt::Key_Select) return false;
    }
    else return false;

    int checked = index.model()->data(index, Qt::DisplayRole).toInt();
    return model->setData(index, checked <= 1 ? !checked : true, Qt::DisplayRole);
}

QWidget* ItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& index) const
{
    if (index.row() == 0) //Type
    {
        QComboBox* editor = new QComboBox(parent);
        QListView* view = new QListView(parent);
        QStandardItemModel* model = new QStandardItemModel(parent);

        QStandardItem* item;
        item = new QStandardItem("传统型");
        item->setToolTip("传统型");
        model->appendRow(item);
        item = new QStandardItem("提交答案型");
        item->setToolTip("提交答案型");
        model->appendRow(item);

        editor->setView(view);
        editor->setModel(model);

        return editor;
    }
    else if (index.row() == 1) //Time Limit
    {
        QDoubleSpinBox* editor = new QDoubleSpinBox(parent);
        editor->setAlignment(Qt::AlignCenter);
        editor->setDecimals(1);
        editor->setMinimum(0);
        editor->setMaximum(3600);
        return editor;
    }
    else if (index.row() == 2) //Memory Limit
    {
        QDoubleSpinBox* editor = new QDoubleSpinBox(parent);
        editor->setAlignment(Qt::AlignCenter);
        editor->setDecimals(1);
        editor->setMinimum(0);
        editor->setMaximum(8192);
        editor->setSingleStep(128);
        return editor;
    }
    else if (index.row() == 3) //Comparer
    {
        QComboBox* editor = new QComboBox(parent);
        QListView* view = new QListView(parent);
        QStandardItemModel* model = new QStandardItemModel(parent);

        QStandardItem* item;
        item = new QStandardItem("全文比较");
        item->setToolTip("全文比较(过滤行末空格及文末回车)");
        model->appendRow(item);

        QString dir1 = QDir().currentPath() + "/checker", dir2 = Global::g_contest.data_path + problemList[index.column()];
        QStringList list1, list2;
#ifdef Q_OS_LINUX
        list1 = QDir(dir1).entryList(QDir::Files | QDir::Executable);
        list2 = QDir(dir2).entryList(QDir::Files | QDir::Executable);
#else
        list1 = QDir(dir1).entryList(QDir::Files);
        list2 = QDir(dir2).entryList(QDir::Files);
#endif
        for (auto i : list1)
        {
#ifdef Q_OS_WIN
            if (!i.endsWith(".exe")) continue;
#endif
            if (i == "fulltext" || i == "fulltext.exe") continue;
            item = new QStandardItem(i);
            item->setToolTip(QString("%1 (位置: %2)").arg(i, dir1));
            model->appendRow(item);
        }
        for (auto i : list2)
        {
#ifdef Q_OS_WIN
            if (!i.endsWith(".exe")) continue;
#endif
            if (i == "fulltext" || i == "fulltext.exe") continue;
            item = new QStandardItem(i);
            item->setToolTip(QString("%1 (位置: %2)").arg(i, dir2));
            model->appendRow(item);
        }

        editor->setView(view);
        editor->setModel(model);

        return editor;
    }
    else return NULL;
}

void ItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    if (index.row() == 0) //Type
    {
        QComboBox* edit = static_cast<QComboBox*>(editor);
        int p = edit->findText(index.model()->data(index, Qt::EditRole).toString());
        if (p == -1) p = 0;
        edit->setCurrentIndex(p);
        return;
    }
    else if (index.row() == 1) //Time Limit
    {
        QDoubleSpinBox* edit = static_cast<QDoubleSpinBox*>(editor);
        bool ok;
        edit->setValue(index.model()->data(index, Qt::EditRole).toDouble(&ok));
        if (!ok) edit->setValue(1);
        return;
    }
    else if (index.row() == 2) //Memory Limit
    {
        QDoubleSpinBox* edit = static_cast<QDoubleSpinBox*>(editor);
        bool ok;
        edit->setValue(index.model()->data(index, Qt::EditRole).toDouble(&ok));
        if (!ok) edit->setValue(128);
        return;
    }
    else if (index.row() == 3) //Comparer
    {
        QComboBox* edit = static_cast<QComboBox*>(editor);
        int p = edit->findText(index.model()->data(index, Qt::EditRole).toString());
        if (p == -1) p = 0;
        edit->setCurrentIndex(p);
        return;
    }
}

void ItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    if (index.row() == 0) //Type
    {
        QComboBox* edit = static_cast<QComboBox*>(editor);
        model->setData(index, edit->currentText());
        return;
    }
    else if (index.row() == 1) //Time Limit
    {
        QDoubleSpinBox* edit = static_cast<QDoubleSpinBox*>(editor);
        model->setData(index, edit->value());
        return;
    }
    else if (index.row() == 2) //Memory Limit
    {
        QDoubleSpinBox* edit = static_cast<QDoubleSpinBox*>(editor);
        model->setData(index, edit->value());
        return;
    }
    else if (index.row() == 3) //Comparer
    {
        QComboBox* edit = static_cast<QComboBox*>(editor);
        model->setData(index, edit->currentText());
        return;
    }
}
