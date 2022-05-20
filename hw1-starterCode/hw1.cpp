/*
  CSCI 420 Computer Graphics, USC
  Assignment 1: Height Fields with Shaders.
  C++ starter code

  Student username: <9566797656 Hsuan Yeh>
*/

#include "basicPipelineProgram.h"
#include "openGLMatrix.h"
#include "imageIO.h"
#include "openGLHeader.h"
#include "glutHeader.h"

#include <iostream>
#include <cstring>
#include <vector>
#include <cmath>
#include <ctime>

#if defined(WIN32) || defined(_WIN32)
  #ifdef _DEBUG
    #pragma comment(lib, "glew32d.lib")
  #else
    #pragma comment(lib, "glew32.lib")
  #endif
#endif

#if defined(WIN32) || defined(_WIN32)
  char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
  char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

using namespace std;

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// TYPE
typedef enum { POINTS, LINES, TRIANGLES, SMOOTHENED} DRAW_TYPE;
DRAW_TYPE type = POINTS;


// state of the world
float landRotate[3] = { 0.0f, 0.0f, 0.0f };// rotate from x,y,z axis
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework I";

ImageIO * heightmapImage;

//color image and grayscale
int BytesPerPixel ;
//BytesPerPixel == 1

//GLuint triVertexBuffer, triColorVertexBuffer;
//GLuint triVertexArray;
//int sizeTri;

OpenGLMatrix matrix;
BasicPipelineProgram * pipelineProgram;

// VBOs and VAOs
GLuint vboPoint, vboLine, vboTriangle;
GLuint vaoPoint, vaoLine, vaoTriangle;

GLuint vboP_left, vboP_right, vboP_up, vboP_down, vboP_triangle;
GLuint vaoSmoothTriangle;

vector<float> pointPosition, pointColor;
vector<float> linePosition, lineColor;
vector<float> trianglePosition, triangleColor;

vector<float> p_left, p_right, p_up, p_down, p_triangle, c_triangle;

int imageWidth, imageHeight;
float height, scale, r, g, b=1.0;

// write a screenshot to the specified filename
void saveScreenshot(const char * filename)
{
  unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
  glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

  ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

  if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
    cout << "File " << filename << " saved successfully." << endl;
  else cout << "Failed to save file " << filename << '.' << endl;

  delete [] screenshotData;
}



void displayFunc()
{
    // render some stuff...
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    matrix.SetMatrixMode(OpenGLMatrix::ModelView);
    matrix.LoadIdentity();
    //matrix.LookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);
    int lookAtZ = (imageWidth < 200)? 0 : (imageWidth < 300)? 200: (imageWidth < 500)? 380: (imageWidth < 700)? 550 : 650;//scaling
    //int xy = (imageWidth < 200)? 128 : (imageWidth < 500)? 150: (imageWidth < 700)? 200 : 300;//scaling
    matrix.LookAt(128, 128, lookAtZ, 0, 0, 0, 0, 1, 0);
    //the smallest jpg is 128*128
    //camera position, which the camera is looking at, up vector
    //camera position need to correspond with pixel position
    //Scaling with the windowWidth
    
    
    //ch8
    //transform
    matrix.Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
    matrix.Rotate(landRotate[0], 1.0, 0.0, 0.0);//x
    matrix.Rotate(landRotate[1], 0.0, 1.0, 0.0);//y
    matrix.Rotate(landRotate[2], 0.0, 0.0, 1.0);//z
    matrix.Scale(landScale[0], landScale[1], landScale[2]);
    
    
    
    float m[16];
    matrix.SetMatrixMode(OpenGLMatrix::ModelView);
    matrix.GetMatrix(m);

    float p[16];
    matrix.SetMatrixMode(OpenGLMatrix::Projection);
    matrix.GetMatrix(p);
    //
    // bind shader
    pipelineProgram->Bind();

    // set variable
    pipelineProgram->SetModelViewMatrix(m);
    pipelineProgram->SetProjectionMatrix(p);

    //glBindVertexArray(triVertexArray);
    //glDrawArrays(GL_POINTS, 0, sizeTri);
    
    switch(type)
    {
        case POINTS:
            glBindVertexArray(vaoPoint);// bind vao
            //GLint first = 0;
            //GLsizei count = numVertices;
            //glDrawArrays(GL_TRIANGLES, first, count);
            glDrawArrays(GL_POINTS, 0, pointPosition.size()/3);//count 3 times
            glBindVertexArray(0);//unbind vao
            break;
        
        case LINES:
            glBindVertexArray(vaoLine);
            //glDrawArrays(GL_LINES, 0, sizeTri);
            glDrawArrays(GL_LINES, 0, linePosition.size()/3);
            glBindVertexArray(0);//unbind vao
            break;

        case TRIANGLES:
            glBindVertexArray(vaoTriangle);
            //glDrawArrays(GL_TRIANGLES, 0, sizeTri);
            glDrawArrays(GL_TRIANGLES, 0, trianglePosition.size()/3);
            glBindVertexArray(0);//unbind vao
            break;
        
        case SMOOTHENED:
            glBindVertexArray(vaoSmoothTriangle);
            //glDrawArrays(GL_TRIANGLES, 0, sizeTri);
            glDrawArrays(GL_TRIANGLES, 0, p_triangle.size()/3);
            glBindVertexArray(0);//unbind vao
            break;
         
    }
    
    glBindVertexArray(0);// unbind vao
    glutSwapBuffers();
}

bool rota = false;
bool autoup = false;
bool restore = false;
bool start = false;
int cu=0;
int cd=0;
int cr=0;
int screenshot=0;
bool animation = false;
bool takeshot = false;
bool animation4 = false;
bool take4shot = false;
int cou=0;
int co=241;
int c=244;//c=0,61,122,183,244 to generate 000 to 299 image

