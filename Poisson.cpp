 #include <cstdio>
#include <cstdlib>
#include <GL/freeglut.h>
#include <jpeglib.h>
#include <jerror.h>
#include <math.h>

// declaration des variables
float angleQueue = 0; // pour l'animation de la queue
float angleNageoirePectorale = 0.0;
bool queueGauche = true; // direction de la queue
float r=0.6, g=0.4, b=0.2;
int zoom = 4, anglex = 0, angley = 0, anglez = 0, xold, yold, presse;
int largeurCorps = 740, hauteurCorps = 740;   // dimensions de la texture1(corps..)
int largeurNageoire = 612, hauteurNageoire = 612; // dimensions de la texture2 (nageoire dorsale)
unsigned char imageCorps[740 * 740 * 3];
unsigned char imageNageoire[612 * 612 * 3];
GLuint textureIDCorps, textureIDNageoire;    // identifiants des textures

GLuint textureID;
float positionX = 0.0;        // position du poisson sur l'axe X
float positionZ = 0.0;        // position du poisson sur l'axe Y (oscillation sinusoïdale)
float angleRotation = 0.0;    // angle de rotation pour le retournement
int direction = 1;            // direction actuelle de poisson(1 = droite, -1 = gauche)
float vitessePoisson = 0.02;  // Vitesse de déplacement
bool enRetournement = false;
bool mouvementActif = false; // pour savoir si  le poisson est en mouvement
bool lumiereHautActive = true; // lumière du haut
bool lumiereGaucheDroiteActive = true; // lumières latérales



//declarations des fonctions principales
void affichage(), clavier(unsigned char touche, int x, int y), tournercamera(int touche, int x, int y);
void souris(int bouton, int etat, int x, int y), sourismouv(int x, int y), redim(int l, int h);
void loadJpegImage(char *fichier, unsigned char *image), initLumiere(), AnimationPoisson(), Poisson3D();
void CorpsPoisson(double rX, double rY, double rZ), QueuePoisson(double largeur, double hauteur);
void NageoireDorsale(double longueur, double hauteur), YeuxPoisson();
void NageoireAnale(double longueur, double hauteur);
void NageoiresPectorales(double longueur, double largeur, bool gauche);
void AnimationNageoiresPectorales();
void AnimationGlobal();
void initTextures();




void Poisson3D() {
    glPushMatrix();



    //corps
    // Translation et rotation pour deplacement du poisson
    glTranslatef(positionX, 0.0, positionZ);
    glRotatef(angleRotation, 0.0, 1.0, 0.0);
    glBindTexture(GL_TEXTURE_2D, textureIDCorps);
    glColor3f(1.0, 1.0, 1.0); // Couleur blanche pour la texture
    CorpsPoisson(1.0, 0.5, 0.3);

    //queue
    glPushMatrix();
    glTranslatef(-1.1, 0.0, 0.0);
    glRotatef(angleQueue, 0, 1, 0);
    QueuePoisson(0.5, 0.3);
    glPopMatrix();


    //nageoire dos
    glPushMatrix();
    glTranslatef(0.0, 0.5, 0.0);
    NageoireDorsale(0.4, 0.3);
    glPopMatrix();

    // Nageoire anale
    glPushMatrix();
    glTranslatef(-0.3, -0.4, 0.0); // Positionner la nageoire anale
    NageoireAnale(0.3, 0.2);       // Taille de la nageoire anale
    glPopMatrix();

    // Nageoire droite
    glPushMatrix();
    glTranslatef(0.5, 0.0, 0.25);
    glRotatef(-130,0,1,0); //pour la tourner horizontalement
    glRotatef(-60,0,0,1); //pour les descendre
    glRotatef(-angleNageoirePectorale, 1.0, 0.0, 0.0); // Inclinaison dynamique
    NageoiresPectorales(0.3, 0.2,false);
    glPopMatrix();

    // Naugeoire gauche
    glPushMatrix();
    glTranslatef(0.5, 0.0, -0.25);
    glRotatef(130,0,1,0);  //pareil
    glRotatef(-60,0,0,1);
    glRotatef(angleNageoirePectorale, 1.0, 0.0, 0.0); // Inclinaison dynamique
    NageoiresPectorales(0.3, 0.2,true);
    glPopMatrix();

    YeuxPoisson();

    glPopMatrix();
}



// methodes pour modeliser les differentes partie du poisson

