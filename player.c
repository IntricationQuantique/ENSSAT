#include <stdbool.h> // bool, true, false
#include <stdlib.h>  // rand
#include <stdio.h>   // printf
#include "lode_runner.h" // types and prototypes used to code the game
#include <stdlib.h> // malloc, free


// global declarations used by the game engine
extern const char BOMB;  // ascii used for the bomb
extern const char BONUS;  // ascii used for the bonuses
extern const char CABLE;  // ascii used for the cable
extern const char ENEMY;  // ascii used for the ennemis
extern const char EXIT;   // ascii used for the exit
extern const char FLOOR;  // ascii used for the floor
extern const char LADDER; // ascii used for the ladder
extern const char PATH;   // ascii used for the pathes
extern const char RUNNER; // ascii used for runner
extern const char WALL;   // ascii used for the walls

extern const int BOMB_TTL; // time to live for bombs

extern bool DEBUG; // true if and only if the game runs in debug mode

const char *students = "Allaeys Arthur"; // replace Radom with the student names here

//structures
typedef struct{
  int x;
  int y; 
}position;

typedef struct{
  int *dist;
  position *pred;
  action *actionpred;
}plvalue;


// local prototypes (add your own prototypes below)
void print_action(action);
plvalue *parcours_largeur(levelinfo, character);
bonus plus_proche_bonus(bonus_list, levelinfo, int *);
action reconstruction(plvalue *, bonus, int, int);
action next_move(levelinfo, character, bonus_list);
bool haut(position *, character, levelinfo, int);
bool bas(position *, character, levelinfo, int);
action cable(position *, character, action, int);
action echelle(position *, character, action, levelinfo, int);
bool tomber(levelinfo, position *, character, action, int);


// BUGS - PROBLEMES
/*
  -mauvaise gestion encerclement sur câble ------------------------------ OK
  -pose bombe sur échelle -> game over ---------------------------------- OK
  -pose bombe pied échelle -> game over --------------------------------- OK
  -tombe sur les ennemis ------------------------------------------------ OK
  -boucle infinie quand sur échelle ennemi en y+2 et y-2 -> faire une fonction inspirée de cable --------- OK
  -monte l'échelle de la sortie sans raison ----------------------------- NON RESOLU
*/


