#include "player.h"

Player::Player(const QString &na,int x)
{
    name=na;
    id=x,type=0;
}

Player::~Player()
{

}

void Player::calcSum()
{
    sum.usedTime=0;
    sum.score=0;
    for (auto&i:problem)
    {
        if (!i.state) i.state=' ';
        if (i.state!='N'&&i.state!='E') i.score=0,i.usedTime=0;
        sum.usedTime+=i.usedTime;
        sum.score+=i.score;
    }
}

void Player::clear()
{
    for (auto i:label) i->deleteLater();
    problem.clear();
    label.clear();
    style.clear();
}