void idleFunc()
{
    // do some stuff...

    // for example, here, you can save the screenshots to disk (to make the animation)

    // make the screen update
    
    //make it scale up and down to look funny, and slowly rotate it with y-axis to show the outcome
    if(start){
        if(autoup && landScale[0] <= 4.0f){
            landScale[0] += 0.05f;
            landScale[1] += 0.05f;
            cu++;
            if(cu==24){
                autoup = false;
                restore = true;
                cu=0;
            }
        }
        
        if(restore && landScale[0] >= 1.0f){
            landScale[0] -= 0.05f;
            landScale[1] -= 0.05f;
            cd++;
            if(cd==24){
                autoup = true;
                restore = false;
                cd=0;
            }
        }
    }
    
    if(rota){
        landRotate[1] += 0.6f;
        cr++;
        if(cr==1200){
            rota = false;
            cr=0;
        }
    }
    
    if(animation || takeshot){
        
        GLint loc = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "mode");
        if(screenshot > 240){
            landScale[0] = 1.0f;
            landScale[1] = 1.0f;
            landRotate[1] = 0.0f;
            landTranslate[0] = 0.0f;
            landTranslate[1] = 0.0f;
            landTranslate[2] = 0.0f;
            animation = false;
            takeshot = false;
            screenshot = 0;
            type = TRIANGLES;
            glUniform1i(loc,0);
        }
        else if(screenshot <= 30){ //screenshots for POINT
            type = POINTS;
            glUniform1i(loc,0);
        }
        else if(screenshot <= 60){ //screenshots for LINE
            type = LINES;
            glUniform1i(loc,0);
        }
        else if(screenshot <= 120){ //screenshots for TRIANGLE
            type = TRIANGLES;
            glUniform1i(loc,0);
        }
        else if(screenshot <= 180){ //screenshots for SMOOTH
            landScale[0] = 1.0f;
            landScale[1] = 1.0f;
            type = SMOOTHENED;
            glUniform1i(loc,1);
        }
        else if(screenshot <= 240){ //screenshots for TRIANGLE
            if(screenshot <= 210){
               landTranslate[0] += 0.5f;
               landTranslate[1] -= 0.5f;
               landTranslate[2] += 0.8f;
           }else{
               landTranslate[0] -= 0.5f;
               landTranslate[1] += 0.5f;
               landTranslate[2] -= 0.8f;
           }
        }
        screenshot++;
    }
    
    
    if(animation4 || take4shot){
        GLint loc = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "mode");
        if(cou > 58){
            animation4 = false;
            take4shot = false;
            screenshot = 0;
        }
        else if(cou <= 15){ //screenshots for POINT
            type = POINTS;
            glUniform1i(loc,0);
        }
        else if(cou <= 30){ //screenshots for LINE
            type = LINES;
            glUniform1i(loc,0);
        }
        else if(cou <= 45){ //screenshots for TRIANGLE
            type = TRIANGLES;
            glUniform1i(loc,0);
        }
        else if(cou <= 58){ //screenshots for SMOOTH
            type = SMOOTHENED;
            glUniform1i(loc,1);
        }
        cou++;
    }
    //save the screenshots to disk total 1 t0 301
    /*if(screenshot!=0){
        cout<<screenshot<<endl;
    }
    */
    
    if(c<300 && screenshot%4==1 && takeshot){
        char filenum[4];
        sprintf(filenum, "%03d", c);
        string filename(string(filenum) + ".jpg");
        saveScreenshot(filename.c_str());
        c++;
    }
    
    
    
    if((co) && take4shot){
        char filenum[4];
        sprintf(filenum, "%03d", co);
        string filename(string(filenum) + ".jpg");
        saveScreenshot(filename.c_str());
        co++;
    }
    
    glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
    glViewport(0, 0, w, h);

    matrix.SetMatrixMode(OpenGLMatrix::Projection);
    matrix.LoadIdentity();
    matrix.Perspective(52.0f, (float)w / (float)h, 0.01f, 1000.0f);
    //Don't over-stretch the z-buffer. It has only finite precision.
    //A good call to setup perspective is:matrix->Perspective(fovy, aspect, 0.01, 1000.0);
}

void mouseMotionDragFunc(int x, int y)
{
    // mouse has moved and one of the mouse buttons is pressed (dragging)

    // the change in mouse position since the last invocation of this function
    int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };
    //angle
    
    switch (controlState)
    {
    // translate the landscape
        case TRANSLATE:
            if (leftMouseButton)
            {
                // control x,y translation via the left mouse button
                landTranslate[0] += mousePosDelta[0] * 0.01f;
                landTranslate[1] -= mousePosDelta[1] * 0.01f;
            }
            if (middleMouseButton)
            {
                // control z translation via the middle mouse button
                landTranslate[2] += mousePosDelta[1] * 0.01f;
            }
            break;

            // rotate the landscape
        case ROTATE:
            if (leftMouseButton)
            {
                // control x,y rotation via the left mouse button
                landRotate[0] += mousePosDelta[1];
                landRotate[1] += mousePosDelta[0];
            }
            if (middleMouseButton)
            {
                // control z rotation via the middle mouse button
                landRotate[2] += mousePosDelta[1];
            }
            break;

            // scale the landscape
        case SCALE:
            if (leftMouseButton)
            {
                // control x,y scaling via the left mouse button
                landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
                landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
            }
            if (middleMouseButton)
            {
                // control z scaling via the middle mouse button
                landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
            }
            break;
    }

    // store the new mouse position
    mousePos[0] = x;
    mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
    // mouse has moved
    // store the new mouse position
    mousePos[0] = x;
    mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
  // a mouse button has has been pressed or depressed

  // keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      leftMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_MIDDLE_BUTTON:
      middleMouseButton = (state == GLUT_DOWN);
    break;

    case GLUT_RIGHT_BUTTON:
      rightMouseButton = (state == GLUT_DOWN);
    break;
  }

  // keep track of whether CTRL and SHIFT keys are pressed
  switch (glutGetModifiers())
  {
    //I cannot use CTRL on macbook M1 clip
          //so I change to alt
    case GLUT_ACTIVE_ALT:
      controlState = TRANSLATE;
    break;

    case GLUT_ACTIVE_SHIFT:
      controlState = SCALE;
    break;

    // if CTRL and SHIFT are not pressed, we are in rotate mode
    default:
      controlState = ROTATE;
    break;
  }

  // store the new mouse position
  mousePos[0] = x;
  mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
    GLint loc = glGetUniformLocation(pipelineProgram->GetProgramHandle(), "mode");//basic.vertexShader.glsl
    switch (key)
    {
        case 27: // ESC key
            exit(0); // exit the program
            break;

        case ' ':
            cout << "You pressed the spacebar." << endl;
            break;

        case 'x':
            // take a screenshot
            saveScreenshot("screenshot.jpg");
            break;
          
        case '1'://point
            type = POINTS;
            // mode
            glUniform1i(loc, 0);//uniform int mode
            cout << "Point." << endl;
            break;
            
        case '2'://line
            type = LINES;
            // mode
            glUniform1i(loc, 0);
            cout << "Lines." << endl;
            break;

        case '3'://triangle
            type = TRIANGLES;
            // mode
            glUniform1i(loc, 0);
            cout << "Triangles." << endl;
        break;
            
        case '4'://triangle
            type = SMOOTHENED;
            // mode
            glUniform1i(loc, 1);
            cout << "SMOOTHENED." << endl;
        break;
        
        case '5'://autoup
            autoup = true;
            start = true;
            cout << "Auto on." << endl;
        break;
        
        case '6'://stop autoup
            start = false;
            landScale[0] = 1.0f;
            landScale[1] = 1.0f;
            cout << "Auto off." << endl;
        break;
            
        case '7'://rotate
            rota = true;
            cout << "Rotate on." << endl;
        break;
            
        case 'a'://Animation
            animation = true;
            cout << "Animation." << endl;
        break;
            
        case 't'://Animation & Take screenshot
            takeshot = true;
            cout << "Animation & Take screenshots." << endl;
        break;
            
        case 'c'://Animation & Take screenshot
            take4shot = true;
            animation4 = true;
            cout << "Animation & Take 4 screenshots." << endl;
        break;
            
        case 'r'://Initialize all the land transform
            landRotate[0] = 0.0f;
            landRotate[1] = 0.0f;
            landRotate[2] = 0.0f;
            landTranslate[0] = 0.0f;
            landTranslate[1] = 0.0f;
            landTranslate[2] = 0.0f;
            landScale[0] =  1.0f;
            landScale[1] =  1.0f;
            landScale[2] =  1.0f;
            cout << "Initialize all the land transforms." << endl;
        break;
        
    }
}
void convertcolortograyscale(int i, int j, int alpha){
    //https://en.wikipedia.org/wiki/Grayscale
    r = heightmapImage->getPixel(i, j, 0) / 255.0;
    g = heightmapImage->getPixel(i, j, 1) / 255.0;
    b = heightmapImage->getPixel(i, j, 2) / 255.0;
    float Ylinear=0.2126*r + 0.7152*g + 0.0722*b;
    float Ysrgb=0.0f;
    if(Ylinear <= 0.0031308){
        Ysrgb = 2.92*Ylinear;
    }else{
        Ysrgb = 1.055*pow(Ylinear, 0.41666667) - 0.055;
    }
    height = Ysrgb*255.0;
    
}


