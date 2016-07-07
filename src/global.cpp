#include "global.h"

int Global::playerNum = 0, Global::problemNum = 0, Global::sumScore = 0;
vector<Player> Global::players = vector<Player>();
vector<Problem> Global::problems = vector<Problem>();
QList<int> Global::problemOrder = QList<int>();
QString Global::testPath = "", Global::srcPath = "", Global::dataPath = "", Global::resultPath = "", Global::testName = "";
bool Global::alreadyJudging = false, Global::isListUsed = false;
Qt::SortOrder Global::preSortOrder = Qt::AscendingOrder;
QFont Global::font = QFont("微软雅黑", 9), Global::boldFont = QFont("微软雅黑", 9, 75);
QElapsedTimer Global::clickTimer = QElapsedTimer();
QList<pair<int, int>> Global::judgeList = QList<pair<int, int>>();

QString Global::labelStyle1[] =
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
QString Global::labelStyle2[] =
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

void Global::clear()
{
    playerNum = problemNum = 0;
    sumScore = 0;

    for (auto i : problems) i.clear();
    for (auto i : players) i.clear();

    problems.clear();
    players.clear();
    problemOrder.clear();
}

int Global::logicalRow(int visualRow)
{
    return preSortOrder == Qt::DescendingOrder ? playerNum - visualRow - 1 : visualRow;
}

QColor Global::ratioColor(int rl, int gl, int bl, int rr, int gr, int br, int x, int y)
{
    x = min(x, y);
    if (!y) return QColor(rl, gl, bl);
    int r = rl + 1.0 * (rr - rl) * x / y + 0.5;
    int g = gl + 1.0 * (gr - gl) * x / y + 0.5;
    int b = bl + 1.0 * (br - bl) * x / y + 0.5;
    return QColor(r, g, b);
}

Problem::CompilerInfo Global::getCompiler(Player* player, Problem* problem)
{
    for (auto i : problem->compilers) if (QFile(Global::srcPath + player->name + "/" + problem->dir + "/" + i.file).exists()) return i;
    return Problem::CompilerInfo();
}

void Global::saveProblemOrder(const QStringList& list)
{
    QFile file(testPath + ".ccr");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out << "<contest>\n";
    out << "    <order>\n";
    for (auto i : list) out << QString("        <problem>%1</problem>\n").arg(i);
    out << "    </order>\n";
    out << "</contest>\n";
    file.close();
}
