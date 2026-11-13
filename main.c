#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#define GL_GLEXT_PROTOTYPES

float angle = 0.0;
float radius = 5;
int mode = 0;
float ratio = 0;

float fpvPosX = 0.0f;
float fpvPosZ = 0.0f;
float fpvAngle = 0.0f;
float fpvDirX = 0.0f;
float fpvDirZ = 0.0f;

typedef struct
{
    float x, y, z;
} Vec3;

int lighting = 1;

GLfloat lightPositions[3][4] = {
    {0.0f, 10.0f, 0.0f, 1.0f},
    {25.0f, 5.0f, 0.0f, 1.0f},
    {0.0f, 20.0f, 20.0f, 0.0f}};

int lightTracker = 0;

void drawQuad(Vec3 point0, Vec3 point1, Vec3 point2, Vec3 point3)
{
    Vec3 cross1 = {point2.x - point0.x, point2.y - point0.y, point2.z - point0.z}; // xyz vector calculation 1
    Vec3 cross2 = {point3.x - point1.x, point3.y - point1.y, point3.z - point1.z}; // xyz vector calc 2
    Vec3 normals = {cross1.y * cross2.z - cross1.z * cross2.y,
                    cross1.z * cross2.x - cross1.x * cross2.z,
                    cross1.x * cross2.y - cross1.y * cross2.x};

    float length = sqrtf(normals.x * normals.x + normals.y * normals.y + normals.z * normals.z);
    if (length > 1e-8f)
    {
        normals.x /= length;
        normals.y /= length;
        normals.z /= length;
    }

    glNormal3f(normals.x, normals.y, normals.z);

    glVertex3f(point0.x, point0.y, point0.z);
    glVertex3f(point1.x, point1.y, point1.z);
    glVertex3f(point2.x, point2.y, point2.z);
    glVertex3f(point3.x, point3.y, point3.z);
}

void drawTriangle(Vec3 point0, Vec3 point1, Vec3 point2)
{
    Vec3 cross1 = {point1.x - point0.x, point1.y - point0.y, point1.z - point0.z}; // xyz vector calculation 1
    Vec3 cross2 = {point2.x - point0.x, point2.y - point0.y, point2.z - point0.z}; // xyz vector calc 2
    Vec3 normals = {cross1.y * cross2.z - cross1.z * cross2.y,
                    cross1.z * cross2.x - cross1.x * cross2.z,
                    cross1.x * cross2.y - cross1.y * cross2.x};

    float length = sqrtf(normals.x * normals.x + normals.y * normals.y + normals.z * normals.z);
    if (length > 1e-8f)
    {
        normals.x /= length;
        normals.y /= length;
        normals.z /= length;
    }

    glNormal3f(normals.x, normals.y, normals.z);

    glVertex3f(point0.x, point0.y, point0.z);
    glVertex3f(point1.x, point1.y, point1.z);
    glVertex3f(point2.x, point2.y, point2.z);
}

void drawHouse(float r, float g, float b)
{
    // draw box for house
    glColor3f(r, g, b);
    float size = 2; // front
    glBegin(GL_QUADS);
    Vec3 pointA = {size, 0, size}, pointB = {-size, 0, size}, pointC = {-size, size, size}, pointD = {size, size, size};
    drawQuad(pointA, pointD, pointC, pointB);
    glEnd();
    glBegin(GL_QUADS); // back
    Vec3 pointA1 = {size, 0, -size}, pointB1 = {-size, 0, -size}, pointC1 = {-size, size, -size}, pointD1 = {size, size, -size};
    drawQuad(pointA1, pointB1, pointC1, pointD1);
    glEnd();
    glBegin(GL_QUADS); // right
    Vec3 pointA2 = {size, 0, size}, pointB2 = {size, 0, -size}, pointC2 = {size, size, -size}, pointD2 = {size, size, size};
    drawQuad(pointA2, pointB2, pointC2, pointD2);
    glEnd();
    glBegin(GL_QUADS); // left
    Vec3 pointA3 = {-size, 0, size}, pointB3 = {-size, 0, -size}, pointC3 = {-size, size, -size}, pointD3 = {-size, size, size};
    drawQuad(pointA3, pointD3, pointC3, pointB3);
    glEnd();
    glBegin(GL_QUADS); // top
    Vec3 pointA4 = {size, size, size}, pointB4 = {-size, size, size}, pointC4 = {-size, size, -size}, pointD4 = {size, size, -size};
    drawQuad(pointA4, pointD4, pointC4, pointB4);
    glEnd();
    glBegin(GL_QUADS); // bottom
    Vec3 pointA5 = {size, 0, size}, pointB5 = {-size, 0, size}, pointC5 = {-size, 0, -size}, pointD5 = {size, 0, -size};
    drawQuad(pointA5, pointB5, pointC5, pointD5);
    glEnd();

    // draw roof
    glColor3f(0.7f, 0.5f, 0.0f);
    Vec3 Tri1A = {size * 1.1, size, size * 1.1}, Tri1B = {-size * 1.1, size, size * 1.1}, Tri1C = {0, size * 2, 0};
    glBegin(GL_TRIANGLES);
    drawTriangle(Tri1B, Tri1A, Tri1C);
    glEnd();
    glColor3f(0.7f, 0.5f, 0.0f);
    Vec3 Tri2A = {-size * 1.1, size, size * 1.1}, Tri2B = {-size * 1.1, size, -size * 1.1}, Tri2C = {0, size * 2, 0};
    glBegin(GL_TRIANGLES);
    drawTriangle(Tri2B, Tri2A, Tri2C);
    glEnd();
    glColor3f(0.7f, 0.5f, 0.0f);
    Vec3 Tri3A = {-size * 1.1, size, -size * 1.1}, Tri3B = {size * 1.1, size, -size * 1.1}, Tri3C = {0, size * 2, 0};
    glBegin(GL_TRIANGLES);
    drawTriangle(Tri3B, Tri3A, Tri3C);
    glEnd();
    glColor3f(0.7f, 0.5f, 0.0f);
    Vec3 Tri4A = {size * 1.1, size, -size * 1.1}, Tri4B = {size * 1.1, size, size * 1.1}, Tri4C = {0, size * 2, 0};
    glBegin(GL_TRIANGLES);
    drawTriangle(Tri4B, Tri4A, Tri4C);
    glEnd();

    // draw a door

    glBegin(GL_QUADS);
    float doorWidth = size * 0.15;
    float doorHeight = size * 0.55;
    glVertex3f(-doorWidth, 0, size + 0.01);
    glVertex3f(doorWidth, 0, size + 0.01);
    glVertex3f(doorWidth, doorHeight, size + 0.01);
    glVertex3f(-doorWidth, doorHeight, size + 0.01);
    glEnd();
}

