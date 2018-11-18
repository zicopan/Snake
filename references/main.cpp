#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/freeglut.h>
#endif

#include <cmath>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
//#include <GL/glut.h>
//#include <SOIL/SOIL.h>
#include <stdio.h>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <chrono>
#include <thread>

float yCamPos = 13.0f;
GLfloat lightAmbient[] = {0.5,0.5,0.5,1};
GLfloat lightDiffuse[] = {1,1,1,1};
GLfloat lightPosition[] = {0,3,0};

GLuint texture[2];
GLuint cube;

float cR = 0; float cG = 1; float cB = 0; //centre line color
float eR = 0; float eG = 1; float eB = 0; //edge lines color
float bR = 47; float bG = 181; float bB = 243; //default block color

enum Direction{
    D_LEFT, D_RIGHT, D_UP, D_DOWN
};

class Fruit{
    public:
        Fruit(int pX, int pZ){
            x = pX;
            z = pZ;
            angle = 0.0f;
        };
        ~Fruit();
        void draw(){
            angle+=2.0f;
            glPushMatrix();
            glTranslatef(-x,0,z);
            //glRotatef(45,0,0,1);
            glRotatef(angle,0,1,0);
            //glutSolidCube(1);
            glBindTexture(GL_TEXTURE_2D,texture[0]);
            float gC = 0.5;
            glColor3f(1,1,1);
            glBegin(GL_TRIANGLES);
                glVertex3f(0-gC,0-gC,0-gC);
                glVertex3f(1-gC,0-gC,0-gC);
                glVertex3f(0.5-gC,0-gC,1-gC);
                glTexCoord2f(0.5,1);glVertex3f(0.5-gC,1-gC,0.5-gC);
                glTexCoord2f(0,0); glVertex3f(0-gC,0-gC,0-gC);
                glTexCoord2f(1,0); glVertex3f(1-gC,0-gC,0-gC);
                glTexCoord2f(0.5,1);glVertex3f(0.5-gC,1-gC,0.5-gC);
                glTexCoord2f(1,0); glVertex3f(0-gC,0-gC,0-gC);
                glTexCoord2f(0,0); glVertex3f(0.5-gC,0-gC,1-gC);
                glTexCoord2f(0.5,1);glVertex3f(0.5-gC,1-gC,0.5-gC);
                glTexCoord2f(0,0); glVertex3f(1-gC,0-gC,0-gC);
                glTexCoord2f(1,0); glVertex3f(0.5-gC,0-gC,1-gC);
            glEnd();
            glBindTexture(GL_TEXTURE_2D,0);
            glPopMatrix();
        };
        float getX(){return x;};
        float getZ(){return z;};
        float setX(float pX){x = pX;};
        float setZ(float pZ){z = pZ;};
        float getAngle(){return angle;};
    private:
        float x,z,angle;
};

Fruit* fruit = new Fruit(1,1);

class Block{ //This class represents the individual blocks that make up the snake object in the game
    public:
        Block(float pX, float pY,float pZ, float pSize=1.0f){
            x = pX;
            y = pY;
            z = pZ;
            size = pSize;
            r = bR; g = bG; b = bB; //rgb colors are set to their default (initialized at the top)
        };
        ~Block();
        void setX(float pX){x=pX;}; //getter and setter functions for x, y and z
        void setY(float pY){y=pY;};
        void setZ(float pZ){z=pZ;};
        float getX(){return x;};
        float getY(){return y;};
        float getZ(){return z;};
        void setColor(float pR,float pG,float pB){r=pR;g=pG;b=pB;}; //... and color
        void draw(){ //function to draw the block
            glPushMatrix();
            glTranslatef(-x,y,z);
            if(r > 1 || g > 1 || b > 1){
                glColor3ub(r,g,b);
            }else{
                glColor3f(r,g,b);
            }
            glCallList(cube);
            glPopMatrix();
        };
        
    private:
        float x,y,z,size,r,g,b;
};

class Snake{ //contains the logic for the snake movement and collision
    public:
        std::vector<Block*> blocks; //snake is just an array of blocks

