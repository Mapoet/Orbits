
#include "glcontrols.h"

HDC			hDC = NULL;		// Private GDI Device Context
HGLRC		hRC = NULL;		// Permanent Rendering Context
HWND		hWnd = NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application

uint		W = 1920;
uint		H = 1080;
BOOL		keys[256];			// Array Used For The Keyboard Routine
BOOL		active = TRUE;		// Window Active Flag Set To TRUE By Default
BOOL		fullscreen = TRUE;	// Fullscreen Flag Set To Fullscreen Mode By Default
uint		vidx[] = {  0,1, 2 };	//view indexs
/******************************begin prepare data****************************************************/
char files[5][200];//commond file,inputfile,outfile,ufile,xyzfile with controlfile and datafile from paramentfile;
/*view data*/
real		TimeScales;// = 1;
real		LengthScales;// = 149597870700;
real		MoveStep;// = 149597870700;
real		ViewScales;// = 3 * 149597870700;
real		AngleStep;// = 0.5;
ViewSet		view = { { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, 0, 0, -1, {0} };// = { { 0, 0, 0 }, { 12 * 149597870700, 12 * 149597870700, 12 * 149597870700 }, { 0, 0, 0 }, { 0, 0, 149597870700 }, { 0, 0, 0 }, 0, 40, -1 };
Mouse		mouse = { {0 }, { 0 }, 0, 0, 0, 0, 0 };
real		mk = 1.0;
Pos			EM = {0,0,0};
real		LEM = 0;
BOOL		withline = FALSE;
GLuint		g_cactus[NPLANET];
GLUquadricObj*g_text;
GLfloat		light_position[] = { 0.0, 0.0, 0.0, 0.0 };
GLfloat		light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };	
GLfloat		light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };	
GLfloat		light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat		basesize,rangesize[2];
GLfloat     pointsize = 0;
/*******************************/
real		time= 0;
real		rkstep;// = DS / 10.;
BOOL		isPause = FALSE;
real		y[2 * NDEM*NPLANET], err[2 * NDEM*NPLANET];
uint		nbody;// = 2;
FILE*		fpath;
real		MassCenter[NDEM];
real		Mass = 0;
/*SOLAR SYSTEM*/
static Body M[NPLANET];
static Pos XS[NPLANET] ,VS[NPLANET] ;
//
//
int WINAPI WinMain(HINSTANCE	hInstance,			// Instance
	HINSTANCE	hPrevInstance,		// Previous Instance
	LPSTR		lpCmdLine,			// Command Line Parameters
	int			nCmdShow)			// Window Show State
{
	MSG		msg;									// Windows Message Structure
	BOOL	done = FALSE;								// Bool Variable To Exit Loop
	// Ask The User Which Screen Mode They Prefer
	if (MessageBox(NULL, "Would You Like To Run In Fullscreen Mode?", "Start FullScreen?", MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		fullscreen = FALSE;							// Windowed Mode
	}

	// Create Our OpenGL Window
	if (!CreateGLWindow("Multiply Bodies", W, H, 16, fullscreen))
	{
		return 0;									// Quit If Window Was Not Created
	}
	glGetFloatv(GL_POINT_SIZE_GRANULARITY, &basesize);
	glGetFloatv(GL_POINT_SIZE_RANGE, rangesize);
	pointsize = rangesize[0];
	readInitData();
	fpath = fopen(files[4], "w");
	for (unsigned int i = 0; i < nbody; i++)
	{
		for (uint j = 0; j < ndem; j++)
		{
			y[i * ndem + j] = XS[i].pos[j];
			y[(i + nbody) * ndem + j] = VS[i].pos[j];
		}
		if (!LoadT8(M[i].img, g_cactus+i))
			return FALSE;
	}
	OutPutU();
	InitRungeKutta7(2 * ndem*NPLANET);
	while (!done)									// Loop That Runs While done=FALSE
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message == WM_QUIT)				// Have We Received A Quit Message?
			{
				done = TRUE;							// If So done=TRUE
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else										// If There Are No Messages
		{
			// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if(UpdateKeys()==1)
				return 0;
			if (!isPause)
				UpdateData();
			if ((active && !DrawGLScene()) || keys[VK_ESCAPE])	// Active?  Was There A Quit Received?
			{
				done = TRUE;							// ESC or DrawGLScene Signalled A Quit
			}
			else									// Not Time To Quit, Update Screen
			{
				SwapBuffers(hDC);					// Swap Buffers (Double Buffering)
			}
			Sleep(50);
		}
	}
	UninitRungeKutta7();
	//free(g_text);
	fclose(fpath);
	// Shutdown
	KillGLWindow();									// Kill The Window
	return (msg.wParam);							// Exit The Program
}
LRESULT CALLBACK WndProc(HWND	hWnd,			// Handle For This Window
	UINT	uMsg,			// Message For This Window
	WPARAM	wParam,			// Additional Message Information
	LPARAM	lParam)			// Additional Message Information
{
	switch (uMsg)									// Check For Windows Messages
	{
		case WM_ACTIVATE:							// Watch For Window Activate Message
		{
			if (!HIWORD(wParam))					// Check Minimization State
			{
				active = TRUE;						// Program Is Active
			}
			else
			{
				active = FALSE;						// Program Is No Longer Active
			}

			return 0;								// Return To The Message Loop
		}

		case WM_SYSCOMMAND:
		{
			switch (wParam)
			{
			case SC_SCREENSAVE:
			case SC_MONITORPOWER:
				return 0;
			}
			break;
		}

		case WM_CLOSE:								// Did We Receive A Close Message?
		{
			PostQuitMessage(0);						// Send A Quit Message
			return 0;								// Jump Back
		}

		case WM_KEYDOWN:							// Is A Key Being Held Down?
		{
			keys[wParam] = TRUE;					// If So, Mark It As TRUE
			return 0;								// Jump Back
		}

		case WM_KEYUP:								// Has A Key Been Released?
		{
			keys[wParam] = FALSE;					// If So, Mark It As FALSE
			return 0;								// Jump Back
		}

		case WM_SIZE:								// Resize The OpenGL Window
		{
			ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));  // LoWord=Width, HiWord=Height
			return 0;								// Jump Back
		}
		case WM_LBUTTONDOWN:
		{
			mouse.keyp[0] = mouse.key[0];
			mouse.key[0] = TRUE;
			mouse.xp = mouse.xp;
			mouse.yp = mouse.yp;
			mouse.x = LOWORD(lParam);
			mouse.y = HIWORD(lParam);
			return 0;								// Jump Back
		}
		case WM_LBUTTONUP:
		{
			mouse.keyp[0] = mouse.key[0];
			mouse.key[0] = FALSE;
			mouse.xp = mouse.xp;
			mouse.yp = mouse.yp;
			mouse.x = LOWORD(lParam);
			mouse.y = HIWORD(lParam);
			return 0;								// Jump Back
		}
		case WM_MBUTTONDOWN:
		{
			mouse.keyp[0] = mouse.key[0];
			mouse.key[1] = TRUE;
			mouse.xp = mouse.xp;
			mouse.yp = mouse.yp;
			mouse.x = LOWORD(lParam);
			mouse.y = HIWORD(lParam);
			return 0;								// Jump Back
		}
		case WM_MBUTTONUP:
		{
			mouse.keyp[0] = mouse.key[0];
			mouse.key[1] = FALSE;
			mouse.xp = mouse.xp;
			mouse.yp = mouse.yp;
			mouse.x = LOWORD(lParam);
			mouse.y = HIWORD(lParam);
			return 0;								// Jump Back
		}
		case WM_RBUTTONDOWN:
		{
			mouse.keyp[2] = mouse.key[2];
			mouse.key[2] = TRUE;
			mouse.xp = mouse.xp;
			mouse.yp = mouse.yp;
			mouse.x = LOWORD(lParam);
			mouse.y = HIWORD(lParam);
			return 0;								// Jump Back
		}
		case WM_RBUTTONUP:
		{
			mouse.keyp[2] = mouse.key[2];
			mouse.key[2] = FALSE;
			mouse.xp = mouse.xp;
			mouse.yp = mouse.yp;
			mouse.x = LOWORD(lParam);
			mouse.y = HIWORD(lParam);
			return 0;								// Jump Back
		}
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
*	title			- Title To Appear At The Top Of The Window				*
*	width			- Width Of The GL Window Or Fullscreen Mode				*
*	height			- Height Of The GL Window Or Fullscreen Mode			*
*	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
*	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/
BOOL CreateGLWindow(char* title, int width, int height, int bits, BOOL fullscreenflag)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left = (long)0;			// Set Left Value To 0
	WindowRect.right = (long)width;		// Set Right Value To Requested Width
	WindowRect.top = (long)0;				// Set Top Value To 0
	WindowRect.bottom = (long)height;		// Set Bottom Value To Requested Height

	fullscreen = fullscreenflag;			// Set The Global Fullscreen Flag

	hInstance = GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc = (WNDPROC)WndProc;					// WndProc Handles Messages
	wc.cbClsExtra = 0;									// No Extra Window Data
	wc.cbWndExtra = 0;									// No Extra Window Data
	wc.hInstance = hInstance;							// Set The Instance
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground = NULL;									// No Background Required For GL
	wc.lpszMenuName = NULL;									// We Don't Want A Menu
	wc.lpszClassName = "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL, "Failed To Register The Window Class.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}

	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth = width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight = height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel = bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL, "The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?", "NeHe GL", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				fullscreen = FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL, "Program Will Now Close.", "ERROR", MB_OK | MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle = WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle = WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle = WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd = CreateWindowEx(dwExStyle,							// Extended Style For The Window
		"OpenGL",							// Class Name
		title,								// Window Title
		dwStyle |							// Defined Window Style
		WS_CLIPSIBLINGS |					// Required Window Style
		WS_CLIPCHILDREN,					// Required Window Style
		0, 0,								// Window Position
		WindowRect.right - WindowRect.left,	// Calculate Window Width
		WindowRect.bottom - WindowRect.top,	// Calculate Window Height
		NULL,								// No Parent Window
		NULL,								// No Menu
		hInstance,							// Instance
		NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Window Creation Error.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	if (!(hDC = GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Create A GL Device Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Find A Suitable PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!SetPixelFormat(hDC, PixelFormat, &pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Set The PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC = wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Create A GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!wglMakeCurrent(hDC, hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Activate The GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd, SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Initialization Failed.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}
GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	W = width;
	H = height;
	if (H == 0)										// Prevent A Divide By Zero By
	{
		H = 1;										// Making Height Equal One
	}
	/********************************************/
	Mass = 0;
	if (view.idnum == -1)for (uint i = 0; i < nbody; i++)Mass += M[i].M;
	else for (int i = 0; i < view.idnum; i++)Mass += M[view.ids[i]].M;
	for (uint k = 0; k < ndem; k++)
	{
		MassCenter[k] = 0;
		if (view.idnum == -1)for (uint i = 0; i < nbody; i++)MassCenter[k] += y[i * ndem + k] * M[i].M;
		else for (int i = 0; i < view.idnum; i++)MassCenter[k] += y[view.ids[i] * ndem + k] * M[view.ids[i]].M;
		MassCenter[k] /= Mass;
		view.EyesPos.pos[k] += MassCenter[k] - view.CenterPos.pos[k];
		view.CenterPos.pos[k] = MassCenter[k];
	}
	/*******************************************/
	glViewport(0, 0, W, H);						// Reset The Current Viewport
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix
	// Calculate The Aspect Ratio Of The Window
	gluPerspective(view.ViewAngle, (GLfloat)width / (GLfloat)height, 0.00001f, ViewScales / AU*1000.0f);
	gluLookAt(view.EyesPos.pos[vidx[0]] / ViewScales, view.EyesPos.pos[vidx[1]] / ViewScales, view.EyesPos.pos[vidx[2]] / ViewScales, view.CenterPos.pos[vidx[0]] / ViewScales, view.CenterPos.pos[vidx[1]] / ViewScales, view.CenterPos.pos[vidx[2]] / ViewScales, view.UpPos.pos[vidx[0]] / ViewScales, view.UpPos.pos[vidx[1]] / ViewScales, view.UpPos.pos[vidx[2]] / ViewScales);
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
	glClearColor(0.1f, 0.1f, 0.15f, 1.f);
	if (withline)
		glClear(/*GL_COLOR_BUFFER_BIT |*/ GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	else
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
}
int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_POINT_SMOOTH);
	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	/***************/
	g_text = gluNewQuadric();
	gluQuadricNormals(g_text, GLU_SMOOTH);
	gluQuadricTexture(g_text, GL_TRUE);
	/****************/
	Mass = 0;
	if (view.idnum == -1)for (uint i = 0; i < nbody; i++)Mass += M[i].M;
	else for (int i = 0; i < view.idnum; i++)Mass += M[view.ids[i]].M;
	for (uint k = 0; k < ndem; k++)
	{
		MassCenter[k] = 0;
		if (view.idnum == -1)for (uint i = 0; i < nbody; i++)MassCenter[k] += y[i * ndem + k] * M[i].M;
		else for (int i = 0; i < view.idnum; i++)MassCenter[k] += y[view.ids[i] * ndem + k] * M[view.ids[i]].M;
		MassCenter[k] /= Mass;
		view.EyesPos.pos[k] += MassCenter[k] - view.CenterPos.pos[k];
		view.CenterPos.pos[k] = MassCenter[k];
	}
	glMatrixMode(GL_PROJECTION);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Current Modelview Matrix
	gluPerspective(view.ViewAngle, (GLfloat)W / (GLfloat)H, 0.00001f, ViewScales / AU*1000.0f);
	gluLookAt(view.EyesPos.pos[vidx[0]] / ViewScales, view.EyesPos.pos[vidx[1]] / ViewScales, view.EyesPos.pos[vidx[2]] / ViewScales, view.CenterPos.pos[vidx[0]] / ViewScales, view.CenterPos.pos[vidx[1]] / ViewScales, view.CenterPos.pos[vidx[2]] / ViewScales, view.UpPos.pos[vidx[0]] / ViewScales, view.UpPos.pos[vidx[1]] / ViewScales, view.UpPos.pos[vidx[2]] / ViewScales);
	/***************/
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glClearColor(0.1f, 0.1f, 0.15f, 0.5f);
	if (withline)
		glClear(/*GL_COLOR_BUFFER_BIT |*/ GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	else
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	//glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	//glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	//glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	//glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	return TRUE;										// Initialization Went OK
}

GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
{
	if (fullscreen)										// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL, 0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL, NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL, "Release Of DC And RC Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL, "Release Rendering Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}
		hRC = NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd, hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL, "Release Device Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hDC = NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL, "Could Not Release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL", hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL, "Could Not Unregister Class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;									// Set hInstance To NULL
	}
}

int DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
{
	Mass = 0;
	if (view.idnum == -1)for (uint i = 0; i < nbody; i++)Mass += M[i].M;
	else for (int i = 0; i < view.idnum; i++)Mass += M[view.ids[i]].M;
	for (uint k = 0; k < ndem; k++)
	{
		MassCenter[k] = 0;
		if (view.idnum == -1)for (uint i = 0; i < nbody; i++)MassCenter[k] += y[i * ndem + k] * M[i].M;
		else for (int i = 0; i < view.idnum; i++)MassCenter[k] += y[view.ids[i] * ndem + k] * M[view.ids[i]].M;
		MassCenter[k] /= Mass;
		view.EyesPos.pos[k] += MassCenter[k] - view.CenterPos.pos[k];
		view.CenterPos.pos[k] = MassCenter[k];
	}
	glMatrixMode(GL_PROJECTION);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Current Modelview Matrix
	gluPerspective(view.ViewAngle, (GLfloat)W / (GLfloat)H, 0.00001f, ViewScales / AU*1000.0f);
	gluLookAt(view.EyesPos.pos[vidx[0]] / ViewScales, view.EyesPos.pos[vidx[1]] / ViewScales, view.EyesPos.pos[vidx[2]] / ViewScales, view.CenterPos.pos[vidx[0]] / ViewScales, view.CenterPos.pos[vidx[1]] / ViewScales, view.CenterPos.pos[vidx[2]] / ViewScales, view.UpPos.pos[vidx[0]] / ViewScales, view.UpPos.pos[vidx[1]] / ViewScales, view.UpPos.pos[vidx[2]] / ViewScales);
	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	//glClearColor(0.2f, 0.2f, 0.45f, 0.5f);				// Black Background
	//glClearColor(1.f, 1.f, 1.f, 0.5f);				// Black Background
	glClearColor(0.1f, 0.1f, 0.15f, 0.5f);
	if (withline)
		glClear(/*GL_COLOR_BUFFER_BIT |*/ GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	else
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer

	glLoadIdentity();									// Reset The Current Modelview Matrix
	glRotatef(view.RotAngle.pos[0], 1.0f, 0.0f, 0.0f);
	glRotatef(view.RotAngle.pos[1], 0.0f, 1.0f, 0.0f);
	glRotatef(view.RotAngle.pos[2], 0.0f, 0.0f, 1.0f);
	EM = psp(view.CenterPos, view.EyesPos);
	LEM = pabs(EM);
	glRotatef(view.DRot, EM.pos[0] / ViewScales, EM.pos[1] / ViewScales, EM.pos[2] / ViewScales);
	//points
	if (withline)
	{
		glPointSize(pointsize);
		glBegin(GL_POINTS);								// Drawing Using Triangles
		for (uint i = 0; i < nbody; i++)
		{
			glColor3f(M[i].A, M[i].B, M[i].C);
			glVertex3f(y[i * ndem + vidx[0]] / ViewScales, y[i * ndem + vidx[1]] / ViewScales, y[i * ndem + vidx[2]] / ViewScales);					// Bottom Left
		}
		glEnd();
	}
	//sphere
	if (!withline)
	{
		glColor3f(1.f, 1.f, 1.f);
		for (uint i = 0; i < nbody; i++)
		{
			//glColor3f(M[i].A, M[i].B, M[i].C);
			glColor3f(1.f, 1.f, 1.f);
			glPushMatrix();
			glTranslatef(y[i * ndem + vidx[0]] / ViewScales, y[i * ndem + vidx[1]] / ViewScales, y[i * ndem + vidx[2]] / ViewScales);
			glPushMatrix();
			glBindTexture(GL_TEXTURE_2D, g_cactus[i]);
			glRotatef((GLfloat)time / M[i].T * 360, 0.0, 0.0, 1.0);
			gluSphere(g_text, M[i].R / ViewScales, 30, 30);
			gluQuadricTexture(g_text, GLU_TRUE);              //建立纹理坐标  
			gluQuadricDrawStyle(g_text, GLU_FILL);            //用面填充
			glPopMatrix();
			glPushMatrix();
			glColor3f(M[i].A, M[i].B, M[i].C);
			glRasterPos2f(M[i].R / ViewScales, M[i].R / ViewScales);
			drawString(M[i].name);
			glPopMatrix();
			glPopMatrix();
		}
		glColor3f(1.f, 1.f, 1.f);
	}
	return TRUE;										// Keep Going
}
GLvoid UpdateData(GLvoid){
	RungeKutta7Interation(Kernel, M, y, 2 * ndem*nbody, &time, rkstep, err);
	fprintf(fpath, "time:%10.3lf\n", time);
	for (uint i = 0; i < nbody; i++)
	{
		fprintf(fpath, "Body%2.2d:POS(%30.10lf", i + 1, y[i * ndem]);
		for (uint j = 1; j < ndem; j++)
			fprintf(fpath, ",%30.10lf", y[i * ndem + j]);
		fprintf(fpath, "),VEL(%30.10lf", y[(i + nbody) * ndem]);
		for (uint j = 1; j < ndem; j++)
			fprintf(fpath, ",%30.10lf", y[(i + nbody) * ndem + j]);
		fprintf(fpath, ")\n");
	}
}
uint UpdateKeys(GLvoid){
	//keys funcs
	if (keys[VK_F1])						// Is F1 Being Pressed?
	{
		keys[VK_F1] = FALSE;					// If So Make Key FALSE
		KillGLWindow();						// Kill Our Current Window
		fullscreen = !fullscreen;				// Toggle Fullscreen / Windowed Mode
		// Recreate Our OpenGL Window
		if (!CreateGLWindow("Multiply Bodies", W, H, 16, fullscreen))
		{
			return 1;						// Quit If Window Was Not Created
		}
	}
	//keyboards
	{
		if (keys['X'])
		{
			keys['X'] = FALSE;
			if (keys['R']){
				//keys['R'] = FALSE;
				if (keys[VK_SHIFT])
				{
					//keys[VK_SHIFT] = FALSE;
					view.RotAngle.pos[0] += AngleStep;
				}
				else
					view.RotAngle.pos[0] -= AngleStep;
				if (view.RotAngle.pos[0] > 180)view.RotAngle.pos[0] -= 360;
				if (view.RotAngle.pos[0] < -180)view.RotAngle.pos[0] += 360;
			}
			else if (keys['T']){
				//keys['T'] = FALSE;
				if (keys[VK_SHIFT])
				{
					//keys[VK_SHIFT] = FALSE;
					view.TransPos.pos[0] += MoveStep;
				}
				else
					view.TransPos.pos[0] -= MoveStep;
			}
			else{
				if (keys[VK_SHIFT])
				{
					//keys[VK_SHIFT] = FALSE;
					view.EyesPos.pos[0] += MoveStep;
				}
				else
					view.EyesPos.pos[0] -= MoveStep;
			}
			return 0;
		}
		if (keys['Y'])
		{
			keys['Y'] = FALSE;
			if (keys['R']){
				//keys['R'] = FALSE;
				if (keys[VK_SHIFT])
				{
					//keys[VK_SHIFT] = FALSE;
					view.RotAngle.pos[1] += AngleStep;
				}
				else
					view.RotAngle.pos[1] -= AngleStep;
				if (view.RotAngle.pos[1] > 180)view.RotAngle.pos[1] -= 360;
				if (view.RotAngle.pos[1] < -180)view.RotAngle.pos[1] += 360;
			}
			else if (keys['T']){
				//keys['T'] = FALSE;
				if (keys[VK_SHIFT])
				{
					//keys[VK_SHIFT] = FALSE;
					view.TransPos.pos[1] += MoveStep;
				}
				else
					view.TransPos.pos[1] -= MoveStep;
			}
			else{
				if (keys[VK_SHIFT])
				{
					//keys[VK_SHIFT] = FALSE;
					view.EyesPos.pos[1] += MoveStep;
				}
				else
					view.EyesPos.pos[1] -= MoveStep;
			}
			return 0;
		}
		if (keys['Z'])
		{
			keys['Z'] = FALSE;
			if (keys['R']){
				//keys['R'] = FALSE;
				if (keys[VK_SHIFT])
				{
					//keys[VK_SHIFT] = FALSE;
					view.RotAngle.pos[2] += AngleStep;
				}
				else
					view.RotAngle.pos[2] -= AngleStep;
				if (view.RotAngle.pos[2] > 180)view.RotAngle.pos[2] -= 360;
				if (view.RotAngle.pos[2] < -180)view.RotAngle.pos[2] += 360;
			}
			else if (keys['T']){
				//keys['T'] = FALSE;
				if (keys[VK_SHIFT])
				{
					//keys[VK_SHIFT] = FALSE;
					view.TransPos.pos[2] += MoveStep;
				}
				else
					view.TransPos.pos[2] -= MoveStep;
			}
			else{
				if (keys[VK_SHIFT])
				{
					//keys[VK_SHIFT] = FALSE;
					view.EyesPos.pos[2] += MoveStep;
				}
				else
					view.EyesPos.pos[2] -= MoveStep;
			}
			return 0;
		}
		if (keys['P'])
		{
			keys['P'] = FALSE;
			if (keys['R']){
				//keys['R'] = FALSE;
				if (keys[VK_SHIFT])
				{
					//keys[VK_SHIFT] = FALSE;
					view.DRot += AngleStep;
				}
				else
					view.DRot -= AngleStep;
			}
			else{
				EM = psp(view.CenterPos, view.EyesPos);
				LEM = pabs(EM);
				mk = 0.5; //log((MoveStep + LEM) / LEM);
				if (keys[VK_SHIFT])
				{
					//keys[VK_SHIFT] = FALSE;
					view.EyesPos = ppp(view.EyesPos, vmp(mk, EM));
				}
				else
					view.EyesPos = psp(view.EyesPos, vmp(mk, EM));
			}
			return 0;
		}
		if (keys['S']){
			keys['S'] = FALSE;
			if (keys['R']){
				//keys['R'] = FALSE;
				if (keys[VK_SHIFT])
				{
					//keys[VK_SHIFT] = FALSE;
					ViewScales *= 0.5;
				}
				else
					ViewScales /= 0.5;
			}
			else{
				if (keys[VK_SHIFT])
				{
					//keys[VK_SHIFT] = FALSE;
					ViewScales += 0.1*AU;
				}
				else
					ViewScales -= 0.1*AU;
			}
			return 0;
		}
		if (keys['T']){
			keys['T'] = FALSE;
			if (keys['R']){
				//keys['R'] = FALSE;
				if (keys[VK_SHIFT])
				{
					//keys[VK_SHIFT] = FALSE;
					rkstep *= 0.5;
				}
				else
					rkstep /= 0.5;
			}
			else{
				if (keys[VK_SHIFT])
				{
					//keys[VK_SHIFT] = FALSE;
					rkstep += TimeScales;
				}
				else
					rkstep -= TimeScales;
			}
			return 0;
		}
		if (keys['M']){
			keys['M'] = FALSE;
			if (keys['R']){
				//keys['R'] = FALSE;
				if (keys[VK_SHIFT])
				{
					//keys[VK_SHIFT] = FALSE;
					MoveStep *= 0.5;
				}
				else
					MoveStep /= 0.5;
			}
			else{
				if (keys[VK_SHIFT])
				{
					//keys[VK_SHIFT] = FALSE;
					MoveStep += 0.1*AU;
				}
				else
					MoveStep -= 0.1*AU;
			}
			return 0;
		}
		if (keys['A']){
			keys['A'] = FALSE;
			if (keys['R']){
				//keys['R'] = FALSE;
				if (keys[VK_SHIFT])
				{
					//keys[VK_SHIFT] = FALSE;
					AngleStep *= 0.5;
				}
				else
					AngleStep /= 0.5;
			}
			else{
				if (keys[VK_SHIFT])
				{
					//keys[VK_SHIFT] = FALSE;
					AngleStep += 0.1;
				}
				else
					AngleStep -= 0.1;
			}
			return 0;
		}
		if (keys['V']){
			keys['V'] = FALSE;
			if (keys['R']){
				//keys['R'] = FALSE;
				if (keys[VK_SHIFT])
				{
					//keys[VK_SHIFT] = FALSE;
					view.ViewAngle *= 0.5;
				}
				else
					view.ViewAngle /= 0.5;
			}
			else{
				if (keys[VK_SHIFT])
				{
					//keys[VK_SHIFT] = FALSE;
					view.ViewAngle += 0.1;
				}
				else
					view.ViewAngle -= 0.1;
			}
			if (view.ViewAngle > 180)view.ViewAngle = 180;
			if (view.ViewAngle < 10)view.ViewAngle = 10;
			return 0;
		}
		if (keys['W']){
			keys['W'] = FALSE;
			withline = !withline;
			return 0;
		}
		if (keys['C']){
			keys['C'] = FALSE;
			for (uint i = 0; i < nbody; i++)
			if (keys['0' + i]){
				//keys['0' + i] = FALSE;
				view.idnum = 1;
				view.ids[0] = i;
				return 0;
			}
			view.idnum = -1;
		}
		if (keys['I']){
			keys['I'] = FALSE;
			isPause = !isPause;
			return 0;
		}
		if (keys['B']){
			keys['B'] = FALSE;
			if (keys[VK_SHIFT])
			{
				if (pointsize < rangesize[1])pointsize += basesize;
			}
			else
			{
				if (pointsize > rangesize[0])pointsize -= basesize;
			}
		}
	}
	//mouse
	{
		//if (mouse.keyp[0]=TRUE&& mouse.key[0] == FALSE)
		//{
		//	real mk = sqrt((mouse.x - mouse.xp)*(mouse.x - mouse.xp) + (mouse.y - mouse.yp)*(mouse.y - mouse.yp));
		//	real sk = sqrt(W*W + H*H);
		//	EM = psp(view.CenterPos, view.EyesPos);
		//	if (mouse.x > mouse.xp)view.EyesPos = psp(view.CenterPos, vmp(mk / sk, EM));
		//	else view.EyesPos = psp(view.CenterPos, vmp(sk / mk, EM));
		//}


	}
	return 0;
}
// ASCII字符总共只有0到127，一共128种字符
void drawString(const char* str) {
	static int isFirstCall = 1;
	static GLuint lists;

	if (isFirstCall) { // 如果是第一次调用，执行初始化
		// 为每一个ASCII字符产生一个显示列表
		isFirstCall = 0;

		// 申请MAX_CHAR个连续的显示列表编号
		lists = glGenLists(MAX_CHAR);

		// 把每个字符的绘制命令都装到对应的显示列表中
		wglUseFontBitmaps(wglGetCurrentDC(), 0, MAX_CHAR, lists);
	}
	// 调用每个字符对应的显示列表，绘制每个字符
	for (; *str != '\0'; ++str)
		glCallList(lists + *str);
}
GLvoid OutPutU(GLvoid){
	FILE* fn=fopen(files[3], "w");
	for (uint i = 0; i < nbody; i++)
		for (uint j = 0; j < nbody; j++)
		{
			if (i != j){
				fprintf(fn, "(%2.2d,%2.2d)->U:%40.15lf,UV:%40.15lf,V:%40.15lf\n",
					i + 1, j + 1,
					-G*M[i].M / pabs(psp(XS[j], XS[i])),
					sqrt(2 * G*M[i].M / pabs(psp(XS[j], XS[i]))),
					pabs(psp(VS[j], VS[i]))
					);
			}
		}
		fclose(fn);
}
BOOL LoadT8(char* filename, GLuint*texture)//调8位贴图
{
	AUX_RGBImageRec *pImage = NULL;
	pImage = auxDIBImageLoad(filename);			// 装入位图		
	if (pImage == NULL)		return FALSE;		// 确保位图数据已经装入
	glGenTextures(1, texture);					// 生成纹理
	glBindTexture(GL_TEXTURE_2D, *texture);	// 捆绑纹理
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, pImage->sizeX, pImage->sizeY, GL_RGB, GL_UNSIGNED_BYTE, pImage->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	free(pImage->data);	//释放位图占据的内存资源
	free(pImage);
	return TRUE;// 返回true
}
BOOL readInitData()
{
	char strline[300], label[100], value[200], values[NPLANET+2][200], *temp;
	FILE*fifid = fopen("parament.txt", "r");
	uint ipos=0, inum=0;
	double lengthunit = 1.,speedunit=1.;
	if (fscanf(fifid, "Files  List\n") != 0)return FALSE;
	if(fscanf(fifid, "Control file:%s\n", strline)!=1)return FALSE;
	struct _iobuf* fcfid = fopen(strline, "r");
	while (fscanf(fcfid, "%[^:]:%s\n", label, value) == 2){
		if (strcmp(label, "GRAVITYTYPE") == 0){
			if (strcmp(value, "Gravity_Newtun")==0){
				GType = Gravity_Newtun;
				continue;
			}
			if (strcmp(value, "Gravity_PN")==0){
				GType = Gravity_PN;
				continue;
			}
			if (strcmp(value, "Gravity_EIH")==0){
				GType = Gravity_EIH;
				continue;
			}
			return FALSE;
		}
		if (strcmp(label,"ANGLESTEP") == 0){
			AngleStep=strtold(value, &temp);
		}
		if (strcmp(label, "NDEM") == 0){
			ndem = strtold(value, &temp);
			if (ndem > NDEM)ndem = NDEM;
		}
		if (strcmp(label, "VIEWANGLE") == 0){
			view.ViewAngle = strtold(value, &temp);
		}
		if (strcmp(label, "TIMESCALES") == 0){
			if (strcmp(value, "DS") == 0)
				TimeScales = DS;
			else
			{
				TimeScales = strtold(value, &temp);
				if (strcmp(temp, "DAY") == 0 || strcmp(temp, "DS") == 0)TimeScales *= DS;
				if (strcmp(temp, "HOUR") == 0)TimeScales *= 3600;
				if (strcmp(temp, "MIN") == 0)TimeScales *= 60;
				if (strcmp(temp, "SEC") == 0)TimeScales *= 1;
				if (strcmp(temp, "") == 0)TimeScales *= 1;
			}
			continue;
		}
		if (strcmp(label, "LENGTHSCALES") == 0){
			if (strcmp(value, "AU") == 0)
				LengthScales = AU;
			else
			{
				LengthScales = strtold(value, &temp);
				if (strcmp(temp, "AU") == 0)LengthScales *= AU;
				if (strcmp(temp, "KM") == 0)LengthScales *= 1000;
				if (strcmp(temp, "M") == 0)LengthScales *= 1;
				if (strcmp(temp, "") == 0)LengthScales *= 1;
			}
			continue;
		}
		if (strcmp(label, "RKSTEP") == 0){
			if (strcmp(value, "TS") == 0)
				rkstep = TimeScales;
			else
			{
				rkstep = strtold(value, &temp);
				if (strcmp(temp, "TS") == 0)rkstep *= TimeScales;
				if (strcmp(temp, "DAY") == 0 || strcmp(temp, "DS") == 0)rkstep *= DS;
				if (strcmp(temp, "HOUR") == 0)rkstep *= 3600;
				if (strcmp(temp, "MIN") == 0)rkstep *= 60;
				if (strcmp(temp, "SEC") == 0)rkstep *= 1;
				if (strcmp(temp, "") == 0)rkstep *= 1;
			}
			continue;
		}
		if (strcmp(label, "MOVESTEP") == 0){
			if (strcmp(value, "AU") == 0)
				MoveStep = AU;
			else if (strcmp(value, "LENGTHSCALES") == 0)
				MoveStep = LengthScales;
			else
			{
				MoveStep = strtold(value, &temp);
				if (strcmp(temp, "AU") == 0)MoveStep *= AU;
				if (strcmp(temp, "LS") == 0)MoveStep *= LengthScales;
				if (strcmp(temp, "LightSpeed") == 0)MoveStep *= c;
				if (strcmp(temp, "KM") == 0)MoveStep *= 1000;
				if (strcmp(temp, "M") == 0)MoveStep *= 1;
				if (strcmp(temp, "") == 0)MoveStep *= 1;
			}
			continue;
		}
		if (strcmp(label, "VIEWSCALES") == 0){
			if (strcmp(value, "AU") == 0)
				ViewScales = AU;
			else if (strcmp(value, "LENGTHSCALES") == 0)
				ViewScales = LengthScales;
			else
			{
				ViewScales = strtold(value, &temp);
				if (strcmp(temp, "AU") == 0)ViewScales *= AU;
				if (strcmp(temp, "LS") == 0)ViewScales *= LengthScales;
				if (strcmp(temp, "LightSpeed") == 0)ViewScales *= c;
				if (strcmp(temp, "KM") == 0)ViewScales *= 1000;
				if (strcmp(temp, "M") == 0)ViewScales *= 1;
				if (strcmp(temp, "") == 0)ViewScales *= 1;
			}
			continue;
		}
		if (sscanf(label, "E%d",&ipos) == 1){
			if (strcmp(value, "AU") == 0)
				view.EyesPos.pos[ipos] = AU;
			else if (strcmp(value, "LENGTHSCALES") == 0)
				view.EyesPos.pos[ipos] = LengthScales;
			else
			{
				view.EyesPos.pos[ipos] = strtold(value, &temp);
				if (strcmp(temp, "AU") == 0)view.EyesPos.pos[ipos] *= AU;
				if (strcmp(temp, "LS") == 0)view.EyesPos.pos[ipos] *= LengthScales;
				if (strcmp(temp, "LightSpeed") == 0)view.EyesPos.pos[ipos] *= c;
				if (strcmp(temp, "KM") == 0)view.EyesPos.pos[ipos] *= 1000;
				if (strcmp(temp, "M") == 0)view.EyesPos.pos[ipos] *= 1;
				if (strcmp(temp, "") == 0)view.EyesPos.pos[ipos] *= 1;
			}
			continue;
		}
		if (sscanf(label, "C%d", &ipos) == 1){
			if (strcmp(value, "AU") == 0)
				view.CenterPos.pos[ipos] = AU;
			else if (strcmp(value, "LENGTHSCALES") == 0)
				view.CenterPos.pos[ipos] = LengthScales;
			else
			{
				view.CenterPos.pos[ipos] = strtold(value, &temp);
				if (strcmp(temp, "AU") == 0)view.CenterPos.pos[ipos] *= AU;
				if (strcmp(temp, "LS") == 0)view.CenterPos.pos[ipos] *= LengthScales;
				if (strcmp(temp, "LightSpeed") == 0)view.CenterPos.pos[ipos] *= c;
				if (strcmp(temp, "KM") == 0)view.CenterPos.pos[ipos] *= 1000;
				if (strcmp(temp, "M") == 0)view.CenterPos.pos[ipos] *= 1;
				if (strcmp(temp, "") == 0)view.CenterPos.pos[ipos] *= 1;
			}
			continue;
		}
		if (sscanf(label, "T%d", &ipos) == 1){
			if (strcmp(value, "AU") == 0)
				view.TransPos.pos[ipos] = AU;
			else if (strcmp(value, "LENGTHSCALES") == 0)
				view.TransPos.pos[ipos] = LengthScales;
			else
			{
				view.TransPos.pos[ipos] = strtold(value, &temp);
				if (strcmp(temp, "AU") == 0)view.TransPos.pos[ipos] *= AU;
				if (strcmp(temp, "LS") == 0)view.TransPos.pos[ipos] *= LengthScales;
				if (strcmp(temp, "LightSpeed") == 0)view.TransPos.pos[ipos] *= c;
				if (strcmp(temp, "KM") == 0)view.TransPos.pos[ipos] *= 1000;
				if (strcmp(temp, "M") == 0)view.TransPos.pos[ipos] *= 1;
				if (strcmp(temp, "") == 0)view.TransPos.pos[ipos] *= 1;
			}
			continue;
		}
		if (sscanf(label, "U%d", &ipos) == 1){
			if (strcmp(value, "AU") == 0)
				view.UpPos.pos[ipos] = AU;
			else if (strcmp(value, "LENGTHSCALES") == 0)
				view.UpPos.pos[ipos] = LengthScales;
			else
			{
				view.UpPos.pos[ipos] = strtold(value, &temp);
				if (strcmp(temp, "AU") == 0)view.UpPos.pos[ipos] *= AU;
				if (strcmp(temp, "LS") == 0)view.UpPos.pos[ipos] *= LengthScales;
				if (strcmp(temp, "LightSpeed") == 0)view.UpPos.pos[ipos] *= c;
				if (strcmp(temp, "KM") == 0)view.UpPos.pos[ipos] *= 1000;
				if (strcmp(temp, "M") == 0)view.UpPos.pos[ipos] *= 1;
				if (strcmp(temp, "") == 0)view.UpPos.pos[ipos] *= 1;
			}
			continue;
		}
	}
	fclose(fcfid);
	if (fscanf(fifid, "Data    file:%s\n", strline) != 1)return FALSE;
	struct _iobuf* fdfid = fopen(strline, "r");
	fscanf(fdfid, "NBODY:%d\n", &nbody);
	if (fscanf(fdfid, "M,R,T,NAME,IMG,A,B,C\n") != 0)return FALSE;
	for (uint i = 0; i < nbody; i++){
		if(fscanf(fdfid, "%[^,],%[^,],%[^,],%[^,],%[^,],%lf,%lf,%lf\n", 
			values[0], values[1], values[2], M[i].name, M[i].img, &M[i].A, &M[i].B, &M[i].C)!=8)return FALSE;
		if (strcmp(values[0], "SM") == 0)
			M[i].M = SM;
		else
		{
			M[i].M = strtold(values[0], &temp);
			if (strcmp(temp, "SM") == 0)M[i].M *= SM;
			if (strcmp(temp, "T") == 0)M[i].M *= 1000;
			if (strcmp(temp, "KG") == 0)M[i].M *= 1;
			if (strcmp(temp, "") == 0)M[i].M *= 1;
		}
		if (strcmp(values[1], "SR") == 0)
			M[i].R = SR;
		else
		{
			M[i].R = strtold(values[1], &temp);
			if (strcmp(temp, "SR") == 0)M[i].R *= SR;
			if (strcmp(temp, "AU") == 0)M[i].R *= AU;
			if (strcmp(temp, "LS") == 0)M[i].R *= LengthScales;
			if (strcmp(temp, "LightSpeed") == 0)M[i].R *= c;
			if (strcmp(temp, "KM") == 0)M[i].R *= 1000;
			if (strcmp(temp, "M") == 0)M[i].R *= 1;
			if (strcmp(temp, "") == 0)M[i].R *= 1;
		}
		if (strcmp(values[2], "DS") == 0 || strcmp(values[2], "DAY") == 0)
			M[i].T = DS;
		else
		{
			M[i].T = strtold(values[2], &temp);
			if (strcmp(temp, "TS") == 0)M[i].T *= TimeScales;
			if (strcmp(temp, "DAY") == 0 || strcmp(temp, "DS") == 0)M[i].T *= DS;
			if (strcmp(temp, "HOUR") == 0)M[i].T *= 3600;
			if (strcmp(temp, "MIN") == 0)M[i].T *= 60;
			if (strcmp(temp, "SEC") == 0)M[i].T *= 1;
			if (strcmp(temp, "") == 0)M[i].T *= 1;
		}
	}
	if (fscanf(fdfid, "TIME:%s\n", value) != 1)return FALSE;
	if (strcmp(value, "DS") == 0 || strcmp(value, "DAY") == 0)
		time = DS;
	else
	{
		time = strtold(value, &temp);
		if (strcmp(temp, "TS") == 0)time *= TimeScales;
		if (strcmp(temp, "DAY") == 0 || strcmp(temp, "DS") == 0)time *= DS;
		if (strcmp(temp, "HOUR") == 0)time *= 3600;
		if (strcmp(temp, "MIN") == 0)time *= 60;
		if (strcmp(temp, "SEC") == 0)time *= 1;
		if (strcmp(temp, "") == 0)time *= 1;
	}
	if (fscanf(fdfid, "LENGTHUINT:%s\n", value) != 1)return FALSE;
	if (strcmp(value, "SR") == 0)
		lengthunit = SR;
	else if (strcmp(value, "AU") == 0)
		lengthunit = AU;
	else if (strcmp(value, "LS") == 0)
		lengthunit = LengthScales;
	else if (strcmp(value, "LightSpeed") == 0)
		lengthunit = c;
	else if (strcmp(value, "KM") == 0)
		lengthunit = 1000;
	else if (strcmp(value, "M") == 0)
		lengthunit = 1;
	else
	{
		lengthunit = strtold(value, &temp);
		if (strcmp(temp, "SR") == 0)lengthunit *= SR;
		if (strcmp(temp, "AU") == 0)lengthunit *= AU;
		if (strcmp(temp, "LS") == 0)lengthunit *= LengthScales;
		if (strcmp(temp, "LightSpeed") == 0)lengthunit *= c;
		if (strcmp(temp, "KM") == 0)lengthunit *= 1000;
		if (strcmp(temp, "M") == 0)lengthunit *= 1;
		if (strcmp(temp, "") == 0)lengthunit *= 1;
	}
	if(fscanf(fdfid, "%s\n", strline)!=1)return FALSE;
	if (sscanf(strline, "SPEEDUINT:%lf%[^/]/%s", &speedunit, values[0], values[1]) != 3);
	if (sscanf(strline, "SPEEDUINT:%[^/]/%s", values[0], values[1]) != 2)return FALSE;
	if (fabs(speedunit) < 1e-10)speedunit = 1;
	if (strcmp(values[0], "SR") == 0)speedunit *= SR;
	if (strcmp(values[0], "AU") == 0)speedunit *= AU;
	if (strcmp(values[0], "LS") == 0)speedunit *= LengthScales;
	if (strcmp(values[0], "LightSpeed") == 0)speedunit *= c;
	if (strcmp(values[0], "KM") == 0)speedunit *= 1000;
	if (strcmp(values[0], "M") == 0)speedunit *= 1;
	if (strcmp(values[0], "") == 0)speedunit *= 1;
	if (strcmp(values[1], "TS") == 0)speedunit /= TimeScales;
	if (strcmp(values[1], "DAY") == 0 || strcmp(values[1], "DS") == 0)speedunit /= DS;
	if (strcmp(values[1], "HOUR") == 0)speedunit /= 3600;
	if (strcmp(values[1], "MIN") == 0)speedunit /= 60;
	if (strcmp(values[1], "SEC") == 0)speedunit /= 1;
	if (strcmp(values[1], "") == 0)speedunit /= 1;
	for (uint i = 0; i < nbody; i++){
		if (fscanf(fdfid, "BODY%d:POS(%[^)]),VEL(%[^)])\n",&inum,values[0],values[1])!=3)return FALSE;
		XS[inum-1].pos[0] = strtold(values[0], &temp)*lengthunit;
		for (uint i = 1; i < ndem; i++)XS[inum-1].pos[i] = strtold(++temp, &temp)*lengthunit;
		VS[inum-1].pos[0] = strtold(values[1], &temp)*speedunit;
		for (uint i = 1; i < ndem; i++)VS[inum-1].pos[i] = strtold(++temp, &temp)*speedunit;
	}
	if (fscanf(fdfid, "%s\n", strline) != 1)return FALSE;
	if (strcmp(strline, "CENTER") == 0){
		if (fscanf(fdfid, "%s\n", strline) != 1)return FALSE;
		if (sscanf(strline, "%[^:]:%s", values[0], values[1]) != 2)return FALSE;
		view.idnum = 0;
		temp = values[1];
		while (temp != "")if (sscanf(temp, "%[^,],%s", values[2 + view.idnum++], temp) != 2)break;
		//if (temp != "")strcpy(values[2 + sysnums++], temp);
		if (strcmp(values[0], "NAMES") == 0){
			for (int i = 0; i < view.idnum; i++)
			for (uint j = 0; j < nbody;j++)
			if (strcmp(values[2+i], M[j].name) == 0)
				view.ids[i] = j;
		}
		else
		{
			for (int i = 0; i < view.idnum; i++)
				view.ids[i] = atoi(values[2 + i]);
		}
	}
	fclose(fdfid);
	if (fscanf(fifid, "Commond file:%s\n", files[0]) != 1)return FALSE;
	if (fscanf(fifid, "Input   file:%s\n", files[1]) != 1)return FALSE;
	if (fscanf(fifid, "Output  file:%s\n", files[2]) != 1)return FALSE;
	if (fscanf(fifid, "U       file:%s\n", files[3]) != 1)return FALSE;
	if (fscanf(fifid, "Path    file:%s\n", files[4]) != 1)return FALSE;

	fclose(fifid);
	return TRUE;
}