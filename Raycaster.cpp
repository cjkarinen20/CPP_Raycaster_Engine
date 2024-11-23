#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>
#include "Textures/Brick1.ppm"
#include "Textures/Map_Textures.ppm"
#include "Textures/Sky_Texture.ppm"
#define PI 3.1415926535
#define P2 PI/2
#define P3 3*PI/2
#define DR 0.0174533 //One degree in radians

typedef struct
{
	int w,a,d,s; //Button state
}
ButtonKeys; ButtonKeys Keys;

float px, py, pdx, pdy, pa; //Player position
float dist(float ax, float ay, float bx, float by, float ang)
{
	return (sqrt((bx-ax) * (bx-ax) + (by-ay) * (by-ay)));
}
float degToRad(float a){ return a*PI/180.0;}
float FixAng(float a){if (a > 359){a -= 360;} if (a < 0){a += 360;} return a;}
float frame1, frame2, fps;
float turnSpeed, walkSpeed;

void init()
{
	glClearColor(0.3, 0.3, 0.3, 0);
	gluOrtho2D(0, 1024, 512, 0);
	px=150; py=400; pdx = cos(pa) * 5; pdy = sin(pa) * 5;
}

//Draw player as a colored pixel/cube
void drawPlayer()
{
	//Draw player as a colored cube
	glColor3f(1, 1, 0);
	glPointSize(8);
	glBegin(GL_POINTS);
	glVertex2i(px, py);
	glEnd();
	
	//Indicate player direction
	glLineWidth(3);
	glBegin(GL_LINES);
	glVertex2i(px, py);
	glVertex2i(px + pdx * 5, py + pdy * 5);
	glEnd();
}

//Initialize map size parameters
int mapX = 8;
int mapY = 8;
int mapS = 64; 


int mapW[] = //Map Wall Layout 
{
	1,1,1,1,1,2,1,1,
	1,0,0,12,0,0,0,1,
	1,0,0,3, 0,0,0,2,
	1,0,0,3, 0,0,0,1,
	1,0,0,3, 0,1,1,1,
	1,0,0,3, 0,0,0,1,
	1,0,0,3, 0,0,0,1,
	1,1,1,1,1,2,1,1,
};
int mapF[]=	//Map Floor Layout
{
 	7,7,7,7,7,7,7,7,
 	7,7,7,7,7,7,7,7,
 	7,7,7,7,7,7,7,7,
 	7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,
	7,7,7,7,7,7,7,7,
 	7,7,7,7,7,7,7,7,
 	7,7,7,7,7,7,7,7,	
};

int mapC[]=	//Map Ceiling Layout
{
 	10,10,10,10,10,10,10,10,
 	10,10,10,10,10,10,10,10,
 	10,10,10,10,10,10,10,10,
 	10,10,10,0,0,10,10,10,
 	10,10,10,0,0,10,10,10,
 	10,10,10,0,0,10,10,10,
 	10,10,10,10,10,10,10,10,
 	10,10,10,10,10,10,10,10,	
};

void drawMap2D() //Draw the map
{
	int x, y, xo, yo;
	for (y = 0; y < mapY; y++)
	{
		for (x = 0; x < mapX; x++)
		{
			//If current cell is a wall, draw a white wall; else, draw a black space
			if (mapW[y * mapX + x] > 0) {glColor3f(1,1,1);} else { glColor3f(0,0,0);} 
			xo = x * mapS; yo = y * mapS;
			glBegin(GL_QUADS);
			glVertex2i(xo         +1, yo        + 1);
			glVertex2i(xo         +1, yo + mapS - 1);
			glVertex2i(xo + mapS - 1, yo + mapS - 1);
			glVertex2i(xo + mapS - 1, yo        + 1);
			glEnd();

		}
	}
}