        Snake(float startX, float startZ, int blockCount){
            for(float i = 0; i < blockCount; i++){
                Block* temp = new Block(startX+i,0,startZ);
                if(i == 0)
                    temp->setColor(0,1,0); //snake head is set to green color
                blocks.push_back(temp);
            }
            isUpdated = false;
            currentDirection = D_LEFT;
            score = 0;
        }; 
        ~Snake();
        void draw(){ //loops through block array and calls their draw functions
            for(int i = 0; i <= blocks.size()-1; i++){
                Block* temp = blocks.at(i);
                temp->draw();
            }
        };

        void move(Direction direction){ //all snake movement handled in this and pushSnake() function below
            Block* snakeHead = blocks.at(0);
            if(direction == D_LEFT && currentDirection != D_RIGHT){
                pushSnake();
                snakeHead->setX(snakeHead->getX()-1.0f);
                currentDirection = direction;
                isUpdated = true;
            }else if(direction == D_RIGHT && currentDirection != D_LEFT){
                pushSnake();
                snakeHead->setX(snakeHead->getX()+1.0f);
                currentDirection = direction;
                isUpdated = true;
            }else if(direction == D_UP && currentDirection != D_DOWN){
                pushSnake();
                snakeHead->setZ(snakeHead->getZ()+1.0f);
                currentDirection = direction;
                isUpdated = true;
            }else if(direction == D_DOWN && currentDirection != D_UP){
                pushSnake();
                snakeHead->setZ(snakeHead->getZ()-1.0f);
                currentDirection = direction;
                isUpdated = true;
            }
        };

        void pushSnake(){ //doesn't change snake head position but advances all other blocks
            for(int i = blocks.size()-1; i >= 1; i--){
                Block* temp = blocks.at(i);
                Block* prevBlock = blocks.at(i-1);
                temp->setX(prevBlock->getX());
                temp->setZ(prevBlock->getZ());
            }
        };

        void update(){ //update function called at every frame
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); //improper fps-capping, will update later
            if(!isUpdated) //if movement wasn't already handled on keypress event, then handle it now
                move(currentDirection);
            isUpdated = false;
            collisionDetection();
        };

        void collisionDetection(){ //checks if snake head hit fruit or it's own body
            float x = blocks.at(0)->getX();
            float z = blocks.at(0)->getZ();
            if(fruit->getX() == x && fruit->getZ() == z){ //check for fruit collision
                score++; //increment score
                printf("Score = %i\n",score);
                bool repeat = false;
                do{ //makes sure that fruit doesn't spawn underneath body
                    repeat = false;
                    fruit->setX(rand()%11-5);
                    fruit->setZ(rand()%11-5);
                    for(int i = 0; i<=blocks.size()-1;i++){
                        x = blocks.at(i)->getX();
                        z = blocks.at(i)->getZ();
                        if(fruit->getX()==x && fruit->getZ()==z)
                            repeat = true;
                    }
                }while(repeat); //(bug) will crash after snake takes up whole grid
                addBlock();
            }
            for(int i = 1; i<= blocks.size()-1; i++){ //check for collision with own body
                if(blocks.at(i)->getX() == blocks.at(0)->getX() && blocks.at(i)->getZ() == blocks.at(0)->getZ()){
                    printf("collision\n"); //so far no game over screen so this is the only thing that happens...
                    blocks.at(i)->setColor(1,0,0); //... and collided block turns red.
                }
            }
        };

        void addBlock(){ //adds a new block to the snake
            Block* newBlock = new Block(-100,0, -100);
            blocks.push_back(newBlock);
        };

        Direction getDirection(){return currentDirection;};

    private:
        Direction currentDirection;
        bool isUpdated;
        int score;
};

Snake* snake = new Snake(0,0,3);

