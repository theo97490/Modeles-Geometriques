// erosion

/**
* Probleme : creuser un trou sur la surf de a
* Arbre très profond d'opérations de A privé de X
* => cout cher 
*
* Objectif : faire tout d'un cout, calculer des inetersection avec la surf
* puis fait l'union de toutes les spheres crée pour creuser
* Puis diff avec la surf 
*
*
* iteratif en n² 
* void spray (){
*   for i = 0..n
*   {
*     p = sphere tracing n, droite 
*     node = diff ( node, sphere crée )
*   }
* }
*
* Ou faire par série : calculer une vague dimpact sur un meme objet
*/
