#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <cmath>
#include "Protobyte/Toroid.h"
#include "Protobyte/Matrix4.h"
#include <iomanip>
#include "Protobyte/Spline3.h"
#include "Protobyte/Tube.h"
#include <ctime> 
#include "Protobyte/Math.h"
#include "Protobyte/Shader.h"




/**********************************
 *           matrices             *
 *********************************/
GLfloat modelViewMatrix[16];
GLfloat projectionMatrix[16];
void printMatrices();

/**********************************
 *             VIEW               *
 *********************************/
void setView(double fovY, double aspect, double zNear, double zFar);

/**********************************
 *             shaders               *
 *********************************/
Shader shader;
void setShaders();


/**********************************
 *           LIGHTING             *
 *********************************/
// Light01
GLfloat light01_ambient[] = {0.0, 0.0, 0.0, 1.0};
GLfloat light01_diffuse[] = {.7, .2, .2, 1.0};
GLfloat light01_specular[] = {1.0, 1.0, 1.0, 1.0};
GLfloat light01_position[] = {1.0, 10.0, 1.0, 0.0};

//materials
GLfloat light01_mat_specular[] = {1.0, 1.0, 1.0, 1.0};
GLfloat light01_mat_shininess[] = {27}; // max 128

void setLights();
void initGL();

