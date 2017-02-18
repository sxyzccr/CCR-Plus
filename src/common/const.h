#ifndef CONST_H
#define CONST_H

#include <QFont>

class QColor;

namespace Global
{

enum ProblemType
{
    OtherProblemType,
    Traditional,
    AnswersOnly,
    Interactive
};

enum CompileResult
{
    CompileSuccessfully,
    CompileError,
    CompileTimeLimitExceeded,
    CompileKilled,
    InvalidCompiler,
    OtherCompileError
};

enum LabelStyle
{
    StyleNone              = -1,
    StyleScore_1_9         =  0,
    StyleScore_10_19       =  1,
    StyleScore_20_29       =  2,
    StyleScore_30_39       =  3,
    StyleScore_40_49       =  4,
    StyleScore_50_59       =  5,
    StyleScore_60_69       =  6,
    StyleScore_70_79       =  7,
    StyleScore_80_89       =  8,
    StyleScore_90_99       =  9,
    StyleScore_100         = 10,
    StyleScore_0           = 11,
    StyleScoreGreater_100  = 12,
    StyleScoreLess_0       = 13,
    StyleInvalidOrStateCFS = 14,
    StyleRunning           = 15,
    StyleStateE            = 16
};

const int MAX_RECENT_CONTEST = 20;

const QFont NORMAL_FONT = QFont("微软雅黑", 9);
const QFont BOLD_FONT = QFont("微软雅黑", 9, QFont::Bold);

int GetLogicalRow(int visualRow);
QColor GetRatioColor(int rl, int gl, int bl, int rr, int gr, int br, int x, int y);

} // namespace Global

#endif // CONST_H

