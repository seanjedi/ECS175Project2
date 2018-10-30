#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include <vector> 
#include <cmath>
#include <string>
#include "Bresenham.h"

using namespace std;

////////////////////
//Global Variables//
////////////////////
float *PixelBuffer;
string inputFile;
char quit, all;
int windowSizeX = 500, windowSizeY = 500, style, mode, polyhedraCount, currentID;
float viewXmin, viewXmax, viewYmin, viewYmax, deltaX, deltaY, deltaZ, Delta;
ifstream inFile;


//Vertex Struct
struct Vertex
{
	float x;
	float y;
	float z;
};

//////////////////////////
//Function Defininitions//
//////////////////////////
void display();
void writeBack();
void getSettings(int, char*[]);
void getSettings2();
void boundBox();
void setScreen();
void setBoundaryBox();
Vertex toNDCtoPixel(float x, float y, float z, int mode);


//Define a Boundary Box struct
struct Boundary
{
	float Xmin, Ymin, Zmin, Xmax, Ymax, Zmax;
};

Boundary boundaryBox{0,0,0,0,0,0};

//Find max function
float max(float a, float b) {
	if (a > b)
		return a;
	else
		return b;
}

//Find min function
float min(float a, float b) {
	if (a < b)
		return a;
	else
		return b;
}
//Bresenham line drawing
void drawBresenham(int x1, int y1, int x2, int y2) {
	Bresenham(x1, x2, y1, y2, PixelBuffer, windowSizeX);
}
///////////////////////////////////
//Polygon Object Class Definition//
///////////////////////////////////
class polyhedraObject {
public:
	int vertexCount;
	int edgeCount;

	struct edge
	{
		int a;
		int b;
	};
	vector<Vertex> vertices;
	vector<edge> edges;

	//Set count of vertices
	void setMatrix(int x) {
		vertexCount = x;
	}

	//Set count of edges
	void setEdges(int x) {
		edgeCount = x;
	}

	//Add a new vertex to the vertice vertex
	void addVertex(float x, float y, float z) {
		Vertex newVertex = { x, y, z };
		vertices.push_back(newVertex);
	}

	//Set new edge connections
	void addEdge(int a, int b) {
		edge newEdge = { a, b };
		edges.push_back(newEdge);
	}


	//Write polygon to the Pixel Buffer
	void drawPolyhedra() {
		Vertex temp1, temp2;
		int edge1, edge2;
		
		for (int i = 0; i < edgeCount; i++) {
		//Draw XY
			edge1 = edges[i].a - 1;
			edge2 = edges[i].b - 1;
			temp1 = toNDCtoPixel(vertices[edge1].x, vertices[edge1].y, vertices[edge1].z, 1);
			temp2 = toNDCtoPixel(vertices[edge2].x, vertices[edge2].y, vertices[edge2].z, 1);
			drawBresenham(temp1.x, temp1.y, temp2.x, temp2.y);

		//Draw XZ
			temp1 = toNDCtoPixel(vertices[edge1].x, vertices[edge1].y, vertices[edge1].z, 2);
			temp2 = toNDCtoPixel(vertices[edge2].x, vertices[edge2].y, vertices[edge2].z, 2);
			drawBresenham(temp1.x, temp1.z, temp2.x, temp2.z);

		//Draw YZ
			temp1 = toNDCtoPixel(vertices[edge1].x, vertices[edge1].y, vertices[edge1].z, 3);
			temp2 = toNDCtoPixel(vertices[edge2].x, vertices[edge2].y, vertices[edge2].z, 3);
			drawBresenham(temp1.y, temp1.z, temp2.y, temp2.z);

		}
	}

	//Function to find a new boundary for each polyhedra
	Boundary getBoundary() {
		Boundary instance{0,0,0,0,0,0};
		for (int i = 0; i < vertexCount; i++) {
			instance.Xmax = max(instance.Xmax, vertices[i].x);
			instance.Ymax = max(instance.Ymax, vertices[i].y);
			instance.Zmax = max(instance.Zmax, vertices[i].z);
			instance.Xmin = min(instance.Xmin, vertices[i].x);
			instance.Ymin = min(instance.Ymin, vertices[i].y);
			instance.Zmin = min(instance.Zmin, vertices[i].z);
		}
		return instance;
	}