void heightField(){// read the heightmap
    //https://learnopengl.com/Guest-Articles/2021/Tessellation/Height-map
    imageWidth = heightmapImage->getWidth();
    imageHeight = heightmapImage->getHeight();
    
    
    
    scale = (imageWidth < 200)? 0.1 : (imageWidth < 500)? 0.25: (imageWidth < 700)? 0.4 : 0.7;
    
    BytesPerPixel = heightmapImage->getBytesPerPixel();// BytesPerPixel == 1 grayscale
    //calculate the XYZ coordinate of the current vertex
    //point
    //v.x = We'll have these range from -width/2 to width/2. This would correspond to the x dimension our ground would span in our scene.
    //v.y = This is the height of each vertex to give our mesh elevation. The y value we get out of our height map is within the range [0, 256]. We use the yScale to serve two purposes: (1) normalize the height map data to be within the range [0.0f, 1.0f] (2) scale it to the desired height we wish to work with. This now puts the values within the range [0.0f, 64.0f]. We finally apply a shift to translate the elevations to our final desired range, in this case [-16.0f, 48.0f]. You can choose the scale and shift you wish to apply based on your application.
    //v.z = We'll have these range from -height/2 to height/2. This would correspond to the z dimension our ground would span in our scene.
    
    for(int i = -imageWidth/2; i<(imageWidth/2)-1; i++){
        for(int j= -imageHeight/2; j<(imageHeight/2)-1; j++){

            if(BytesPerPixel == 1){//grayscale
                height = heightmapImage->getPixel(i+imageWidth/2, j+imageHeight/2, 0);
                //getPixel has 3 channel
                //height = scale * heightmapImage->getPixel(i, j, 0); cannot put together
                //scale is not big enough
                r = g = b = height / 255.0;
                //gray scale rgb = height and set it between 0 to 1
                height *= scale;
            }
            else if(BytesPerPixel == 3){
                convertcolortograyscale(i+imageWidth/2,j+imageHeight/2,0);
                height *= scale;
            }
            
            pointPosition.push_back((float)i);
            pointPosition.push_back(height);
            pointPosition.push_back(-(float)j);
            //create color matrix
            pointColor.push_back(r);
            pointColor.push_back(g);
            pointColor.push_back(b);
            pointColor.push_back(1.0);// alpha
   
        }
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //line
    //This is needed on recent Mac OS X versions to correctly display the window.
    //glutReshapeWindow(windowWidth - 1, windowHeight - 1);
    for(int i = -imageWidth/2; i<(imageWidth/2)-1; i++){
        for(int j= -imageHeight/2; j<(imageHeight/2)-1; j++){
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //horizontal with width
            
            if(i < (imageWidth/2)-1){
                if(BytesPerPixel == 1){//grayscale
                    height = heightmapImage->getPixel(i+imageWidth/2, j+imageHeight/2, 0);
                    //height = scale * heightmapImage->getPixel(i, j, 0); cannot put together
                    //scale is not big enough
                    r = g = b = height / 255.0;
                    //gray scale rgb = height and set it between 0 to 1
                    height *= scale;
                }
                else if(BytesPerPixel == 3){
                    convertcolortograyscale(i+imageWidth/2,j+imageHeight/2,0);
                    height *= scale;
                }
                
                
                linePosition.push_back((float)i);
                linePosition.push_back(height);
                linePosition.push_back(-(float)j);
                lineColor.push_back(r);
                lineColor.push_back(g);
                lineColor.push_back(b);
                lineColor.push_back(1.0);
                
                if(BytesPerPixel == 1){//grayscale
                    height = heightmapImage->getPixel((i+1)+imageWidth/2, j+imageHeight/2, 0);
                    //height = scale * heightmapImage->getPixel(i, j, 0); cannot put together
                    //scale is not big enough
                    r = g = b = height / 255.0;
                    //gray scale rgb = height and set it between 0 to 1
                    height *= scale;
                }
                else if(BytesPerPixel == 3){
                    convertcolortograyscale(i+imageWidth/2,j+imageHeight/2,0);
                    height *= scale;
                }
                
                
                linePosition.push_back((float)(i+1));
                linePosition.push_back(height);
                linePosition.push_back(-(float)j);
                lineColor.push_back(r);
                lineColor.push_back(g);
                lineColor.push_back(b);
                lineColor.push_back(1.0);
            }
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //vertical with width
            
            if(i < (imageHeight/2)-1){
                if(BytesPerPixel == 1){//grayscale
                    height = heightmapImage->getPixel(i+imageWidth/2, j+imageHeight/2, 0);
                    //height = scale * heightmapImage->getPixel(i, j, 0); cannot put together
                    //scale is not big enough
                    r = g = b = height / 255.0;
                    //gray scale rgb = height and set it between 0 to 1
                    height *= scale;
                }
                else if(BytesPerPixel == 3){
                    convertcolortograyscale(i+imageWidth/2,j+imageHeight/2,0);
                    height *= scale;
                }
                
                
                linePosition.push_back((float)i);
                linePosition.push_back(height);
                linePosition.push_back(-(float)j);
                lineColor.push_back(r);
                lineColor.push_back(g);
                lineColor.push_back(b);
                lineColor.push_back(1.0);
                
                if(BytesPerPixel == 1){//grayscale
                    height = heightmapImage->getPixel((i+1)+imageWidth/2, (j+1)+imageHeight/2, 0);
                    //height = scale * heightmapImage->getPixel(i, j, 0); cannot put together
                    //scale is not big enough
                    r = g = b = height / 255.0;
                    //gray scale rgb = height and set it between 0 to 1
                    height *= scale;
                }
                else if(BytesPerPixel == 3){
                    convertcolortograyscale(i+imageWidth/2,j+imageHeight/2,0);
                    height *= scale;
                }
                
                
                linePosition.push_back((float)i);
                linePosition.push_back(height);
                linePosition.push_back(-(float)(j+1));
                lineColor.push_back(r);
                lineColor.push_back(g);
                lineColor.push_back(b);
                lineColor.push_back(1.0);
            }
        }
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //triangle
    for(int i = -imageWidth/2; i < (imageWidth/2)-1; i++)
    {
        for(int j = -imageHeight/2; j < (imageHeight/2)-1; j++)
        {
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //up triangle (i,j) (i+1,j) (i+1,j+1) anticlockwise
            if(BytesPerPixel == 1){//grayscale
                height = heightmapImage->getPixel(i+imageWidth/2, j+imageHeight/2, 0);
                r = g = b = height / 255.0;
                //gray scale rgb = height and set it between 0 to 1
                height *= scale;
            }
            else if(BytesPerPixel == 3){
                convertcolortograyscale(i+imageWidth/2,j+imageHeight/2,0);
                height *= scale;
            }
            
            trianglePosition.push_back((float)i);
            trianglePosition.push_back(height);
            trianglePosition.push_back(-(float)j);
            triangleColor.push_back(r);
            triangleColor.push_back(g);
            triangleColor.push_back(b);
            triangleColor.push_back(1.0);
            
            if(BytesPerPixel == 1){//grayscale
                height = heightmapImage->getPixel((i+1)+imageWidth/2, j+imageHeight/2, 0);
                r = g = b = height / 255.0;
                //gray scale rgb = height and set it between 0 to 1
                height *= scale;
            }
            else if(BytesPerPixel == 3){
                convertcolortograyscale(i+imageWidth/2,j+imageHeight/2,0);
                height *= scale;
            }
            
            trianglePosition.push_back((float)(i+1));
            trianglePosition.push_back(height);
            trianglePosition.push_back(-(float)j);
            triangleColor.push_back(r);
            triangleColor.push_back(g);
            triangleColor.push_back(b);
            triangleColor.push_back(1.0);
            
            if(BytesPerPixel == 1){//grayscale
                height = heightmapImage->getPixel((i+1)+imageWidth/2, (j+1)+imageHeight/2, 0);
                r = g = b = height / 255.0;
                //gray scale rgb = height and set it between 0 to 1
                height *= scale;
            }
            else if(BytesPerPixel == 3){
                convertcolortograyscale(i+imageWidth/2,j+imageHeight/2,0);
                height *= scale;
            }
            
            trianglePosition.push_back((float)(i+1));
            trianglePosition.push_back(height);
            trianglePosition.push_back(-(float)(j+1));
            triangleColor.push_back(r);
            triangleColor.push_back(g);
            triangleColor.push_back(b);
            triangleColor.push_back(1.0);
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //down triangle (i,j) (i+1,j+1) (i,j+1) anticlockwise
            if(BytesPerPixel == 1){//grayscale
                height = heightmapImage->getPixel(i+imageWidth/2, j+imageHeight/2, 0);
                r = g = b = height / 255.0;
                //gray scale rgb = height and set it between 0 to 1
                height *= scale;
            }
            else if(BytesPerPixel == 3){
                convertcolortograyscale(i+imageWidth/2,j+imageHeight/2,0);
                height *= scale;
            }
            
            trianglePosition.push_back((float)i);
            trianglePosition.push_back(height);
            trianglePosition.push_back(-(float)j);
            triangleColor.push_back(r);
            triangleColor.push_back(g);
            triangleColor.push_back(b);
            triangleColor.push_back(1.0);
            
            if(BytesPerPixel == 1){//grayscale
                height = heightmapImage->getPixel((i+1)+imageWidth/2, (j+1)+imageHeight/2, 0);
                r = g = b = height / 255.0;
                //gray scale rgb = height and set it between 0 to 1
                height *= scale;
            }
            else if(BytesPerPixel == 3){
                convertcolortograyscale(i+imageWidth/2,j+imageHeight/2,0);
                height *= scale;
            }
            
            trianglePosition.push_back((float)(i+1));
            trianglePosition.push_back(height);
            trianglePosition.push_back(-(float)(j+1));
            triangleColor.push_back(r);
            triangleColor.push_back(g);
            triangleColor.push_back(b);
            triangleColor.push_back(1.0);
            
            if(BytesPerPixel == 1){//grayscale
                height = heightmapImage->getPixel(i+imageWidth/2, (j+1)+imageHeight/2, 0);
                r = g = b = height / 255.0;
                //gray scale rgb = height and set it between 0 to 1
                height *= scale;
            }
            else if(BytesPerPixel == 3){
                convertcolortograyscale(i+imageWidth/2,j+imageHeight/2,0);
                height *= scale;
            }
            
            trianglePosition.push_back((float)i);
            trianglePosition.push_back(height);
            trianglePosition.push_back(-(float)(j+1));
            triangleColor.push_back(r);
            triangleColor.push_back(g);
            triangleColor.push_back(b);
            triangleColor.push_back(1.0);
        }
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    //float heightl, heightr, heightd, heightu=1.0;
    //for Pup Pleft Pright Pdown
    for(int i = -imageWidth/2+1; i < (imageWidth/2)-2; i++)
    {
        for(int j = -imageHeight/2+1; j < (imageHeight/2)-2; j++)
        {

            //up triangle (i,j) (i+1,j) (i+1,j+1) anticlockwise
            //p_center(i,j) left and down may over bound
            //p_left(i-1,j)
            if( i == -imageWidth/2)
                height = scale*heightmapImage->getPixel(i+imageWidth/2, j+imageHeight/2, 0);
            else
                height = scale*heightmapImage->getPixel((i-1)+imageWidth/2, j+imageHeight/2, 0);

            p_left.push_back((float)(i-1));
            p_left.push_back(height);
            p_left.push_back(-(float)j);

            //p_right(i+1,j)
            height = scale*heightmapImage->getPixel((i+1)+imageWidth/2, j+imageHeight/2, 0);

            p_right.push_back((float)(i+1));
            p_right.push_back(height);
            p_right.push_back(-(float)j);

            //p_down(i,j-1)
            if( i == -imageHeight/2)
                height = scale*heightmapImage->getPixel(i+imageWidth/2, j+imageHeight/2, 0);
            else
                height = scale*heightmapImage->getPixel(i+imageWidth/2, (j-1)+imageHeight/2, 0);
        
            p_down.push_back((float)i);
            p_down.push_back(height);
            p_down.push_back(-(float)(j-1));
            
            //p_up(i,j+1)
            height = scale*heightmapImage->getPixel(i+imageWidth/2, (j+1)+imageHeight/2, 0);
            
            p_up.push_back((float)i);
            p_up.push_back(height);
            p_up.push_back(-(float)(j+1));
            
            /*
            p_center.push_back((float)i);
            p_center.push_back((heightl+heightr+heightd+heightu)/4.0);
            p_center.push_back(-(float)j);
            pcolor.push_back(((heightl+heightr+heightd+heightu)/4.0)/255.0);
            pcolor.push_back(((heightl+heightr+heightd+heightu)/4.0)/255.0);
            pcolor.push_back(((heightl+heightr+heightd+heightu)/4.0)/255.0);
            pcolor.push_back(1.0);
            */
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            //p_center(i+1,j) right and down may over bound
            //p_left(i,j)
            height = scale*heightmapImage->getPixel(i+imageWidth/2, j+imageHeight/2, 0);

            p_left.push_back((float)i);
            p_left.push_back(height);
            p_left.push_back(-(float)j);

            //p_right(i+2,j)
            if( i == imageWidth/2-2)
                height = scale*heightmapImage->getPixel((i+1)+imageWidth/2, j+imageHeight/2, 0);
            else
                height = scale*heightmapImage->getPixel((i+2)+imageWidth/2, j+imageHeight/2, 0);

            p_right.push_back((float)(i+2));
            p_right.push_back(height);
            p_right.push_back(-(float)j);

            //p_down(i+1,j-1)
            if( i == -imageHeight/2)
                height = scale*heightmapImage->getPixel((i+1)+imageWidth/2, j+imageHeight/2, 0);
            else
                height = scale*heightmapImage->getPixel((i+1)+imageWidth/2, (j-1)+imageHeight/2, 0);

            p_down.push_back((float)(i+1));
            p_down.push_back(height);
            p_down.push_back(-(float)(j-1));
            
            
            //p_up(i+1,j+1)
            height = scale*heightmapImage->getPixel((i+1)+imageWidth/2, (j+1)+imageHeight/2, 0);

            p_up.push_back((float)(i+1));
            p_up.push_back(height);
            p_up.push_back(-(float)(j+1));
            
            /*
            p_center.push_back((float)i);
            p_center.push_back((heightl+heightr+heightd+heightu)/4.0);
            p_center.push_back(-(float)j);
            pcolor.push_back(((heightl+heightr+heightd+heightu)/4.0)/255.0);
            pcolor.push_back(((heightl+heightr+heightd+heightu)/4.0)/255.0);
            pcolor.push_back(((heightl+heightr+heightd+heightu)/4.0)/255.0);
            pcolor.push_back(1.0);
            */
            
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //p_center(i+1,j+1) right and up may over bound
            //p_left(i,j+1)
            height = scale*heightmapImage->getPixel(i+imageWidth/2, (j+1)+imageHeight/2, 0);

            p_left.push_back((float)i);
            p_left.push_back(height);
            p_left.push_back(-(float)(j+1));

            //p_right(i+2,j+1)
            if( i == imageWidth/2-2)
                height = scale*heightmapImage->getPixel((i+1)+imageWidth/2, (j+1)+imageHeight/2, 0);
            else
                height = scale*heightmapImage->getPixel((i+2)+imageWidth/2, (j+1)+imageHeight/2, 0);

            p_right.push_back((float)(i+2));
            p_right.push_back(height);
            p_right.push_back(-(float)(j+1));

            //p_down(i+1,j)
            height = scale*heightmapImage->getPixel((i+1)+imageWidth/2, j+imageHeight/2, 0);

            p_down.push_back((float)(i+1));
            p_down.push_back(height);
            p_down.push_back(-(float)j);
            
            
            //p_up(i+1,j+2)
            if( i == imageHeight/2-2)
                height = scale*heightmapImage->getPixel((i+1)+imageWidth/2, (j+1)+imageHeight/2, 0);
            else
                height = scale*heightmapImage->getPixel((i+1)+imageWidth/2, (j+2)+imageHeight/2, 0);
                
            p_up.push_back((float)(i+1));
            p_up.push_back(height);
            p_up.push_back(-(float)(j+2));
            
            /*
            p_center.push_back((float)i);
            p_center.push_back((heightl+heightr+heightd+heightu)/4.0);
            p_center.push_back(-(float)j);
            pcolor.push_back(((heightl+heightr+heightd+heightu)/4.0)/255.0);
            pcolor.push_back(((heightl+heightr+heightd+heightu)/4.0)/255.0);
            pcolor.push_back(((heightl+heightr+heightd+heightu)/4.0)/255.0);
            pcolor.push_back(1.0);
            */
            
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //down triangle (i,j) (i+1,j+1) (i,j+1) anticlockwise
            //p_center(i,j) left and down may over bound
            //p_left(i-1,j)
            if( i == -imageWidth/2)
                height = scale*heightmapImage->getPixel(i+imageWidth/2, j+imageHeight/2, 0);
            else
                height = scale*heightmapImage->getPixel((i-1)+imageWidth/2, j+imageHeight/2, 0);
            
            p_left.push_back((float)(i-1));
            p_left.push_back(height);
            p_left.push_back(-(float)j);

            //p_right(i+1,j)
            height = scale*heightmapImage->getPixel((i+1)+imageWidth/2, j+imageHeight/2, 0);
                
            p_right.push_back((float)(i+1));
            p_right.push_back(height);
            p_right.push_back(-(float)j);

            
            //p_down(i,j-1)
            if( i == -imageHeight/2)
                height = scale*heightmapImage->getPixel(i+imageWidth/2, j+imageHeight/2, 0);
            else
                height = scale*heightmapImage->getPixel(i+imageWidth/2, (j-1)+imageHeight/2, 0);
                
            p_down.push_back((float)i);
            p_down.push_back(height);
            p_down.push_back(-(float)(j-1));
            
            //p_up(i,j+1)
            height = scale*heightmapImage->getPixel(i+imageWidth/2, (j+1)+imageHeight/2, 0);
            
            p_up.push_back((float)i);
            p_up.push_back(height);
            p_up.push_back(-(float)(j+1));

            
            /*
            p_center.push_back((float)i);
            p_center.push_back((heightl+heightr+heightd+heightu)/4.0);
            p_center.push_back(-(float)j);
            pcolor.push_back(((heightl+heightr+heightd+heightu)/4.0)/255.0);
            pcolor.push_back(((heightl+heightr+heightd+heightu)/4.0)/255.0);
            pcolor.push_back(((heightl+heightr+heightd+heightu)/4.0)/255.0);
            pcolor.push_back(1.0);
            */
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //p_center(i+1,j+1) right and up may over bound
            //p_left(i,j+1)
            height = scale*heightmapImage->getPixel(i+imageWidth/2, (j+1)+imageHeight/2, 0);
            
            p_left.push_back((float)i);
            p_left.push_back(height);
            p_left.push_back(-(float)(j+1));

            //p_right(i+2,j+1)
            if( i == imageWidth/2-2)
                height = scale*heightmapImage->getPixel((i+1)+imageWidth/2, (j+1)+imageHeight/2, 0);
            else
                height = scale*heightmapImage->getPixel((i+2)+imageWidth/2, (j+1)+imageHeight/2, 0);
            
            p_right.push_back((float)(i+2));
            p_right.push_back(height);
            p_right.push_back(-(float)(j+1));

            //p_down(i+1,j)
            height = scale*heightmapImage->getPixel(i+imageWidth/2, j+imageHeight/2, 0);

            p_down.push_back((float)(i+1));
            p_down.push_back(height);
            p_down.push_back(-(float)j);
            
            //p_up(i+1,j+2)
            if( i == imageHeight/2-2)
                height = scale*heightmapImage->getPixel((i+1)+imageWidth/2, (j+1)+imageHeight/2, 0);
            else
                height = scale*heightmapImage->getPixel((i+1)+imageWidth/2, (j+2)+imageHeight/2, 0);

            p_up.push_back((float)(i+1));
            p_up.push_back(height);
            p_up.push_back(-(float)(j+2));
            
            /*
            p_center.push_back((float)i);
            p_center.push_back((heightl+heightr+heightd+heightu)/4.0);
            p_center.push_back(-(float)j);
            pcolor.push_back(((heightl+heightr+heightd+heightu)/4.0)/255.0);
            pcolor.push_back(((heightl+heightr+heightd+heightu)/4.0)/255.0);
            pcolor.push_back(((heightl+heightr+heightd+heightu)/4.0)/255.0);
            pcolor.push_back(1.0);
            */
            
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //p_center(i,j+1) left and up may over bound
            //p_left(i-1,j+1)
            if( i == -imageWidth/2)
                height = scale*heightmapImage->getPixel(i+imageWidth/2, (j+1)+imageHeight/2, 0);
            else
                height = scale*heightmapImage->getPixel((i-1)+imageWidth/2, (j+1)+imageHeight/2, 0);
 
            p_left.push_back((float)(i-1));
            p_left.push_back(height);
            p_left.push_back(-(float)(j+1));

            //p_right(i+1,j+1)

            height = scale*heightmapImage->getPixel((i+1)+imageWidth/2, (j+1)+imageHeight/2, 0);
            
            p_right.push_back((float)(i+1));
            p_right.push_back(height);
            p_right.push_back(-(float)(j+1));
            
            //p_down(i,j)
            height = scale*heightmapImage->getPixel(i+imageWidth/2, j+imageHeight/2, 0);

            p_down.push_back((float)i);
            p_down.push_back(height);
            p_down.push_back(-(float)j);
            
            //p_up(i,j+2)
            if( i == imageHeight/2-2)
                height = scale*heightmapImage->getPixel(i+imageWidth/2, (j+1)+imageHeight/2, 0);
            else
                height = scale*heightmapImage->getPixel(i+imageWidth/2, (j+2)+imageHeight/2, 0);
            
            p_up.push_back((float)i);
            p_up.push_back(height);
            p_up.push_back(-(float)(j+2));
            /*
            p_center.push_back((float)i);
            p_center.push_back((heightl+heightr+heightd+heightu)/4.0);
            p_center.push_back(-(float)j);
            pcolor.push_back(((heightl+heightr+heightd+heightu)/4.0)/255.0);
            pcolor.push_back(((heightl+heightr+heightd+heightu)/4.0)/255.0);
            pcolor.push_back(((heightl+heightr+heightd+heightu)/4.0)/255.0);
            pcolor.push_back(1.0);
            */
            
            //up triangle (i,j) (i+1,j) (i+1,j+1) anticlockwise
            if(BytesPerPixel == 1){//grayscale
                height = heightmapImage->getPixel(i+imageWidth/2, j+imageHeight/2, 0);
                r = g = b = height / 255.0;
                //gray scale rgb = height and set it between 0 to 1
                height *= scale;
            }
            else if(BytesPerPixel == 3){
                convertcolortograyscale(i+imageWidth/2,j+imageHeight/2,0);
                height *= scale;
            }
            
            p_triangle.push_back((float)i);
            p_triangle.push_back(height);
            p_triangle.push_back(-(float)j);
            c_triangle.push_back(r);
            c_triangle.push_back(g);
            c_triangle.push_back(b);
            c_triangle.push_back(1.0);
            
            if(BytesPerPixel == 1){//grayscale
                height = heightmapImage->getPixel((i+1)+imageWidth/2, j+imageHeight/2, 0);
                r = g = b = height / 255.0;
                //gray scale rgb = height and set it between 0 to 1
                height *= scale;
            }
            else if(BytesPerPixel == 3){
                convertcolortograyscale(i+imageWidth/2,j+imageHeight/2,0);
                height *= scale;
            }
            
            p_triangle.push_back((float)(i+1));
            p_triangle.push_back(height);
            p_triangle.push_back(-(float)j);
            c_triangle.push_back(r);
            c_triangle.push_back(g);
            c_triangle.push_back(b);
            c_triangle.push_back(1.0);
            
            if(BytesPerPixel == 1){//grayscale
                height = heightmapImage->getPixel((i+1)+imageWidth/2, (j+1)+imageHeight/2, 0);
                r = g = b = height / 255.0;
                //gray scale rgb = height and set it between 0 to 1
                height *= scale;
            }
            else if(BytesPerPixel == 3){
                convertcolortograyscale(i+imageWidth/2,j+imageHeight/2,0);
                height *= scale;
            }
            
            p_triangle.push_back((float)(i+1));
            p_triangle.push_back(height);
            p_triangle.push_back(-(float)(j+1));
            c_triangle.push_back(r);
            c_triangle.push_back(g);
            c_triangle.push_back(b);
            c_triangle.push_back(1.0);
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //down triangle (i,j) (i+1,j+1) (i,j+1) anticlockwise
            if(BytesPerPixel == 1){//grayscale
                height = heightmapImage->getPixel(i+imageWidth/2, j+imageHeight/2, 0);
                r = g = b = height / 255.0;
                //gray scale rgb = height and set it between 0 to 1
                height *= scale;
            }
            else if(BytesPerPixel == 3){
                convertcolortograyscale(i+imageWidth/2,j+imageHeight/2,0);
                height *= scale;
            }
            
            p_triangle.push_back((float)i);
            p_triangle.push_back(height);
            p_triangle.push_back(-(float)j);
            c_triangle.push_back(r);
            c_triangle.push_back(g);
            c_triangle.push_back(b);
            c_triangle.push_back(1.0);
            
            if(BytesPerPixel == 1){//grayscale
                height = heightmapImage->getPixel((i+1)+imageWidth/2, (j+1)+imageHeight/2, 0);
                r = g = b = height / 255.0;
                //gray scale rgb = height and set it between 0 to 1
                height *= scale;
            }
            else if(BytesPerPixel == 3){
                convertcolortograyscale(i+imageWidth/2,j+imageHeight/2,0);
                height *= scale;
            }
            
            p_triangle.push_back((float)(i+1));
            p_triangle.push_back(height);
            p_triangle.push_back(-(float)(j+1));
            c_triangle.push_back(r);
            c_triangle.push_back(g);
            c_triangle.push_back(b);
            c_triangle.push_back(1.0);
            
            if(BytesPerPixel == 1){//grayscale
                height = heightmapImage->getPixel(i+imageWidth/2, (j+1)+imageHeight/2, 0);
                r = g = b = height / 255.0;
                //gray scale rgb = height and set it between 0 to 1
                height *= scale;
            }
            else if(BytesPerPixel == 3){
                convertcolortograyscale(i+imageWidth/2,j+imageHeight/2,0);
                height *= scale;
            }
            
            p_triangle.push_back((float)i);
            p_triangle.push_back(height);
            p_triangle.push_back(-(float)(j+1));
            c_triangle.push_back(r);
            c_triangle.push_back(g);
            c_triangle.push_back(b);
            c_triangle.push_back(1.0);

        }
    }
}

void vbovao(){
    
    //Point
    glGenBuffers(1, &vboPoint); // get handle on VBO buffer
    glBindBuffer(GL_ARRAY_BUFFER, vboPoint);  // bind the VBO buffer
    
    // init VBO’s size
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * pointPosition.size() + sizeof(float) * pointColor.size(), NULL, GL_STATIC_DRAW);
    // but don’t assign any data to it
    
    //fill sub-regions with corresponding data
    //chapter4 color slide
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * pointPosition.size(), (float*)(pointPosition.data()));//upload position data
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * pointPosition.size(), sizeof(float) * pointColor.size(), (float*)(pointColor.data()));//upload color data
    
    glGenVertexArrays(1, &vaoPoint);
    glBindVertexArray(vaoPoint); // bind the VAO
    glBindBuffer(GL_ARRAY_BUFFER, vboPoint);
    GLuint loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
    glEnableVertexAttribArray(loc); // enable the "position" attribute
    GLboolean normalized = GL_FALSE;
    GLsizei stride = 0;
    //just state stride to 0
    glVertexAttribPointer(loc, 3, GL_FLOAT, normalized, stride, (const void*) 0); // set the layout of the "position" attribute data
    // get location index of the "color" shader variable
    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");// vertex color
    glEnableVertexAttribArray(loc); // enable the "color" attribute
    //just state stride to 0
    glVertexAttribPointer(loc, 4, GL_FLOAT, normalized, stride, (const void*)(sizeof(float)*pointPosition.size()));
    // set the layout of the “color” attribute data
    // need to relocate the position
    glBindVertexArray(0); // unbind the VAO
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    //Line
    glGenBuffers(1, &vboLine); // get handle on VBO buffer
    glBindBuffer(GL_ARRAY_BUFFER, vboLine);  // bind the VBO buffer
    
    // to allocate memory
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * linePosition.size() + sizeof(float) * lineColor.size(), NULL, GL_STATIC_DRAW);
    //No data yet, just allocate
    
    //fill sub-regions with corresponding data
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * linePosition.size(), (float*)(linePosition.data()));
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * linePosition.size(), sizeof(float) * lineColor.size(), (float*)(lineColor.data()));
    
    glGenVertexArrays(1, &vaoLine);
    glBindVertexArray(vaoLine); // bind the VAO
    glBindBuffer(GL_ARRAY_BUFFER, vboLine);
    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
    glEnableVertexAttribArray(loc); // enable the "position" attribute
    normalized = GL_FALSE;
    // set the layout of the “position” attribute data
    stride = 0;
    //just state stride to 0
    glVertexAttribPointer(loc, 3, GL_FLOAT, normalized, stride, (const void*) 0); // set the layout of the "position" attribute data
    // get location index of the "color" shader variable
    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");// vertex color
    glEnableVertexAttribArray(loc); // enable the "color" attribute
    //just state stride to 0
    glVertexAttribPointer(loc, 4, GL_FLOAT, normalized, stride, (const void*) (sizeof(float)*linePosition.size()));// set the layout of the “color” attribute data
    // need to relocate the position
    glBindVertexArray(0); // unbind the VAO
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    //triangle
    glGenBuffers(1, &vboTriangle); // get handle on VBO buffer
    glBindBuffer(GL_ARRAY_BUFFER, vboTriangle);  // bind the VBO buffer
    
    // init VBO’s size
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * trianglePosition.size() + sizeof(float) * triangleColor.size(), NULL, GL_STATIC_DRAW);
    // but don’t assign any data to it
    
    //fill sub-regions with corresponding data
    //chapter4 color slide
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * trianglePosition.size(), (float*)(trianglePosition.data()));//upload position data
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * trianglePosition.size(), sizeof(float) * triangleColor.size(), (float*)(triangleColor.data()));//upload color data
    
    glGenVertexArrays(1, &vaoTriangle);
    glBindVertexArray(vaoTriangle); // bind the VAO
    glBindBuffer(GL_ARRAY_BUFFER, vboTriangle);
    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
    glEnableVertexAttribArray(loc); // enable the "position" attribute
    normalized = GL_FALSE;
    // set the layout of the “position” attribute data
    stride = 0;
    //just state stride to 0
    glVertexAttribPointer(loc, 3, GL_FLOAT, normalized, stride, (const void*) 0); // set the layout of the "position" attribute data
    // get location index of the "color" shader variable
    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");// vertex color
    glEnableVertexAttribArray(loc); // enable the "color" attribute

    //just state stride to 0
    glVertexAttribPointer(loc, 4, GL_FLOAT, normalized, stride, (const void*)(sizeof(float)*trianglePosition.size()));// set the layout of the “color” attribute data
    // need to relocate the position
    glBindVertexArray(0); // unbind the VAO
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    
}
void svbovao(){
    //smoothened triangle 5vbo+1vao left right down up
    GLuint stride = 0;
    GLuint normalized = GL_FALSE;
    //p_left
    glGenBuffers(1, &vboP_left); // get handle on VBO buffer
    glBindBuffer(GL_ARRAY_BUFFER, vboP_left);  // bind the VBO buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * p_left.size(), (float*)(p_left.data()), GL_STATIC_DRAW);

    
    glGenVertexArrays(1, &vaoSmoothTriangle);
    glBindVertexArray(vaoSmoothTriangle); // bind the VAO
    
    
    glBindBuffer(GL_ARRAY_BUFFER, vboP_left);
    GLuint loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "P_left");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, normalized, stride, (const void*) 0); // set the layout of the "P_left" attribute data

    //p_right
    glGenBuffers(1, &vboP_right); // get handle on VBO buffer
    glBindBuffer(GL_ARRAY_BUFFER, vboP_right);  // bind the VBO buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * p_right.size(), (float*)(p_right.data()), GL_STATIC_DRAW);
    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "P_right");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, normalized, stride, (const void*) 0); // set the layout of the "P_right" attribute data
    
    //p_down
    glGenBuffers(1, &vboP_down); // get handle on VBO buffer
    glBindBuffer(GL_ARRAY_BUFFER, vboP_down);  // bind the VBO buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * p_down.size(), (float*)(p_down.data()), GL_STATIC_DRAW);
    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "P_down");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, normalized, stride, (const void*) 0); // set the layout of the "P_down" attribute data
    
    //p_up
    glGenBuffers(1, &vboP_up); // get handle on VBO buffer
    glBindBuffer(GL_ARRAY_BUFFER, vboP_up);  // bind the VBO buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * p_up.size(), (float*)(p_up.data()), GL_STATIC_DRAW);
    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "P_up");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, normalized, stride, (const void*) 0); // set the layout of the "P_up" attribute data

    //triangle
    glGenBuffers(1, &vboP_triangle); // get handle on VBO buffer
    glBindBuffer(GL_ARRAY_BUFFER, vboP_triangle);  // bind the VBO buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * p_triangle.size() + sizeof(float) * c_triangle.size(), NULL, GL_STATIC_DRAW);
    // but don’t assign any data to it
    //fill sub-regions with corresponding data
    //chapter4 color slide
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * p_triangle.size(), (float*)(p_triangle.data()));//upload position data
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * p_triangle.size(), sizeof(float) * c_triangle.size(), (float*)(c_triangle.data()));//upload color data
    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
    glEnableVertexAttribArray(loc); // enable the "position" attribute
    glVertexAttribPointer(loc, 3, GL_FLOAT, normalized, stride, (const void*) 0); // set the layout of the "position" attribute data
    // get location index of the "color" shader variable
    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");// vertex color
    glEnableVertexAttribArray(loc); // enable the "color" attribute
    glVertexAttribPointer(loc, 4, GL_FLOAT, normalized, stride, (const void*)(sizeof(float)*p_triangle.size()));// set the layout of the “color” attribute data
    
    glBindVertexArray(0); // unbind the VAO

}

