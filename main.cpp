/*******************************************************************
		   Hierarchical Multi-Part Model Example
********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <cmath>
#include <GL/glew.h>
#include <gl/glut.h>
#include <GL/freeglut.h>
#include <vector>
#include "surfaceModeller.h"
#include "subdivcurve.h"
#include "VECTOR3D.h"
#include "cube.h"

const int vWidth = 650;    // Viewport width in pixels
const int vHeight = 500;    // Viewport height in pixels

// Zeppelin part dimensions
float zepBodyLength = 3.0;
float zepBodyRadius = zepBodyLength * 0.3;
float frontEndRadius = zepBodyRadius;
float backEndRadius = zepBodyRadius;
float driveShaftLength = backEndRadius * 1.1;
float driveShaftRadius = backEndRadius * 0.1;
float propellorCapLength = driveShaftRadius * 0.7;
float propellorCapRadius = driveShaftRadius * 1.2;
float propellorBladeLength = backEndRadius * 0.25;
float propellorBladeWidth = backEndRadius * 0.075;
float propellorBladeThickness = backEndRadius * 0.05;
float cabinLength = zepBodyLength * 0.3;
float cabinWidth = zepBodyRadius * 0.5;
float cabinHeight = zepBodyRadius * 0.8;
float finLength = zepBodyLength * 0.5;
float finThickness = zepBodyRadius * 0.1;
float finWidth = zepBodyRadius / 3;

// Control rotation of drive shaft
float driveShaftAngle = 0.0;

// Control rotation of zeppelin (which way it's facing)
float zepAngle = 0.0;

// Control X, Y, Z position of zeppelin
float zepXPosition = 0.0;
float zepYPosition = 0.0;
float zepZPosition = 0.0;

// Calculate the forward vector based on the zeppelin's rotation
float forwardX = cosf(zepAngle * 3.14159265 / 180.0);
float forwardZ = -sinf(zepAngle * 3.14159265 / 180.0);


// Lighting/shading and material properties for robot - upcoming lecture - just copy for now
// Robot RGBA material properties (NOTE: we will learn about this later in the semester)
GLfloat zep_mat_ambient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat zep_mat_specular[] = { 0.45f,0.55f,0.45f,1.0f };
GLfloat zep_mat_diffuse[] = { 0.1f,0.35f,0.1f,1.0f };
GLfloat zep_mat_shininess[] = { 32.0F };

// Light properties
GLfloat light_position0[] = { -4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_position1[] = { 4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };


// Mouse button
int currentButton;

// A template cube mesh
CubeMesh* cubeMesh = createCubeMesh();

// Structure defining a bounding box, currently unused
typedef struct BoundingBox {
	VECTOR3D min;
	VECTOR3D max;
} BBox;

// Prototypes for functions in this module
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
void customFin();

void drawZeppelin();
void drawZepBody();
void drawFrontEnd();
void drawBackEnd();
void drawDriveShaft();
void drawPropellorCap();
void drawPropellorBlades();
void drawCabin();
void drawFins();

void readOBJ();


int main(int argc, char** argv)
{
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(vWidth, vHeight);
	glutInitWindowPosition(200, 30);
	glutCreateWindow("3D Hierarchical Example");

	// Initialize GL
	initOpenGL(vWidth, vHeight);

	// Register callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(functionKeys);

	// Start event loop, never returns
	glutMainLoop();

	return 0;
}


// Set up OpenGL. For viewport and projection setup see reshape(). 
void initOpenGL(int w, int h)
{
	// Set up and enable lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);   // This second light is currently off

	// Other OpenGL setup
	glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
	glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
	glClearColor(0.4F, 0.4F, 0.4F, 0.0F);  // Color and depth for glClear
	glClearDepth(1.0f);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}


// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	// Create Viewing Matrix V
	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	// Draw zeppelin
	drawZeppelin();

	glutSwapBuffers();   // Double buffering, swap buffers
}

void drawZeppelin()
{	
	glPushMatrix();

	// Control position of zeppelin in the world
	glTranslatef(zepXPosition, zepYPosition, zepZPosition);

	// Turn zeppelin
	glTranslatef(-0.5 * zepBodyLength, 0, 0);
	glRotatef(zepAngle, 0.0, 1.0, 0.0);
	glTranslatef(-0.5 * zepBodyLength, 0, 0);

	// Draw all parts
	drawZepBody();
	drawFrontEnd();
	drawBackEnd();

	glPopMatrix();
}

void drawZepBody()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, zep_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zep_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, zep_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, zep_mat_shininess);

	// Position zeppelin body
	glPushMatrix();
	glRotatef(90, 0.0, 1.0, 0.0);

	// Draw zeppelin body
	glPushMatrix();
	glScalef(zepBodyRadius, zepBodyRadius, zepBodyLength);
	gluCylinder(gluNewQuadric(), 1.0, 1.0, 1.0, 20, 10);
	glPopMatrix();

	// Build cabin wrt zeppelin body
	drawCabin();
	drawFins();

	glPopMatrix();
}

void drawFrontEnd()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, zep_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zep_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, zep_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, zep_mat_shininess);


	glPushMatrix();

	gluSphere(gluNewQuadric(), frontEndRadius, 30, 30);

	glPopMatrix();
}

void drawBackEnd()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, zep_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, zep_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, zep_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, zep_mat_shininess);

	// Parent: drawZepBody
	// Children: drawDriveShaft, drawPropellor

	glPushMatrix();

	// Position backend wrt body
	glTranslatef(zepBodyLength, 0, 0);

	// Build back end
	glPushMatrix();
	gluSphere(gluNewQuadric(), backEndRadius, 30, 30);
	glPopMatrix();

	// Build drive shaft wrt back end
	drawDriveShaft();

	glPopMatrix();
}

void drawDriveShaft()
{
	// Parent: drawBackEnd
	// Children: drawPropellorCap, drawPropellorBlades


	// Position drive shaft wrt back end
	glPushMatrix();
	glRotatef(90, 0.0, 1.0, 0.0);

	// Control spin of drive shaft during movement
	glPushMatrix();
	glRotatef(driveShaftAngle, 0, 0, 1.0);

	// Draw drive shaft
	glPushMatrix();
	glScalef(driveShaftRadius, driveShaftRadius, driveShaftLength);
	gluCylinder(gluNewQuadric(), 1.0, 1.0, 1.0, 20, 10);
	glPopMatrix();

	// Draw propellor wrt drive shaft
	drawPropellorCap();

	// Draw propellor blades wrt drive shaft
	drawPropellorBlades();

	glPopMatrix();
	glPopMatrix();
}

void drawPropellorCap()
{
	// Parents: drawDriveShaft, drawBackEnd

	// Position propellor cap wrt drive shaft
	glPushMatrix();
	glTranslatef(0, 0, driveShaftLength);
	
	// Draw propellor cap
	glPushMatrix();
	glScalef(propellorCapRadius, propellorCapRadius, propellorCapLength);
	glutSolidCone(1.0, 1.0, 10, 10);
	glPopMatrix();

	glPopMatrix();
}

void drawPropellorBlades()
{
	// Parents: drawDriveShaft, drawBackEnd


	// Position propellor blades along the length of the drive shaft
	glPushMatrix();
	glTranslatef(0, 0, driveShaftLength + 0.5 * propellorCapLength);

	// Position first propellor blade on drive shaft
	glPushMatrix();
	glRotatef(60.0, 0, 0, 1.0);
	glTranslatef(0, propellorBladeLength, 0);

	// Draw first propellor blade
	glPushMatrix();
	glScalef(propellorBladeWidth, propellorBladeLength, propellorBladeThickness);
	gluSphere(gluNewQuadric(), 1, 30, 30);
	glPopMatrix();

	glPopMatrix();
	// Finished first blade

	// Position second propellor blade on drive shaft
	glPushMatrix();
	glRotatef(180.0, 0, 0, 1.0);
	glTranslatef(0, propellorBladeLength, 0);

	// Draw second propellor blade
	glPushMatrix();
	glScalef(propellorBladeWidth, propellorBladeLength, propellorBladeThickness);
	gluSphere(gluNewQuadric(), 1, 30, 30);
	glPopMatrix();

	glPopMatrix();
	// Finished second blade

	// Position third propellor blade on drive shaft
	glPushMatrix();
	glRotatef(300.0, 0, 0, 1.0);
	glTranslatef(0, propellorBladeLength, 0);

	// Draw third propellor blade
	glPushMatrix();
	glScalef(propellorBladeWidth, propellorBladeLength, propellorBladeThickness);
	gluSphere(gluNewQuadric(), 1, 30, 30);
	glPopMatrix();

	glPopMatrix();
	// Finished third blade

	glPopMatrix();
}

void drawCabin()
{
	// Parent: drawZepBody

	// Position cabin wrt zeppelin body
	glPushMatrix();
	glTranslatef(0, -zepBodyRadius / 2, zepBodyLength / 3);
	glRotatef(90.0, 1.0, 0, 0);

	// Draw cabin body
	glPushMatrix();
	glScalef(cabinWidth, cabinLength, cabinHeight);
	gluCylinder(gluNewQuadric(), 1.0, 1.0, 1.0, 20, 10);

	glPopMatrix();
	glPopMatrix();
}

void drawFins()
{
	// Parent: drawZepBody
	
	// Position fins along the length of the body
	glPushMatrix();
	glTranslatef(0, 0, zepBodyLength);

	// Position first fin
	glPushMatrix();
	glRotatef(-90, 1.0, 0, 0);
	glTranslatef(zepBodyRadius / 2, 0, 0);

	// Draw first fin
	glPushMatrix();
	glScalef(finWidth, finLength, finThickness);
	customFin();
	glPopMatrix();

	glPopMatrix();
	// Finished drawing first fin

	// Position second fin
	glPushMatrix();
	glRotatef(-90, 1.0, 0, 0);
	glRotatef(-90, 0, 1.0, 0);
	glTranslatef(zepBodyRadius / 2, 0, 0);

	// Draw second fin
	glPushMatrix();
	glScalef(finWidth, finLength, finThickness);
	customFin();
	glPopMatrix();

	glPopMatrix();
	// Finished drawing second fin

	// Position third fin
	glPushMatrix();
	glRotatef(-90, 1.0, 0, 0);
	glRotatef(-180, 0, 1.0, 0);
	glTranslatef(zepBodyRadius / 2, 0, 0);

	// Draw third fin
	glPushMatrix();
	glScalef(finWidth, finLength, finThickness);
	customFin();
	glPopMatrix();

	glPopMatrix();
	// Finished drawing third fin

	glPopMatrix();
}

void customFin()
{
	glBegin(GL_QUADS);
		// top rectangle
		glVertex3f(0, 0, 0);
		glVertex3f(2.0, 0, 0);
		glVertex3f(2.0, 1.0, 0);
		glVertex3f(0, 1.0, 0);

		// bottom rectangle
		glVertex3f(0, 0, 0.2);
		glVertex3f(2.0, 0, 0.2);
		glVertex3f(2.0, 1.0, 0.2);
		glVertex3f(0, 1.0, 0.2);

		// inside edge
		glVertex3f(0, 0, 0);
		glVertex3f(3.0, 0, 0);
		glVertex3f(3.0, 0.0, 0.2);
		glVertex3f(0, 0, 0.2);

		// back edge
		glVertex3f(0, 0, 0);
		glVertex3f(0, 1.0, 0);
		glVertex3f(0, 1.0, 0.2);
		glVertex3f(0, 0, 0.2);

		// outside edge straight
		glVertex3f(0, 1.0, 0);
		glVertex3f(2.0, 1.0, 0);
		glVertex3f(2.0, 1.0, 0.2);
		glVertex3f(0, 1.0, 0.2);

		// outside edge angled
		glVertex3f(2.0, 1.0, 0);
		glVertex3f(3.0, 0, 0);
		glVertex3f(3.0, 0, 0.2);
		glVertex3f(2.0, 1.0, 0.2);
	glEnd();
	glBegin(GL_TRIANGLES);
		// top triangle
		glVertex3f(2.0, 0, 0);
		glVertex3f(3.0, 0, 0);
		glVertex3f(2.0, 1.0, 0);

		// bottom triangle
		glVertex3f(2.0, 0, 0.2);
		glVertex3f(3.0, 0, 0.2);
		glVertex3f(2.0, 1.0, 0);
	glEnd();
}
// Callback, called at initialization and whenever user resizes the window.
void reshape(int w, int h)
{
	// Set up viewport, projection, then change to modelview matrix mode - 
	// display function will then set up camera and do modeling transforms.
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLdouble)w / h, 0.2, 40.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

bool stop = false;

// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
		// Calculate the forward vector based on the zeppelin's rotation
		forwardX = cosf(zepAngle * 3.14159265 / 180.0);
		forwardZ = -sinf(zepAngle * 3.14159265 / 180.0);

		// Update the zeppelin's position
		zepXPosition -= 0.1 * forwardX;
		zepZPosition -= 0.1 * forwardZ;

		// spin propellor
		driveShaftAngle += 25.0;
		break;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}

// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{
	if (key == GLUT_KEY_LEFT)
	{
		zepAngle += 2.0;
	}
	else if (key == GLUT_KEY_RIGHT)
	{
		zepAngle -= 2.0;
	}
	else if (key == GLUT_KEY_DOWN)
	{
		zepYPosition -= 0.05;
	}
	else if (key == GLUT_KEY_UP)
	{
		zepYPosition += 0.05;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}








void drawMeshVBO();
void readOBJ();


Vertex	 *varray;
Vector3D *positions;
Vector3D *normals;
GLuint	 *indices;

#define NUMBEROFSIDES 16

unsigned int numTris = 0;
unsigned int numVertices = 0;
unsigned int numIndices = 0;

GLuint VAO;
GLuint VBOv, VBOn, VBOi;

void drawMeshVBO()
{
	// Generate VBOs
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBOv);
	glGenBuffers(1, &VBOn);
	glGenBuffers(1, &VBOi);

	glBindVertexArray(VAO);

	// vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, VBOv);
	glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vector3D), positions, GL_STATIC_DRAW);
	glVertexPointer(3, GL_FLOAT, 0, NULL);
	glEnableClientState(GL_VERTEX_ARRAY);

	// vertex normals
	glBindBuffer(GL_ARRAY_BUFFER, VBOn);
	glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vector3D), normals, GL_STATIC_DRAW);
	glNormalPointer(GL_FLOAT, 3 * sizeof(float), NULL);
	glEnableClientState(GL_NORMAL_ARRAY);

	// indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOi);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * numIndices, indices, GL_STATIC_DRAW);

	// bind mesh VAO
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, NULL);
}

void readOBJ()
{
	char buf[1024];
	char key[1024];
	int n;
	FILE* fin;

	int fc = 0; // face count
	int vc = 0; // vertex count
	int nc = 0; // normal count

	if ((fin = fopen("mesh.obj", "r")))
	{
		/* Process each line of the OBJ file, invoking the handler for each. */

		while (fgets(buf, 1024, fin))
			if (sscanf(buf, "%s%n", key, &n) >= 1)
			{
				const char* c = buf + n;

				if (!strcmp(key, "f"))
				{
					sscanf(buf, "%d/%d/%d", &indices[fc], &indices[fc + 1], &indices[fc + 2]);
					fc += 3;
				}
				else if (!strcmp(key, "vn"))
				{
					sscanf(buf, "%f %f %f", &positions[vc].x, &positions[vc].y, &positions[vc].z);
					vc++;
				}
				else if (!strcmp(key, "v"))
				{
					sscanf(buf, "%f %f %f", &normals[vc].x, &normals[vc].y, &normals[vc].z);
					nc++;
				}
			}
		fclose(fin);

		numTris = fc / 3;
		numIndices = fc;
		numVertices = vc;
	}
}