	//Translation
	void translate(float tx, float ty, float tz) {
		for (int i = 0; i < vertexCount; i++) {
			vertices[i].x += tx;
			vertices[i].y += ty;
			vertices[i].z += tz;
		}
	}

	//Scaling
	void scale(float scale) {
		double centerX = 0, centerY = 0, centerZ = 0;
		//Find Centroid
		for (int i = 0; i < vertexCount; i++) {
			centerX += vertices[i].x;
			centerY += vertices[i].y;
			centerZ += vertices[i].z;
		}
		centerX /= vertexCount;
		centerY /= vertexCount;
		centerZ /= vertexCount;
		//Translate by -C
		translate(-centerX, -centerY, -centerZ);
		for (int i = 0; i < vertexCount; i++) {
			vertices[i].x *= scale;
			vertices[i].y *= scale;
			vertices[i].z *= scale;
		}
		//Translate back by C
		translate(centerX, centerY, centerZ);
	}

	//Rotation
	void rotate(double theta, Vertex p1, Vertex p2) {
		//Make temperary vertices
		Vertex temp1, temp2;
		// Do this for every vertex
		for (int i = 0; i < vertexCount; i++) {
			//Step 1 Translate
			temp1.x = vertices[i].x - p1.x;
			temp1.y = vertices[i].y - p1.y;
			temp1.z = vertices[i].z - p1.z;

			//get variables
			float axisVectLength = sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y) + (p2.z - p1.z) * (p2.z - p1.z));
			float ux = (p2.x - p1.x) / axisVectLength;
			float uy = (p2.y - p1.y) / axisVectLength;
			float uz = (p2.z - p1.z) / axisVectLength;
			float d = sqrt(uy*uy + uz * uz);

			//Step 2 rotate space about the x axis 
			if (d != 0) {
				temp2.x = temp1.x;
				temp2.y = temp1.y * uz / d - temp1.z * uy / d;
				temp2.z = temp1.y * uy / d + temp1.z * uz / d;
			}
			else {
				temp2 = temp1;
			}
			// Step 3 rotate space about the y axis 
			temp1.x = temp2.x * d - temp2.z * ux;
			temp1.y = temp2.y;
			temp1.z = temp2.x * ux + temp2.z * d;

			// Step 4 perform the desired rotation by theta about the z axis
			temp2.x = temp1.x * cos(theta) - temp1.y * sin(theta);
			temp2.y = temp1.x * sin(theta) + temp1.y * cos(theta);
			temp2.z = temp1.z;

			// Inverse of step 3
			temp1.x = temp2.x * d + temp2.z * ux;
			temp1.y = temp2.y;
			temp1.z = -temp2.x * ux + temp2.z * d;

			// Inverse of step 2
			if (d != 0) {
				temp2.x = temp1.x;
				temp2.y = temp1.y * uz / d + temp1.z * uy / d;
				temp2.z = -temp1.y * uy / d + temp1.z * uz / d;
			}
			else {
				temp2 = temp1;
			}

			// Inverse of step 1 
			temp1.x = temp2.x + p1.x;
			temp1.y = temp2.y + p1.y;
			temp1.z = temp2.z + p1.z;

			//Store back values into vertices
			vertices[i].x = temp1.x;
			vertices[i].y = temp1.y;
			vertices[i].z = temp1.z;
		}
	}

	//Write Polygon Buffer to output file!
	void writeBack(std::ofstream& file) {
		file << vertexCount << endl;
		for (int i = 0; i < vertexCount; i++) {
			file << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << endl;
		}
		file << edgeCount << endl;
		for (int i = 0; i < edgeCount; i++) {
			file << edges[i].a << " " << edges[i].b << endl;
		}
	}

};