/* 
  function to code: it may use as many modules (functions and procedures) as needed
  Input (see lode_runner.h for the type descriptions): 
    - level provides the information for the game level
    - characterl is the linked list of all the characters (runner and enemies)
    - bonusl is the linked list of all the bonuses that have not been collected yet
    - bombl is the linked list of all the bombs that are still active
  Output
    - the action to perform
*/
action lode_runner(
  levelinfo level,
  character_list characterl,
  bonus_list bonusl,
  bomb_list bombl
  )
{
  action a; // action to choose and then return
  bool ok; // boolean to control the do while loops

  //On compte le nombre d'ennemis restants.
  //On met leur position dans un tableau pour plus simplement itérer dessus 
  int nbennemis = 0;
  character_list temp = characterl;
  while(temp != NULL){
    if(temp->c.item == RUNNER){
      temp = temp->next;
    }
    else{
      nbennemis++;
      temp = temp->next;
    }
  }


  int xr; // runner's x position
  int yr; // runner's y position
  position *ennemis = malloc(nbennemis * sizeof(position));
  character_list pchar=characterl; // iterator on the character list

  // looking for the runner ; we know s.he is in the list
  int cpt = 0;
  while(pchar != NULL){ 
    if(pchar->c.item==RUNNER) // runner found
    {
      xr=pchar->c.x; 
      yr=pchar->c.y;
    }
    else{ // otherwise move on next character
      position ennemi = {x : pchar->c.x, y : pchar->c.y};
      ennemis[cpt] = ennemi;
      cpt++;
    } 
    pchar=pchar->next;
  }
  ok=false; // ok will become true when a valid action will be guessed
  do
  {
    /*
    a = rand() % 7; // randomly guess a integer between 0 and 6 as we have 7 possible actions
    switch (a)
    {
    case NONE:
      ok = true; // it's always possible, though often useless, to do nothing ;-)
      break;
    case UP:
      if (level.map[yr][xr] == LADDER)
        ok = true; // it's possible to go up if on a ladder
      break;
    case DOWN:
      if (level.map[yr + 1][xr] == LADDER || level.map[yr + 1][xr] == PATH)
        ok = true; // it's possible to go down if there is a ladder or nothing (jump) below 
      break;
    case LEFT:
      if (level.map[yr][xr - 1] != WALL && level.map[yr][xr - 1] != FLOOR)
        ok = true; // it's possible to go left if there is no wall or floor
      break;
    case RIGHT:
      if (level.map[yr][xr + 1] != WALL && level.map[yr][xr + 1] != FLOOR)
        ok = true; // it's possible to go right if there is no wall or floor
      break;
    case BOMB_LEFT: 
      if (level.map[yr + 1][xr - 1] == FLOOR && level.map[yr][xr - 1] == PATH)
        ok = true; // it's possible to bomb left if there is some floor that can be destroyed
      break;
    case BOMB_RIGHT:
      if (level.map[yr + 1][xr + 1] == FLOOR && level.map[yr][xr + 1] == PATH)
        ok = true; // it's possible to bomb right if there is some floor that can be destroyed
      break;
    */

    //On note la position du joueur
    character joueur;
    joueur.x = xr;
    joueur.y = yr;
    //On cherche quelle action réaliser pour ce rapprocher du bonus
    a = next_move(level, joueur, bonusl);
    //On initialise un compteur pour détecter les boucles infinies
    int cpt = 0;
    //On vérifie qu'on peut réaliser l'action et le faire en sécurité
    while(ok == false){
      cpt++;
      // On vérifie qu'il n'y a pas de boucle infinie
      if(cpt > 20){
        // Il y a une boucle infinie car le joueur n'a pas d'option gagnante. Il fait alors l'action NONE.
        a = NONE;
      }
      switch (a){
        case UP:
          if (level.map[yr][xr] == LADDER){
            ok = true; // it's possible to go up if on a ladder
            a = echelle(ennemis, joueur, a, level, nbennemis);
            if(a != UP){
              // On ne peut pas monter car il y a un ennemi. On va chercher la nouvelle action.
              ok = false;
            }
          }
          else{
            //Si on ne peut pas monter par une échelle, on essaie d'aller à droite
            a = RIGHT;
            ok = false;
          }
          break;
        //bas --------------------------------------------------------------------------------------------------------------------
        case DOWN:
          if(level.map[yr + 1][xr] == LADDER){
            ok = true; // it's possible to go down if there is a ladder
            a = echelle(ennemis, joueur, a, level, nbennemis);
            if(a != DOWN){
              // On ne peut pas descendre car il y a un ennemi. On va chercher la nouvelle action.
              ok = false;
            }
          }
          else{
            if(level.map[yr + 1][xr] == PATH && level.map[yr + 1][xr] != LADDER){
              //On veut sauter
              ok = tomber(level, ennemis, joueur, a, nbennemis);
              if(!ok){ 
                //si on ne peut pas sauter en sécurité, alors on essaie d'aller à droite (par défaut)
                a = RIGHT;
              }
            }
            else{
              //Si on ne peut pas aller à descendre par une échelle ou un saut, on essaie d'aller à droite
              a = RIGHT;
              ok = false;
            }
          }
          break;
        //gauche ----------------------------------------------------------------------------------------------------------------  
        case LEFT:
          if(level.map[yr - 1][xr] == CABLE){
            //le joueur est sur un câble
            ok = true;
            a = cable(ennemis, joueur, a, nbennemis);
          }
          else{
            if (level.map[yr][xr - 1] != WALL && level.map[yr][xr - 1] != FLOOR){
              //le joueur veut sauter
              ok = true; // it's possible to go left if there is no wall or floor             
              if(level.map[yr + 1][xr - 1] == PATH){
                // On vérifie que c'est le vide, pour ne pas entrer en conflit avec les échelles
                if(!tomber(level, ennemis, joueur, a, nbennemis)){
                  //il ne peut pas le faire sans danger
                  ok = false;
                  a = RIGHT;
                }
              } 
              else{
                //le joueur se déplace sur une plateforme
                for(int i = 0; i< nbennemis; i++){
                  if((ennemis[i].x == joueur.x - 2) && (ennemis[i].y == joueur.y)){
                    // important de le mettre avant le deuxième if, pour prioriser le deuxième (plus dangereux)
                    a = BOMB_LEFT;
                    ok = false; //il y a un ennemi sur le chemin
                  }
                  if(((ennemis[i].x == joueur.x - 1) && (ennemis[i].y == joueur.y)) //ennemi à gauche
                      || ((ennemis[i].x == joueur.x - 1) && (ennemis[i].y == joueur.y - 1)) //ennemi en à gauche en haut
                      || ((ennemis[i].x == joueur.x - 1) && (ennemis[i].y == joueur.y + 1)  && (level.map[yr + 1][xr - 1] == LADDER)) 
                          //ennemi à gauche en bas, on vérifie qu'il y a une échelle, sinon l'ennemi est dans un trou de bombe
                    ){
                    a = RIGHT;
                    ok = false; //il y a un ennemi sur le chemin
                  }
                }
              }
            }
            else{
              //Si on ne peut pas aller à gauche à cause d'un mur ou du sol, on essaie d'aller à droite
              a = RIGHT;
              ok = false;
            }
          }
          break;
        //droite ----------------------------------------------------------------------------------------------------------
        case RIGHT:
          if(level.map[yr - 1][xr] == CABLE){
            //le joueur est sur un câble
            ok = true;
            a = cable(ennemis, joueur, a, nbennemis);
          }
          else{
            if (level.map[yr][xr + 1] != WALL && level.map[yr][xr + 1] != FLOOR){
              //le joueur veut sauter
              ok = true; // it's possible to go right if there is no wall or floor
              if(level.map[yr + 1][xr + 1] == PATH){
                // On vérifie que c'est le vide, pour ne pas entrer en conflit avec les échelles
                if(!tomber(level, ennemis, joueur, a, nbennemis)){
                  //il ne peut pas le faire sans danger
                  ok = false;
                  a = LEFT;
                }
              }
              else{
                //le joueur se déplace sur une plateforme
                for(int i = 0; i< nbennemis; i++){
                  if((ennemis[i].x == joueur.x + 2) && (ennemis[i].y == joueur.y)){
                    // important de le mettre avant le deuxième if, pour prioriser le deuxième (plus dangereux)
                    a = BOMB_RIGHT;
                    ok = false; //il y a un ennemi sur le chemin
                  }
                  if(((ennemis[i].x == joueur.x + 1) && (ennemis[i].y == joueur.y)) //ennemi à droite
                      || ((ennemis[i].x == joueur.x + 1) && (ennemis[i].y == joueur.y - 1)) //ennemi en haut à droite 
                      || ((ennemis[i].x == joueur.x + 1) && (ennemis[i].y == joueur.y + 1) && (level.map[yr + 1][xr + 1] == LADDER)) 
                          //ennemi en bas à droite, on vérifie qu'il y a une échelle, sinon l'ennemi est dans un trou de bombe
                    ){
                    a = LEFT;
                    ok = false; //il y a un ennemi sur le chemin
                  }
                }
              }
            }
            else{
              //Si on ne peut pas aller à droite à cause d'un mur ou du sol, on essaie d'aller à gauche
              a = LEFT;
              ok = false;
            }
          }
          break;
        //none ------------------------------------------------------------------------------------------------------------
        case NONE:
          ok= true;
          break;
        case BOMB_LEFT:
          // On vérifie qu'on ne pose pas de bombe sur une échelle
          if((level.map[yr + 1][xr - 1] == LADDER) || (level.map[yr][xr - 1] == LADDER)){
            a = RIGHT;
          }
          ok= true;
          break;
        case BOMB_RIGHT:
          // On vérifie qu'on ne pose pas de bombe sur une échelle
          if((level.map[yr + 1][xr + 1] == LADDER) || (level.map[yr][xr + 1] == LADDER)){
            a = LEFT;
          }
          ok = true;
          break;
      }
    }
    if(DEBUG) // only when the game is in debug mode
    {
      printf("[Player] Candidate action ");
      print_action(a);
      if(ok){
        printf(" is valid"); 
      }
      else{
        printf(" not valid");
      }
      printf(".\n");
    }
  }while (!ok);
  free(ennemis);
  return a; // action to perform
}

