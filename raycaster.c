#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>
#include <float.h>

// Global variables

#define PI 3.1415926535
#define degree 0.0174533

double playerX = 300, playerY = 300;
double pAngle = 0;

int mapWidth = 8, mapHeight = 8, mapSize = 64;
int map[] = {
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 1, 0, 1, 0, 1, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 1, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 1, 1, 1, 1, 1, 1, 1
};

void init() {
	glClearColor(0.3, 0.3, 0.3, 0);
	gluOrtho2D(0, 1024, 512, 0);
}

double distance(double x, double y, double destX, double destY) {
	return sqrt((x - destX) * (x - destX) + (y - destY) * (y - destY));
}

void drawPlayer() {
	glColor3f(1, 1, 0);
	glPointSize(8);
	glBegin(GL_POINTS);
	glVertex2i(playerX, playerY);
	glEnd();
	
	glLineWidth(3);
	glBegin(GL_LINES);
	glVertex2i(playerX, playerY);
	glVertex2i(playerX + cos(pAngle) * 25, playerY + sin(pAngle) * 25);
	glEnd();
}

void drawMap() {
	int y, x;
	for (y = 0; y < mapHeight; y++) {
		for (x = 0; x < mapWidth; x++) {
			// Get tile type and set color accordingly.
			int mapTile = map[y * mapWidth + x];
			if (mapTile == 0) {
				glColor3f(0, 0, 0);
			} else if (mapTile == 1) {
				glColor3f(1, 1, 1);
			}
			
			// Draw tile.
			int drawX = x * mapSize, drawY = y * mapSize;
			int padding = 1;
			glBegin(GL_QUADS);
			glVertex2i(drawX + padding			, drawY + padding);
			glVertex2i(drawX + mapSize - padding, drawY + padding);
			glVertex2i(drawX + mapSize - padding, drawY + mapSize - padding);
			glVertex2i(drawX + padding			, drawY + mapSize - padding);
			glEnd();
		}	
	}
}