void CorpsPoisson(double rX, double rY, double rZ) {
    int numU = 30; // nombre de segments autour de l'axe horizontal
    int numV = 20; // nombre de segments autour de l'axe vertical
    double pi = M_PI;
// on utilise une premitive pour faire le corps du poisson
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, textureIDCorps); // appliquer la texture du corps
    glBegin(GL_QUADS);
    for (int i = 0; i < numU; i++) {
        double u1 = i * 2.0 * pi / numU;
        double u2 = (i + 1) * 2.0 * pi / numU;
        for (int j = 0; j < numV; j++) {
            double v1 = j * pi / numV;
            double v2 = (j + 1) * pi / numV;

            glTexCoord2f((float)i / numU, (float)j / numV);
            glVertex3f(rX * cos(u1) * sin(v1), rY * sin(u1) * sin(v1), rZ * cos(v1));

            glTexCoord2f((float)i / numU, (float)(j + 1) / numV);
            glVertex3f(rX * cos(u1) * sin(v2), rY * sin(u1) * sin(v2), rZ * cos(v2));

            glTexCoord2f((float)(i + 1) / numU, (float)(j + 1) / numV);
            glVertex3f(rX * cos(u2) * sin(v2), rY * sin(u2) * sin(v2), rZ * cos(v2));

            glTexCoord2f((float)(i + 1) / numU, (float)j / numV);
            glVertex3f(rX * cos(u2) * sin(v1), rY * sin(u2) * sin(v1), rZ * cos(v1));
        }
    }
    glEnd();
    glPopMatrix();
}

// methode pour faire la queue du poisson

void QueuePoisson(double longueur, double hauteur) {
    int numU = 20; // résolution le long de u
    int numV = 10; // résolution le long de v

    glPushMatrix();
    glColor3f(r, g, b); // couleur pour la queue
    glTranslatef(-0.35, 0.0, 0.0);
    glRotatef(180.0, 0.0, 1.0, 0.0);
// primitives pour faire la queue
    glBegin(GL_QUADS);
    for (int i = 0; i < numU; i++) {
        double u1 = (double)i / (double)numU;         // valeur de u pour le premier point
        double u2 = (double)(i + 1) / (double)numU;   // valeur de u pour le second point
        for (int j = 0; j < numV; j++) {
            double v1 = (double)j / (double)numV * 2.0 - 1.0;   // valeur de v pour le premier point
            double v2 = (double)(j + 1) / (double)numV * 2.0 - 1.0; // valeur de v pour le second point

            // calcul des points
            double x1 = -u1 * longueur, y1 = v1 * hauteur * (1 - u1 * u1), z1 = 0;
            double x2 = -u1 * longueur, y2 = v2 * hauteur * (1 - u1 * u1), z2 = 0;
            double x3 = -u2 * longueur, y3 = v2 * hauteur * (1 - u2 * u2), z3 = 0;
            double x4 = -u2 * longueur, y4 = v1 * hauteur * (1 - u2 * u2), z4 = 0;

            // dessiner le quadrilatère
            glVertex3f(x1, y1, z1);
            glVertex3f(x2, y2, z2);
            glVertex3f(x3, y3, z3);
            glVertex3f(x4, y4, z4);
        }
    }
    glEnd();

    glPopMatrix();
}
// modeliser les Nageoire

// nageoire dorsale
void NageoireDorsale(double longueur, double hauteur) {
    int segments = 10;
    //pas de couleur car texture
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, textureIDNageoire); // appliquer la texture de la nageoire
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= segments; i++) {
        double ratio = (double)i / (double)segments;
        double x = longueur * (ratio - 0.5);
        double y = hauteur * (1.0 - ratio * ratio);
        glTexCoord2f(ratio, 0.0); glVertex3f(x, 0.0, 0.0);
        glTexCoord2f(ratio, 1.0); glVertex3f(x, y, 0.0);
    }
    glEnd();
    glPopMatrix();
}

// nageoire anale

void NageoireAnale(double longueur, double hauteur) {
    int segments = 10; // on utilise plus de segments pour avoir une courbe plus lisse
    glPushMatrix();

    glColor3f(r, g, b); // couleur pour la nageoire

    // Ppsitionnement de la nageoire anale sous le poisson
    glTranslatef(-0.28, 0.0, 0.0); // ajuster cette position selon votre poisson
    glRotated(-20,0,0,1);

    // dessin de la nageoire avec des triangles
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= segments; i++) {
        double ratio = (double)i / (double)segments; // avancement dans la courbe
        double x = longueur * (ratio - 0.5);         // position horizontale
        double y = hauteur * (1.0 - ratio * ratio); // courbure vers le bas

        glVertex3f(x, 0.0, 0.0); // point à la base
        glVertex3f(x, -y, 0.0);  // point au sommet
    }
    glEnd();

    glPopMatrix();
}

