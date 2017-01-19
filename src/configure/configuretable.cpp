#include <QListView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QComboBox>
#include <QScrollBar>
#include <QDoubleSpinBox>
#include <QMouseEvent>
#include <QApplication>

#include "common/global.h"
#include "common/player.h"
#include "common/problem.h"
#include "configure/configuretable.h"

QRect ConfigureTableItemDelegate::checkBoxRect(const QStyleOptionViewItem& viewItemStyleOptions)
{
    QStyleOptionButton checkBoxStyleOptionButton;
    QRect rect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkBoxStyleOptionButton);
    QPoint point(viewItemStyleOptions.rect.x() + viewItemStyleOptions.rect.width()  / 2 - rect.width()  / 2,
                 viewItemStyleOptions.rect.y() + viewItemStyleOptions.rect.height() / 2 - rect.height() / 2);
    return QRect(point, rect.size());
}



QWidget* ConfigureTableItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& index) const
{
    switch (index.row())
    {
        case 0: // Type
        {
            QComboBox* editor = new QComboBox(parent);
            QStandardItemModel* model = new QStandardItemModel(editor);

            QStandardItem* item;
            item = new QStandardItem("传统型");
            item->setToolTip("传统型");
            model->appendRow(item);
            item = new QStandardItem("提交答案型");
            item->setToolTip("提交答案型");
            model->appendRow(item);

            editor->setModel(model);
            editor->setView(new QListView(editor));
            return editor;
        }
        case 3: // Checker
        {
            QComboBox* editor = new QComboBox(parent);
            QStandardItemModel* model = new QStandardItemModel(editor);
            auto& builtin_checker = Problem::BUILTIN_CHECKER_MAP;

            QStandardItem* item;
            for (auto checker : builtin_checker)
            {
                item = new QStandardItem(checker.first);
                item->setToolTip(checker.second);
                item->setFont(Global::BOLD_FONT);
                model->appendRow(item);
            }

            QStringList dirs = { QDir().currentPath() + "/checker",
                                 Global::g_contest.data_path + problem_list[index.column()]
                               };
            for (auto dir : dirs)
            {
#ifdef Q_OS_WIN
                QStringList list = QDir(dir).entryList(QDir::Files);
#else
                QStringList list = QDir(dir).entryList(QDir::Files | QDir::Executable);
#endif
                for (auto checker : list)
                {
#ifdef Q_OS_WIN
                    if (!checker.endsWith(".exe")) continue;
#endif
                    if (Problem::IsBuiltinChecker(checker)) continue;

                    item = new QStandardItem(checker);
                    item->setToolTip(QString("%1 (位置: %2)").arg(checker, dir));
                    model->appendRow(item);
                }
            }

            editor->setModel(model);
            editor->setView(new QListView(editor));
            return editor;
        }
        case 1: // Time Limit
        {
            QDoubleSpinBox* editor = new QDoubleSpinBox(parent);
            editor->setAlignment(Qt::AlignCenter);
            editor->setDecimals(1);
            editor->setMinimum(0);
            editor->setMaximum(3600);
            return editor;
        }
        case 2: // Memory Limit
        {
            QDoubleSpinBox* editor = new QDoubleSpinBox(parent);
            editor->setAlignment(Qt::AlignCenter);
            editor->setDecimals(1);
            editor->setMinimum(0);
            editor->setMaximum(8192);
            editor->setSingleStep(128);
            return editor;
        }
        default:
            return nullptr;
    }
}

void ConfigureTableItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.row() != 4)
    {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    QStyleOptionButton checkBoxOption;
    int checked = index.model()->data(index, Qt::DisplayRole).toInt();
    if (checked != 2)
        checkBoxOption.state |= QStyle::State_Enabled;
    if (checked)
        checkBoxOption.state |= QStyle::State_On;
    else
        checkBoxOption.state |= QStyle::State_Off;
    checkBoxOption.rect = checkBoxRect(option);

    QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkBoxOption, painter);
}

void ConfigureTableItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    switch (index.row())
    {
        case 0: // Type
        case 3: // Checker
        {
            QComboBox* edit = static_cast<QComboBox*>(editor);
            int p = edit->findText(index.model()->data(index, Qt::DisplayRole).toString());
            if (p == -1) p = 0;
            edit->setCurrentIndex(p);
            return;
        }
        case 1: // Time Limit
        case 2: // Memory Limit
        {
            QDoubleSpinBox* edit = static_cast<QDoubleSpinBox*>(editor);
            bool ok;
            edit->setValue(index.model()->data(index, Qt::DisplayRole).toDouble(&ok));
            if (!ok) edit->setValue(index.row() == 1 ? 1 : 128);
            return;
        }
    }
}

void ConfigureTableItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    switch (index.row())
    {
        case 0: // Type
        case 3: // Checker
        {
            QComboBox* edit = static_cast<QComboBox*>(editor);
            model->setData(index, edit->currentText());
            return;
        }
        case 1: // Time Limit
        case 2: // Memory Limit
        {
            QDoubleSpinBox* edit = static_cast<QDoubleSpinBox*>(editor);
            model->setData(index, edit->value());
            return;
        }
    }
}

bool ConfigureTableItemDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (index.row() != 4)
        return QStyledItemDelegate::editorEvent(event, model, option, index);

    if (event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
        if (mouse_event->button() == Qt::LeftButton && checkBoxRect(option).contains(mouse_event->pos()))
        {
            int checked = index.model()->data(index, Qt::DisplayRole).toInt();
            model->setData(index, checked <= 1 ? !checked : 2, Qt::DisplayRole);
        }
    }
    return false;
}





ConfigureTable::ConfigureTable(const QList<Problem*>& problems, QWidget* parent) : QTableView(parent),
    model(new QStandardItemModel(this)), problems(problems), is_changing_data(false)
{
    this->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    this->setFocusPolicy(Qt::NoFocus);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setAlternatingRowColors(true);
    this->setSelectionMode(QAbstractItemView::NoSelection);
    this->setStyleSheet(QLatin1String("QHeaderView"
                                      "{"
                                      "  background:#FFFFFF;"
                                      "}"
                                      "QTableView\n"
                                      "{"
                                      "  background-color:#F8F8F8;"
                                      "  alternate-background-color:#FFFFFF;"
                                      "}"));

    this->horizontalHeader()->setSectionsMovable(true);
    this->horizontalHeader()->setTextElideMode(Qt::ElideRight);
    this->horizontalHeader()->setHighlightSections(false);
    this->horizontalHeader()->setDefaultSectionSize(85);
    this->horizontalHeader()->setMinimumSectionSize(85);
    this->horizontalHeader()->setFixedHeight(25);
    this->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    this->verticalHeader()->setHighlightSections(false);
    this->verticalHeader()->setDefaultSectionSize(27);
    this->verticalHeader()->setMinimumSectionSize(27);
    this->verticalHeader()->setFixedWidth(80);
    this->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    this->setModel(model);

    QStringList list;
    for (auto i : problems) list.append(i->Name());
    this->setItemDelegate(new ConfigureTableItemDelegate(list, this));

    loadProblems();

    connect(model, &QAbstractItemModel::dataChanged, this, &ConfigureTable::onDataChanged);
}

void ConfigureTable::setColumnDataNew(int column)
{
    problems[column]->ConfigureNew("传统型", 1, 128, "全文比较");

    SetItemText(0, column, "传统型");
    SetItemData(1, column, 1);
    SetItemData(2, column, 128);
    SetItemText(3, column, "全文比较");
    SetItemData(4, column, 2); // 0:off; 1:on; 2:on|disable

    for (int i = 0; i < 4; i++)
    {
        SetItemChanged(i, column);
        model->item(i, column)->setEditable(true);
    }
}

