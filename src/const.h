#ifndef CONST_H
#define CONST_H

#include <QFont>
#include <QColor>

enum ProblemType
{
    OtherType,
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
    OtherError
};

const int MAX_RECENT_CONTEST = 20;

const QFont FONT = QFont("微软雅黑", 9);
const QFont BOLD_FONT = QFont("微软雅黑", 9, 75);

const QString LABEL_STYLE_SOFT[] =
{
    "QLabel"
    "{"
    "   color:rgba(0,0,0,128);"
    "   background:rgba(235,29,0,192);"
    "}", //0 1~9
    "QLabel"
    "{"
    "   color:rgba(0,0,0,128);"
    "   background:rgba(226,58,0,192);"
    "}", //1 10~19
    "QLabel"
    "{"
    "   color:rgba(0,0,0,128);"
    "   background:rgba(217,87,0,192);"
    "}", //2 20~29
    "QLabel"
    "{"
    "   color:rgba(0,0,0,128);"
    "   background:rgba(209,116,0,192);"
    "}", //3 30~39
    "QLabel"
    "{"
    "   color:rgba(0,0,0,128);"
    "   background:rgba(200,145,0,192);"
    "}", //4 40~49
    "QLabel"
    "{"
    "   color:rgba(0,0,0,128);"
    "   background:rgba(185,159,0,192);"
    "}", //5 50~59
    "QLabel"
    "{"
    "   color:rgba(0,0,0,128);"
    "   background:rgba(160,158,1,192);"
    "}", //6 60~69
    "QLabel"
    "{"
    "   color:rgba(0,0,0,128);"
    "   background:rgba(134,157,2,192);"
    "}", //7 70~79
    "QLabel"
    "{"
    "   color:rgba(0,0,0,128);"
    "   background:rgba(107,156,3,192);"
    "}", //8 80~89
    "QLabel"
    "{"
    "   color:rgba(0,0,0,128);"
    "   background:rgba(75,166,4,192);"
    "}", //9 90~99
    "QLabel"
    "{"
    "   color:rgba(0,0,0,128);"
    "   background:rgba(51,185,6,192);"
    "}", //10 100
    "QLabel"
    "{"
    "   color:rgba(0,0,0,128);"
    "   background:rgba(244,0,0,192);"
    "}", //11 0
    "QLabel"
    "{"
    "   color:rgba(0,0,0,128);"
    "   background:rgba(6,238,6,192);"
    "}", //12 >100
    "QLabel"
    "{"
    "   color:rgba(0,0,0,128);"
    "   background:rgba(75,75,75,192);"
    "}", //13 <0
    "QLabel"
    "{"
    "   color:rgba(0,0,0,128);"
    "   background:rgba(180,180,180,192);"
    "}", //14 CFS,invalid
    "QLabel"
    "{"
    "   color:rgba(0,0,0,128);"
    "   background:rgba(0,161,241,192);"
    "}", //15 running
    "QLabel"
    "{"
    "   color:rgba(0,0,0,128);"
    "   background:rgba(227,58,218,192);"
    "}" //16 E
};
const QString LABEL_STYLE_HARD[] =
{
    "QLabel"
    "{"
    "   color:rgba(0,0,0,255);"
    "   background:rgba(235,29,0,255);"
    "}", //0 1~9
    "QLabel"
    "{"
    "   color:rgba(0,0,0,255);"
    "   background:rgba(226,58,0,255);"
    "}", //1 10~19
    "QLabel"
    "{"
    "   color:rgba(0,0,0,255);"
    "   background:rgba(217,87,0,255);"
    "}", //2 20~29
    "QLabel"
    "{"
    "   color:rgba(0,0,0,255);"
    "   background:rgba(209,116,0,255);"
    "}", //3 30~39
    "QLabel"
    "{"
    "   color:rgba(0,0,0,255);"
    "   background:rgba(200,145,0,255);"
    "}", //4 40~49
    "QLabel"
    "{"
    "   color:rgba(0,0,0,255);"
    "   background:rgba(185,159,0,255);"
    "}", //5 50~59
    "QLabel"
    "{"
    "   color:rgba(0,0,0,255);"
    "   background:rgba(160,158,1,255);"
    "}", //6 60~69
    "QLabel"
    "{"
    "   color:rgba(0,0,0,255);"
    "   background:rgba(134,157,2,255);"
    "}", //7 70~79
    "QLabel"
    "{"
    "   color:rgba(0,0,0,255);"
    "   background:rgba(107,156,3,255);"
    "}", //8 80~89
    "QLabel"
    "{"
    "   color:rgba(0,0,0,255);"
    "   background:rgba(75,166,4,255);"
    "}", //9 90~99
    "QLabel"
    "{"
    "   color:rgba(0,0,0,255);"
    "   background:rgba(51,185,6,255);"
    "}", //10 100
    "QLabel"
    "{"
    "   color:rgba(0,0,0,255);"
    "   background:rgba(244,0,0,255);"
    "}", //11 0
    "QLabel"
    "{"
    "   color:rgba(0,0,0,255);"
    "   background:rgba(6,238,6,255);"
    "}", //12 >100
    "QLabel"
    "{"
    "   color:rgba(0,0,0,255);"
    "   background:rgba(75,75,75,255);"
    "}", //13 <0
    "QLabel"
    "{"
    "   color:rgba(0,0,0,255);"
    "   background:rgba(180,180,180,255);"
    "}", //14 CFS,invalid
    "QLabel"
    "{"
    "   color:rgba(0,0,0,255);"
    "   background:rgba(0,161,241,255);"
    "}", //15 running
    "QLabel"
    "{"
    "   color:rgba(0,0,0,255);"
    "   background:rgba(227,58,218,255);"
    "}" //16 E
};

int GetLogicalRow(int visualRow);
QColor GetRatioColor(int rl, int gl, int bl, int rr, int gr, int br, int x, int y);

#endif // CONST_H