void buildDLs(){ //Build display lists, right now only compiles a cube
    cube = glGenLists(1);
    glNewList(cube,GL_COMPILE);
        glBegin(GL_QUADS);
            glNormal3f(0,0,1);
            glVertex3f(0.5,0.5,0.5); //back face
            glVertex3f(-0.5,0.5,0.5);
            glVertex3f(-0.5,-0.5,0.5);
            glVertex3f(0.5,-0.5,0.5);
            glNormal3f(0,0,-1);
            glVertex3f(0.5,0.5,-0.5); //front face
            glVertex3f(-0.5,0.5,-0.5);
            glVertex3f(-0.5,-0.5,-0.5);
            glVertex3f(0.5,-0.5,-0.5);
            glNormal3f(0,-1,0); //bottom face
            glVertex3f(-0.5,-0.5,-0.5);
            glVertex3f(-0.5,-0.5,0.5);
            glVertex3f(0.5,-0.5,0.5);
            glVertex3f(0.5,-0.5,-0.5);
            glNormal3f(0,1,0); //top face
            glVertex3f(-0.5,0.5,-0.5);
            glVertex3f(-0.5,0.5,0.5);
            glVertex3f(0.5,0.5,0.5);
            glVertex3f(0.5,0.5,-0.5);
            glNormal3f(-1,0,0); //right face
            glVertex3f(-0.5,0.5,-0.5);
            glVertex3f(-0.5,0.5,0.5);
            glVertex3f(-0.5,-0.5,0.5);
            glVertex3f(-0.5,-0.5,-0.5);
            glNormal3f(1,0,0); //left face
            glVertex3f(0.5,0.5,-0.5);
            glVertex3f(0.5,0.5,0.5);
            glVertex3f(0.5,-0.5,0.5);
            glVertex3f(0.5,-0.5,-0.5);
        glEnd();
    glEndList();
}