void renderScene(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    if (mode == 0)
    {
        float radAngle = angle * M_PI / 180.0f;
        /*gluLookAt((radius * sin(radAngle)), 1.0f, (radius * cos(radAngle)),
                  0.0f, 0.0f, 0.0f,
                  0.0f, 1.0f, 0.0f);*/
        gluLookAt(75.0f, 75.0f, 75.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    }
    else if (mode == 1)
    {
        gluLookAt(75, 75, 75, 0, 0, 0, 0, 1, 0);
    }
    else if (mode == 2)
    {
        float angleRad = fpvAngle * M_PI / 180.0f;
        printf("%f\n", fpvAngle);
        printf("%f\n", angleRad);
        fpvDirX = sin(angleRad);
        fpvDirZ = -cos(angleRad);
        float gluCenterX = (fpvPosX + fpvDirX);
        float gluCenterZ = (fpvPosZ + fpvDirZ);
        gluLookAt(fpvPosX, 5.0f, fpvPosZ, gluCenterX, 5.0f, gluCenterZ, 0.0f, 1.0f, 0.0f);
        printf("gluLookAt args %f %f %f %f %f %f %f %f %f\n", fpvPosX, 5.0f, fpvPosZ, gluCenterX, 5.0f, gluCenterZ, 0.0f, 1.0f, 0.0f);
    }

    glLightfv(GL_LIGHT0, GL_POSITION, lightPositions[lightTracker]);
    // floor
    float floorSize = 100;
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    Vec3 floor1 = {-floorSize, 0, floorSize}, floor2 = {-floorSize, 0, -floorSize}, floor3 = {floorSize, 0, -floorSize}, floor4 = {floorSize, 0, floorSize};
    drawQuad(floor3, floor2, floor1, floor4);
    glEnd();
    // drawHouse(1, 1, 0); //first house
    /*glBegin(GL_QUADS);
    float floorSize = 100;
    glVertex3f(-floorSize, 0, floorSize);
    glVertex3f(-floorSize, 0, -floorSize);
    glVertex3f(floorSize, 0, -floorSize);
    glVertex3f(floorSize, 0, floorSize);
    glEnd();*/

    // road
    /*glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_QUADS);
    Vec3 road1 = {floorSize, 0.01, -5}, road2 = {floorSize, 0.01, 5}, road3 = {-floorSize, 0.01, 5}, road4 = {-floorSize, 0.01, -5};
    drawQuad(road3, road2, road1, road4);*/
    glEnd();
    // drawHouse(1, 1, 0); //first house
    glPushMatrix(); // second house
    glTranslatef(5, 0.0f, 8);
    glRotatef(180, 0.0f, 1.0f, 0.0f);
    glScalef(1, 2, 1);
    drawHouse(1, 0, 1);
    glPopMatrix();

    glPushMatrix(); // third house
    glTranslatef(5, 0.0f, -8);
    glScalef(2, 2, 1);
    drawHouse(0, 1, 1);
    glPopMatrix();

    glPushMatrix(); // fourth house
    glTranslatef(10, 0.0f, 8);
    glRotatef(180, 0.0f, 1.0f, 0.0f);
    glScalef(1, 1, 1);
    drawHouse(1, 1, 0);
    glPopMatrix();

    glPushMatrix(); // fifth house
    glTranslatef(15, 0.0f, -8);
    glScalef(1, 1, 1);
    drawHouse(0, 1, 0);
    glPopMatrix();

    glPushMatrix(); // sixth house
    glTranslatef(-15, 0.0f, -8);
    glScalef(1, 1, 1);
    drawHouse(0, 0.5, 0.9);
    glPopMatrix();

    glPushMatrix(); // seventh house
    glTranslatef(0, 0.0f, 8);
    glScalef(1, 2, 1);
    glRotatef(180, 0.0f, 1.0f, 0.0f);
    drawHouse(0, 0.4, 0.3);
    glPopMatrix();

    glutSwapBuffers();
}

void swapPerspective()
{
    switch (mode)
    {
    case 0:
    {
        printf("switching to ortho projection\n");
        float orthoWidth = 60 * ratio;
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-(orthoWidth / 2), (orthoWidth / 2), -30, 30, -150, 250);
        glMatrixMode(GL_MODELVIEW);
        glutPostRedisplay();
        break;
    }
    case 1:
    {
        printf("switching to perspective projection\n");
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(80, ratio, 10, 250);
        glMatrixMode(GL_MODELVIEW);
        glutPostRedisplay();
        break;
    }
    case 2:
    {
        printf("switching to perspective projection with FPV\n");
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(80, ratio, 1, 250);
        glMatrixMode(GL_MODELVIEW);
        glutPostRedisplay();
        break;
    }
    }
}