/*
  Procedure that print the action name based on its enum type value
  Input:
    - the action a to print
*/
void print_action(action a){
  switch (a){
  case NONE:
    printf("NONE");
    break;
  case UP:
    printf("UP");
    break;
  case DOWN:
    printf("DOWN");
    break;
  case LEFT:
    printf("LEFT");
    break;
  case RIGHT:
    printf("RIGHT");
    break;
  case BOMB_LEFT:
    printf("BOMB_LEFT");
    break;
  case BOMB_RIGHT:
    printf("BOMB_RIGHT");
    break;
  }
}




//Nouveau code : fonctionnel (le premier, non-fonctionnel, est commenté à la fin)


plvalue *parcours_largeur(levelinfo lvlinfo, character joueur){
  int taille = lvlinfo.xsize * lvlinfo.ysize;
  // On initialise la structure pour stocker toutes les informations nécessaires pour la suite
  plvalue *infopl = malloc(sizeof(plvalue));
  // On initalise les variables du parcours en largeur
  position courant;
  int debut = 0;
  int fin = 0;
  // On initialise le tableau des distances
  int *distances = malloc(taille * sizeof(int));
  for(int i = 0; i < taille; i++){
    distances[i]= -1;
  }
  // On initialise la file, le tableau des prédécesseurs et de leurs actions correspondantes
  position *file = malloc(taille * sizeof(position));
  position *predecesseurs = malloc(taille * sizeof(position));
  action* action_precedente = malloc(taille * sizeof(action));
  for(int i = 0; i < taille; i++){
    predecesseurs[i].x = -1;
    predecesseurs[i].y = -1;
  }
  
  // On enfile le sommet du joueur dans la file
  file[debut].x = joueur.x;
  file[debut].y = joueur.y;
  distances[joueur.y * lvlinfo.xsize + joueur.x] = 0;
  fin++;

  while(debut < fin){
    // On dépile le premier sommet de la file
    courant = file[debut];
    debut++;
    if(lvlinfo.map[courant.y][courant.x] == LADDER){
      // On enfile le sommet du dessus s'il est accessible
      if(distances[(courant.y - 1) * lvlinfo.xsize + courant.x] == -1){
        // On l'enfile s'il n'a pas déjà été exploré
        file[fin].x = courant.x;
        file[fin].y = courant.y - 1;
        distances[file[fin].y * lvlinfo.xsize + file[fin].x] = distances[courant.y * lvlinfo.xsize + courant.x] + 1;
        predecesseurs[file[fin].y * lvlinfo.xsize + file[fin].x] = courant;
        action_precedente[file[fin].y * lvlinfo.xsize + file[fin].x] = UP;
        fin++;
      }
    }
    if (lvlinfo.map[courant.y + 1][courant.x] == LADDER || lvlinfo.map[courant.y + 1][courant.x] == PATH){
      // On enfile le sommet du dessous s'il est accessible
      if(distances[(courant.y + 1) * lvlinfo.xsize + courant.x] == -1){
        // On l'enfile s'il n'a pas déjà été exploré
        file[fin].x = courant.x;
        file[fin].y = courant.y + 1;
        distances[file[fin].y * lvlinfo.xsize + file[fin].x] = distances[courant.y * lvlinfo.xsize + courant.x] + 1;
        predecesseurs[file[fin].y * lvlinfo.xsize + file[fin].x] = courant;
        action_precedente[file[fin].y * lvlinfo.xsize + file[fin].x] = DOWN;
        fin++;
      }
    }
    if (lvlinfo.map[courant.y][courant.x - 1] != WALL && lvlinfo.map[courant.y][courant.x - 1] != FLOOR){
      // On enfile le sommet de gauche s'il est accessible
      if(distances[courant.y * lvlinfo.xsize + courant.x - 1] == -1){
        // On l'enfile s'il n'a pas déjà été exploré
        file[fin].x = courant.x - 1;
        file[fin].y = courant.y;
        distances[file[fin].y * lvlinfo.xsize + file[fin].x] = distances[courant.y * lvlinfo.xsize + courant.x] + 1;
        predecesseurs[file[fin].y * lvlinfo.xsize + file[fin].x] = courant;
        action_precedente[file[fin].y * lvlinfo.xsize + file[fin].x] = LEFT;
        fin++;
      }
    }
    if (lvlinfo.map[courant.y][courant.x + 1] != WALL && lvlinfo.map[courant.y][courant.x + 1] != FLOOR){
      // On enfile le sommet de droite s'il est accessible
      if(distances[courant.y * lvlinfo.xsize + courant.x + 1] == -1){
        // On l'enfile s'il n'a pas déjà été exploré
        file[fin].x = courant.x + 1;
        file[fin].y = courant.y;
        distances[file[fin].y * lvlinfo.xsize + file[fin].x] = distances[courant.y * lvlinfo.xsize + courant.x] + 1;
        predecesseurs[file[fin].y * lvlinfo.xsize + file[fin].x] = courant;
        action_precedente[file[fin].y * lvlinfo.xsize + file[fin].x] = RIGHT;
        fin++;
      }
    }
  }
  // On renvoit toutes les informations nécessaires pour la suite
  infopl->dist = distances;
  infopl->pred = predecesseurs;
  infopl->actionpred = action_precedente;
  free(file);
  return infopl;
}

