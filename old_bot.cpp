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
int *current_inv;
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

string cast_random_spell(int inv[4], vector<Action> spells)
{
    vector<int> ready_spells;
    for (int i = 4; i < spells.size(); i++)
    {
        cerr << spells[i].castable<<endl;
        if (spells[i].castable)
            ready_spells.push_back(i);
    }
    if (ready_spells.empty())
        return( "");
    int r = (rand() % (ready_spells.size())) ;
    Action s = spells[ready_spells[r]];
    if (is_valid_action(delta_add(inv,s.delta)))
        return(s.execute_action());
    else
        return ("");
}

string get_tier(int inv[4], int tier, int times, vector<Action> spells)
{
    if (tier == 0)
    {   if (spells[tier].castable)
            return (spells[tier].execute_action()+(spells[tier].repeatable?" "+to_string(times): ""));
    }
    else 
    {
        if (inv[tier - 1] < 1)
            return (get_tier(inv, tier - 1, times,spells));
        else if (spells[tier].castable)
            return (spells[tier].execute_action()+(spells[tier].repeatable?" "+to_string(times): ""));
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
            cerr << "need tier"<<i<<endl;
            return (get_tier(inv, i, abs(delta[i]) ,spells));
        }
        i--;
    }
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

Action get_easiest(int inv[4],vector<Action> potions)
{
    int min = 0;
    int min_turns = 100;
    for (int i = 0; i < potions.size() ; i++)
    {   int tmp = count_turns_required(inv, potions[i]);
        if (tmp <= min_turns && potions[i].price >= potions[2].price)
        {
            min_turns = tmp;
            min = i;
        }
    }
    cerr<<"easiest potion : turns : "<< count_turns_required(inv, potions[min]) << " " + to_string(potions[min].actionId) << endl;
    return (potions[min]);
}

bool compare_rpm(Action p1,Action p2)
{
    return ((float)p1.price / (float)count_turns_required(current_inv, p1) > (float)p2.price / (float)count_turns_required(current_inv, p2));
}

Action get_best_action(int inv[4], int enemy_inv[4], vector<Action> potions)
{
    int i = 0;
    float rpm;
    float rpm_enemy;

    float t;
    t = (float)count_turns_required(inv, potions[0]);
    rpm = (float)potions[0].price / t;
    t = (float)count_turns_required(inv, potions[0]);
    rpm_enemy = (float)potions[0].price / t;

    cerr << "rpm: " << rpm << " | e_rpm: " << rpm_enemy << endl;
    Action best = potions[i + (rpm > rpm_enemy ? 0 : 1)];
    cerr << "best id : " << best.actionId << endl;
    return (best);
}

string Brain(int inv[4], int enemy_inv[4], vector<Action> potions, vector<Action> spells, vector<Action> enemy_spells, vector<Action> learn_spells)
{
    sort(potions.begin(), potions.end(), compare_scores);
    //sort(potions.begin(), potions.end(), compare_rpm);
    
    
    //for (auto p : potions){
        //cerr << to_string(p.actionId) + " :"<< count_turns_required(inv, p)<< endl;
        Action p = potions[0];
        //Action p = get_easiest(inv, potions);
        //Action p = get_best_action(inv, enemy_inv, potions);

        cerr << "\ttarget id  " << p.actionId << endl;
        int *D = delta_add(inv, p.delta);
        if (is_valid_action(D))
        {
            return (p.execute_action());
        }
        else
        {
            //cerr << "invalid potion " << p.actionId << endl;

            for (auto s : spells)
            {
                int *DD = delta_add(D, s.delta);
                if (s.castable && is_valid_action(DD) && is_valid_action(delta_add(inv,s.delta)))
                {
                    //cerr << "\tValid spell " << s.actionId << endl;
                    return (s.execute_action());
                }
            }
        }
    //}

    if (rand()%100 < 5)
    {
        //return learn_spells[rand()%3].execute_action();
    }
    if (spells.size()>4 && rand()%100 < 75)
    {
        cerr << "random spell"<<endl;
        string s=cast_random_spell(inv, spells);
        if (s!="")
            return s;
    }
    return (gather_ingredients(inv,( 0 ? get_easiest(inv, potions) : potions[0]), spells));
    //return (gather_ingredients(inv,( 1? get_best_action(inv, enemy_inv, potions) : potions[0]), spells));
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