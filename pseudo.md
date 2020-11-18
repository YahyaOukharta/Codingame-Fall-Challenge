- Strategy :
    loop over potions:
        try to brew it 
        if cant brew it look for a spell that will complete recipe:
            if found : cast spell if not exhausted otherwise REST
    


- Procedures :
    - get_tier0 : spell 0
    - get_tier1 : spell 0 + spell 1
    - get_tier2 : spell 0 + spell 1 + spell 2 
    - get_tier3 : spell 0 + spell 1 + spell 2 + spell 3

    - get_tier0 : spell 0
    - get_tier1 : get_tier0 + spell 1
    - get_tier2 : get_tier1 + spell 2
    - get_tier3 : get_tier2 + spell 3


- Potion [x0,x1,x2,x3]   

- Inv    [i0,i1,i2,i3]

- Sum    [d0,d1,d2,d3]

if sum is valid inventory : brew 
else 
    loop over tiers in reverse: 
        if d3 < 0: 
            execute get_tier3 procedure abs(d3) times 
        if d2 < 0: 
            execute get_tier2 procedure abs(d2) times 
        if d1 < 0: 
            execute get_tier1 procedure abs(d1) times 
        if d0 < 0: 
            execute get_tier0 procedure abs(d0) times 


- count number of turns required for a potion

tier3 = 4 turns + rest
tier2 = 3 turns + rest
tier1 = 2 turns + rest
 tier0 = 1 turn + rest

turns = 0
for d in delta:
    loop over tiers in reverse: 
        turns += tier + 2



- target best potion: find out if it will take the enemy less turns : if so target to the next best potion

- update get_tier function as they will look for the best spell combination to make that tier
- learn spells if a spell will allow get_tier function to finish in less turns

- must be able to compare BREW actions ???? most profitable ?  
- 2 turns 8 rupees vs 4 turns for 20 rupees // (rupees per turn)

- https://www.geeksforgeeks.org/generate-a-combination-of-minimum-coins-that-results-to-a-given-value/