bonus plus_proche_bonus(bonus_list bonusl, levelinfo lvlinfo, int *distances){
  if(bonusl == NULL){
    // S'il n'y a plus de bonus à aller chercher, alors on va à la sortie
    bonus sortie = {x : lvlinfo.xexit, y : lvlinfo.yexit};
    return sortie;
  }
  else{
    // On cherche quel bonus de la liste est le plus proche
    bonus ppbonus = bonusl->b;
    int min = distances[ppbonus.y * lvlinfo.xsize + ppbonus.x];
    while (bonusl != NULL){
      if(distances[bonusl->b.y * lvlinfo.xsize + bonusl->b.x] < min){
        ppbonus = bonusl->b;
        min = distances[ppbonus.y * lvlinfo.xsize + ppbonus.x];
      }
      bonusl = bonusl->next;
    }
    return ppbonus;
  }
}

action reconstruction(plvalue *infopl, bonus ppbonus, int posjoueur, int x){
  // On cherche quelle action correspond au sommet où l'on veut aller
  int courant = ppbonus.y * x + ppbonus.x;
  while(infopl->pred[courant].y * x + infopl->pred[courant].x != posjoueur){
    courant = infopl->pred[courant].y * x + infopl->pred[courant].x;
  }
  return infopl->actionpred[courant];
}



