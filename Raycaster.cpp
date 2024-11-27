#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>
#include "Textures/Brick1.ppm"
#include "Textures/Map_Textures.ppm"
#include "Textures/Sky_Texture.ppm"
#include "Textures/StartScreen.ppm"
#include "Textures/WinScreen.ppm"
#include "Textures/GameOver.ppm"
#include "Textures/Sprite_Sheet.ppm"
#define PI 3.1415926535
#define P2 PI/2
#define P3 3*PI/2
#define DR 0.008726646 //One degree in radians

typedef struct
{
	int w,a,d,s; //Button state
}
ButtonKeys; ButtonKeys Keys;
typedef struct       //All veriables per sprite
{
	 int type;           //static, key, enemy
	 int state;          //on off
	 int map;            //texture to show
	 float x,y,z;        //position
}sprite; sprite sp[4];
int depth[120];      //hold wall line depth to compare for sprite depth


float px, py, pdx, pdy, pa; //Player position
float degToRad(float a){ return a*PI/180.0;} //Convert degrees into radians
float FixAng(float a){ if(a>359){ a-=360;} if(a<0){ a+=360;} return a;}
float distance(float ax, float ay, float bx,float by,float ang)
{ return cos(degToRad(ang))*(bx-ax)-sin(degToRad(ang))*(by-ay);}
float frame1, frame2, fps; //Used to compare the difference between frames for movement calculations
int gameState=0, timer=0; //game state. init, start screen, game loop, win/lose
float fade=0;             //the 3 screens can fade up from black
float turnSpeed, walkSpeed; //Player movement parameters

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