// nageoire pectorale
void NageoiresPectorales(double longueur, double largeur, bool gauche) {
    glPushMatrix();

    glColor3f(r, g, b);

    // dessiner la nageoire en utilisant des triangles
    glBegin(GL_TRIANGLES);
    glVertex3f(0.0, 0.0, 0.0);                      // point à la base
    glVertex3f(longueur, largeur / 2.0, 0.0);       // point à l'extrémité supérieure
    glVertex3f(longueur, -largeur / 2.0, 0.0);      // point à l'extrémité inférieure
    glEnd();

    glPopMatrix();
}

// fonction pour faire les yeux du poisson
void YeuxPoisson() {
    //oeil gauche
    glPushMatrix();
    glColor3f(1.0, 1.0, 1.0);
    glTranslatef(0.8, 0.22, -0.22);
    glutSolidSphere(0.1, 20, 20);
    glPopMatrix();

    //pupille gauche
    glPushMatrix();
    glColor3f(0.0, 0.0, 0.0);
    glTranslatef(0.87, 0.22, -0.22);
    glutSolidSphere(0.05, 20, 20);
    glPopMatrix();

    //oeil droit
    glPushMatrix();
    glColor3f(1.0, 1.0, 1.0);
    glTranslatef(0.8, 0.22, 0.22);
    glutSolidSphere(0.1, 20, 20);
    glPopMatrix();

    //pupille droite
    glPushMatrix();
    glColor3f(0.0, 0.0, 0.0);
    glTranslatef(0.87, 0.22, 0.22);
    glutSolidSphere(0.05, 20, 20);
    glPopMatrix();
}


// methode pour l'affichage

void affichage() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-zoom, zoom, -zoom, zoom, -zoom, zoom);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(angley, 1.0, 0.0, 0.0);
    glRotatef(anglex, 0.0, 1.0, 0.0);
    glRotatef(anglez, 0.0, 0.0, 1.0);
    Poisson3D();
    glutSwapBuffers();
}
// fonctionnalites du clavier
void clavier(unsigned char touche, int x, int y) {
    switch (touche) {
        case 'p': glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); break;
        case 'f': glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); break;
        case 's': glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); break;
        case 'z': zoom++; break;
        case 'Z': zoom--; break;
        case 'b': mouvementActif = true; break; //pour faire bouger le poisoon
        case 'n': mouvementActif = false; break; //arreter le poisson
        case 'h': //Activer/desactiver la lumière du haut
            lumiereHautActive = !lumiereHautActive;
            // activation/desactivation des lumiere
            if (lumiereHautActive) glEnable(GL_LIGHT0);
            else glDisable(GL_LIGHT0);
            break;
        case 'g':
            lumiereGaucheDroiteActive = !lumiereGaucheDroiteActive;
            if (lumiereGaucheDroiteActive) {
                glEnable(GL_LIGHT1); //activer lumière gauche
                glEnable(GL_LIGHT2); //activer lumière droite
            } else {
                glDisable(GL_LIGHT1); //desactiver lumière gauche
                glDisable(GL_LIGHT2); //desactiver lumière droite
            }
            break;
        case 'q': exit(0);
    }
    glutPostRedisplay();
}

//gestion de la camera
void tournercamera(int touche, int x, int y) {
    switch (touche) {
        case GLUT_KEY_UP: angley += 5; break;
        case GLUT_KEY_DOWN: angley -= 5; break;
        case GLUT_KEY_LEFT: anglex -= 5; break;
        case GLUT_KEY_RIGHT: anglex += 5; break;
    }
    glutPostRedisplay();
}

// fnction souris pour pouvoir bouger le poisoon avec la souris
void souris(int bouton, int etat, int x, int y) {
    if (bouton == GLUT_LEFT_BUTTON && etat == GLUT_DOWN) { presse = 1; xold = x; yold = y; }
    else if (bouton == GLUT_LEFT_BUTTON && etat == GLUT_UP) presse = 0;
}

void sourismouv(int x, int y) {
    if (presse) { anglex += (x - xold); angley += (y - yold); glutPostRedisplay(); }
    xold = x; yold = y;
}

void redim(int l, int h) {
    if (l < h) glViewport(0, (h - l) / 2, l, l);
    else glViewport((l - h) / 2, 0, h, h);
}

void loadJpegImage(char *fichier, unsigned char *image, int largeur, int hauteur) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *file;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    if ((file = fopen(fichier, "rb")) == 0) {
        fprintf(stderr, "Erreur : impossible d'ouvrir le fichier %s\n", fichier);
        exit(1);
    }
    jpeg_stdio_src(&cinfo, file);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);
    unsigned char *ligne = image;
    while (cinfo.output_scanline < cinfo.output_height) {
        ligne = image + 3 * largeur * cinfo.output_scanline;
        jpeg_read_scanlines(&cinfo, &ligne, 1);
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(file);
}

