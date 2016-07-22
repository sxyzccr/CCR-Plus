#ifndef GLOBAL_H
#define GLOBAL_H

#include "contest.h"

#include <Qt>

namespace Global
{

extern bool g_is_judging;
extern bool g_judge_stoped;
extern bool g_contest_closed;
extern Qt::SortOrder g_pre_sort_order;
extern Contest g_contest;

}

#endif // GLOBAL_H
