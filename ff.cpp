#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <unordered_set>
#include <tuple>
#include <algorithm>
#include <cstring>
#include <chrono>

using namespace std;
using namespace std::chrono;

# define min(a,b) (a < b ? a : b)

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
string gather_ingredients(int inv[4], Action potion, vector<Action> spells, int old_tier,vector<Action> learn_spells);
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

int inv_size(int inv[4])
{
    int s = 0;
    for (int i = 0; i < 4; i++)
        s += inv[i];
    return s;
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
    int lim = (spells[0].actionType == "CAST" ? 99 : 5);
    int max = min(spells.size(), lim);
    for (int i =0;i< max; i++)
    {
        if (spells[i].delta[tier]>0)
            result.push_back(spells[i]);
    }
    sort(result.begin(), result.end(), compare_benefit);
    return (result);
}

string empty_inventory(int inv[4], vector<Action> spells, vector<Action> learn_spells)
{
    int tier;
    int max=0;

    for(int i= 0; i<4; i++)
    {
        if (inv[i]>max)
        {
            max = inv[i];
            tier = i;
        }
    }
    Action s;
    int *d;
    for (auto sp : spells) // todo: pick best
    {
        if (sp.delta[tier] < 0)
        {
            d = delta_add(inv, sp.delta);
            if (is_valid_action(d) && inv_size(d) <= inv_size(inv))
                return (sp.castable ? sp.execute_action() : "REST");
        }
    }
    max = 0;
    Action l_spell;
    for (auto lsp : learn_spells)
    {
        if (lsp.delta[tier] < 0 && -lsp.delta[tier] > max)
        {
            d = delta_add(inv, lsp.delta);
            if (is_valid_action(d) && inv_size(d) <= inv_size(inv))
            { 
                max = -lsp.delta[tier];
                l_spell = lsp;
            }
        }
    }
    if (max)
        return (l_spell.execute_action());
    return ("WAIT get rid of tier "+to_string(tier));
}

string get_tier(int inv[4], int tier, int times, vector<Action> spells, vector<Action> learn_spells)
{
    vector<Action> tier_spells = get_tier_spells(tier, spells);
    int i = 0;
    int d[4];    

    //todo : push good spells to another vector
    while (i < tier_spells.size() && !tier_spells[i].castable ) 
    {
        if (get_spell_benefit(tier_spells[i]) > 2)
            return "REST";
        i++;
    }
    if (i == tier_spells.size())
        return("REST");

    cerr << "best spell for tier "<< tier << " : " << tier_spells[i].actionId << endl;

    if (is_valid_action(delta_add(inv,tier_spells[i].delta)))
        return(tier_spells[i].execute_action());
    else
    {
        string s;
        while (i < tier_spells.size() && (s = gather_ingredients(inv, tier_spells[i], spells, tier, learn_spells)) == "")
            i++;
        if (s != "") return (s);
        else
        return (empty_inventory(inv,spells,learn_spells));
    }
}

