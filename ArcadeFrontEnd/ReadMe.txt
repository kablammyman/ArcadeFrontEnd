This is an arcade front end for mame and mame cabinets.

I coudlnt find a front end that did what I wanted...or even worked properly, so I'm writing my own.

You will notice that everything looks very simple, and that was on purpose. 
The real star should be the games, so the feature that will let this front end stand apart from the others is that it will load a random game to show its attract screen for X minutes. 
If the game goes unplayed for another X mins, it exits that game and will randomly load another one. Wash rinse repeat.

This is built using SDL2, winapi, and driectInput. I also use my Utils lib thats in another repo here on git. 

This is very much a WIP. I have a lot of optimzations and features to add, but it works as is so far.