// initialisations des lumieres
void initLumiere() {
    glEnable(GL_LIGHTING);          // activer l'éclairage global
    glEnable(GL_LIGHT0);            // lumière du haut activée par défaut
    glEnable(GL_LIGHT1);            // lumière gauche activée par défaut
    glEnable(GL_LIGHT2);            // lumière droite activée par défaut
    glEnable(GL_COLOR_MATERIAL);

    // llumière ambiante globale
    GLfloat global_ambient[] = {0.1, 0.1, 0.1, 1.0};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

    // lumière du haut
    GLfloat light0_diffuse[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat light0_specular[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat light0_position[] = {0.0, 5.0, 0.75, 1.0};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

    // lumière gauche
    GLfloat light1_diffuse[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat light1_specular[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat light1_position[] = {0.0, 0.0, -5.0, 1.0};
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1_specular);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);

    // lumière droite
    GLfloat light2_diffuse[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat light2_specular[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat light2_position[] = {0.0, 0.0, 5.0, 1.0};
    glLightfv(GL_LIGHT2, GL_DIFFUSE, light2_diffuse);
    glLightfv(GL_LIGHT2, GL_SPECULAR, light2_specular);
    glLightfv(GL_LIGHT2, GL_POSITION, light2_position);
}

 // initialisations des textures
void initTextures() {
    // Charger la texture pour le corps
    loadJpegImage("./texture1.jpg", imageCorps, largeurCorps, hauteurCorps);
    glGenTextures(1, &textureIDCorps);
    glBindTexture(GL_TEXTURE_2D, textureIDCorps);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, largeurCorps, hauteurCorps, 0, GL_RGB, GL_UNSIGNED_BYTE, imageCorps);

    // Charger la texture pour la nageoire dorsale
    loadJpegImage("./texture2.jpg", imageNageoire, largeurNageoire, hauteurNageoire);
    glGenTextures(1, &textureIDNageoire);
    glBindTexture(GL_TEXTURE_2D, textureIDNageoire);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, largeurNageoire, hauteurNageoire, 0, GL_RGB, GL_UNSIGNED_BYTE, imageNageoire);
}

// animations de poisson pour les movements de ce dernier
void AnimationPoisson() {
    if (queueGauche) { angleQueue += 2.0; if (angleQueue > 30) queueGauche = false; }
    else { angleQueue -= 2.0; if (angleQueue < -30) queueGauche = true; }
    glutPostRedisplay();
}

void AnimationNageoiresPectorales() {
    static bool nageoireVersHaut = true;
    if (nageoireVersHaut) {
        angleNageoirePectorale += 2.0;
        if (angleNageoirePectorale > 15.0) nageoireVersHaut = false; // changer de  direction quand il arriver a une bordures
    } else {
        angleNageoirePectorale -= 2.0;
        if (angleNageoirePectorale < -15.0) nageoireVersHaut = true;
    }
    glutPostRedisplay(); // redessine la scène
}

void AnimationPoissonComplet() {
    if (!mouvementActif) return; //si le mouvement est inactif, ne rien faire


    static float time = 0.0;

    //si le poisson est en train de se retourner
    if (enRetournement) {
        //animer le retournement
        if (direction == 1 && angleRotation > 0.0) {
            angleRotation -= 2.0; //réduction progressive de l'angle
        } else if (direction == -1 && angleRotation < 180.0) {
            angleRotation += 2.0; //augmentation progressive de l'angle
        } else {
            enRetournement = false; //le poisson c'est retourner
        }
    } else {
        //déplacement linéaire sur l'axe X
        positionX += direction * vitessePoisson;

        //limites de la scène : déclencher un retournement
        if (positionX > 2.8 || positionX < -2.8) {
            enRetournement = true; // le poisson se retourne
            direction *= -1;       // change la direction
        }

        //mouvement sinusoïdal sur l'axe Y pour simuler une nage ondulée
        positionZ = 0.3 * sin(time);


        time += 0.1;
    }

    //redessiner la scène
    glutPostRedisplay();
}


void AnimationGlobal() {
    AnimationPoisson();            //anime la queue
    AnimationNageoiresPectorales(); //anime les nageoires
    AnimationPoissonComplet(); //anime le poisson complet
}


int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Poisson 3D");

    //on doit charger les textures en faisant appel a initTextures()
    initTextures();

    //initialisation OpenGL
    glClearColor(0.1, 0.1, 0.2, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    initLumiere();


    glutDisplayFunc(affichage);
    glutKeyboardFunc(clavier);
    glutSpecialFunc(tournercamera);
    glutMouseFunc(souris);
    glutMotionFunc(sourismouv);
    glutReshapeFunc(redim);
    glutIdleFunc(AnimationGlobal);

    glutMainLoop();
    return 0;
}
