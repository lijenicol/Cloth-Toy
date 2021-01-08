////////////////////////////////////////
// Tester.cpp
////////////////////////////////////////

#include "Tester.h"
#include <iostream>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glut.h"
#include "imgui/imgui_impl_opengl2.h"

////////////////////////////////////////////////////////////////////////////////

static Tester *TESTER=0;

int main(int argc, char **argv) {
	glutInit(&argc, argv);

	TESTER=new Tester("Cloth Toy",argc,argv);
	glutMainLoop();
	delete TESTER;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

// These are really HACKS to make glut call member functions instead of static functions
static void display()									{TESTER->Draw();}
static void idle()										{TESTER->Update();}
static void resize(int x,int y)							{TESTER->Resize(x,y);}
static void keyboard(unsigned char key,int x,int y)		{TESTER->Keyboard(key,x,y);}
static void specialKeys(int key, int x, int y) { TESTER->SpecialKeys(key, x, y); }
static void mousebutton(int btn,int state,int x,int y)	{TESTER->MouseButton(btn,state,x,y);}
static void mousemotion(int x, int y)					{TESTER->MouseMotion(x,y);}

////////////////////////////////////////////////////////////////////////////////

Tester::Tester(const char *windowTitle,int argc,char **argv) {
	WinX=1280;
	WinY=720;
	LeftDown=MiddleDown=RightDown=false;
	MouseX=MouseY=0;
	prevTime = 0;
	currentTime = 0;
	deltaTime = 0;
	windSpeed = 1.0f;
	windX = 0.0f;
	windY = 0.0f;
	windZ = 1.0f;
	toggleWind = false;
	interactMode = false;
	displayGround = true;

	// Create the window
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize( WinX, WinY );
	glutInitWindowPosition( 100, 100 );
	WindowHandle = glutCreateWindow( windowTitle );
	glutSetWindowTitle( windowTitle );
	glutSetWindow( WindowHandle );

	// Background color
	glClearColor( 0.95, 0.95, 0.95, 1. );

	// Callbacks
	glutDisplayFunc( display );
	glutIdleFunc( idle );
	glutKeyboardFunc( keyboard );
	glutSpecialFunc(specialKeys);
	glutMouseFunc( mousebutton );
	glutMotionFunc( mousemotion );
	glutPassiveMotionFunc( mousemotion );
	glutReshapeFunc( resize );

	// Initialize GLEW
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Setup Dear ImGui context
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplGLUT_Init();
	ImGui_ImplGLUT_InstallFuncs();
	ImGui_ImplOpenGL2_Init();

	// Initialize components
	Program=new ShaderProgram("Model.glsl",ShaderProgram::eRender);
	Cam=new Camera;
	Cam->SetAspect(float(WinX)/float(WinY));

	//initialize cloth
	cloth = new Cloth(30,30);

	//initialize plane (only for rendering)
	plane = new Plane();
}

////////////////////////////////////////////////////////////////////////////////

Tester::~Tester() {
	delete Program;
	delete Cam;

	// Cleanup imgui
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplGLUT_Shutdown();
	ImGui::DestroyContext();

	glFinish();
	glutDestroyWindow(WindowHandle);
}

////////////////////////////////////////////////////////////////////////////////

void Tester::Update() {
	//calculate delta time
	currentTime = glutGet(GLUT_ELAPSED_TIME);
	deltaTime = (currentTime - prevTime)/1000.f;
	prevTime = currentTime;
	
	// Calculate wind direction/speed
	glm::vec3 wind(windSpeed * glm::normalize(glm::vec3(windX, windY, windZ)));

	// Update the components in the world
	Cam->Update();

	cloth->applyAcceleration(glm::vec3(0,-9.8,0));
	cloth->applyAerodynamicForce(toggleWind ? wind : glm::vec3(0, 0, 0));
	cloth->update(deltaTime);

	// Tell glut to re-display the scene
	glutSetWindow(WindowHandle);
	glutPostRedisplay();
}

////////////////////////////////////////////////////////////////////////////////

void Tester::Reset() {
	cloth->Reset();
}

////////////////////////////////////////////////////////////////////////////////

void Tester::Draw() {
	// Begin drawing scene
	glViewport(0, 0, WinX, WinY);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//draw plane
	if(displayGround)
		plane->draw(Cam->GetViewProjectMtx(), Program->GetProgramID());

	// Draw cloth
	cloth->draw(Cam->GetViewProjectMtx(), Program->GetProgramID());

	// Render GUI
	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplGLUT_NewFrame();
	{
		ImGui::Begin("Cloth Toy"); 
		ImGui::Text("Move cloth around with WASD keys. To move up/down use U and I keys.");
		ImGui::Checkbox("Interact w/ Cloth", &interactMode);
		ImGui::Checkbox("Display Ground", &displayGround);
		ImGui::Checkbox("Wind", &toggleWind);
		ImGui::SliderFloat("Wind Speed", &windSpeed, 0.f, 15.f);

		// Wind direction
		ImGui::PushItemWidth(100);
		ImGui::Text("Wind Direction:"); ImGui::SameLine();
		ImGui::SliderFloat("X", &windX, -1.f, 1.f); ImGui::SameLine();
		ImGui::SliderFloat("Y", &windY, -1.f, 1.f); ImGui::SameLine();
		ImGui::SliderFloat("Z", &windZ, -1.f, 1.f); 
		ImGui::PopItemWidth();

		if (ImGui::Button("Reset"))
			Reset();
		ImGui::SameLine();
		if (ImGui::Button("Free Cloth"))
			cloth->freeClothFixedPoints();

		// Framerate
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}
	glUseProgram(0);
	ImGui::Render();
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

	// Finish drawing scene
	glFinish();
	glutSwapBuffers();
}

////////////////////////////////////////////////////////////////////////////////

void Tester::Quit() {
	glFinish();
	glutDestroyWindow(WindowHandle);
	exit(0);
}

////////////////////////////////////////////////////////////////////////////////

void Tester::Resize(int x,int y) {
	WinX = x;
	WinY = y;
	Cam->SetAspect(float(WinX)/float(WinY));

	ImGui_ImplGLUT_ReshapeFunc(x, y);
}

////////////////////////////////////////////////////////////////////////////////

void Tester::Keyboard(int key,int x,int y) {
	ImGui_ImplGLUT_KeyboardFunc(key, x, y);

	switch(key) {
		case 0x1b:		// Escape
			Quit();
			break;
		case 'r':
			Reset();
			break;
		case 'p':
			Cloth::printCloth(cloth);
			break;
		case 'b':
			toggleWind = !toggleWind;
			std::cout << (toggleWind ? "WIND ON" : "WIND OFF") << std::endl;
			break;
		case 'q':
			interactMode = !interactMode;
			std::cout << (interactMode ? "INTERACTION MODE" : "LOOK MODE") << std::endl;
			break;
		case 'f':
			cloth->freeClothFixedPoints();
			break;
		case 'w':
			cloth->movePosition(glm::vec3(0, 0, -0.05f));
			break;
		case 's':
			cloth->movePosition(glm::vec3(0, 0, 0.05f));
			break;
		case 'a':
			cloth->movePosition(glm::vec3(-0.05f, 0, 0));
			break;
		case 'd':
			cloth->movePosition(glm::vec3(0.05f, 0, 0));
			break;
		case 'u':
			cloth->movePosition(glm::vec3(0, 0.05f, 0));
			break;
		case 'i':
			cloth->movePosition(glm::vec3(0, -0.05f, 0));
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////

void Tester::SpecialKeys(int key, int x, int y) {
	switch (key) {	
		case GLUT_KEY_UP:
			windSpeed += 0.1f;
			std::cout << "WIND SPEED: " << windSpeed << std::endl;
			break;
		case GLUT_KEY_DOWN:
			windSpeed -= 0.1f;
			std::cout << "WIND SPEED: " << windSpeed << std::endl;
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////

void Tester::MouseButton(int btn,int state,int x,int y) {
	// Send mouse inputs to GUI
	ImGui_ImplGLUT_MouseFunc(btn, state, x, y);

	// Don't read input if mouse inside GUI
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	if(btn==GLUT_LEFT_BUTTON) {
		LeftDown = (state==GLUT_DOWN);

		//cloth interactions
		if (LeftDown && interactMode) {
			//get ray direction
			glm::vec3 rayDirection = calculateRayDirectionFromScreen(x, y);

			//select particle on cloth if there exists such intersection
			cloth->selectParticle(rayDirection, Cam->getPosition());
		}
		else if (!LeftDown && interactMode) {
			//deselect point
			cloth->deselectParticle();
		}
	}
	else if(btn==GLUT_MIDDLE_BUTTON) {
		MiddleDown = (state==GLUT_DOWN);
	}
	else if(btn==GLUT_RIGHT_BUTTON) {
		RightDown = (state==GLUT_DOWN);
	}
}

////////////////////////////////////////////////////////////////////////////////

glm::vec3 Tester::calculateRayDirectionFromScreen(int x, int y) {
	//flip the y axis 
	y = WinY - y;

	//calculate ray direction in NDC
	glm::vec4 rayStart_NDC(
		((float)x / (float)WinX - 0.5f) * 2.0f,
		((float)y / (float)WinY - 0.5f) * 2.0f,
		-1.0,
		1.0f
	);
	glm::vec4 rayEnd_NDC(
		((float)x / (float)WinX - 0.5f) * 2.0f,
		((float)y / (float)WinY - 0.5f) * 2.0f,
		0.0,
		1.0f
	);

	//perform transformations to get points in world coordinates
	glm::mat4 M = glm::inverse(Cam->GetViewProjectMtx());
	glm::vec4 rayStart_world = M * rayStart_NDC; rayStart_world /= rayStart_world.w;
	glm::vec4 rayEnd_world = M * rayEnd_NDC; rayEnd_world /= rayEnd_world.w;

	//calculate ray direction in world coordinates
	glm::vec3 rayDirection(rayEnd_world - rayStart_world);
	return glm::normalize(rayDirection);
}

////////////////////////////////////////////////////////////////////////////////

void Tester::MouseMotion(int nx,int ny) {
	// Send mouse inputs to GUI
	ImGui_ImplGLUT_MotionFunc(nx, ny);

	// Don't read input if mouse inside GUI
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	int maxDelta=100;
	int dx = glm::clamp(nx - MouseX,-maxDelta,maxDelta);
	int dy = glm::clamp(-(ny - MouseY),-maxDelta,maxDelta);

	MouseX = nx;
	MouseY = ny;

	// Move camera
	// NOTE: this should really be part of Camera::Update()
	if(LeftDown && !interactMode) {
		const float rate=1.0f;
		Cam->SetAzimuth(Cam->GetAzimuth()+dx*rate);
		Cam->SetIncline(glm::clamp(Cam->GetIncline()-dy*rate,-90.0f,90.0f));
	}
	if(RightDown) {
		const float rate=0.005f;
		float dist=glm::clamp(Cam->GetDistance()*(1.0f-dx*rate),0.01f,1000.0f);
		Cam->SetDistance(dist);
	}

	//Interact with cloth 
	if (LeftDown && interactMode) {
		glm::vec3 ray = calculateRayDirectionFromScreen(nx, ny);
		cloth->moveSelectedParticle(ray, Cam->getPosition());
	}
}

////////////////////////////////////////////////////////////////////////////////