void initScene(int argc, char *argv[])
{
    // load the image from a jpeg disk file to main memory
    heightmapImage = new ImageIO();
    if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK)
    {
        cout << "Error reading image " << argv[1] << "." << endl;
        exit(EXIT_FAILURE);
    }

    
    BytesPerPixel = heightmapImage->getBytesPerPixel();
    //BytesPerPixel == 1  grayscale
    
    if(BytesPerPixel == 3){//it is better to view the color.jpg
        landRotate[0] = 30.0f;
    }

    
    
    
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    /*
    // modify the following code accordingly
    glm::vec3 triangle[3] = {
        glm::vec3(0, 0, 1),
        glm::vec3(0, 1, 0),
        glm::vec3(1, 0, 0)
    };

    glm::vec4 color[3] = {
        {0, 0, 1, 1},
        {1, 0, 0, 1},
        {0, 1, 0, 1},
    };
    
     
    heightField();
    
    glGenBuffers(1, &vboPoint);
    glBindBuffer(GL_ARRAY_BUFFER, vboPoint);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 3, triangle, GL_STATIC_DRAW);

    glGenBuffers(1, &triColorVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, triColorVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * 3, color, GL_STATIC_DRAW);

    
    pipelineProgram = new BasicPipelineProgram;
    int ret = pipelineProgram->Init(shaderBasePath);
    if (ret != 0) abort();
    
    pipelineProgram->Bind();
    
    glGenVertexArrays(1, &vaoPoint);
    glBindVertexArray(vaoPoint);
    glBindBuffer(GL_ARRAY_BUFFER, vboPoint);

    GLuint loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

    glBindBuffer(GL_ARRAY_BUFFER, triColorVertexBuffer);
    loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (const void *)0);
 
    glEnable(GL_DEPTH_TEST);
    
    sizeTri = 50000;

    std::cout << "GL error: " << glGetError() << std::endl;
*/
    
    heightField();
    //first initialize, then create pipeline, vbo, vao
    //bind vao, and call glDrawArrays(), then later unbind vao
    pipelineProgram = new BasicPipelineProgram;
    int ret = pipelineProgram->Init(shaderBasePath);
    if (ret != 0) abort();

    pipelineProgram->Bind();
    
    vbovao();
    svbovao();
    
    
    glEnable(GL_DEPTH_TEST);

    std::cout << "GL error: " << glGetError() << std::endl;
    
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    cout << "The arguments are incorrect." << endl;
    cout << "usage: ./hw1 <heightmap file>" << endl;
    exit(EXIT_FAILURE);
  }

  cout << "Initializing GLUT..." << endl;
  glutInit(&argc,argv);

  cout << "Initializing OpenGL..." << endl;

  #ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #else
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
  #endif

  glutInitWindowSize(windowWidth, windowHeight);
  glutInitWindowPosition(0, 0);
  glutCreateWindow(windowTitle);

  cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
  cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
  cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

  #ifdef __APPLE__
    // This is needed on recent Mac OS X versions to correctly display the window.
    glutReshapeWindow(windowWidth - 1, windowHeight - 1);
  #endif

  // tells glut to use a particular display function to redraw
  glutDisplayFunc(displayFunc);   //glDrawArrays()
  // perform animation inside idleFunc
  glutIdleFunc(idleFunc);
  // callback for mouse drags
  glutMotionFunc(mouseMotionDragFunc);
  // callback for idle mouse movement
  glutPassiveMotionFunc(mouseMotionFunc);
  // callback for mouse button changes
  glutMouseFunc(mouseButtonFunc);
  // callback for resizing the window
  glutReshapeFunc(reshapeFunc);
  // callback for pressing the keys on the keyboard
  glutKeyboardFunc(keyboardFunc);

  // init glew
  #ifdef __APPLE__
    // nothing is needed on Apple
  #else
    // Windows, Linux
    GLint result = glewInit();
    if (result != GLEW_OK)
    {
      cout << "error: " << glewGetErrorString(result) << endl;
      exit(EXIT_FAILURE);
    }
  #endif

  // do initialization
  initScene(argc, argv);

  // sink forever into the glut loop
  glutMainLoop();
}




