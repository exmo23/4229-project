#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
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

// lighting globals
int lighting = 1;
int lightTracker = 0;

// texture globals
GLuint mechTex, groundTex;

GLfloat lightPositions[3][4] = {
    {0.0f, 10.0f, 0.0f, 1.0f},
    {25.0f, 5.0f, 0.0f, 1.0f},
    {0.0f, 20.0f, 20.0f, 0.0f}};

static float dist3(Vec3 a, Vec3 b) // all this does is calculate distance between two points
{
    float dx = b.x - a.x, dy = b.y - a.y, dz = b.z - a.z;
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

// mech state tracking
typedef struct
{
    float x, z, heading; // location
    float speed, turn;   // present speed and turn speed
    float phase;         // walk cycle
    float hipL, hipR, kneeL, kneeR;
} Mech;

Mech enemy = {0, 0, 0};
Mech player = {10, 10, 180};

// texture loading stuff
GLuint loadTexture(const char *path)
{
    int w, h, n;
    stbi_set_flip_vertically_on_load(1);
    unsigned char *data = stbi_load(path, &w, &h, &n, 4); // force RGBA
    if (!data)
    {
        printf("failed to load %s\n", path);
        return 0;
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    return tex;
}
//
//
// MECH STATE FUNCTIONS BELOW
//
//
void updateMech(Mech *m, float dt)
{

    m->phase += 4.0f * dt; // walking speed for animation

    m->heading += m->turn * dt; // turn speed
    float rad = m->heading * 3.14159f / 180.0f;
    m->x += m->speed * dt * sinf(rad); // move forward/back
    m->z += m->speed * dt * cosf(rad);

    m->hipL = 30 * sinf(m->phase);
    m->hipR = 30 * sinf(m->phase + 3.14159f);
    m->kneeL = 40 * fmaxf(0, -cosf(m->phase)); // knee bends while leg swings forward
    m->kneeR = 40 * fmaxf(0, cosf(m->phase));

    glutPostRedisplay();
}

float wp[][2] = {{-8, -8}, {8, -8}, {8, 8}, {-8, 8}}; // 2d array of patrol waypoints
int wpIndex = 0;                                      // which waypoint are we on?

void enemyPatrol(Mech *m)
{
    float dx = wp[wpIndex][0] - m->x;
    float dz = wp[wpIndex][1] - m->z;

    printf("controller called, dx=%.2f dz=%.2f\n", dx, dz);

    if (dx * dx + dz * dz < 1.0f) // needn't be perfect
        wpIndex = (wpIndex + 1) % 4;

    float want = atan2f(dx, dz) * 180.0f / 3.14159f; // desired angle calculation
    float diff = want - m->heading;                  // difference between desired and current angle
    while (diff > 180)
        diff -= 360; // prevents obtuse/unnecessary turns
    while (diff < -180)
        diff += 360;

    m->turn = fmaxf(-90, fminf(90, diff * 3));
    m->speed = (fabsf(diff) < 30) ? 2.5f : 0.5f;
}

void idle(void)
{
    printf("idle\n");
    static int last = 0;
    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = (now - last) / 1000.0f;
    last = now;

    enemyPatrol(&enemy);
    printf("spd %.2f turn %.2f x %.2f z %.2f hdg %.1f\n",
           enemy.speed, enemy.turn, enemy.x, enemy.z, enemy.heading);
    updateMech(&enemy, dt);

    glutPostRedisplay();
}

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

    float u = dist3(point0, point1);
    float v = dist3(point0, point3);

    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(point0.x, point0.y, point0.z);
    glTexCoord2f(u, 0.0f);
    glVertex3f(point1.x, point1.y, point1.z);
    glTexCoord2f(u, v);
    glVertex3f(point2.x, point2.y, point2.z);
    glTexCoord2f(0.0f, v);
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

void drawCube(float w, float h, float d)
{
    // draw box for house
    glColor3f(1, 0, 0);
    float size = 0.5; // front
    float sizeW = w * size;
    float sizeH = h;
    float sizeD = d * size;
    glBegin(GL_QUADS);
    Vec3 pointA = {sizeW, 0, sizeD}, pointB = {-sizeW, 0, sizeD}, pointC = {-sizeW, sizeH, sizeD}, pointD = {sizeW, sizeH, sizeD};
    drawQuad(pointA, pointD, pointC, pointB);
    glEnd();
    glBegin(GL_QUADS); // back
    Vec3 pointA1 = {sizeW, 0, -sizeD}, pointB1 = {-sizeW, 0, -sizeD}, pointC1 = {-sizeW, sizeH, -sizeD}, pointD1 = {sizeW, sizeH, -sizeD};
    drawQuad(pointA1, pointB1, pointC1, pointD1);
    glEnd();
    glBegin(GL_QUADS); // right
    Vec3 pointA2 = {sizeW, 0, sizeD}, pointB2 = {sizeW, 0, -sizeD}, pointC2 = {sizeW, sizeH, -sizeD}, pointD2 = {sizeW, sizeH, sizeD};
    drawQuad(pointA2, pointB2, pointC2, pointD2);
    glEnd();
    glBegin(GL_QUADS); // left
    Vec3 pointA3 = {-sizeW, 0, sizeD}, pointB3 = {-sizeW, 0, -sizeD}, pointC3 = {-sizeW, sizeH, -sizeD}, pointD3 = {-sizeW, sizeH, sizeD};
    drawQuad(pointA3, pointD3, pointC3, pointB3);
    glEnd();
    glBegin(GL_QUADS); // top
    Vec3 pointA4 = {sizeW, sizeH, sizeD}, pointB4 = {-sizeW, sizeH, sizeD}, pointC4 = {-sizeW, sizeH, -sizeD}, pointD4 = {sizeW, sizeH, -sizeD};
    drawQuad(pointA4, pointD4, pointC4, pointB4);
    glEnd();
    glBegin(GL_QUADS); // bottom
    Vec3 pointA5 = {sizeW, 0, sizeD}, pointB5 = {-sizeW, 0, sizeD}, pointC5 = {-sizeW, 0, -sizeD}, pointD5 = {sizeW, 0, -sizeD};
    drawQuad(pointA5, pointB5, pointC5, pointD5);
    glEnd();
}

void drawCylinder(float height, int slices)
{
    int i;
    float a, x, z;

    // cylinder wall
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= slices; i++)
    {
        a = 2.0f * (float)M_PI * i / slices;
        x = cosf(a);
        z = sinf(a);
        glNormal3f(x, 0.0f, z);
        glTexCoord2f((float)i / slices, 0.0f);
        glVertex3f(x, 0.0f, z);
        glTexCoord2f((float)i / slices, 1.0f);
        glVertex3f(x, height, z);
    }
    glEnd();

    // top face
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.5f, 0.5f);
    glVertex3f(0.0f, height, 0.0f);
    for (i = slices; i >= 0; i--)
    {
        a = 2.0f * (float)M_PI * i / slices;
        x = cosf(a);
        z = sinf(a);
        glTexCoord2f(0.5f + 0.5f * x, 0.5f + 0.5f * z);
        glVertex3f(x, height, z);
    }
    glEnd();

    // bottom face
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(0.5f, 0.5f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    for (i = 0; i <= slices; i++)
    {
        a = 2.0f * (float)M_PI * i / slices;
        x = cosf(a);
        z = sinf(a);
        glTexCoord2f(0.5f + 0.5f * x, 0.5f + 0.5f * z);
        glVertex3f(x, 0.0f, z);
    }
    glEnd();
}

void drawMech(Mech *m)
{
    glPushMatrix();
    glTranslatef(m->x, 0, m->z);    // general mech movement
    glRotatef(m->heading, 0, 1, 0); // general mech rotation

    glPushMatrix();          //
    glTranslatef(0, 4, 0);   //  draw mech body
    drawCube(3, 3, 3);       //
    glTranslatef(-1, -1, 0); //  draw mech body underside left
    drawCube(1, 1, 2);       //
    glTranslatef(2, 0, 0);   //  draw mech body underside right
    drawCube(1, 1, 2);       //
    glPopMatrix();           //

    glPushMatrix();
    glTranslatef(1, 4, 0);       //
    glRotatef(m->hipL, 1, 0, 0); // bend according to hip angle
    glTranslatef(0, -2, 0);      //
    glPushMatrix();
    glScalef(0.5, 1, 0.5);
    drawCylinder(2, 20); // thigh
    glPopMatrix();
    glRotatef(m->kneeL, 1, 0, 0); // bend according to knee angle
    glTranslatef(0, -2, 0);       //
    glPushMatrix();
    glScalef(0.45, 1, 0.45);
    drawCylinder(2, 20); // shin
    glPopMatrix();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-1, 4, 0);      //
    glRotatef(m->hipR, 1, 0, 0); // bend according to hip angle
    glTranslatef(0, -2, 0);      //
    glPushMatrix();
    glScalef(0.5, 1, 0.5);
    drawCylinder(2, 20); // thigh
    glPopMatrix();
    glRotatef(m->kneeR, 1, 0, 0); // bend according to knee angle
    glTranslatef(0, -2, 0);       //
    glPushMatrix();
    glScalef(0.45, 1, 0.45);
    drawCylinder(2, 20); // shin
    glPopMatrix();
    glPopMatrix();

    glPushMatrix();         //
    glTranslatef(2, 5, 0);  // draw mech left arm
    drawCube(1, 1, 2);      //
    glTranslatef(0, -1, 1); // draw mech left forearm
    drawCube(1, 1, 2);      //
    glPopMatrix();          //

    glPushMatrix();         //
    glTranslatef(-2, 5, 0); // draw mech right arm
    drawCube(1, 1, 2);      //
    glTranslatef(0, -1, 1); // draw mech right forearm
    drawCube(1, 1, 2);      //
    glPopMatrix();          //

    glPopMatrix(); // this is the overall mech transform that lets it move
}

void renderScene(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

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
    glColor3f(1, 1, 1);
    glBindTexture(GL_TEXTURE_2D, groundTex);
    glBegin(GL_QUADS);
    Vec3 floor1 = {-floorSize, 0, floorSize}, floor2 = {-floorSize, 0, -floorSize}, floor3 = {floorSize, 0, -floorSize}, floor4 = {floorSize, 0, floorSize};
    drawQuad(floor3, floor2, floor1, floor4);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, mechTex);
    drawMech(&enemy);

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
    glutCreateWindow("Alex Mohr Project");

    mechTex = loadTexture("mechTex.png");
    groundTex = loadTexture("groundTex.png");

    // glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glClearColor(0.2, 0.2, 0.4, 1.0);

    // register callback
    glutDisplayFunc(renderScene);
    glutReshapeFunc(reshape);
    glutSpecialFunc(specialKeys);
    glutKeyboardFunc(keyHandler);
    glutIdleFunc(idle);

    // init
    glEnable(GL_DEPTH_TEST);

    glutMainLoop();

    return 0;
}