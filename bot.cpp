#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

int *current_inv;
int targetId = 0;


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
string gather_ingredients(int inv[4], Action potion, vector<Action> spells);
bool compare_scores(Action p1,Action p2)
{
    return (p1.price > p2.price);
}

int *delta_add(int d1[4], int d2[4])
{
    int *d = (int *)malloc(sizeof(int)*4);
    d[0] = d1[0] + d2[0];
    d[1] = d1[1] + d2[1];
    d[2] = d1[2] + d2[2];
    d[3] = d1[3] + d2[3];
    return (d);
}

bool is_valid_action(int d[4])
{
    for (int i = 0; i < 4; i++)
    {
        if (d[i] < 0)
            return(false);
    }
    int s = 0;
    for (int i = 0; i < 4; i++)
    {
        s += d[i];
        if (s > 10)
            return false;
    }
    return(true);
}

float   get_spell_benefit(Action spell)
{
    float value=0;
    for (int i = 0; i < 4; i++)
        value += spell.delta[i]*(i+0.5);
    return (value);
}
bool compare_benefit(Action p1,Action p2)
{
    return (get_spell_benefit(p1) > get_spell_benefit(p2));
}
vector<Action> get_tier_spells(int tier, vector<Action> spells)
{
    vector<Action> result;

    for (auto s : spells)
    {
        if (s.delta[tier]>0)
            result.push_back(s);
    }    
    sort(result.begin(), result.end(), compare_benefit);
    return (result);
}

string get_tier(int inv[4], int tier, int times, vector<Action> spells)
{
    vector<Action> tier_spells = get_tier_spells(tier, spells);
    int i = 0;
    int d[4];    

    while (i < tier_spells.size() && !tier_spells[i].castable) 
    {
        if (get_spell_benefit(tier_spells[i]) > 1)
            return "REST";
        i++;
    }
    if (i == tier_spells.size())
        return("REST");

    cerr << "best spell for tier "<< tier << " : " << tier_spells[i].actionId;
    if (is_valid_action(delta_add(inv,tier_spells[i].delta)))
        return(tier_spells[i].execute_action());
    else 
        return gather_ingredients(inv, tier_spells[i], spells);
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
            cerr << "need tier"<<i<<endl;
            return (get_tier(inv, i, abs(delta[i]) ,spells));
        }
        i--;
    }
    targetId = 0;
    return (potion.execute_action());
}

int count_turns_required(int inv[4], Action potion)
{
    int *delta;
    int sum = 0;
    delta = delta_add(inv, potion.delta);
    for (int i =3; i>=0; i--)
    {
        if (delta[i] < 0)
            sum += i + 0.5;
    }
    return (sum);
}

int get_action_index(vector<Action> actions, int id)
{

    for (int i= 0;i<actions.size(); i++)
    {
        if (actions[i].actionId == id)
            return (i);
    }
    return (-1);
}

Action pick_best_target(int inv[4], int enemy_inv[4], vector<Action> potions, vector<Action> spells, vector<Action> enemy_spells, vector<Action> learn_spells)
{
    sort(potions.begin(),potions.end(),compare_scores);
    if (count_turns_required(inv, potions[0]) >= count_turns_required(enemy_inv, potions[0]))
        return (potions[0]);
    else
        return (potions[1]);
}


string Brain(int inv[4], int enemy_inv[4], vector<Action> potions, vector<Action> spells, vector<Action> enemy_spells, vector<Action> learn_spells)
{
    Action target;
    int d[4];

    if (!targetId || get_action_index(potions, targetId) == -1)
    {
        target = pick_best_target(inv, enemy_inv, potions, spells, enemy_spells, learn_spells);
        targetId = target.actionId;
    }
    return (gather_ingredients(inv, target, spells));
}

int main()
{
    // game loop
    while (1) {
        int inv[4];             //Player inventory
        int score;
        int enemy_inv[4];       //Enemy inventory
        int enemy_score;

        vector<Action> potions;         //List of BREW
        vector<Action> spells;          //List of CAST
        vector<Action> enemy_spells;    //List of OPPONENT_CAST
        vector<Action> learn_spells;    //List of OPPONENT_CAST

        int actionCount;        //The number of spells and recipes in play
        cin >> actionCount; cin.ignore();

        for (int i = 0; i < actionCount; i++)
        {
            Action act;
            cin >> act.actionId >> act.actionType >> act.delta[0] >> act.delta[1] >> act.delta[2] >> act.delta[3] >> act.price >> act.tomeIndex >> act.taxCount >> act.castable >> act.repeatable; cin.ignore();
            if (act.actionType == "BREW")
                potions.push_back(act);
            if (act.actionType == "CAST")
                spells.push_back(act);
            if (act.actionType == "OPPONENT_CAST")
                enemy_spells.push_back(act);
            if (act.actionType == "LEARN")
                learn_spells.push_back(act);
        }

        // Reading player and enemy inventories 
        cin >> inv[0] >> inv[1] >> inv[2] >> inv[3] >> score; cin.ignore();
        cin >> enemy_inv[0] >> enemy_inv[1] >> enemy_inv[2] >> enemy_inv[3] >> enemy_score; cin.ignore();
        
        current_inv = inv;
        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;


        // in the first league: BREW <id> | WAIT; later: BREW <id> | CAST <id> [<times>] | LEARN <id> | REST | WAIT
        cout << Brain(inv,enemy_inv,potions,spells,enemy_spells,learn_spells) << endl;
    }
}