//------------------MAP------------------

	//----------Map Size Parameters------
	int mapX = 8; //Map Width
	int mapY = 8; //Map Height
	int mapS = 64; //Map Unit Size

	/*
		Unit Value Key:
		0 = Empty Space
		1 = Brick1
		2 = Brick1_Grate
		3 = Brick2
		4 = Brick2_Grate
		5 = Metal1
		6 = Metal2
		7 = Tile_Grey
		8 = Tile_Tan
		9 = Tile_Green
		10 = Stone
		11 = CeilingTile
		12 - Door
		13 - ExitDoor
	*/

	//----------Map Wall Layout----------
	int mapW[] = //Map Wall Layout 
	{
		1,1,1,1,1,2,1,1,
		13,0,0,12,0,0,0,1,
		1,0,0,3, 0,0,0,2,
		1,0,0,3, 0,0,0,1,
		1,3,12,3, 0,1,1,1,
		1,0,0,3, 0,0,0,1,
		1,0,0,3, 0,0,0,1,
		1,1,1,1,1,2,1,1,
	};
	//----------Map Floor Layout--------
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
	//----------Map Ceiling Layout------
	int mapC[]=	//Map Ceiling Layout
	{
	 	10,10,10,0,0,0,0,0,
	 	10,10,10,10,0,0,0,0,
	 	10,10,10,0,0,0,0,0,
	 	10,10,10,0,0,0,0,0,
	 	10,10,10,0,0,0,0,0,
	 	10,10,10,0,0,0,0,0,
	 	10,10,10,0,0,0,0,0,
	 	10,10,10,0,0,0,0,0,	
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
void drawRays2D()
{
	int r,mx,my,mp,dof,side; float vx,vy,rx,ry,ra,xo,yo,disV,disH; 
 
 	ra=FixAng(pa+30);                                                              //ray set back 30 degrees
 
	 for(r=0;r<120;r++)
	 {
		  int vmt=0,hmt=0;                                                              //vertical and horizontal map texture number 
		  //---Vertical--- 
		  dof=0; side=0; disV=100000;
		  float Tan=tan(degToRad(ra));
		       if(cos(degToRad(ra))> 0.001){ rx=(((int)px>>6)<<6)+64;      ry=(px-rx)*Tan+py; xo= 64; yo=-xo*Tan;}//looking left
		  else if(cos(degToRad(ra))<-0.001){ rx=(((int)px>>6)<<6) -0.0001; ry=(px-rx)*Tan+py; xo=-64; yo=-xo*Tan;}//looking right
		  else { rx=px; ry=py; dof=8;}                                                  //looking up or down. no hit  
		
		  while(dof<8) 
		  { 
			   mx=(int)(rx)>>6; my=(int)(ry)>>6; mp=my*mapX+mx;                     
			   if(mp>0 && mp<mapX*mapY && mapW[mp]>0){ vmt=mapW[mp]-1; dof=8; disV=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}//hit         
			   else{ rx+=xo; ry+=yo; dof+=1;}                                               //check next horizontal
		  } 
		  vx=rx; vy=ry;
		
		  //---Horizontal---
		  dof=0; disH=100000;
		  Tan=1.0/Tan; 
		       if(sin(degToRad(ra))> 0.001){ ry=(((int)py>>6)<<6) -0.0001; rx=(py-ry)*Tan+px; yo=-64; xo=-yo*Tan;}//looking up 
		  else if(sin(degToRad(ra))<-0.001){ ry=(((int)py>>6)<<6)+64;      rx=(py-ry)*Tan+px; yo= 64; xo=-yo*Tan;}//looking down
		  else{ rx=px; ry=py; dof=8;}                                                   //looking straight left or right
		 
		  while(dof<8) 
		  { 
			   mx=(int)(rx)>>6; my=(int)(ry)>>6; mp=my*mapX+mx;                          
			   if(mp>0 && mp<mapX*mapY && mapW[mp]>0){ hmt=mapW[mp]-1; dof=8; disH=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}//hit         
			   else{ rx+=xo; ry+=yo; dof+=1;}                                               //check next horizontal
		  } 
		  
		  float shade=1;
		  glColor3f(0,0.8,0);
		  if(disV<disH){ hmt=vmt; shade=0.5; rx=vx; ry=vy; disH=disV; glColor3f(0,0.6,0);}//horizontal hit first
		    
		  int ca=FixAng(pa-ra); disH=disH*cos(degToRad(ca));                            //fix fisheye 
		  int lineH = (mapS*640)/(disH); 
		  float ty_step=32.0/(float)lineH; 
		  float ty_off=0; 
		  if(lineH>640){ ty_off=(lineH-640)/2.0; lineH=640;}                            //line height and limit
		  int lineOff = 320 - (lineH>>1);                                               //line offset
		
		  depth[r]=disH; //save this line's depth
		  //---draw walls---
		  int y;
		  float ty=ty_off*ty_step;//+hmt*32;
		  float tx;
		  if(shade==1){ tx=(int)(rx/2.0)%32; if(ra>180){ tx=31-tx;}}  
		  else        { tx=(int)(ry/2.0)%32; if(ra>90 && ra<270){ tx=31-tx;}}
		  for(y=0;y<lineH;y++)
		  {
			   int pixel=((int)ty*32+(int)tx)*3+(hmt*32*32*3);
			   int red   =Map_Textures[pixel+0]*shade;
			   int green =Map_Textures[pixel+1]*shade;
			   int blue  =Map_Textures[pixel+2]*shade;
			   glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(r*8,y+lineOff); glEnd();
			   ty+=ty_step;
		  }
		 
		  //---draw floors---
		 for(y=lineOff+lineH;y<640;y++)
		 {
			  float dy=y-(640/2.0), deg=degToRad(ra), raFix=cos(degToRad(FixAng(pa-ra)));
			  tx=px/2 + cos(deg)*158*2*32/dy/raFix;
			  ty=py/2 - sin(deg)*158*2*32/dy/raFix;
			  int mp=mapF[(int)(ty/32.0)*mapX+(int)(tx/32.0)]*32*32;
			  int pixel=(((int)(ty)&31)*32 + ((int)(tx)&31))*3+mp*3;
			  int red   =Map_Textures[pixel+0]*0.7;
			  int green =Map_Textures[pixel+1]*0.7;
			  int blue  =Map_Textures[pixel+2]*0.7;
			  glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(r*8,y); glEnd();
			
			 //---draw ceiling---
			  mp=mapC[(int)(ty/32.0)*mapX+(int)(tx/32.0)]*32*32;
			  pixel=(((int)(ty)&31)*32 + ((int)(tx)&31))*3+mp*3;
			  red   =Map_Textures[pixel+0] * 0.4;
			  green =Map_Textures[pixel+1] * 0.4;
			  blue  =Map_Textures[pixel+2] * 0.4;
			  if(mp>0){ glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(r*8,640-y); glEnd();}
		 }
		 
		 ra=FixAng(ra-0.5);                                                               //go to next ray, 60 total
 	}
}

void drawSprite()
{
	 int x,y,s;
	 if(px<sp[0].x+30 && px>sp[0].x-30 && py<sp[0].y+30 && py>sp[0].y-30){ sp[0].state=0;} //pick up key 	
	 if(px<sp[3].x+30 && px>sp[3].x-30 && py<sp[3].y+30 && py>sp[3].y-30){ gameState=4;} //enemy kills
	
	 //enemy attack
	 int spx=(int)sp[3].x>>6,          spy=(int)sp[3].y>>6;          //normal grid position
	 int spx_add=((int)sp[3].x+15)>>6, spy_add=((int)sp[3].y+15)>>6; //normal grid position plus     offset
	 int spx_sub=((int)sp[3].x-15)>>6, spy_sub=((int)sp[3].y-15)>>6; //normal grid position subtract offset
	 if(sp[3].x>px && mapW[spy*8+spx_sub]==0){ sp[3].x-=0.04*fps;}
	 if(sp[3].x<px && mapW[spy*8+spx_add]==0){ sp[3].x+=0.04*fps;}
	 if(sp[3].y>py && mapW[spy_sub*8+spx]==0){ sp[3].y-=0.04*fps;}
	 if(sp[3].y<py && mapW[spy_add*8+spx]==0){ sp[3].y+=0.04*fps;}
	
	 for(s=0;s<4;s++)
	 {
		  float sx=sp[s].x-px; //temp float variables
		  float sy=sp[s].y-py;
		  float sz=sp[s].z;
		
		  float CS=cos(degToRad(pa)), SN=sin(degToRad(pa)); //rotate around origin
		  float a=sy*CS+sx*SN; 
		  float b=sx*CS-sy*SN; 
		  sx=a; sy=b;
		
		  sx=(sx*108.0/sy)+(120/2); //convert to screen x,y
		  sy=(sz*108.0/sy)+( 80/2);
		
		  int scale=32*80/b;   //scale sprite based on distance
		  if(scale<0){ scale=0;} if(scale>120){ scale=120;}  
		
		  //texture
		  float t_x=0, t_y=31, t_x_step=31.5/(float)scale, t_y_step=32.0/(float)scale;
		
		  for(x=sx-scale/2;x<sx+scale/2;x++)
		  {
			   t_y=31;
			   for(y=0;y<scale;y++)
			   {
				    if(sp[s].state==1 && x>0 && x<120 && b<depth[x])
				    {
					     int pixel=((int)t_y*32+(int)t_x)*3+(sp[s].map*32*32*3);
					     int red   =Sprite_Sheet[pixel+0];
					     int green =Sprite_Sheet[pixel+1];
					     int blue  =Sprite_Sheet[pixel+2];
					     if(red!=255, green!=0, blue!=255) //dont draw if purple
					     {
					      	glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(x*8,sy*8-y*8); glEnd(); //draw point 
					     }
					     t_y-=t_y_step; if(t_y<0){ t_y=0;}
				    }
			   }
			   t_x+=t_x_step;
		  }
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
	    	glPointSize(8); glColor3ub(red, green, blue); glBegin(GL_POINTS); glVertex2i(x * 8, y * 8); glEnd();
		}
	}
}