int main() {
    // create the window
    sf::Window window(sf::VideoMode(800, 600), "OpenGL", sf::Style::Default, sf::ContextSettings(32));
    window.setVerticalSyncEnabled(true);

    initGL();

    setLights();

    setShaders();

    // about GL internal matrices
    // GL_MODELVIEW matrix is for position of camera - inverse of object transformations
    // GL_PROJECTIOIN matrix may be thought of as lens component of camera - 3d to 2d mapping


    // make projection matrix active
    glMatrixMode(GL_PROJECTION);
    // set projection matrix to identity
    glLoadIdentity();
    // update projection matrix with perspective values
    setView(75.0, window.getSize().x / (float) window.getSize().y, .1, 800);



    // make Modelview matrix active
    glMatrixMode(GL_MODELVIEW); // reload modelview matrix
    //set ModelView Matrix to identity
    glLoadIdentity();
    //glTranslatef(20, 20, 220);

    // enable random vals
    srand(time(0)); // should only be called once (called 1 x per second)

    // print out current state of MODELVIEW and PROJECTION matrices
    printMatrices();

    Toroid toroid(Vector3(0, 0, -60), Vector3(100, 180, 0),
            Dimension3<float>(30, 30, 30), Color4<float>(0.8, 0.2, 0.1, .75), 30, 30, .87, .22);

    Toroid toroid2(Vector3(0, 0, -350), Vector3(100, 125, -240),
            Dimension3<float>(8, 8, 8), Color4<float>(0.5, 0.5, 0.7, .3), 56, 56, .3);

    Toroid toroid3(Vector3(0, 0, -100), Vector3(270, 125, -240),
            Dimension3<float>(10, 10, 10), Color4<float>(0.7, 0.5, 0.7, 1.0), 24, 24, .8);

    // test spline curve

    std::vector<Vector3> cps;
    int controlPts = 30;
    float x, y, z = 5;
    float t = 0;
    for (int i = 0; i < controlPts; i++) {
        //rand() % 100;  // v1 in the range 0 to 99
        x = -1 + rand() % 3; // (-5 to 5)
        y = -1 + rand() % 3; // (-5 to 5)
        z = -2 + rand() % 5; // (-5 to 5)
        //std::cout << "x = " << x << std::endl;
        //std::cout << "y = " << y << std::endl;
        //std::cout << "( " << x << ", " << y << ", " << z << " )" << std::endl;
        //x = cos(t)*2;
        //y = sin(t)*2;
        //z=0;
        cps.push_back(Vector3(x, y, z));
        t += M_PI * 2 / (controlPts);
    }

    // for testing
    /*cps.push_back(Vector3(-4.0, 0, -2.2));
    cps.push_back(Vector3(-2.0, 0, -2.2));
    cps.push_back(Vector3(0, 0, -2.2));
    cps.push_back(Vector3(2.0, 0, -2.2));
    cps.push_back(Vector3(4.0, 0, -2.2));*/
    int interpDetail = 3;
    float smoothness = .8;
    Spline3 spline(cps, interpDetail, false, smoothness);

    int totalSegCount = (controlPts - 1) * interpDetail + controlPts;
    std::vector<float> radii;
    radii.resize(totalSegCount);
    std::vector< Color4<float> > cols;
    cols.resize(totalSegCount);


    for (int i = 0; i < totalSegCount; i++) {
        // mult radii
        static double theta2 = 0.0;
        radii.at(i) = .15 + sin(theta2)*.09;
        //std::cout << " radii.at(i) = " << radii.at(i) << std::endl;
        theta2 += M_PI / 5.0;

        // mult colors
        static float r = 0.0, g = 0.0, b = 0.0, a = 1.0;
        static float nudger = 1.0 / totalSegCount;
        cols.at(i) = Color4<float>(r += nudger, g += nudger, b += nudger, a);
    }

    //Tube tube(Vector3(0, 0, -200), Vector3(0, 0, 0), Dimension3<float>(40, 40, 40), Color4<float>(0.7, 0.2, 0.3, 1.0), spline, radii, 24);
    //Tube tube(Vector3(0, 0, -200), Vector3(0, 0, 0), Dimension3<float>(40, 40, 40), cols, spline, radii, 24);

    // tube around toroid
    interpDetail = 2;
    smoothness = .55;
    std::vector<Vector3> cps2;
    int segs = 400/*400*/;


    //int loops = 4;
    cps2.resize(segs);
    float xx, yy, zz = .1;

    float tempX, tempZ;
    for (int i = 0; i < segs; i++) {
        static float theta = 0.0, phi = 0.0;
        xx = .8 - .02 + cos(theta)*.33;
        yy = sin(theta)*.31;
        theta += M_PI / 1.95;


        tempZ = cos(phi) * zz - sin(phi) * xx;
        tempX = sin(phi) * zz + cos(phi) * xx;
        cps2.at(i) = Vector3(tempX, yy, tempZ);

        //phi += (-2+rand() % 5)*M_PI/360;
        phi += M_PI / proto::Math::random(2.0, 50.0);
        // zz += .2 / segs;

    }

    totalSegCount = (segs - 1) * interpDetail + segs;
    radii.clear();
    radii.resize(totalSegCount);
    cols.clear();
    cols.resize(totalSegCount);

    for (int i = 0; i < totalSegCount; i++) {
        // mult radii
        static double theta2 = 0.0;
        radii.at(i) = .02 + sin(theta2)*.005;
        //std::cout << " radii.at(i) = " << radii.at(i) << std::endl;
        theta2 += M_PI / 5.0;

        // mult colors
        static float r = 0.0, g = 0.0, b = 0.0, a = 1.0;
        static float nudger = 1.0 / totalSegCount;
        cols.at(i) = Color4<float>(r += nudger, g += nudger, b += nudger, a);
    }
    /*Toroid toroid(Vector3(0, 0, -60), Vector3(100, 180, 0),
           Dimension3<float>(30, 30, 30), Color4<float>(0.7, 0.2, 0.1, .85), 56, 56, .8, .2);*/

    Spline3 spline2(cps2, interpDetail, false, smoothness);
    Tube tube2(Vector3(0, 0, -60), Vector3(100, 180, 0), Dimension3<float>(30, 30, 30), cols, spline2, radii, 5);



    // TRACK
    int trackLen = 30;
    std::vector<Vector3> track;
    track.resize(trackLen);

    for (int i = 0; i < trackLen; i++) {
        static float ttt = 0;
        track.at(i) = Vector3(proto::Math::random(-50, 50), proto::Math::random(-50, 50), proto::Math::random(-250, -150));
        ttt += M_PI * 2.0 / trackLen;
    }
    Spline3 splinePath(track, 150, false, .5);
    std::vector<Vector3> path = splinePath.getVerts();
    std::cout << "  path.size() = " << path.size() << std::endl;

    // END TRACK

    // run the main loop
    bool running = true;
    while (running) {
        // handle events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                // end the program
                running = false;
            } else if (event.type == sf::Event::Resized) {

                // adjust the viewport when the window is resized
                glViewport(0, 0, event.size.width, event.size.height);
            }
        }

        // clear buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);



        /*
         * // manually set modelview matrix
        // as example             
        static float rotX = 0;
        static float rotY = 0;
        static float rotZ = 0;

        float newM[16];
        newM[0] = cos(rotY) * cos(rotZ);
        newM[1] = cos(rotY) * sin(rotZ);
        newM[2] = -sin(rotY);
        newM[3] = 0;

        newM[4] = -cos(rotX) * sin(rotZ) + sin(rotX) * sin(rotY) * cos(rotZ);
        newM[5] = cos(rotX) * cos(rotZ) + sin(rotX) * sin(rotY) * sin(rotZ);
        newM[6] = sin(rotX) * cos(rotY);
        newM[7] = 0;

        newM[8] = sin(rotX) * sin(rotZ) + cos(rotX) * sin(rotY) * cos(rotZ);
        newM[9] = -sin(rotX) * cos(rotZ) + cos(rotX) * sin(rotY) * sin(rotZ);
        newM[10] = cos(rotX) * cos(rotY);
        newM[11] = 0;

        newM[12] = 0;
        newM[13] = 0;
        newM[14] = 0;
        newM[15] = 1;
        rotX += M_PI / 180 * 3.2;
        rotY += M_PI / 180 * 1.2;
        rotZ += M_PI / 180 * 2.2;

        //glLoadMatrixf(newM); // update modelview // turned this off
         */

        //toroid2.display(GeomBase::VERTEX_BUFFER_OBJECT); // draw opaque first 
        /*gluLookAt(	
        GLdouble  	eyeX,
        GLdouble  	eyeY,
        GLdouble  	eyeZ,
        GLdouble  	centerX,
        GLdouble  	centerY,
        GLdouble  	centerZ,
        GLdouble  	upX,
        GLdouble  	upY,
        GLdouble  	upZ);
         */
        static float val = 0, t1 = 0.0, t2 = 0.0;
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();




        // gluLookAt(50, 0, -100, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
        static int ctr = 0;
        if (ctr < path.size()) {
            //glLoadIdentity();
            // gluLookAt(path.at(ctr).x, path.at(ctr).y, path.at(ctr).z, 0, path.at(ctr).y, path.at(ctr).z+200, path.at(ctr).x, path.at(ctr).y, path.at(ctr).z);
            // std::cout << "  path.at(ctr).x = " << path.at(ctr).x << std::endl;
            ctr++;
        } else {
            ctr = 0;
        }
        t1 += M_PI / 720.0;
        t2 += M_PI / 720.0;
        //std::cout<< "val = " << val << std::endl;
        val++;
        //glMatrixMode(GL_MODELVIEW);
        //glLoadIdentity();
        //glTranslatef(0, 0, val++);


        tube2.display(GeomBase::VERTEX_BUFFER_OBJECT, GeomBase::SURFACE);
        // use shader for toroid
        shader.bind();
        toroid.display(GeomBase::VERTEX_BUFFER_OBJECT);
        shader.unbind();
        //toroid3.display(GeomBase::DISPLAY_LIST);




        //glPushMatrix();
        //glLoadIdentity();
        //glTranslatef(0, 0, -5);
        glDisable(GL_LIGHTING);
        glLineWidth(1.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //spline.display();
        //spline.displayControlPts();
        //spline.displayInterpPts();
        //glPopMatrix();


        //spline.displayFrenetFrames(.2);



        // put back fill state
        glEnable(GL_LIGHTING);
        glPolygonMode(GL_FRONT, GL_FILL);


        // end the current frame (internally swaps the front and back buffers)
        window.display();
    }

    // release resources...

    return 0;
}

