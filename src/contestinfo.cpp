#include "contestinfo.h"

namespace ContestInfo
{
Info info;

void Info::clear()
{
    playerNum = problemNum = 0;
    sumScore = 0;

    for (auto i : problems) i.clear();
    for (auto i : players) i.clear();

    problems.clear();
    players.clear();
    problemOrder.clear();
}

void Info::saveProblemOrder(const QStringList& list)
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

}