string gather_ingredients(int inv[4], Action potion, vector<Action> spells, int old_tier, vector<Action> learn_spells)
{
    int *delta;

    delta = delta_add(inv, potion.delta);
    int s = 0;
    int i = 3;
    while (i >= 0)
    {
        if (delta[i] < 0)
        {
            cerr << "need tier "<< i << " | old tier : " << old_tier <<endl;

            if ( old_tier != -1 && old_tier < i)
            {
                cerr << "recalling old tier" << endl;
                return ("");
            }
            return (get_tier(inv, i, abs(delta[i]) ,spells, learn_spells));
        }
        else s+=delta[i];
        i--;
    }
    if (potion.actionType == "BREW")
        targetId = 0;
    else if (!potion.castable)
        return "REST";
    if(s>10)
        return("");
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
            sum += (i + 0.5) * -delta[i];
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
Action get_action_from_id(vector<Action> actions, int id)
{
    Action tmp;
    for (int i= 0;i<actions.size(); i++)
    {
        if (actions[i].actionId == id)
            return (actions[i]);
    }
    return (tmp);
}
bool compare_turns_required(Action p1,Action p2)
{
    return (count_turns_required(current_inv, p1) < count_turns_required(current_inv, p2));
}
bool compare_rpt(Action p1,Action p2)
{
    return (((float)p1.price / (float)count_turns_required(current_inv, p1)) < ((float)p2.price / (float)count_turns_required(current_inv, p2)));
}
Action get_easiest(int inv[4], vector<Action> potions)
{
    vector<Action> tmp = potions;

    sort(tmp.begin(), tmp.end(), compare_turns_required);
    return (tmp[0]);
}
Action get_best_rpt(int inv[4], vector<Action> potions)
{
    vector<Action> tmp = potions;

    sort(tmp.begin(), tmp.end(), compare_rpt);
    return (tmp[0]);
}

int g_score;

Action pick_best_target(int inv[4], int enemy_inv[4], vector<Action> potions, vector<Action> spells, vector<Action> enemy_spells, vector<Action> learn_spells)
{
    if (count_turns_required(inv, potions[0]) < count_turns_required(enemy_inv, potions[0]))
        return (potions[0]);
    else
        return (potions[1]);
}

Action best_learn_spell(int tier, vector<Action> learn_spells, vector<Action> spells)
{
    //vector<Action> tier_spells = get_tier_spells(tier,spells);
    vector<Action> tier_learn_spells = get_tier_spells(tier,learn_spells);
    return (tier_learn_spells[0]);
}

string get_learn_spell(int inv[4], vector<Action> spells, vector<Action> learn_spells, int MAX_SPELLS)
{
    vector<Action> tier_spells;
    vector<Action> tier_learn_spells;
    if(spells.size() < MAX_SPELLS)
    {
        //return(learn_spells[0].execute_action());
        for(int tier =0; tier < 4; tier++)
        {
            tier_learn_spells = get_tier_spells(tier, learn_spells);

            if (tier_learn_spells.size() > 0)
            {
                for(int i = 0; i < tier_learn_spells.size(); i++)
                {   
                    bool bad = 0;
                    for (int j = tier+1;j < 4;j++)
                    {
                        if (tier_learn_spells[i].delta[j] < -1)
                            bad = 1;
                    }   
                    if (!bad)
                    {
                        tier_spells = get_tier_spells(tier, spells);
                        if (get_spell_benefit(tier_learn_spells[i]) > get_spell_benefit(tier_spells[0]))
                        {
                            if (inv[0] < get_action_index(learn_spells, tier_learn_spells[i].actionId))
                                return (get_tier(inv, 0, 1, spells, learn_spells));
                            return (tier_learn_spells[i].execute_action());
                        }
                    }
                }
            }
        }
    }
    return "";
}
//BFS 
unsigned char *delta2chars(int d[4])
{
    unsigned char *delta = (unsigned char *)malloc(4);
    for(int i = 0; i < 4; i++)
        delta[i] = (unsigned char)d[i];
    return (delta);
}

int delta2int(unsigned char *d)
{
    int delta = 0;

    for(int i = 0; i < 4; i++)
        delta = delta * 256 + d[i];

    return (delta);

}

int get_tier_from_int(int delta, int tier)
{
    return (int)((char)(delta >> (8 * (3 - tier))));
}

char *get_spells_ids(vector<Action> spells)
{
    char *ids = (char *)malloc(spells.size()+1);
    for (int i = 0; i < spells.size();i++)
        ids[i] = (char)spells[i].actionId;
    ids[spells.size()] = 0;
    return (ids);
}

unsigned char *get_spells_deltas(vector<Action> spells)
{
    unsigned char *deltas = (unsigned char *)malloc(4*spells.size() + 1);

    for (int i = 0; i < spells.size(); i++)
        memcpy(&deltas[i*4], delta2chars(spells[i].delta), 4);
    deltas[spells.size()*4] = 0;
    return (deltas);
}

unsigned char *delta_add_ch(unsigned char *d1, unsigned char* d2)
{
    unsigned char *delta = (unsigned char *)malloc(4);
    for(int i = 0; i < 4; i++)
        delta[i] = (unsigned char)((char)d1[i] + (char)d2[i]);
    return (delta);
}

bool done_searching(unsigned char *d)
{
    for(int i = 0; i < 4; i++)
        if ((char)d[i] < 0) return (false);
    return (true);
}

char *bfs_get_spell_comb(char *spell_ids, unsigned char *spell_deltas, unsigned char *potion_d, unsigned char *inv)
{
    unsigned char *diff = delta_add_ch(inv,potion_d);
    
    queue< tuple<unsigned char *, char*> > q; //  1 for current_sum // 2 for storing paths each char is actionId

    q.push({diff, strdup("")});

    unordered_set<int>visited;

    while (q.size())
    {
        int s = q.size();
        while (s--)
        {
            auto front = q.front();
            unsigned char *current_sum = get<0>(front);
            char *path = get<1>(front);
            if (done_searching(current_sum))
            {
                return (path);
            }
            q.pop();
            if (visited.find(delta2int(current_sum)) != visited.end())
                continue;
            visited.insert(delta2int(current_sum));
            for (int i = 0;i < strlen(spell_ids);i++)
            {
                char *tmp = (char *)malloc(strlen(path) + 2);
                memcpy(tmp, path, strlen(path));
                tmp[strlen(path)] = spell_ids[i];
                tmp[strlen(path)+1] = 0;
                
                q.push({delta_add_ch(current_sum,&spell_deltas[i*4]), tmp});
            }
        }
    }
    return (strdup(""));
}

int bfs_get_depth(char *spell_ids, unsigned char *spell_deltas, unsigned char *potion_d, unsigned char *inv)
{
    unsigned char *diff = delta_add_ch(inv,potion_d);
    
    queue< unsigned char * > q;

    q.push(diff);

    unordered_set<int>visited;

    int d= 0
    
    while (q.size())
    {
        int s = q.size();
        while (s--)
        {
            auto ;
            unsigned char *current_sum = q.front()
            if (done_searching(current_sum))
                return (d);
            q.pop();
            if (visited.find(delta2int(current_sum)) != visited.end())
                continue;
            visited.insert(delta2int(current_sum));
            for (int i = 0;i < strlen(spell_ids);i++)
                q.push(delta_add_ch(current_sum,&spell_deltas[i*4]));
        }
        d++;
    }
    return (-1);
}
void print_path(char *path, float time)
{
    for (int i = 0; i < strlen(path); i++)
        cerr << " " << (int)path[i];
    cerr << endl <<"time : " << time << " ms" << endl;
}
//end BFS
int turn = 0;
string Brain(int inv[4], int enemy_inv[4], vector<Action> potions, vector<Action> spells, vector<Action> enemy_spells, vector<Action> learn_spells)
{
    Action target;
    int d[4];
    vector<Action> ori = learn_spells;
    sort(potions.begin(),potions.end(),compare_scores);
    if (!targetId || get_action_index(potions, targetId) == -1 
        || count_turns_required(inv, get_action_from_id(potions, targetId)) > count_turns_required(enemy_inv, get_action_from_id(potions, targetId)))
    {
        target = pick_best_target(inv, enemy_inv, potions, spells, enemy_spells, learn_spells);
        targetId = target.actionId;
    }
    if(inv_size(inv) > 8)
    {
        for (auto p : potions)
        {
            if (is_valid_action(delta_add(inv, p.delta)))
            {
                return (p.execute_action());
            }
        } 
    }
    else
        target = get_action_from_id(potions, targetId);

    
    //target=get_easiest(inv, potions);
    //target=get_best_rpt(inv, potions);
    //target=potions[0];
    cerr << "Target ID : " << target.actionId << ", Price : " << target.price << "  fir:"<<spells[0].actionId<< endl;

    if (is_valid_action(delta_add(inv, target.delta)))
    {
        return (target.execute_action());
    }
    int max_spells = 12;
    string s = get_learn_spell(inv, spells, learn_spells, max_spells);
    if (s!="")
        return(s);
       
    auto start = high_resolution_clock::now(); 
    
    char *path = bfs_get_spell_comb(get_spells_ids(spells), get_spells_deltas(spells), delta2chars(target.delta), delta2chars(inv));
    
    auto stop = high_resolution_clock::now(); 
    auto duration = duration_cast<microseconds>(stop - start).count()/(float)1000; 

    print_path(path,duration);
    if (strlen(path))
    {
        for (int i = 0; i< strlen(path); i++)
        {
            Action sp = get_action_from_id(spells, (int)path[i]);
            if (sp.castable && is_valid_action(delta_add(inv,sp.delta)))
            {
                return (sp.execute_action() +" "+ to_string(1));
            }
        }
    }
    return (gather_ingredients(inv, target, spells, -1,ori));
}

int main()
{
    //Learn when u need tier0
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
        g_score = score;
        current_inv = inv;
        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;
      

        // in the first league: BREW <id> | WAIT; later: BREW <id> | CAST <id> [<times>] | LEARN <id> | REST | WAIT
        cout << Brain(inv,enemy_inv,potions,spells,enemy_spells,learn_spells) << endl;
        turn ++;
    }
}