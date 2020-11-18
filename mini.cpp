#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

#define max(a,b) (a > b ? a : b)

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

bool is_brewable(int inv[4], Action potion)
{
    int *d = delta_add(inv, potion.delta);
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

bool is_castable(int inv[4], Action spell)
{
    if (!spell.castable) return (false);
    int *d = delta_add(inv, spell.delta);
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
bool is_learnable(int inv[4], Action learn_spell)
{
    int tax[4] = {-learn_spell.tomeIndex,0,0,0};

    int *d = delta_add(inv, tax);
    for (int i = 0; i < 4; i++)
    {
        if (d[i] < 0)
            return(false);
    }
    return(true);
}

Action *get_action_with_id(vector<Action> actions, int id)
{
    for (auto a : actions)
        if (a.actionId == id)
            return(&a);
}

int inv_size(int inv[4])
{
    int s = 0;
    for (int i = 0; i < 4; i++)
        s += inv[i];
    return (s);
}

string inv_to_string(int inv[4])
{
    string res = "[";
    for (int i =0; i<4;i++)
    {
        if(i)
            res += ", ";
        res += to_string(inv[i]);
    }
    return (res + "]");
}

string actions_to_string(vector<Action> actions)
{
    string res = "";
    for (int i = 0; i < actions.size(); i++)
    {
        if (i)
            res += " , ";
        res += inv_to_string(actions[i].delta);
        if (actions[i].actionType == "LEARN")
            res += " tax:"+to_string(actions[i].tomeIndex)+" taxCount:"+to_string(actions[i].taxCount);
        if (actions[i].actionType == "BREW")
            res += " price:"+to_string(actions[i].price);
    }
    return (res);
}

class State
{   public:
        int round;
        int score,enemy_score;
        int potions_count;
        int enemy_potions_count;

        int *inv,*enemy_inv;
        vector<Action> potions;
        vector<Action> spells;
        vector<Action> learn_spells;
        vector<Action> enemy_spells;



        //Constructor
        State(  int r,int sc,int e_sc,int *inv,int *e_inv,
                vector<Action> p, vector<Action> s, vector<Action> l_s, vector<Action> e_s,
                int p_count, int e_p_count)
        {
            round = r;
            score = sc;
            enemy_score =e_sc;
            potions_count = p_count;
            enemy_potions_count = e_p_count;

            inv = inv;
            enemy_inv = e_inv;
            potions = p;
            spells = s;
            learn_spells = l_s;
            enemy_spells = e_s;
        }

        bool game_over() // true if game is over
        {
            if (round == 100 || potions_count > 6 || enemy_potions_count > 6)
                return (1);
            return (0);
        }

        int state_score()
        {
            return (score - enemy_score);
        }
        
        void set_spell_castability(vector<Action> sps, int id, bool castable)
        {
            Action *s = get_action_with_id(sps, id);
            s->castable = castable;
        }

        vector<Action> get_possible_player_moves()
        {
            vector<Action> moves; 
            for (auto p : potions)
                if(is_brewable(inv, p)) moves.push_back(p);
            for (auto s : spells)
                if (is_castable(inv, s)) moves.push_back(s);
            for (auto ls : learn_spells)
                if (is_learnable(inv, ls)) moves.push_back(ls);
            return (moves);
        }
        
        vector<Action> get_possible_enemy_moves()
        {
            vector<Action> moves; 
            for (auto p : potions)
                if(is_brewable(enemy_inv, p)) moves.push_back(p);
            for (auto s : enemy_spells)
                if (is_castable(enemy_inv, s)) moves.push_back(s);
            for (auto ls : learn_spells)
                if (is_learnable(enemy_inv, ls)) moves.push_back(ls);
        }

        State get_next_state_player(Action a) //todo: test if player or enemy: update accordingly
        {
            State st = *this; // cloning the current state
            
            if (a.actionType == "BREW") //
            {
                //add to score 
                st.score += a.price;
                //update inventory
                st.inv = delta_add(st.inv, a.delta);
                //remove potions from potions list // only when both player play // depends on who plays first // set potion as crafted then remove it when both player are done
                //st.potions.erase(remove(st.potions.begin(), st.potions.end(), a),st.potions.end());
                // update potions count
                st.potions_count+=1;
            }
            if (a.actionType == "CAST") //
            {
                //update inventory
                st.inv = delta_add(st.inv, a.delta);
                //set spell as uncastable
                set_spell_castability(st.spells,a.actionId, 0);
            }
            if (a.actionType == "LEARN") //
            {
                //update inventory
                int tax[4] = {-a.tomeIndex,0,0,0};
                st.inv = delta_add(st.inv, tax);
                //collect tier_0 already placed
                st.inv[0] += min(10 - inv_size(st.inv), a.taxCount);
                //change type to spell
                a.actionType = "CAST";
                //add to spells
                st.spells.push_back(a);
                //remove from learn_spells
                st.learn_spells.erase(remove(st.learn_spells.begin(), st.learn_spells.end(), a),st.learn_spells.end());
            }
            if (a.actionType == "REST")
            {
                for (auto s : st.spells)
                    s.castable = 1;
            }
            return (st);
        }

        State get_next_state_enemy(Action a) //todo: test if player or enemy: update accordingly
        {
            State st = *this; // cloning the current state
            
            if (a.actionType == "BREW") //
            {
                //add to score 
                st.enemy_score += a.price;
                //update inventory
                st.enemy_inv = delta_add(st.enemy_inv, a.delta);
                //remove potions from potions list 
                //st.potions.erase(remove(st.potions.begin(), st.potions.end(), a),st.potions.end());
                // update potions count
                st.potions_count+=1;
            }
            if (a.actionType == "OPPONENT_CAST") //
            {
                //update inventory
                st.enemy_inv = delta_add(st.enemy_inv, a.delta);
                //set spell as uncastable
                set_spell_castability(st.enemy_spells,a.actionId, 0);
            }
            if (a.actionType == "LEARN") //
            {
                //update inventory
                int tax[4] = {-a.tomeIndex,0,0,0};
                st.enemy_inv = delta_add(st.enemy_inv, tax);
                //collect tier_0 already placed
                st.enemy_inv[0] += min(10 - inv_size(st.enemy_inv), a.taxCount);
                //change type to spell
                a.actionType = "OPPONENT_CAST";
                //add to spells
                st.enemy_spells.push_back(a);
                //remove from learn_spells
                st.learn_spells.erase(remove(st.learn_spells.begin(), st.learn_spells.end(), a),st.learn_spells.end());
            }
            if (a.actionType == "REST")
            {
                for (auto s : st.enemy_spells)
                    s.castable = 1;
            }
            return (st);
        }

        void print()
        {
            cerr << "Round " << round << " | potions_count : " << potions_count<< " e_potions_count : " << enemy_potions_count <<endl;
            cerr << "Score = " << score << " Enemy_score : " << enemy_score << endl;
            cerr << "inv = " << inv_to_string(inv) << " Enemy_inv : " << inv_to_string(enemy_inv) << endl;
            cerr << "Player spells : " << endl << actions_to_string(spells) << endl;
            cerr << "Enemy spells : " << endl << actions_to_string(enemy_spells) << endl;
            cerr << "Learn spells : " << endl << actions_to_string(learn_spells) << endl;
            cerr << "Potions : " << endl << actions_to_string(potions) << endl;
        }

        vector<State> get_children() //compute all possible moves and apply them to the current state and return
        {
            vector<State> children;


            return (children);
        }
};

int     minimax(State s, int depth, bool maximizing)
{
    if (depth == 0 || s.game_over())
        return (s.state_score());

    if (maximizing)
    {
        int max_score = -500; //min score achievable  (max_potion_score * 6)
        vector<State> children = s.get_children();
        for (auto c : children)
            max_score = max(max_score, minimax(c, depth - 1, false));
        return (max_score);
    }
    else
    {
        int min_score = 500;
        vector<State> children = s.get_children();
        for (auto c : children)
            min_score = min(min_score, minimax(c, depth - 1, true));
        return (min_score);
    }
}