action next_move(levelinfo lvl, character joueur, bonus_list bonusl){
  /* On renvoie quelle action faire, et on libère toutes les informations récupérées par le parcours en largeur.
     En effet, celles-ci ne seront plus utiles pour la suite. */
  int x = lvl.xsize;
  int posjoueur = joueur.y * x + joueur.x;
  plvalue *infopl = parcours_largeur(lvl, joueur);
  bonus ppbonus = plus_proche_bonus(bonusl, lvl, infopl->dist);
  action deplacement = reconstruction(infopl, ppbonus, posjoueur, x);
  free(infopl->dist);
  free(infopl->pred);
  free(infopl->actionpred);
  free(infopl);
  return deplacement;
}


bool haut(position *ennemis, character joueur, levelinfo level, int nbennemis){
  /* On vérifie qu'il n'y a pas d'ennemis en haut.
     On renvoit true s'il y en a, false sinon. */
  for(int i = 0; i< nbennemis; i++){
    if(((ennemis[i].x == joueur.x) && (ennemis[i].y == joueur.y - 1 || ennemis[i].y == joueur.y - 2))
        || ((ennemis[i].x == joueur.x - 1) && (ennemis[i].y == joueur.y - 1) 
            && ((level.map[joueur.y][joueur.x - 1] == FLOOR) || (level.map[joueur.y - 2][joueur.x - 1] == CABLE)))
        || ((ennemis[i].x == joueur.x + 1) && (ennemis[i].y == joueur.y - 1) 
            && ((level.map[joueur.y][joueur.x + 1] == FLOOR) || (level.map[joueur.y - 2][joueur.x + 1] == CABLE)))
        || ((ennemis[i].x == joueur.x -2) && (ennemis[i].y == joueur.y - 1) 
            && ((level.map[joueur.y][joueur.x -2] == FLOOR) || (level.map[joueur.y - 2][joueur.x - 2] == CABLE)))
        || ((ennemis[i].x == joueur.x + 2) && (ennemis[i].y == joueur.y - 1) 
            && ((level.map[joueur.y][joueur.x + 2] == FLOOR) || (level.map[joueur.y - 2][joueur.x + 2] == CABLE)))){
      return true;
    }
  }
  return false;
}