void reshapePerspective(int w, int h)
{
    ratio = ((float)w / (float)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, w, h);
    gluPerspective(80, ratio, 1, 250);
    glMatrixMode(GL_MODELVIEW);
    glutPostRedisplay();
}

void reshapeFPV(int w, int h)
{
    ratio = ((float)w / (float)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, w, h);
    gluPerspective(80, ratio, 10, 250);
    glMatrixMode(GL_MODELVIEW);
    glutPostRedisplay();
}

void reshapeOrtho(int w, int h)
{
    ratio = ((float)w / (float)h);
    float orthoWidth = 60 * ratio;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, w, h);
    glOrtho(-(orthoWidth / 2), (orthoWidth / 2), -30, 30, -150, 150);
    glMatrixMode(GL_MODELVIEW);
    glutPostRedisplay();
}

void reshape(int w, int h)
{
    printf("reshaping %d x %d\n", w, h);

    if (mode == 0)
    {
        reshapeOrtho(w, h);
    }
    else if (mode == 1)
    {
        reshapePerspective(w, h);
    }
    else if (mode == 2)
    {
        reshapePerspective(w, h);
    }
}

void specialKeys(int key, int x, int y)
{
    switch (mode)
    {
    case 0:
        return;
    case 1:
        return;
    case 2:
    {
        float angleRad = fpvAngle * M_PI / 180;
        switch (key)
        {
        case GLUT_KEY_UP:
        {
            fpvPosX += 3 * fpvDirX;
            fpvPosZ += 3 * fpvDirZ;
            glutPostRedisplay();
            break;
        }
        case GLUT_KEY_DOWN:
        {
            fpvPosX -= 3 * fpvDirX;
            fpvPosZ -= 3 * fpvDirZ;
            glutPostRedisplay();
            break;
        }
        case GLUT_KEY_LEFT:
        {
            fpvAngle -= 5;
            glutPostRedisplay();
            break;
        }
        case GLUT_KEY_RIGHT:
        {
            fpvAngle += 5;
            glutPostRedisplay();
            break;
        }
        }
    }
    }
    /*switch (key)
    {
    case GLUT_KEY_LEFT:
        angle -= 0.5f;
        glutPostRedisplay();
        printf("%f\n", angle);
        break;
    case GLUT_KEY_RIGHT:
        angle += 0.5f;
        glutPostRedisplay();
        printf("%f\n", angle);
        break;
    }*/
}

void keyHandler(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'm':
    {
        switch (mode)
        {
        case 0:
        {
            mode = 1;
            swapPerspective();
            glutPostRedisplay();
            printf("Perspective mode\n");
            break;
        }
        case 1:
        {
            mode = 2;
            swapPerspective();
            glutPostRedisplay();
            printf("FPV mode\n");
            break;
        }
        case 2:
        {
            mode = 0;
            swapPerspective();
            glutPostRedisplay();
            printf("Ortho mode\n");
            break;
        }
        }
        break;
    }
    case 'l':
    {
        lighting = !lighting;
        if (lighting)
        {
            glEnable(GL_LIGHTING);
            printf("Turn lighting on\n");
        }
        else
        {
            glDisable(GL_LIGHTING);
            printf("Turn lighting off\n");
        }
        glutPostRedisplay();
        break;
    }
    case 'o':
    {
        lightTracker = (lightTracker + 1) % 3;
        printf("Lighting Mode %d\n", lightTracker);
        glutPostRedisplay();
    }
    }
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(1920, 1080);
    glutCreateWindow("Alex Mohr Homework 5");

    // glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glClearColor(0.2, 0.2, 0.4, 1.0);

    // register callback
    glutDisplayFunc(renderScene);
    glutReshapeFunc(reshape);
    glutSpecialFunc(specialKeys);
    glutKeyboardFunc(keyHandler);

    // init
    glEnable(GL_DEPTH_TEST);

    glutMainLoop();

    return 0;
}