void ConfigureTable::setColumnData(int column)
{
    const Problem* problem = problems[column];

    if (!problem->TestCaseCount())
    {
        setColumnDataNew(column);
        return;
    }

    SetItemData(4, column, 0);
    SetItemText(3, column, problem->BuiltinCheckerName());
    switch (problem->Type())
    {
        case Global::Traditional:
        {
            SetItemText(0, column, "传统型");
            double minT = 1e9, maxT = 0, minM = 1e9, maxM = 0;
            for (int i = 0; i < problem->TestCaseCount(); i++)
            {
                const TestCase* point = problem->TestCaseAt(i);
                minT = std::min(minT, point->TimeLimit()),   maxT = std::max(maxT, point->TimeLimit());
                minM = std::min(minM, point->MemoryLimit()), maxM = std::max(maxM, point->MemoryLimit());
            }

            if (minT == maxT)
                SetItemData(1, column, minT);
            else if (0 <= minT && maxT <= 3600)
                SetItemText(1, column, QString("%1~%2").arg(minT).arg(maxT));
            else
            {
                SetItemText(1, column, QString("无效"));
                SetItemBold(1, column);
            }

            if (minM == maxM)
                SetItemData(2, column, minM);
            else if (0 <= minM && maxM <= 8192)
                SetItemData(2, column, QString("%1~%2").arg(minM).arg(maxM));
            else
            {
                SetItemText(2, column, QString("无效"));
                SetItemBold(2, column);
            }
            break;
        }
        case Global::AnswersOnly:
        {
            SetItemText(0, column, QString("提交答案型"));
            SetItemText(1, column, "");
            SetItemText(2, column, "");
            model->item(1, column)->setEditable(false);
            model->item(2, column)->setEditable(false);
            break;
        }
        default:
        {
            SetItemText(0, column, QString("无效"));
            SetItemBold(0, column);
            SetItemText(1, column, "");
            SetItemText(2, column, "");
            model->item(1, column)->setEditable(false);
            model->item(2, column)->setEditable(false);
            break;
        }
    }
}

void ConfigureTable::loadProblems()
{
    model->setRowCount(5);
    model->setVerticalHeaderLabels({"题目类型", "时间限制", "内存限制", "比较方式", "清空原配置"});
    model->verticalHeaderItem(0)->setToolTip("试题的类型。");
    model->verticalHeaderItem(1)->setToolTip("试题每个测试点拥有的运行时间上限(仅限传统型试题)。单位: 秒(s)");
    model->verticalHeaderItem(2)->setToolTip("试题每个测试点拥有的运行内存上限(仅限传统型试题)。单位: 兆字节(MB)");
    model->verticalHeaderItem(3)->setToolTip("选手程序输出文件(或答案文件)与标准输出文件的比较方式。");
    model->verticalHeaderItem(4)->setToolTip("清空原来的所有配置。");
    for (int i = 0; i < 5; i++) model->verticalHeaderItem(i)->setTextAlignment(Qt::AlignCenter);

    int num = problems.size();
    model->setColumnCount(num);
    for (int i = 0; i < num; i++)
    {
        QStandardItem* item = new QStandardItem(problems[i]->Name());
        item->setToolTip(item->text());
        model->setHorizontalHeaderItem(i, item);
        setColumnData(i);
    }

    int w = std::min(std::max(num, 3), 12) * this->horizontalHeader()->defaultSectionSize() + this->verticalHeader()->width() + 2 * this->frameWidth();
    int h = 5 * this->verticalHeader()->defaultSectionSize() + this->horizontalHeader()->height() + 2 * this->frameWidth();
    if (num > 12) h += this->horizontalScrollBar()->sizeHint().height();
    this->setFixedSize(w, h);
}



void ConfigureTable::onDataChanged(const QModelIndex& topLeft, const QModelIndex&)
{
    if (is_changing_data) return;
    is_changing_data = true;

    int r = topLeft.row(), c = topLeft.column();
    SetItemText(r, c, ItemText(r, c));
    SetItemChanged(r, c);

    switch (r)
    {
        case 0: // Type
        {
            if (ItemText(r, c) == "提交答案型")
            {
                SetItemText(1, c, "");
                SetItemText(2, c, "");
                model->item(1, c)->setEditable(false);
                model->item(2, c)->setEditable(false);
                problems[c]->Configure("提交答案型", 0, 0, "");
            }
            else if (ItemText(r, c) == "传统型")
            {
                SetItemData(1, c, 1);
                SetItemData(2, c, 128);
                SetItemChanged(1, c);
                SetItemChanged(2, c);
                model->item(1, c)->setEditable(true);
                model->item(2, c)->setEditable(true);
                problems[c]->Configure("传统型", 1, 128, "");
            }
            break;
        }
        case 1: // Time Limit
            problems[c]->Configure("", ItemData(r, c).toDouble(), -1, "");
            break;
        case 2: // Memory Limit
            problems[c]->Configure("", -1, ItemData(r, c).toDouble(), "");
            break;
        case 3: // Checker
            problems[c]->Configure("", -1, -1, ItemText(r, c));
            break;
        case 4: // Clean
        {
            if (ItemData(r, c).toBool())
            {
                setColumnDataNew(c);
                SetItemData(4, c, 1);
            }
            break;
        }
        default:
            break;
    }
    is_changing_data = false;
}