bool bas(position *ennemis, character joueur, levelinfo level, int nbennemis){
  /* On vérifie qu'il n'y a pas d'ennemis en bas.
     On renvoit true s'il y en a, false sinon. */
  for(int i = 0; i< nbennemis; i++){
    if(((ennemis[i].x == joueur.x) && (ennemis[i].y == joueur.y + 1))
        || ((ennemis[i].x == joueur.x) && (ennemis[i].y == joueur.y + 2) && (level.map[joueur.y + 2][joueur.x] == LADDER))
        || ((ennemis[i].x == joueur.x - 1) && (ennemis[i].y == joueur.y + 1)) 
        || ((ennemis[i].x == joueur.x + 1) && (ennemis[i].y == joueur.y + 1))){
      return true;
    }
  }
  return false;
}

action cable(position *ennemis, character joueur, action a, int nbennemis){
  /*On cherche s'il y a des ennemis autour de nous sur un câble.
    Si oui, alors on détermine quelle action faire pour leur échapper.*/
  bool gauche = false;
  bool droite = false;
  for(int i = 0; i< nbennemis; i++){
    if(((ennemis[i].x == joueur.x - 1) && (ennemis[i].y == joueur.y))
        || ((ennemis[i].x == joueur.x - 2) && (ennemis[i].y == joueur.y))
        || ((ennemis[i].x == joueur.x - 1) && (ennemis[i].y == joueur.y - 1))
        || ((ennemis[i].x == joueur.x - 1) && (ennemis[i].y == joueur.y + 1))){
      gauche = true; //ennemi à gauche
    }
    if(((ennemis[i].x == joueur.x + 1) && (ennemis[i].y == joueur.y))
        || ((ennemis[i].x == joueur.x + 2) && (ennemis[i].y == joueur.y))
        || ((ennemis[i].x == joueur.x + 1) && (ennemis[i].y == joueur.y - 1))
        || ((ennemis[i].x == joueur.x + 1) && (ennemis[i].y == joueur.y + 1))){
      droite = true; //ennemi à droite
    }
  }
  if(gauche && droite){ //ennemis des 2 côtés, on saute
    a = DOWN;
  }
  else{
    if(gauche){
      a = RIGHT;
    }
    if(droite){
      a = LEFT;
    }
  }
  return a;
}


action echelle(position *ennemis, character joueur, action a,levelinfo level, int nbennemis){
  /*On cherche s'il y a des ennemis autour de nous sur une échelle.
    Si oui, alors on détermine quelle action faire pour leur échapper.*/
  bool dessus = haut(ennemis, joueur, level, nbennemis);
  bool dessous = bas(ennemis, joueur, level,nbennemis);
  if(dessus && dessous){ //ennemis au dessus et en dessous
    a = RIGHT; //par défaut on cherche à sauter à droite
  }
  else{
    if(dessus){ //il y a un ennemi au dessus
      a = DOWN;
    }
    if(dessous){ //il y a un ennemi au dessus
      a = UP;
    }
  }
  return a;
}