///////////////////////////
//Reset Pixel Buffer to 0//
///////////////////////////
void setPixelBuffer(float* PixelBuffer) {
	for (int i = 0; i < windowSizeX; i++) {
		for (int j = 0; j < windowSizeY; j++) {
			PixelBuffer[((windowSizeX * j) + i) * 3] = 0;
		}
	}
}

/////////////////////////////////
//Polygon Vector Initialization//
/////////////////////////////////
vector<polyhedraObject> polyhedras;

/////////////////
//Main Function//
/////////////////
int main(int argc, char *argv[])
{
	//allocate new pixel buffer, need initialization!!
	getSettings(argc, argv);
	setBoundaryBox();
	PixelBuffer = new float[windowSizeX * windowSizeY * 3];
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE);
	//set window size to windowSizeX by windowSizeX
	glutInitWindowSize(windowSizeX, windowSizeY);
	//set window position
	glutInitWindowPosition(100, 100);

	//create and set main window title
	int MainWindow = glutCreateWindow("Hello Graphics!!");
	glClearColor(0, 0, 0, 0); //clears the buffer of OpenGL
	//sets display function
	while (1) {
		display();
		cout << "Would you like to quit? (y/n)\nChooice: ";
		cin >> quit;
		if (quit == 'y') {
			exit(0);
		}
		getSettings2();
	}
	
	//glutDisplayFunc(display);

	glutMainLoop();//main display loop, will display until terminate
	return 0;
}



///////////////////////
//Make Pixel Function//
///////////////////////

void makePixel(int x, int y, float* PixelBuffer)
{
	//Make sure it is within range
	if( x > 0 && x < 500 && y > 0 && y < 500)
		PixelBuffer[((windowSizeX * y) + x) * 3] = 5;
}

/////////////////////////////////////////////////////////////////////////////
//main display loop, this function will be called again and again by OpenGL//
/////////////////////////////////////////////////////////////////////////////
void display(){

	//Misc.
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	setPixelBuffer(PixelBuffer);

	setScreen();
	if (all == 'n') {
		polyhedras[currentID - 1].drawPolyhedra();
	}
	else {
		for (int i = 0; i < polyhedraCount; i++)
			polyhedras[i].drawPolyhedra();
	}

	
	//draws pixel on screen, width and height must match pixel buffer dimension
	glDrawPixels(windowSizeX, windowSizeY, GL_RGB, GL_FLOAT, PixelBuffer);

	//window refresh
	glFlush();
}

//////////////////////////////
//Handles File Input//////////
//Handles Initial User Input//
//////////////////////////////
void getSettings(int argc, char* argv[]){
	//Make sure that the right number of arguments are passed
	if (argc > 2) {
		cerr << "Too Many Arguments!\nStopping Execution";
		exit(1);
	}
	else if (argc == 1) {
		//If no input file specified, ask for one!
		cout << "Specify Input File: ";
		getline(cin, inputFile);
		inFile.open(inputFile);
		if (!inFile) {
			cerr << "Unable to open input file \nStopping Execution";
			exit(1);
		}
	}
	else {
		//If failed to open, print error and exit
		inFile.open(argv[1]);
		if (!inFile) {
			cerr << "Unable to open input file \nStopping Execution";
			exit(1);
		}
	}
		string space;
		int vertexCount, edgeCount, point1, point2;
		float x, y, z;
		//Read input file
		inFile >> polyhedraCount;
		polyhedras.resize(polyhedraCount);
		for (int i = 0; i < polyhedraCount; i++) {
			getline(inFile, space);
			inFile >> vertexCount;
			polyhedras[i].setMatrix(vertexCount);
			for (int j = 0; j < vertexCount; j++) {
				inFile >> x;
				inFile >> y;
				inFile >> z;
				polyhedras[i].addVertex(x, y, z);
			}
			inFile >> edgeCount;
			polyhedras[i].setEdges(edgeCount);
			for (int j = 0; j < edgeCount; j++) {
				inFile >> point1;
				inFile >> point2;
				polyhedras[i].addEdge(point1, point2);
			}
		}
	//Go to polyhedra Menu
	getSettings2();

}
//////////////////////
//WriteBack Function//
//////////////////////
void writeBack() {
	//Write back to open file!
	ofstream myfile(inputFile);
	if (myfile.is_open()) {
		myfile << polyhedraCount << endl;
		for (int i = 0; i < polyhedraCount; i++) {
			myfile << endl;
			polyhedras[i].writeBack(myfile);
		}
	}
	else {
		cout << "Error! Unable to open file!";
		exit(-1);
	}
	myfile.close();
}