void drawGrid(void){ //hardcoded values for grid...
    glColor3ub(24,221,0);
    glLineWidth(2);
    glBegin(GL_LINES);
    for(float i = -7; i <= 5; i++){
        glVertex3f(-5-0.5,0-0.5f,i+0.5f);
        glVertex3f(6-0.5,0-0.5f,i+0.5f);
    }
    for(float i = -5; i <= 6; i++){
        glVertex3f(i-0.5,-0.5f,-6-0.5f);
        glVertex3f(i-0.5,-0.5f,6-0.5f);
    }
    glEnd();
    glLineWidth(1);
    glColor3ub(30,2,63);
    glBegin(GL_QUADS);
        glNormal3f(0,1,0);
        glVertex3f(-5-0.5,-0.51,-6.5);
        glVertex3f(-5-0.5,-0.51,6.5);
        glVertex3f(6-0.5,-0.51,6.5);
        glVertex3f(6-0.5,-0.51,-6.5);
    glEnd();

    glColor3ub(30,2,63);
    for(int i = -5; i <= 5; i++){
        glPushMatrix();
        glTranslatef(-6,0,i);
        glCallList(cube);

        glColor3f(cR,cG,cB);
        glLineWidth(10);
        glBegin(GL_LINES);
            glVertex3f(0,0.51,-0.5);
            glVertex3f(0,0.51,0.5);
        glEnd();
        glLineWidth(2);
        glBegin(GL_LINES);
            glVertex3f(0.5,0.51,-0.5);
            glVertex3f(0.5,0.51,0.5);
            glVertex3f(-0.5,0.51,-0.5);
            glVertex3f(-0.5,0.51,0.5);
        glEnd();
        glColor3ub(30,2,63);
        

        glPopMatrix();
        glPushMatrix();
        glTranslatef(6,0,i);
        glCallList(cube);

        glColor3f(cR,cG,cB);
        glLineWidth(10);
        glBegin(GL_LINES);
            glVertex3f(0,0.51,-0.5);
            glVertex3f(0,0.51,0.5);
        glEnd();
        glLineWidth(2);
        glBegin(GL_LINES);
            glVertex3f(0.5,0.51,-0.5);
            glVertex3f(0.5,0.51,0.5);
            glVertex3f(-0.5,0.51,-0.5);
            glVertex3f(-0.5,0.51,0.5);
        glEnd();
        glColor3ub(30,2,63);

        glPopMatrix();
        glPushMatrix();
        glTranslatef(i,0,-6);
        glCallList(cube);

        glColor3f(cR,cG,cB);
        glLineWidth(10);
        glBegin(GL_LINES);
            glVertex3f(-0.5,0.51,0);
            glVertex3f(0.5,0.51,0);
        glEnd();
        glLineWidth(2);
        glBegin(GL_LINES);
            glVertex3f(-0.5,0.51,0.5);
            glVertex3f(0.5,0.51,0.5);
            glVertex3f(-0.5,0.51,-0.5);
            glVertex3f(0.5,0.51,-0.5);
        glEnd();
        glColor3ub(30,2,63);

        glPopMatrix();
        glPushMatrix();
        glTranslatef(i,0,6);
        glCallList(cube);

        glColor3f(cR,cG,cB);
        glLineWidth(10);
        glBegin(GL_LINES);
            glVertex3f(-0.5,0.51,0);
            glVertex3f(0.5,0.51,0);
        glEnd();
        glLineWidth(2);
        glBegin(GL_LINES);
            glVertex3f(-0.5,0.51,0.5);
            glVertex3f(0.5,0.51,0.5);
            glVertex3f(-0.5,0.51,-0.5);
            glVertex3f(0.5,0.51,-0.5);
        glEnd();
        glColor3ub(30,2,63);

        glPopMatrix();
    }

    glPushMatrix();
    glTranslatef(6,0,6); // top left
    glCallList(cube);
    glColor3f(cR,cG,cB);
        glLineWidth(10);
        glBegin(GL_LINES);
            glVertex3f(-0.5,0.51,0);
            glVertex3f(0,0.51,0);
            glVertex3f(0,0.51,0);
            glVertex3f(0,0.51,-0.5);
        glEnd();
        glLineWidth(2);
        glBegin(GL_LINES);
            glVertex3f(-0.5,0.51,0.5);
            glVertex3f(0.5,0.51,0.5);
            glVertex3f(0.5,0.51,0.5);
            glVertex3f(0.5,0.51,-0.5);
            glVertex3f(-0.51,0.51,-0.51);
            glVertex3f(-0.51,-0.51,-0.51);
        glEnd();
        glColor3ub(30,2,63);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-6,0,6); // top right
    glCallList(cube);
    glColor3f(cR,cG,cB);
        glLineWidth(10);
        glBegin(GL_LINES);
            glVertex3f(0.5,0.51,0);
            glVertex3f(0,0.51,0);
            glVertex3f(0,0.51,0);
            glVertex3f(0,0.51,-0.5);
        glEnd();
        glLineWidth(2);
        glBegin(GL_LINES);
            glVertex3f(-0.5,0.51,0.5);
            glVertex3f(0.5,0.51,0.5);
            glVertex3f(-0.5,0.51,0.5);
            glVertex3f(-0.5,0.51,-0.5);
            glVertex3f(0.51,0.51,-0.51);
            glVertex3f(0.51,-0.51,-0.51);
        glEnd();
        glColor3ub(30,2,63);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-6,0,-6); // bottom right
    glCallList(cube);
    glColor3f(cR,cG,cB);
        glLineWidth(10);
        glBegin(GL_LINES);
            glVertex3f(0.5,0.51,0);
            glVertex3f(0,0.51,0);
            glVertex3f(0,0.51,0);
            glVertex3f(0,0.51,0.5);
        glEnd();
        glLineWidth(2);
        glBegin(GL_LINES);
            glVertex3f(-0.5,0.51,-0.5);
            glVertex3f(0.5,0.51,-0.5);
            glVertex3f(-0.5,0.51,0.5);
            glVertex3f(-0.5,0.51,-0.5);
            glVertex3f(0.51,0.51,0.51);
            glVertex3f(0.51,-0.51,0.51);
        glEnd();
        glColor3ub(30,2,63);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(6,0,-6); // bottom left
    glCallList(cube);
    glColor3f(cR,cG,cB);
        glLineWidth(10);
        glBegin(GL_LINES);
            glVertex3f(-0.5,0.51,0);
            glVertex3f(0,0.51,0);
            glVertex3f(0,0.51,0);
            glVertex3f(0,0.51,0.5);
        glEnd();
        glLineWidth(2);
        glBegin(GL_LINES);
            glVertex3f(-0.5,0.51,-0.5);
            glVertex3f(0.5,0.51,-0.5);
            glVertex3f(0.5,0.51,0.5);
            glVertex3f(0.5,0.51,-0.5);
            glVertex3f(-0.51,0.51,0.51);
            glVertex3f(-0.51,-0.51,0.51);
        glEnd();
        glColor3ub(30,2,63);
    glPopMatrix();
}