bool tomber(levelinfo level, position *ennemis, character joueur, action a, int nbennemis){
  //renvoie true si on peur tomber sans danger, false si on tomberait sur un ennemi
  int posx = joueur.x;
  int posy = joueur.y;
  bool cable = false;
  if(a == RIGHT){
    posx++;
  }
  else{
    posx--;
  }
  while(level.map[posy][posx] != FLOOR){
    /*Tant qu'on n'arrive pas à un étage ou un câble, on descend.
      On prend y - 1 pour le cable afin de vérifier s'il n'y a pas d'ennemi en dessous.*/
    posy++;
    cable = false; //si on était sur un câble, on ne l'est plus
    if(level.map[posy][posx] == CABLE){
      cable = true; //on indique qu'on peut atterir sur un câble
      posy++; //c'est un câble, il ne peut pas y avoir d'ennemi à ce niveau
    }
    for(int i = 0; i<nbennemis; i++){
      //on regarde si un ennemi se trouve dans la zone d'atterissage
      if(ennemis[i].x == posx && ennemis[i].y == posy){
        // il y a un ennemi en dessous
        return false;
      }
      if((level.map[posy + 1][posx] == FLOOR) || (cable && level.map[posy - 1][posx] == CABLE)){ 
        //une fois arrivé au sol, on vérifie une zone plus large     
        if((ennemis[i].x == posx - 1 && ennemis[i].y == posy)
            || (ennemis[i].x == posx + 1 && ennemis[i].y == posy)){
              // il y a un ennemi à côté, il nous aura quand on atterirra
              return false;
        }
      }
    }
    if(cable){
      //on peut atterir sur un câble et il n'y a pas d'ennemi dessus (en réalité sous le câble)
      return true;
    } 
  }
  return true;
}