//Cast rays
void drawRays3D()
{
	int r, mx, my, mp, dof; 
	float rx, ry, ra, xo, yo, disT; 
	ra = pa - DR * 30;
	if(ra < 0) { ra += 2 * PI; } 
	if (ra > 2 * PI) { ra -= 2 * PI; } 
	for (r = 0; r < 60; r++) 
	{ 
		int vmt = 0, hmt = 0; //Vertical and Horizontal map texture numbers
		//Check horizontal lines 
		dof = 0; 
		float disH = 1000000, hx = px, hy = py; 
		float aTan = -1/tan(ra); 
		if (ra > PI) { ry = (((int) py >> 6) << 6) - 0.0001; rx = (py - ry) * aTan + px; yo = -64; xo = -yo * aTan; } 
		if (ra < PI) { ry = (((int) py >> 6) << 6) + 64; rx = (py - ry) * aTan + px; yo = 64; xo = -yo * aTan; } 
		if (ra == 0 || ra == PI) { rx = px; ry = py; dof = 8; } 
		while(dof < 8) 
		{ 
			mx = (int)(rx) >> 6; my = (int)(ry) >> 6; mp = my * mapX + mx;
			if(mx >= 0 && mx < mapX && my >= 0 && my < mapY) 
			{ 
				mp = my * mapX + mx; 
				if (mapW[mp] > 0) 
				{ 
					hmt = mapW[mp] - 1;
					hx=rx; hy=ry; disH=dist(px,py,hx,hy,ra); dof = 8; 
				} 
				else 
				{ 
					rx += xo; ry += yo; dof += 1; 
				} 
			} 
			else { dof = 8; } 
		} 
		//Check vertical lines 
		dof = 0; 
		float disV = 1000000, vx = px, vy = py; 
		float nTan = -tan(ra); 
		if (ra > P2 && ra < P3) { rx = (((int) px >> 6) << 6) - 0.0001; ry = (px - rx) * nTan + py; xo = -64; yo = -xo * nTan; } 
		if (ra < P2 || ra > P3) { rx = (((int) px >> 6) << 6) + 64; ry = (px - rx) * nTan + py; xo = 64; yo = -xo * nTan; } 
		if (ra == 0 || ra == PI) { rx = px; ry = py; dof = 8; } 
		while(dof < 8) 
		{ 
			mx = (int)(rx) >> 6; 
			my = (int)(ry) >> 6; 
			if(mx >= 0 && mx < mapX && my >= 0 && my < mapY) 
			{ 
				mp = my * mapX + mx; 
				if (mapW[mp] > 0) 
				{ 
					vmt = mapW[mp] - 1;
					vx=rx; vy=ry; disV=dist(px,py,vx,vy,ra); dof = 8; 
				} 
				else 
				{ 
					rx += xo; ry += yo; dof += 1; 
				} 
			} 
			else 
			{ dof = 8; } 
		}
		float shade = 1;
		glColor3f(0,0.8,0);
		if(disV < disH) {hmt = vmt; shade = 0.5; rx = vx; ry = vy; disT = disV; glColor3f(0.9,0,0);} //Vertical wall hit
		if(disH < disV) {rx = hx; ry = hy; disT = disH; glColor3f(0.7,0,0);} // Horizontal wall hit
		glLineWidth(3); glBegin(GL_LINES); glVertex2i(px, py); glVertex2i(rx, ry); glEnd(); 
		
		

		float ca = pa - ra; if(ca < 0) {ca += 2 * PI;} if (ca > 2 * PI) { ca -= 2 * PI;} disT = disT * cos(ca); //Fix fisheye warping
		float lineH = (mapS * 500)/disT; 
		float texY_Step = 32.0/(float)lineH;
		float texY_Offset = 0;
		if (lineH > 320) {texY_Offset = (lineH - 320)/2.0; lineH = 320;} //Line height
		float lineO = 160 - lineH/2; //Line offset
		
		//Draw Walls
		int y;
		float texY = texY_Offset * texY_Step; //hmt * 32;
		float texX;
		if (shade == 1){texX = (int)(rx/2.0) % 32; if (ra > 180) {texX = 31 - texX;}}
		else {texX = (int)(ry / 2.0) % 32; if (ra > 90 && ra < 270) {texX = 31 - texX;}}
		for (y = 0; y < lineH; y++)
		{
			//Wall Texture Mapping
			int pixel = ((int)texY*32+(int)texX)*3 + (hmt * 32 * 32 * 3);
			int red = Map_Textures[pixel+0] * shade;
			int green = Map_Textures[pixel+1] * shade;
			int blue = Map_Textures[pixel+2] * shade;
			glPointSize(8); glColor3ub(red, green, blue); glBegin(GL_POINTS); glVertex2i (r * 8 + 530, y + lineO); glEnd();
			texY += texY_Step;
		}
		// Draw Floors and Ceilings
		for (y = lineO + lineH; y < 320; y++) 
		{
    		float dy = y - 160.0; // 160 = 320 / 2

    		//Calculate straight distance to the floor/ceiling point
    		float straight_dist = (mapS * 160) / dy;
    		float corrected_dist = straight_dist / cos(ra - pa); //Corrected distance for fisheye effect

    		//Texture coordinates calculation
   			texX = px + cos(ra) * corrected_dist;
    		texY = py + sin(ra) * corrected_dist;
    		
    		// Initialize floorShade and ceilingShade based on distance 
			float floorShade = 0.7 * (1 - corrected_dist / 1000.0); 
			if (floorShade < 0.3) floorShade = 0.3; 
			float ceilingShade = 0.4 * (1 - corrected_dist / 1000.0); 
			if (ceilingShade < 0.2) ceilingShade = 0.2;
			
	    	// Floor Texture Mapping
	    	int mp = mapF[(int)(texY / 64.0) * mapX + (int)(texX / 64.0)] * 32 * 32;
	    	int pixel = (((int)(texY) & 31) * 32 + ((int)(texX) & 31)) * 3 + mp * 3;
	    	int red = Map_Textures[pixel + 0] * floorShade;
	    	int green = Map_Textures[pixel + 1] * floorShade;
	    	int blue = Map_Textures[pixel + 2] * floorShade;
	    	glColor3ub(red, green, blue);
	    	glPointSize(8);
	    	glBegin(GL_POINTS);
	    	glVertex2i(r * 8 + 530, y);
	    	glEnd();
	
	    	//Ceiling Texture Mapping
	    	mp = mapC[(int)(texY / 64.0) * mapX + (int)(texX / 64.0)] * 32 * 32;
	   		pixel = (((int)(texY) & 31) * 32 + ((int)(texX) & 31)) * 3 + mp * 3;
	    	red = Map_Textures[pixel + 0] * ceilingShade; //Darker ceiling
	    	green = Map_Textures[pixel + 1] * ceilingShade;
	    	blue = Map_Textures[pixel + 2] * ceilingShade;
	    	if (mp > 0) //If ceiling value is greater than 0, i.e. not the sky
			{
	    		glColor3ub(red, green, blue);
	    		glPointSize(8);
	    		glBegin(GL_POINTS);
	    		glVertex2i(r * 8 + 530, 320 - y);
	    		glEnd();	
	    	}

		}
		ra += DR;
		if (ra < 0) {ra += 2 * PI;}
		if (ra > 2 * PI) {ra -= 2 * PI;}
	}
}