void getSettings2() {
	int choice = 4;
	//Ask which polyhedra you want to manipulate!
	cout << "Which Polyhedra would you like to manipulate? (id's 1 to " << polyhedraCount << ")\nID:";
	cin >> currentID;
	while (currentID <= 0 || currentID > polyhedraCount) {
		cout << "Wrong ID, Please choose a possible ID!\nID:";
		cin >> currentID;
	}

	//Ash which transformation you want
	cout << "Which transformation would you like to do next?\n1) Translate\n2) Scale\n3) Rotation\n4) Display\nChoose:";
	cin >> choice;
	while (choice < 1 || choice > 5) {
		cout << "Wrong Choice, Please choose a possible Action!\nChoose:";
		cin >> choice;
	}


	if (choice == 1) {//Translate
		float tx, ty, tz;
		cout << "tx: ";
		cin >> tx;
		cout << "ty: ";
		cin >> ty;
		cout << "tx: ";
		cin >> tz;
		polyhedras[currentID - 1].translate(tx, ty, tz);
		setBoundaryBox();
		writeBack();
	}

	else if (choice == 2) {//Scaling
		float scale;
		cout << "Scale Factor: ";
		cin >> scale;
		polyhedras[currentID - 1].scale(scale);
		setBoundaryBox();
		writeBack();
	}

	else if (choice == 3) {//Rotation
		float alpha;
		Vertex a;
		Vertex b;
		cout << "Radians Rotate: ";
		cin >> alpha;
		cout << "Point 1 x: ";
		cin >> a.x;
		cout << "Point 1 y: ";
		cin >> a.y;
		cout << "Point 1 z: ";
		cin >> a.z;
		cout << "Point 2 x: ";
		cin >> b.x;
		cout << "Point 2 y: ";
		cin >> b.y;
		cout << "Point 2 z: ";
		cin >> b.z;
		polyhedras[currentID - 1].rotate(alpha, a, b);
		setBoundaryBox();
		writeBack();
	}
	//Ask whether or not you want to draw all the polyhedras at once, or one at a time
	cout << "Would you like to draw all the Polyhedra at once? (y/n)\nChooice:";
	cin >> all;
	while (all != 'y' && all != 'n') {
		cin >> all;
		cout << "y for yes, b for no\nChooice:";
	}

	return;
}

/////////////////////////////////////////////////////////////////////////////////
//Sets Screen to divide it by 4 differnt quadrants, and adds which one is which//
/////////////////////////////////////////////////////////////////////////////////
void setScreen() {
	//Draw borders
	drawBresenham(0, 250, 500, 250);
	drawBresenham(250, 0, 250, 500);

	//Draw XY
	drawBresenham(0, 270, 10, 250);
	drawBresenham(10, 270, 0, 250);
	drawBresenham(20, 270, 25, 260);
	drawBresenham(30, 270, 20, 250);

	//Draw XZ
	drawBresenham(250, 270, 260, 250);
	drawBresenham(260, 270, 250, 250);
	drawBresenham(270, 270, 280, 270);
	drawBresenham(280, 270, 270, 253);
	drawBresenham(270, 253, 280, 253);

	//Draw YZ
	drawBresenham(0, 20, 5, 10);
	drawBresenham(10, 20, 0, 0);
	drawBresenham(20, 20, 30, 20);
	drawBresenham(30, 20, 20, 3);
	drawBresenham(20, 3, 30, 3);
}