void drawSnakeLines(){ //hardcoded values for fancy snake lines
    Block* block = snake->blocks.at(0);
    Block* blockAfter = NULL;
    Block* blockBefore = NULL;
    glColor3f(cR,cG,cB);
    float c_lw = 20; //centre line width
    float e_lw = 3; //edge line width
    glLineWidth(3);
    if(snake->getDirection() == D_UP || snake->getDirection() == D_DOWN){
        glPushMatrix();
        glTranslatef(-block->getX(),block->getY(),block->getZ());
        glBegin(GL_LINES);
            //glVertex3f(0,0.51,-0.5);
            //glVertex3f(0,0.51,0.5);
            glColor3f(eR,eG,eB);
            glVertex3f(0.5,0.51,-0.5);
            glVertex3f(0.5,0.51,0.5);
            glVertex3f(-0.5,0.51,-0.5);
            glVertex3f(-0.5,0.51,0.5);
            if(snake->getDirection() == D_UP){
                glVertex3f(0.5,0.51,0.5);
                glVertex3f(-0.5,0.51,0.5);
            }else if(snake->getDirection() == D_DOWN){
                glVertex3f(0.5,0.51,-0.5);
                glVertex3f(-0.5,0.51,-0.5);
            }
        glEnd();
        glPopMatrix();
    }else{
        glPushMatrix();
        glTranslatef(-block->getX(),block->getY(),block->getZ());
        glBegin(GL_LINES);
            //glVertex3f(-0.5,0.51,0);
            //glVertex3f(0.5,0.51,0);
            glColor3f(eR,eG,eB);
            glVertex3f(-0.5,0.51,0.5);
            glVertex3f(0.5,0.51,0.5);
            glVertex3f(-0.5,0.51,-0.5);
            glVertex3f(0.5,0.51,-0.5);
            if(snake->getDirection() == D_LEFT){
                glVertex3f(0.5,0.51,-0.5);
                glVertex3f(0.5,0.51,0.5);
            }else if(snake->getDirection() == D_RIGHT){
                glVertex3f(-0.5,0.51,-0.5);
                glVertex3f(-0.5,0.51,0.5);
            }
        glEnd();
        glPopMatrix();
    }
    for(int i = 1; i <= snake->blocks.size()-2; i++){
        block = snake->blocks.at(i);
        blockAfter = snake->blocks.at(i+1);
        blockBefore = snake->blocks.at(i-1);
        float x = block->getX();
        float z = block->getZ();
        float aX = blockAfter->getX();
        float aZ = blockAfter->getZ();
        float bX = blockBefore->getX();
        float bZ = blockBefore->getZ();
        glPushMatrix();
        glTranslatef(-x,block->getY(),z);
        if(z != bZ && z != aZ){ //up or down
            glLineWidth(c_lw);
            glBegin(GL_LINES);
                glColor3f(cR,cG,cB);
                glVertex3f(0,0.51,-0.5); //centre line
                glVertex3f(0,0.51,0.5);
            glEnd();
            glLineWidth(e_lw);
            glBegin(GL_LINES);
                glColor3f(eR,eG,eB);
                glVertex3f(-0.5,0.51,-0.5); //edges below
                glVertex3f(-0.5,0.51,0.5);
                glVertex3f(0.5,0.51,-0.5);
                glVertex3f(0.5,0.51,0.5);
            glEnd();
            glPopMatrix();
        }else if(x != bX && x != aX){ //left or right
            glLineWidth(c_lw);
            glBegin(GL_LINES);
                glColor3f(cR,cG,cB);
                glVertex3f(-0.5,0.51,0); //centre line
                glVertex3f(0.5,0.51,0);
            glEnd();
            glLineWidth(e_lw);
            glBegin(GL_LINES);
                glColor3f(eR,eG,eB);
                glVertex3f(-0.5,0.51,-0.5); //edges below
                glVertex3f(0.5,0.51,-0.5);
                glVertex3f(-0.5,0.51,0.5);
                glVertex3f(0.5,0.51,0.5);
            glEnd();
        }else if(x > bX && z < aZ){  //b_right && a_up
            glLineWidth(c_lw);
            glBegin(GL_LINES);
                glColor3f(cR,cG,cB);
                glVertex3f(0,0.51,0);
                glVertex3f(0.5,0.51,0);
                glVertex3f(0,0.51,0.5);
                glVertex3f(0,0.51,0);
            glEnd();
            glLineWidth(e_lw);
            glBegin(GL_LINES);
                glColor3f(eR,eG,eB);
                glVertex3f(-0.5,0.51,-0.5);
                glVertex3f(0.5,0.51,-0.5);
                glVertex3f(-0.5,0.51,0.5);
                glVertex3f(-0.5,0.51,-0.5);
            glEnd();
        }else if(x < bX && z < aZ){ //b_left && a_up
            glLineWidth(c_lw);
            glBegin(GL_LINES);
                glColor3f(cR,cG,cB);
                glVertex3f(0,0.51,0);
                glVertex3f(-0.5,0.51,0);
                glVertex3f(0,0.51,0.5);
                glVertex3f(0,0.51,0);
            glEnd();
            glLineWidth(e_lw);
            glBegin(GL_LINES);
                glColor3f(eR,eG,eB);
                glVertex3f(0.5,0.51,-0.5);
                glVertex3f(-0.5,0.51,-0.5);
                glVertex3f(0.5,0.51,0.5);
                glVertex3f(0.5,0.51,-0.5);
            glEnd();
        }else if(x > bX && z > aZ){ //b_right && a_down
            glLineWidth(c_lw);
            glBegin(GL_LINES);
                glColor3f(cR,cG,cB);
                glVertex3f(0,0.51,0);
                glVertex3f(0.5,0.51,0);
                glVertex3f(0,0.51,-0.5);
                glVertex3f(0,0.51,0);
            glEnd();
            glLineWidth(e_lw);
            glBegin(GL_LINES);
                glColor3f(eR,eG,eB);
                glVertex3f(-0.5,0.51,0.5);
                glVertex3f(0.5,0.51,0.5);
                glVertex3f(-0.5,0.51,-0.5);
                glVertex3f(-0.5,0.51,0.5);
            glEnd();
        }else if(x < bX && z > aZ){ //b_left && a_down
            glLineWidth(c_lw);
            glBegin(GL_LINES);
                glColor3f(cR,cG,cB);
                glVertex3f(0,0.51,0);
                glVertex3f(-0.5,0.51,0);
                glVertex3f(0,0.51,-0.5);
                glVertex3f(0,0.51,0);
            glEnd();
            glLineWidth(e_lw);
            glBegin(GL_LINES);
                glColor3f(eR,eG,eB);
                glVertex3f(0.5,0.51,0.5);
                glVertex3f(-0.5,0.51,0.5);
                glVertex3f(0.5,0.51,-0.5);
                glVertex3f(0.5,0.51,0.5);
            glEnd();
        }else if(x < aX && z < bZ){  //a_right && b_up
            glLineWidth(c_lw);
            glBegin(GL_LINES);
                glColor3f(cR,cG,cB);
                glVertex3f(0,0.51,0);
                glVertex3f(-0.5,0.51,0);
                glVertex3f(0,0.51,0.5);
                glVertex3f(0,0.51,0);
            glEnd();
            glLineWidth(e_lw);
            glBegin(GL_LINES);
                glColor3f(eR,eG,eB);
                glVertex3f(0.5,0.51,-0.5);
                glVertex3f(-0.5,0.51,-0.5);
                glVertex3f(0.5,0.51,0.5);
                glVertex3f(0.5,0.51,-0.5);
            glEnd();
        }else if(x > aX && z < bZ){ //a_left && b_up
            glLineWidth(c_lw);
            glBegin(GL_LINES);
                glColor3f(cR,cG,cB);
                glVertex3f(0,0.51,0);
                glVertex3f(0.5,0.51,0);
                glVertex3f(0,0.51,0.5);
                glVertex3f(0,0.51,0);
            glEnd();
            glLineWidth(e_lw);
            glBegin(GL_LINES);
                glColor3f(eR,eG,eB);
                glVertex3f(-0.5,0.51,-0.5);
                glVertex3f(0.5,0.51,-0.5);
                glVertex3f(-0.5,0.51,0.5);
                glVertex3f(-0.5,0.51,-0.5);
            glEnd();
        }else if(x < aX && z > bZ){ //a_right && b_down
            glLineWidth(c_lw);
            glBegin(GL_LINES);
                glColor3f(cR,cG,cB);
                glVertex3f(0,0.51,0);
                glVertex3f(-0.5,0.51,0);
                glVertex3f(0,0.51,-0.5);
                glVertex3f(0,0.51,0);
            glEnd();
            glLineWidth(e_lw);
            glBegin(GL_LINES);
                glColor3f(eR,eG,eB);
                glVertex3f(0.5,0.51,0.5);
                glVertex3f(-0.5,0.51,0.5);
                glVertex3f(0.5,0.51,-0.5);
                glVertex3f(0.5,0.51,0.5);
            glEnd();
        }else if(x > aX && z > bZ){ //a_left && b_down
            glLineWidth(c_lw);
            glBegin(GL_LINES);
                glColor3f(cR,cG,cB);
                glVertex3f(0,0.51,0);
                glVertex3f(0.5,0.51,0);
                glVertex3f(0,0.51,-0.5);
                glVertex3f(0,0.51,0);
            glEnd();
            glLineWidth(e_lw);
            glBegin(GL_LINES);
                glColor3f(eR,eG,eB);
                glVertex3f(-0.5,0.51,0.5);
                glVertex3f(0.5,0.51,0.5);
                glVertex3f(-0.5,0.51,-0.5);
                glVertex3f(-0.5,0.51,0.5);
            glEnd();
        }
        glPopMatrix();
    }
    block = snake->blocks.at(snake->blocks.size()-1); //last block
    blockBefore = snake->blocks.at(snake->blocks.size()-2);
    glPushMatrix();
    glTranslatef(-block->getX(),block->getY(),block->getZ());
    if(block->getX() > blockBefore->getX()){ //left
        glLineWidth(c_lw);
        glBegin(GL_LINES);
            glColor3f(cR,cG,cB);
            glVertex3f(0,0.51,0);
            glVertex3f(0.5,0.51,0);
        glEnd();
        glLineWidth(e_lw);
        glBegin(GL_LINES);
            glColor3f(eR,eG,eB);
            glVertex3f(-0.5,0.51,0.5);
            glVertex3f(0.5,0.51,0.5);
            glVertex3f(-0.5,0.51,-0.5);
            glVertex3f(0.5,0.51,-0.5);
            glVertex3f(-0.5,0.51,-0.5);
            glVertex3f(-0.5,0.51,0.5);
        glEnd();
    }else if(block->getX() < blockBefore->getX()){ //right
        glLineWidth(c_lw);
        glBegin(GL_LINES);
            glColor3f(cR,cG,cB);
            glVertex3f(-0.5,0.51,0);
            glVertex3f(0,0.51,0);
        glEnd();
        glLineWidth(e_lw);
        glBegin(GL_LINES);
            glColor3f(eR,eG,eB);
            glVertex3f(-0.5,0.51,0.5);
            glVertex3f(0.5,0.51,0.5);
            glVertex3f(-0.5,0.51,-0.5);
            glVertex3f(0.5,0.51,-0.5);
            glVertex3f(0.5,0.51,-0.5);
            glVertex3f(0.5,0.51,0.5);
        glEnd();
    }else if(block->getZ() < blockBefore->getZ()){ //up
        glLineWidth(c_lw);
        glBegin(GL_LINES);
            glColor3f(cR,cG,cB);
            glVertex3f(0,0.51,0);
            glVertex3f(0,0.51,0.5);
        glEnd();
        glLineWidth(e_lw);
        glBegin(GL_LINES);
            glColor3f(eR,eG,eB);
            glVertex3f(0.5,0.51,-0.5);
            glVertex3f(0.5,0.51,0.5);
            glVertex3f(-0.5,0.51,-0.5);
            glVertex3f(-0.5,0.51,0.5);
            glVertex3f(0.5,0.51,-0.5);
            glVertex3f(-0.5,0.51,-0.5);
        glEnd();
    }else if(block->getZ() > blockBefore->getZ()){ //down
        glLineWidth(c_lw);
        glBegin(GL_LINES);
            glColor3f(cR,cG,cB);
            glVertex3f(0,0.51,-0.5);
            glVertex3f(0,0.51,0);
        glEnd();
        glLineWidth(e_lw);
        glBegin(GL_LINES);
            glColor3f(eR,eG,eB);
            glVertex3f(0.5,0.51,-0.5);
            glVertex3f(0.5,0.51,0.5);
            glVertex3f(-0.5,0.51,-0.5);
            glVertex3f(-0.5,0.51,0.5);
            glVertex3f(0.5,0.51,0.5);
            glVertex3f(-0.5,0.51,0.5);
        glEnd();
    }
    glPopMatrix();
    glLineWidth(1);
}

