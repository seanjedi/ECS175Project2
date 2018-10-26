#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include <vector> 
#include <cmath>
#include <string>

using namespace std;

////////////////////
//Global Variables//
////////////////////
string inputFile;
int windowSizeX, windowSizeY, style, mode, polyhedraCount, currentID;
float viewXmin, viewXmax, viewYmin, viewYmax;
ifstream inFile;


//////////////////////////
//Function Defininitions//
//////////////////////////
void display();
void writeBack();
void getSettings(int, char*[]);
void getSettings2();

//Vertex Struct
struct Vertex
{
	float x;
	float y;
	float z;
};
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
	void drawPolygon() {

	}

	void translate(float tx, float ty, float tz) {
		for (int i = 0; i < vertexCount; i++) {
			vertices[i].x += tx;
			vertices[i].y += ty;
			vertices[i].z += tz;
		}
	}

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

	void rotate(double theta, Vertex p1, Vertex p2) {
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

	}

	//Write Polygon Buffer to output file!
	void writeBack(std::ofstream& file) {
		file << vertexCount << endl;;
		for (int i = 0; i < vertexCount; i++) {
			file << vertices[i].x << " " << vertices[i].y << endl;
		}
		for (int i = 0; i < edgeCount; i++) {
			file << edges[i].a << " " << edges[i].b << endl;
		}
	}

};

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

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE);
	//set window size to windowSizeX by windowSizeX
	glutInitWindowSize(500, 500);
	//set window position
	glutInitWindowPosition(100, 100);

	//create and set main window title
	int MainWindow = glutCreateWindow("Hello Graphics!!");
	glClearColor(0, 0, 0, 0); //clears the buffer of OpenGL
	//sets display function
	while (1) {
		display();
		getSettings2();
	}
	
	//glutDisplayFunc(display);

	glutMainLoop();//main display loop, will display until terminate
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//main display loop, this function will be called again and again by OpenGL//
/////////////////////////////////////////////////////////////////////////////
void display()
{
	//Misc.
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();


	
	//draws pixel on screen, width and height must match pixel buffer dimension
	//glDrawPixels(windowSizeX, windowSizeY, GL_RGB, GL_FLOAT, PixelBuffer);

	//window refresh
	glFlush();
}

//////////////////////////////
//Handles File Input//////////
//Handles Initial User Input//
//////////////////////////////
void getSettings(int argc, char* argv[]){
		if (argc > 2) {
		cerr << "Too Many Arguments!\nStopping Execution";
		exit(1);
	}
	else if (argc == 1) {
		cout << "Specify Input File: ";
		getline(cin, inputFile);
		inFile.open(inputFile);
		if (!inFile) {
			cerr << "Unable to open input file \nStopping Execution";
			exit(1);
		}
	}
	else {
		inFile.open(argv[1]);
		if (!inFile) {
			cerr << "Unable to open input file \nStopping Execution";
			exit(1);
		}
	}
		string space;
		int vertexCount, edgeCount, point1, point2;
		float x, y, z;

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
	getSettings2();

}

void writeBack() {
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

	cout << "Which Polyhedra would you like to manipulate? (id's 1 to " << polyhedraCount << ")\nID:";
	while (currentID <= 0 || currentID > polyhedraCount) {
			cin >> currentID;
			if (currentID > polyhedraCount || currentID <= 0) {
				cout << "Wrong ID, Please choose a possible ID!\nID:";
			}
	}
	cout << "Which transformation would you like to do next?\n1) Translate\n2) Scale\n3) Rotation\n4) Display\n5) exit\nChoose:";
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
		//writeBack();
	}

	else if (choice == 2) {//Scaling
		float scale;
		cout << "Scale Factor: ";
		cin >> scale;
		polyhedras[currentID - 1].scale(scale);
		//writeBack();
	}

	else if (choice == 3) {//Rotation
		float alpha;
		Vertex a;
		Vertex b;
		cout << "Degrees Rotate: ";
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
		//writeBack();
	}
	else if (choice == 5) {//Exit
		exit(0);
	}
	return;
}