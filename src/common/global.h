#ifndef GLOBAL_H
#define GLOBAL_H

#include <QtCore>
#include <Qt>

#include "common/contest.h"

namespace Global
{

extern bool g_is_judge_stoped;
extern bool g_is_contest_closed;
extern int g_sort_key_col;
extern Qt::SortOrder g_pre_sort_order;
extern Contest g_contest;

}

#endif // GLOBAL_H