void drawRays() {
	int numberOfRays = 60, r;
	double rAngle = pAngle - degree * numberOfRays / 2;
	double rayX, rayY, rayHX, rayHY, rayVX, rayVY, yOffset, xOffset;
	
	for (r = 0; r < numberOfRays; r++, rAngle += degree) {
		if (rAngle < 0) rAngle += 2 * PI;
		else if (rAngle > 2 * PI) rAngle -= 2 * PI;
		
		int dof = 0;
		
		// Get coordinates of closest intersection with horizontal lines if any.
		double horizontalDistance = DBL_MAX;
		double aTan = -1 / tan(rAngle);
		if (rAngle != 0 && rAngle < PI) {
			// We are looking up.
			rayHY = (((int)playerY >> 6) << 6) + 64;
			rayHX = (playerY - rayHY) * aTan + playerX;
			yOffset = 64;
			xOffset = -yOffset * aTan;
		} else if (rAngle > PI) {
			// We are looking down.
			rayHY = (((int)playerY >> 6) << 6) - 0.0001;
			rayHX = (playerY - rayHY) * aTan + playerX;
			yOffset = -64;
			xOffset = -yOffset * aTan;
		} else {
			rayHX = playerX;
			rayHY = playerY;
			dof = 8;
		}
		
		while (dof < 8) {
			int mapX = (int) (rayHX) >> 6;
			int mapY = (int) (rayHY) >> 6;
			int mapIndex = mapY * mapWidth + mapX;
			if (mapIndex >= 0 && mapIndex < mapSize && map[mapIndex] == 1) {
				dof = 8;
				horizontalDistance = distance(playerX, playerY, rayHX, rayHY);
			} else {
				rayHX += xOffset;
				rayHY += yOffset;
				dof += 1;
			}
		}

		// Get coordinates of closest intersection with vertical lines if any.
		dof = 0;
		double verticalDistance = DBL_MAX;
		double nTan = -tan(rAngle);
		if (rAngle > PI / 2 && rAngle < 3 * PI / 2) {
			// We are looking left.
			rayVX = (((int)playerX >> 6) << 6) - 0.0001;
			rayVY = (playerX - rayVX) * nTan + playerY;
			xOffset = -64;
			yOffset = -xOffset * nTan;
		} else if (rAngle < PI / 2 || rAngle > 3 * PI / 2) {
			// We are looking right.
			rayVX = (((int)playerX >> 6) << 6) + 64;
			rayVY = (playerX - rayVX) * nTan + playerY;
			xOffset = 64;
			yOffset = -xOffset * nTan;
		} else {
			rayVX = playerX;
			rayVY = playerY;
			dof = 8;
		}
		
		while (dof < 8) {
			int mapX = (int) (rayVX) >> 6;
			int mapY = (int) (rayVY) >> 6;
			int mapIndex = mapY * mapWidth + mapX;
			if (mapIndex >= 0 && mapIndex < mapSize && map[mapIndex] == 1) {
				dof = 8;
				verticalDistance = distance(playerX, playerY, rayVX, rayVY);
			} else {
				rayVX += xOffset;
				rayVY += yOffset;
				dof += 1;
			}
		}
		
		// Pick shortest line
		double dist;
		if (horizontalDistance != DBL_MAX || verticalDistance != DBL_MAX) {
			if (horizontalDistance < verticalDistance) {
				rayX = rayHX;
				rayY = rayHY;
				dist = horizontalDistance;
				glColor3f(0.7, 0.7, 0.7);
			} else if (verticalDistance < horizontalDistance) {
				rayX = rayVX;
				rayY = rayVY;
				dist = verticalDistance;
				glColor3f(0.9, 0.9, 0.9);
			}
			
			glLineWidth(1);
			glBegin(GL_LINES);
			glVertex2i(playerX, playerY);
			glVertex2i(rayX, rayY);
			glEnd();	
		}
		
		// Draw 3D scene, begin with walls. Draw sky and floor at the same time.
		double angleDiff = pAngle - rAngle;
		if (angleDiff < 0) angleDiff += 2 * PI;
		else if (angleDiff > 2 * PI) angleDiff -= 2 * PI;
		dist *= cos(angleDiff);
		double lineHeight = (mapSize * 320) / dist;
		double lineOffset = 160 - lineHeight / 2;
		
		glLineWidth(8);
		glBegin(GL_LINES);
		glVertex2i(r * 8 + 530, lineOffset);
		glVertex2i(r * 8 + 530, lineHeight + lineOffset);
		glEnd();
		
		// Draw sky
		glColor3f(0, 0, 1);
		glBegin(GL_LINES);
		glVertex2i(r * 8 + 530, 0);
		glVertex2i(r * 8 + 530, lineOffset);
		glEnd();
		
		// Draw floor
		glColor3f(0, 1, 0);
		glBegin(GL_LINES);
		glVertex2i(r * 8 + 530, lineHeight + lineOffset);
		glVertex2i(r * 8 + 530, 512);
		glEnd();
	}
}

void handleInput(unsigned char key, int x, int y) {
	double playerDx = cos(pAngle) * 5;
	double playerDy = sin(pAngle) * 5;
	
	if (key == 'w') {
		playerX += playerDx;
		playerY += playerDy;
	} else if (key == 's') {
		playerX -= playerDx;
		playerY -= playerDy;
	} else if (key == 'a') {
		pAngle -= 0.1;
	} else if (key == 'd') {
		pAngle += 0.1;
	}
	
	if (pAngle < 0) {
		pAngle += 2 * PI;
	} else if (pAngle > 2 * PI) {
		pAngle -= 2 * PI;
	}
	
	glutPostRedisplay();
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT);
	drawMap();
	drawRays();
	drawPlayer();
	glutSwapBuffers();
}

int main(int argc, char** argv) { 
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(1024,512);
	glutCreateWindow("OpenGL");
	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(handleInput);
	glutMainLoop();
	return 0;
}