//Renders the skybox image
void drawSky()
{
	int x,y;
	for (y = 0; y < 40; y++)
	{
		for (x = 0; x < 120; x++)
		{
			int xo = (int) pa * 2 - x; if (xo < 0){xo += 120;} xo = xo % 120;
	    	int pixel = (y * 120 + xo) * 3;
	    	int red = Sky_Texture[pixel + 0];
	    	int green = Sky_Texture[pixel + 1];
	    	int blue = Sky_Texture[pixel + 2];
	    	glPointSize(4); glColor3ub(red, green, blue); glBegin(GL_POINTS); glVertex2i(x * 4 + 530, y * 4); glEnd();
		}
	}
}

void display()
{
	//Frames per second
	frame2 = glutGet(GLUT_ELAPSED_TIME); fps = (frame2 - frame1); frame1 = glutGet(GLUT_ELAPSED_TIME);
	
	turnSpeed = 0.005; walkSpeed = 0.025;
	
	if (Keys.a==1){ pa -= turnSpeed * fps; if (pa < 0) {pa += 2 * PI;} pdx = cos(pa) * 5; pdy = sin(pa) * 5;}
	if (Keys.d==1){ pa += turnSpeed * fps; if (pa > 2 * PI){pa -= 2 * PI;} pdx = cos(pa) * 5; pdy = sin(pa) * 5;}
	
	//Collision
	int xo = 0; if (pdx < 0){xo = -20;} else {xo = 20;}
	int yo = 0; if (pdy < 0){yo = -20;} else {yo = 20;}
	int ipx = px/64.0, ipx_add_xo = (px + xo)/64.0, ipx_sub_xo = (px-xo)/64.0;
	int ipy = py/64.0, ipy_add_yo = (py + yo)/64.0, ipy_sub_yo = (py-yo)/64.0;
	
	if (Keys.w==1) //Calculate movement and forwards collsion
	{ 
		if (mapW[ipy * mapX + ipx_add_xo] == 0) {px += pdx * walkSpeed * fps;}
		if (mapW[ipy_add_yo * mapX + ipx] == 0) {py += pdy * walkSpeed * fps;}
	}
	if (Keys.s==1) //Calculate movement and backwards collsion
	{ 
		if (mapW[ipy * mapX + ipx_sub_xo] == 0) {px -= pdx * walkSpeed * fps;}
		if (mapW[ipy_sub_yo * mapX + ipx] == 0) {py -= pdy * walkSpeed * fps;}
	}
	glutPostRedisplay();
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawMap2D();
	drawPlayer();
	drawSky();
	drawRays3D();
	glutSwapBuffers();
	

}

