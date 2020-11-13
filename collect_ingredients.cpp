#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;
class Action
{
    public:
        int actionId; // the unique ID of this spell or recipe
        string actionType; // in the first league: BREW; later: CAST, OPPONENT_CAST, LEARN, BREW
        int delta[4];
        int price; // the price in rupees if this is a potion
        int tomeIndex; // in the first two leagues: always 0; later: the index in the tome if this is a tome spell, equal to the read-ahead tax
        int taxCount; // in the first two leagues: always 0; later: the amount of taxed tier-0 ingredients you gain from learning this spell
        bool castable; // in the first league: always 0; later: 1 if this is a castable player spell
        bool repeatable; // for the first two leagues: always 0; later: 1 if this is a repeatable player spell

        string execute_action()
        {
            return (actionType + " " + to_string(actionId));
        }
};

int *delta_add(int d1[4], int d2[4])
{
    int *d = (int *)malloc(sizeof(int)*4);
    d[0] = d1[0] + d2[0];
    d[1] = d1[1] + d2[1];
    d[2] = d1[2] + d2[2];
    d[3] = d1[3] + d2[3];
    return (d);
}

string get_tier(int inv[4], int tier, int times, vector<Action> spells)
{
    if (tier == 0)
    {   if (spells[tier].castable)
            return (spells[tier].execute_action()+""+to_string(times));
    }
    else 
    {
        if (inv[tier - 1] < times)
            return (get_tier(inv, tier - 1, times,spells));
        else if (spells[tier].castable)
            return (spells[tier].execute_action()+""+to_string(times));
    }
    return "REST";
}

string gather_ingredients(int inv[4], Action potion, vector<Action> spells)
{
    int *delta;

    delta = delta_add(inv, potion.delta);

    int i = 3;
    while (i >= 0)
    {
        if (delta[i] < 0)
        {
            return (get_tier(inv, i, abs(delta[i]) ,spells) );
        }
        i--;
    }
    return ("REST");
}

int count_turns_required(int inv[4], Action potion)
{
    int *delta;
    int sum = 0;
    delta = delta_add(inv, potion.delta);
    for (int i =3; i>=0; i--)
    {
        if (delta[i] < 0)
            sum += (i + 2)*abs(delta[i]);
    }
    return (sum);
}