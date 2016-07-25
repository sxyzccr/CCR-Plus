#include "const.h"
#include "global.h"

QColor Global::GetRatioColor(int rl, int gl, int bl, int rr, int gr, int br, int x, int y)
{
    x = std::min(x, y);
    if (!y) return QColor(rl, gl, bl);
    int r = rl + 1.0 * (rr - rl) * x / y + 0.5;
    int g = gl + 1.0 * (gr - gl) * x / y + 0.5;
    int b = bl + 1.0 * (br - bl) * x / y + 0.5;
    return QColor(r, g, b);
}

int Global::GetLogicalRow(int visualRow)
{
    return g_pre_sort_order == Qt::DescendingOrder ? g_contest.player_num - visualRow - 1 : visualRow;
}