void screen(int v) //draw any full screen image. 120x80 pixels
{
	 int x,y;
	 int *T;
	 if(v==1){ T=StartScreen;}
	 if(v==2){ T=WinScreen;}
	 if(v==3){ T=GameOver;}
	 for(y=0;y<80;y++)
	 {
		  for(x=0;x<120;x++)
		  {
			   int pixel=(y*120+x)*3;
			   int red   =T[pixel+0]*fade;
			   int green =T[pixel+1]*fade;
			   int blue  =T[pixel+2]*fade;
			   glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(x*8,y*8); glEnd();
		  }	
	 }	
	 if(fade<1){ fade+=0.001*fps;} 
	 if(fade>1){ fade=1;}
}


void ButtonDown(unsigned char key,int x,int y)	//keyboard button pressed down
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

void init()
{
	 glClearColor(0.3,0.3,0.3,0);
	 px=300; py=400; pa=90;
	 pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));                                 //init player
	 mapW[11]=12; mapW[34]=12; //close doors
	
	
	 sp[0].type=1; sp[0].state=1; sp[0].map=0; sp[0].x=1.5*64; sp[0].y=6*64;   sp[0].z=20; //key
	 sp[1].type=2; sp[1].state=1; sp[1].map=1; sp[1].x=2*64; sp[1].y=6*64; sp[1].z= 0; //light 1
	 sp[2].type=2; sp[2].state=1; sp[2].map=1; sp[2].x=2*64; sp[2].y=2.5*64; sp[2].z= 0; //light 2
	 //sp[3].type=3; sp[3].state=1; sp[3].map=2; sp[3].x=2.5*64; sp[3].y=2*64;   sp[3].z=20; //enemy

}