//première tentative non-concluante
/*
int **creer_mat(int);
void detruit_mat(int **, int);
void update_mat(int **, levelinfo, int, int);
void mat_adj(int **, character_list, levelinfo);
int *init_dist(int *, int);
void parcours_largeur(int **, int, int *, int *, int);
tuple plus_court_chemin(int **, character_list, levelinfo, character, bonus);
int bonus_proche(int tab[7][2], int);
int one_to_all(int **, bonus_list, levelinfo, character, character_list);
action prochain_mouvement(int, character, levelinfo);

typedef struct{
  int dist;
  int chemin;
}tuple;

//matrice d'adjacences du graphe de la carte

int **creer_mat(int taille){
  int **mat = malloc(taille * sizeof(int*));
  for(int i = 0; i < taille; i++){
    mat[i] = malloc(taille * sizeof(int));
    for(int j = 0; j < taille; j++){
      mat[i][j] = 0;
    }  
  }
  return mat;
}



void detruit_mat(int **mat, int taille){
  for(int i=0; i<taille; i++){
    free(mat[i]);
  }
  free(mat);
}


    
void update_mat(int **mat, levelinfo lvlinfo, int posx, int posy){
  int x = lvlinfo.xsize;
  int y = lvlinfo.ysize;
  
  if (posx >= 0 && posx < x && posy >= 0 && posy < y) {
    if (posx > 0){
      mat[posy * x + posx][posy * x + (posx - 1)] = 1;
      //printf("gauche");
    }
    if (posx < (x - 1)){
      mat[posy * x + posx][posy * x + (posx + 1)] = 1;
      //printf("droite");
    }
    if (posy > 0){
      mat[posy * x + posx][(posy - 1) * x + posx] = 1;
      //printf("haut");
    }
    if (posy < (y - 1)){
      mat[posy * x + posx][(posy + 1) * x + posx] = 1;
      //printf("bas");
    }
  }
  else {
    printf("erreur: indices invalides %d, %d\n", posx, posy);
  }
}



void mat_adj(int **mat, character_list characterl, levelinfo lvlinfo){
  int x = lvlinfo.xsize;
  int y = lvlinfo.ysize;
  for(int i = 0; i < y; i++){
    for(int j = 0; j < x; j++){
      if(lvlinfo.map[i][j] == CABLE || lvlinfo.map[i][j] == FLOOR || lvlinfo.map[i][j] == LADDER){
        update_mat(mat, lvlinfo, j, i);    
      }
    }
  }
}



//calcul du plus court chemin entre le joueur et un bonus avec la matrice d'ajacences

int *init_dist(int *dist, int taille){
  // initialise les distances à -1 (= pas encore vu)
  for(int i = 0; i < taille; i++){
    dist[i] = -1;
  }
  return dist;
}



void parcours_largeur(int **mat, int taille, int *distances, int* predecesseurs, int pos_joueur){
  bool noeuds_vus[taille];
  int file[taille];
  int debut = 0;
  int fin = 0;

  for(int i = 0; i<taille; i++){
    noeuds_vus[i] = false;
    predecesseurs[i] = -1;
  }
  noeuds_vus[pos_joueur] = true;
  distances[pos_joueur] = 0;
  file[fin++] = pos_joueur;
  
  while(debut < fin){
    int courant = file[debut++];
    for(int i=0; i<taille; i++){
      if (mat[courant][i] == 1){printf("OK\n");}
      if (mat[courant][i] == 1 && !noeuds_vus[i]){
        printf("if parcours largeur");
        noeuds_vus[i] = true;
        distances[i] = distances[courant] + 1;
        predecesseurs[i] = courant;
        file[fin++] = i;
      }
    }
  }
}



tuple plus_court_chemin(int ** mat, character_list characterl, levelinfo lvlinfo, character joueur, bonus pos_bonus){
  //calcule le plus court chemin vers UN bonus et renvoie la première case de ce chemin, ainsi que la distance au bonus
  int taille = lvlinfo.xsize * lvlinfo.ysize;
  int *distances = malloc(taille * sizeof(int));
  int *predecesseurs = malloc(taille * sizeof(int));
  int pos_joueur = joueur.y * lvlinfo.xsize + joueur.x;
  tuple res;

  init_dist(distances, taille);
  parcours_largeur(mat, taille, distances, predecesseurs, pos_joueur);

  int pos_bonus_index = pos_bonus.y * lvlinfo.xsize + pos_bonus.x;
  res.dist = distances[pos_bonus_index];

  if (res.dist == -1) {
    // Le bonus n'est pas atteignable
    res.chemin = -1;
    res.dist = taille; //plus facile à gérer pour trouver la distance minimale
  } 
  else {
    // Trouver la première position après pos_joueur
    int actuel = pos_bonus_index;
    bool trouve = false;
    while (actuel != -1 && !trouve) {
      // Cherche le prédécesseur du noeud actuel qui est adjacent à pos_joueur
      if (predecesseurs[actuel] == pos_joueur) {
        res.chemin = actuel; // La première position après pos_joueur
        trouve = true;
      }
      actuel = predecesseurs[actuel];
    }
    // Si aucune position trouvée
    if (!trouve) {
      res.chemin = -1;
    }
  }

  free(distances);
  free(predecesseurs);

  return res;
}



int bonus_proche(int tab[7][2], int n){
  //renvoie le numéro de la première case du chemin menant au bonus le plus proche
  int min = tab[0][0];
  int indice = 0;
  for(int i = 0; i < n; i++){
    printf("distance : %d, chemin : %d\n", tab[i][0], tab[i][0]);
    if(tab[i][0] < min){
      min = tab[i][0];
      indice = i;
    }
  }
  return tab[indice][1];
}



int one_to_all(int ** mat, bonus_list lstbonus, levelinfo lvlinfo, character joueur, character_list characterl){
  //cherche le plus court chemin POUR CHAQUE bonus
  bonus_list bonus_actu = lstbonus;
  bonus pos_bonus;
  tuple res;
  int tab[7][2];
  int cpt = 0;
  
  while(bonus_actu != NULL && cpt < 7){
    pos_bonus = bonus_actu->b;
    res = plus_court_chemin(mat, characterl, lvlinfo, joueur,pos_bonus);
    tab[cpt][0] = res.dist;
    tab[cpt][1] = res.chemin;
    bonus_actu = bonus_actu->next;
    cpt++;
  }
  return bonus_proche(tab, cpt);
}



action prochain_mouvement(int pos, character joueur, levelinfo lvlinfo){
  if(pos == (joueur.y + 1) * lvlinfo.xsize + joueur.x - 1){
    return LEFT;
  }
  else{
    if(pos == (joueur.y + 1) * lvlinfo.xsize + joueur.x + 1){
      return RIGHT;
    }
    else{
      if(pos == joueur.y * lvlinfo.xsize + joueur.x){
        return UP;
      }
      else{
        if(pos == (joueur.y + 1) * lvlinfo.xsize + joueur.x){
          return DOWN;
        }
        else{
          return NONE;
        }
      }
    }
  }
}
*/
