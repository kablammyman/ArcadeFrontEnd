This is an arcade front end for mame and mame cabinets.

I coudlnt find a front end that did what I wanted...or even worked properly, so I'm wiring my own.

You will notice that everything looks very simple, and that was on purpose. 
The real star should be the games, so the feature that will let this front end stand apart from the others is that it will load a random game to show its attract screen for X minutes. 
If the game goes unplayed for another x mins, it exits that game and will randomly load another one.

This is built using SDL2 and winapi. I also use my Utils lib thats in another repo here on git. 

This is very much a WIP, and the main attraction of auto running games isnt implemented yet, but the basic menu, getting game data and rom lists and showing snap shots are implemented.