void display()
{
	 //frames per second
	 frame2=glutGet(GLUT_ELAPSED_TIME); fps=(frame2-frame1); frame1=glutGet(GLUT_ELAPSED_TIME); 
	 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	 
	 if(gameState==0){ init(); fade=0; timer=0; gameState=1;} //init game
	 if(gameState==1){ screen(1); timer+=1*fps; if(timer>2000){ fade=0; timer=0; gameState=2;}} //start screen
	 if(gameState==2) //The main game loop
	 {
	  //buttons
	  if(Keys.a==1){ pa+=0.2*fps; pa=FixAng(pa); pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));} 	
	  if(Keys.d==1){ pa-=0.2*fps; pa=FixAng(pa); pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));} 
	
	  int xo=0; if(pdx<0){ xo=-20;} else{ xo=20;}                                    //x offset to check map
	  int yo=0; if(pdy<0){ yo=-20;} else{ yo=20;}                                    //y offset to check map
	  int ipx=px/64.0, ipx_add_xo=(px+xo)/64.0, ipx_sub_xo=(px-xo)/64.0;             //x position and offset
	  int ipy=py/64.0, ipy_add_yo=(py+yo)/64.0, ipy_sub_yo=(py-yo)/64.0;             //y position and offset
	  if(Keys.w==1)                                                                  //move forward
	  {  
	   if(mapW[ipy*mapX        + ipx_add_xo]==0){ px+=pdx*0.2*fps;}
	   if(mapW[ipy_add_yo*mapX + ipx       ]==0){ py+=pdy*0.2*fps;}
	  }
	  if(Keys.s==1)                                                                  //move backward
	  { 
	   if(mapW[ipy*mapX        + ipx_sub_xo]==0){ px-=pdx*0.2*fps;}
	   if(mapW[ipy_sub_yo*mapX + ipx       ]==0){ py-=pdy*0.2*fps;}
	  } 
	  drawSky();
	  drawRays2D();
	  drawSprite();
	  if( ((int)px>>6==1 && (int)py>>6==1 ) && sp[0].state==0){ fade=0; timer=0; gameState=3;} //Entered block 1, Win game!!
	 }
	
	 if(gameState==3){ screen(2); timer+=1*fps; if(timer>2000){ fade=0; timer=0; gameState=0;}} //won screen
	 if(gameState==4){ screen(3); timer+=1*fps; if(timer>2000){ fade=0; timer=0; gameState=0;}} //lost screen
	
	 glutPostRedisplay();
	 glutSwapBuffers();  
}

void resize (int w, int h)
{
	glutReshapeWindow(960, 640);
}

int main(int argc, char** argv)
{
	 glutInit(&argc, argv);
	 glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	 glutInitWindowSize(960,640);
	 glutInitWindowPosition( glutGet(GLUT_SCREEN_WIDTH)/2-960/2 ,glutGet(GLUT_SCREEN_HEIGHT)/2-640/2 );
	 glutCreateWindow("Raycaster");
	 gluOrtho2D(0,960,640,0);
	 init();
	 glutDisplayFunc(display);
	 glutReshapeFunc(resize);
	 glutKeyboardFunc(ButtonDown);
	 glutKeyboardUpFunc(ButtonUp);
	 glutMainLoop();
}