void draw(void){ //GLUT doesn't let me change it's main loop so the drawing loop is also where the game update logic goes too

    snake->update(); //update snake position
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(
        0,yCamPos,-5, //eye
        0,0,0,  //centre
        0,1,0   //up
    );
    drawGrid();
    snake->draw();
    drawSnakeLines();
    fruit->draw();
    glutSwapBuffers();
}

void resize(int w, int h){ //function called on resize of window
    if(h == 0)
        h = 1;
    float ratio = w*1.0f/h;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0,0,w,h);
    gluPerspective(50,ratio,1,200);
    glMatrixMode(GL_MODELVIEW);
}

void keyEvents(unsigned char key, int x, int y){ //function called when normal keys are pressed
    switch(key){
        case 27: //escape key
            exit(0);
            break;
    }
}

void specialKeys(int key, int x, int y){ //function called when special keys are pressed
    switch(key){
        case GLUT_KEY_F1:
            exit(0);
            break;
        case GLUT_KEY_LEFT:
            snake->move(D_LEFT);
            break;
        case GLUT_KEY_RIGHT:
            snake->move(D_RIGHT);
            break;
        case GLUT_KEY_UP:
            snake->move(D_UP);
            break;
        case GLUT_KEY_DOWN:
            snake->move(D_DOWN);
            break;
    }
}

