#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
using namespace std;

string In, Out, Ans, Log;
FILE* fout, *fans, *flog;

void End(const string& info, double x, int state = 0)
{
    fprintf(flog, "%.3lf\n%s\n", x, info.c_str());
    exit(state);
}

inline void filter(string& s)
{
    for (; s.size() && isspace(s[s.size() - 1]); s.erase(s.size() - 1));
}

string elided(const string& s, int p)
{
    string pre = "", suf = "";
    for (int i = 0; i < p; i++) pre.push_back(s[i]);
    if (pre.size() > 3) pre = string("…") + pre.substr(pre.size() - 3, 3);
    int l = s.size() - p;
    if (pre.size() + l >= 13) l = 11 - pre.size(), suf = "…";
    for (int i = 0; i < l; i++) pre.push_back(s[p + i]);
    return pre + suf;
}

int compare(const string& a, const string& b)
{
    int la = a.length(), lb = b.length();
    for (int i = 0; i < la && i < lb; i++) if (a[i] != b[i]) return i;
    return la != lb ? min(la, lb) : -1;
}

void Open()
{
    if (Log.size()) flog = fopen(Log.c_str(), "w"); else flog = stdout;
    if (flog == NULL) exit(1);
    if ((fans = fopen(Ans.c_str(), "r")) == NULL) exit(1);
    if ((fout = fopen(Out.c_str(), "r")) == NULL) exit(1);
}

void Check()
{
    char s[256];
    for (int i = 1; !feof(fout) || !feof(fans); i++)
    {
        string s1 = "", s2 = "";
        char c1 = -1, c2 = -1;
        for (; !feof(fans) && (c1 = fgetc(fans)) != '\n';) if (c1 != -1) s1.push_back(c1);
        for (; !feof(fout) && (c2 = fgetc(fout)) != '\n';) if (c2 != -1) s2.push_back(c2);
        if (feof(fout) && s1 != "" && s2 == "")
        {
            if (i == 1) End("选手输出为空", 0);
            sprintf(s, "第%d行 标准输出:\"%s\" 选手输出已结尾", i, elided(s1, 0).c_str());
            End(s, 0);
        }
        if (feof(fans) && s1 == "" && s2 != "")
        {
            sprintf(s, "第%d行 标准输出已结尾 选手输出:\"%s\"", i, elided(s2, 0).c_str());
            End(s, 0);
        }
        filter(s1), filter(s2);
        int p = compare(s1, s2);
        if (p >= 0)
        {
            sprintf(s, "第%d行 标准输出:\"%s\" 选手输出:\"%s\"", i, elided(s1, p).c_str(), elided(s2, p).c_str());
            End(s, 0);
        }
    }
}

int main(int argc, char* argv[])
{
    In = "";
    Ans = argc < 3 ? "" : argv[2];
    Out = argc < 4 ? "" : argv[3];
    Log = argc < 5 ? "" : argv[4];
    Open();
    Check();
    End("", 1);
}