//Set a new boundary box if values are outside the current range!
void setBoundaryBox() {
	Boundary temp;
	for (int i = 0; i < polyhedraCount; i++) {
		//Get the boundaries of the new values
		temp = polyhedras[i].getBoundary();
		//Check if the new values cause a new boundary, if so change boundary Box size
		//Add or subtract 10% to have some space between the screen edges
		if (max(boundaryBox.Xmax, temp.Xmax) > boundaryBox.Xmax) {
			boundaryBox.Xmax = temp.Xmax;
		}
		if (max(boundaryBox.Xmax, temp.Ymax) > boundaryBox.Ymax) {
			boundaryBox.Ymax = temp.Ymax;
		}
		if (max(boundaryBox.Zmax, temp.Zmax) > boundaryBox.Zmax) {
			boundaryBox.Ymax = temp.Ymax;
		}
		if (min(boundaryBox.Xmin, temp.Xmin) < boundaryBox.Xmin) {
			boundaryBox.Xmin = temp.Xmin;
		}
		if (min(boundaryBox.Ymin, temp.Ymin) < boundaryBox.Ymin) {
			boundaryBox.Ymin = temp.Ymin;
		}
		if (min(boundaryBox.Zmin, temp.Zmin) < boundaryBox.Zmin) {
			boundaryBox.Zmin = temp.Zmin;
		}
	}
	//Create new values for deltas
	deltaX = boundaryBox.Xmax - boundaryBox.Xmin;
	deltaY = boundaryBox.Ymax - boundaryBox.Ymin;
	deltaZ = boundaryBox.Zmax - boundaryBox.Zmin;
	//Take max of the deltas
	Delta = max(deltaX, deltaY);
	Delta = max(Delta, deltaZ);
}

//Turn world to NDC then to Pixel
Vertex toNDCtoPixel(float x, float y, float z, int mode) {
	Vertex point{ 0,0,0 };
	float xNDC, yNDC, zNDC;
	//If mode is 1, set for XY quadrant
	if (mode == 1) {
		xNDC = (x - boundaryBox.Xmin) / Delta;
		yNDC = (y - boundaryBox.Ymin) / Delta;
		point.x = int(xNDC * 230);
		point.y = int(yNDC * 230);
		point.y += 259;
		point.x += 9;
		return point;
	}//If mode is 2, set for XZ quadrant
	else if (mode == 2) {
		xNDC = (x - boundaryBox.Xmin) / Delta;
		zNDC = (z - boundaryBox.Zmin) / Delta;
		point.x = int(xNDC * 230);
		point.z = int(zNDC * 230);
		point.x += 259;
		point.z += 259;
		return point;
	}
	//Else it set for YZ quadrant
	yNDC = (y - boundaryBox.Ymin) / Delta;
	zNDC = (z - boundaryBox.Zmin) / Delta;
	point.y = int(yNDC * 230);
	point.z = int(zNDC * 230);
	point.y += 9;
	point.z += 9;
	return point;
}


//Old functions I no longer need, were combined above for toNDCtoPixel
/*
//Turn World to NDC
void toNDC(float x, float y, float z) {
	float xNDC, yNDC, zNDC;
	xNDC = (x - boundaryBox.Xmin) / Delta;
	yNDC = (y - boundaryBox.Ymin) / Delta;
	zNDC = (z - boundaryBox.Zmin) / Delta;
}

//Turn NDC to Pixel
void toPixel(float x, float y, int mode) {
	//Multiply by 250 to fit into quadrant
	x *= 250;
	y *= 250;
	//If mode is 1, set for XY quadrant
	if (mode == 1) {
		y += 250;
	}//If mode is 2, set for XZ quadrant
	else if (mode == 2) {
		x += 250;
		y += 250;
	}
	//Else it is set for XY quadrant
	
}
*/