/*void loadGLTexture(std::string filePath, unsigned int textureNum){ //function loads textures
    texture[textureNum] = SOIL_load_OGL_texture(
    filePath.c_str(),
    SOIL_LOAD_AUTO,
    SOIL_CREATE_NEW_ID,
    SOIL_FLAG_INVERT_Y
    );
    glBindTexture(GL_TEXTURE_2D,texture[textureNum]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D,0);
}*/

void initGL(void){
    //loadGLTexture("luminati.png",0);
    //loadGLTexture("swag2.jpg",1);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glLightfv(GL_LIGHT1,GL_AMBIENT,lightAmbient);
    glLightfv(GL_LIGHT1,GL_DIFFUSE,lightDiffuse);
    glLightfv(GL_LIGHT1,GL_POSITION,lightPosition);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_TEXTURE_2D);
}

void mouseEvents(int button, int dir, int x, int y){ //function called on mouse events
    switch(button){
        case 3: //scroll up and zoom in
            printf("zoom in\n");
            yCamPos -= 0.5;
            break;
        case 4: //scroll down and zoom out
            printf("zoom out\n");
            yCamPos += 0.5;
            break;
    }
}

int main(int argc, char **argv){
    srand(time(NULL));
    glutInit(&argc,argv);
    glutInitWindowPosition(-1,-1);
    glutInitWindowSize(600,600);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutCreateWindow("Swag");

    initGL();
    buildDLs();

    glutDisplayFunc(draw);
    glutIdleFunc(draw);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keyEvents);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouseEvents);

    glutMainLoop();

    return 1;
}