// function from nehe.gamedev.net (http://nehe.gamedev.net/article/replacement_for_gluperspective/21002/)

void setView(double fovY, double aspect, double zNear, double zFar) {
    const double pi = 3.1415926535897932384626433832795;
    double fW, fH;
    //Formula below corrected by Carsten Jurenz: 
    //fH = tan( (fovY / 2) / 180 * pi ) * zNear reduces to
    fH = tan(fovY / 360 * pi) * zNear;
    fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, zNear, zFar);
    //gluPerspective(fovY, aspect, zNear, zFar);
}

void setLights() {
    // light 01
    glMaterialfv(GL_FRONT, GL_SPECULAR, light01_mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, light01_mat_shininess);

    glLightfv(GL_LIGHT0, GL_AMBIENT, light01_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light01_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light01_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light01_position);


    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}

void initGL() {
    glFrontFace(GL_CCW); // default
    glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    //glDisable(GL_CULL_FACE);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glShadeModel(GL_SMOOTH); // smooth by default
    //glShadeModel(GL_FLAT); 
    glEnable(GL_COLOR_MATERIAL); // incorporates per vertex color with lights
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE); //  good for uniform scaling

    glClearColor(1, 1, 1, 1); // background color
    glClearStencil(0); // clear stencil buffer
    glClearDepth(1.0f); // 0 is near, 1 is far
    glDepthFunc(GL_LEQUAL);

}

//============================================================================
// Set up shaders
//============================================================================

void setShaders() {
    const char* vert = "resources/shaders/shader.vert";
    const char* frag = "resources/shaders/shader.frag";
    std::cout << "vert = " << &vert << std::endl;
    std::cout << "frag = " << &frag << std::endl;

    //shader = Shader();
    shader.init(vert, frag);
}

void printMatrices() {

    glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMatrix);
    std::cout << "size of modelViewMatrix = " << sizeof (modelViewMatrix) << std::endl;
    transpose(modelViewMatrix);
    std::cout << "\n  ==================GL_MODELVIEW MATRIX==================" << "\n";
    for (int i = 0; i < 16; i++) {
        if (i % 4 == 0 && i != 0) {
            std::cout << " |  \n |  ";
        }
        if (i == 0) {
            std::cout << " |  ";
        }
        std::cout << std::setw(12) << modelViewMatrix[i] << " ";
        if (i == 15) {
            std::cout << " |  ";
        }
    }
    std::cout << "\n  =======================================================" << "\n";


    glGetFloatv(GL_PROJECTION_MATRIX, projectionMatrix);
    //inverse(projectionMatrix);
    std::cout << "\n  ==================GL_PROJECTION MATRIX=================" << "\n";
    for (int i = 0; i < 16; i++) {
        if (i % 4 == 0 && i != 0) {
            std::cout << " |  \n |  ";
        }
        if (i == 0) {
            std::cout << " |  ";
        }
        std::cout << std::setw(12) << projectionMatrix[i] << " ";
        if (i == 15) {
            std::cout << " |  ";
        }
    }
    std::cout << "\n  =======================================================" << "\n";
}