// Kompajliranje:
// g++ -o prozor prozor.cpp util.cpp -lGLEW -lGL -lGLU -lglut -lpthread

#ifdef _WIN32
#include <windows.h> //bit ce ukljuceno ako se koriste windows
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

#include <GL/glew.h>
#include <GL/freeglut.h>

// Ukljuci potporu za osnovne tipove glm-a: vektore i matrice
#include <glm/glm.hpp>

// Ukljuci funkcije za projekcije, transformacije pogleda i slicno
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

// Ako ste na Ubuntu-u i program se odmah po pokretanju srusi sa segmentation
// fault, radi se o poznatom bugu:
// https://bugs.launchpad.net/ubuntu/+source/nvidia-graphics-drivers-319/+bug/1248642
// potreban je sljedeci define i program treba linkati s -lpthread:
#ifndef _WIN32
#define LINUX_UBUNTU_SEGFAULT
#endif

#ifdef LINUX_UBUNTU_SEGFAULT
// ss1
#include <pthread.h>
#endif

// Nasa pomocna biblioteka za ucitavanje, prevodenje i linkanje programa shadera
#include "util.hpp"
#include "loader.h"

//*********************************************************************************
//	Pokazivac na glavni prozor i pocetna velicina.
//*********************************************************************************

GLuint window;
GLuint sub_width = 800, sub_height = 600;

//*********************************************************************************
//	Function Prototypes.
//*********************************************************************************

void myDisplay();
void myReshape(int width, int height);
void myMouse(int button, int state, int x, int y);
void myKeyboard(unsigned char theKey, int mouseX, int mouseY);

GLuint vertexArrayID;
GLuint programID;
GLuint MVPMatrixID;

GLuint vertexBuffer;
GLuint vertexLineBuffer;

bool init_data(); // nasa funkcija za inicijalizaciju podataka

//*********************************************************************************
//	Glavni program.
//*********************************************************************************

long long startingtime;
Obj *obj;
CubicSpl *spl;
long long segmentperiod = 1000;

int main(int argc, char **argv) {
    obj = new Obj(ifstream("../obj/tetrahedron.obj"));
    spl = new CubicSpl(ifstream("../obj/spiral.spline"));
// Sljedeci blok sluzi kao bugfix koji je opisan gore
#ifdef LINUX_UBUNTU_SEGFAULT
    // ss2
    int i = pthread_getconcurrency();
#endif

    glutInitContextVersion(3, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitContextFlags(GLUT_DEBUG);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(sub_width, sub_height);
    glutInitWindowPosition(100, 100);
    glutInit(&argc, argv);

    window = glutCreateWindow("Glut OpenGL Prozor");
    glutDisplayFunc(myDisplay);
    glutIdleFunc(myDisplay);

    glewExperimental = GL_TRUE;
    glewInit();
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    if (!init_data())
        return 1;

    // Omogući uporabu Z-spremnika
    glEnable(GL_DEPTH_TEST);
    // Prihvaćaj one fragmente koji su bliže kameri u smjeru gledanja
    glDepthFunc(GL_LESS);

    startingtime = timer();
    glutMainLoop();
    return 0;
}

//*********************************************************************************
//	Funkcija u kojoj se radi inicijalizacija potrebnih VAO-a i VBO-ova.
//*********************************************************************************
vector<float> *points;

bool init_data() {
    // PRVI KONTEKST

    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    glGenBuffers(1, &vertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 10000, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, obj->vertices.size() * sizeof(float),
                    &obj->vertices[0]);

    // MISC

    std::cout << "Going to load programs... " << std::endl << std::flush;

    programID = loadShaders("SimpleVertexShader.vertexshader",
                            "SimpleFragmentShader.fragmentshader");
    if (programID == 0) {
        std::cout << "Zbog grešaka napuštam izvođenje programa." << std::endl;
        return false;
    }

    // Get a handle for our "MVP" uniform for later when drawing...
    MVPMatrixID = glGetUniformLocation(programID, "MVP");
    glUseProgram(programID);
    glPointSize(2);

    return true;
}

//*********************************************************************************
//	Osvjezavanje prikaza. (nakon preklapanja prozora)
//*********************************************************************************
int counter;
vector<float> firsttang;
bool done;

void myDisplay() {
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection =
        glm::perspective(glm::radians(45.0f),
                         (float)sub_width / (float)sub_height, 0.1f, 100.0f);

    glm::mat4 view = glm::lookAt(
        glm::vec3(30, 30, 65), // Camera is at (4,3,3), in World Space
        glm::vec3(0, 0, 30),   // and looks at the origin
        glm::vec3(0, 0, 1)     // Head is up (set to 0,-1,0 to look upside-down)
    );

    auto pos = spl->eval(timer() - startingtime, segmentperiod);

    auto diff1 = spl->eval_first(timer() - startingtime, segmentperiod);
    if (!done) {
        firsttang = diff1;
        done = true;
    }
    auto diff2 = spl->eval_second(timer() - startingtime, segmentperiod);
    auto cross = crossprod(diff1, diff2);

    auto crossrot = crossprod(firsttang, diff1);
    float angle =
        acos(dot3f(firsttang, diff1) / (norm3f(firsttang) * norm3f(diff1)));

    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, glm::vec3(pos[0], pos[1], pos[2]));
    model = glm::scale(model, glm::vec3(2, 2, 2));
    model = glm::rotate(model, angle,
                        glm::vec3{crossrot[0], crossrot[1], crossrot[2]});
    glm::mat4 mvp = projection * view * model;
    glm::mat4 vp = projection * view;

    vector<float> total;
    total.insert(total.end(), pos.begin(), pos.end());
    total.insert(total.end(), diff1.begin(), diff1.end());
    total.insert(total.end(), pos.begin(), pos.end());
    total.insert(total.end(), diff2.begin(), diff2.end());
    total.insert(total.end(), pos.begin(), pos.end());
    total.insert(total.end(), cross.begin(), cross.end());
    int pis = 6;

    glBufferSubData(GL_ARRAY_BUFFER, obj->vertices.size() * sizeof(float) * 3,
                    pis * 3 * sizeof(float), &(total[0]));

    if (counter < 1000) {
        glBufferSubData(GL_ARRAY_BUFFER,
                        (obj->vertices.size() + pis + counter) * sizeof(float) *
                            3,
                        pos.size() * sizeof(float), &(pos[0]));
        counter++;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glUniformMatrix4fv(MVPMatrixID, 1, GL_FALSE, &mvp[0][0]);
    glDrawElements(GL_TRIANGLES, obj->faces.size(), GL_UNSIGNED_INT,
                   &(obj->faces[0]));
    glUniformMatrix4fv(MVPMatrixID, 1, GL_FALSE, &vp[0][0]);
    glDrawArrays(GL_LINES, obj->vertices.size(), pis);
    glDrawArrays(GL_POINTS, obj->vertices.size() + pis, counter);

    glutSwapBuffers();
}