void ButtonDown(unsigned char key,int x,int y)                                  //keyboard button pressed down
{
 if(key=='a'){ Keys.a=1;} 	
 if(key=='d'){ Keys.d=1;} 
 if(key=='w'){ Keys.w=1;}
 if(key=='s'){ Keys.s=1;}
 	if (key == 'e') //Open Door
 	{
 		int xo = 0; if (pdx < 0){xo = -25;} else {xo = 25;}
		int yo = 0; if (pdy < 0){yo = -25;} else {yo = 25;}
		int ipx = px/64.0, ipx_add_xo = (px + xo)/64.0;
		int ipy = py/64.0, ipy_add_yo = (py + yo)/64.0;
		if (mapW[ipy_add_yo * mapX + ipx_add_xo] == 12) 
		{
			mapW[ipy_add_yo * mapX + ipx_add_xo] = 0;
			//If pvalue at player offset equals 4(a door)
			//and e is pressed then set the door's map value to 0
		}
	}
 	glutPostRedisplay();
}
 
void ButtonUp(unsigned char key,int x,int y)                                    //keyboard button pressed up
{
 if(key=='a'){ Keys.a=0;} 	
 if(key=='d'){ Keys.d=0;} 
 if(key=='w'){ Keys.w=0;}
 if(key=='s'){ Keys.s=0;}
 glutPostRedisplay();
}

void resize (int w, int h)
{
	glutReshapeWindow(1024, 512);
}
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(1024, 512);
	glutInitWindowPosition(200,200);
	glutCreateWindow("Raycaster");
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutKeyboardFunc(ButtonDown);
	glutKeyboardUpFunc(ButtonUp);
	glutMainLoop();
	
}
