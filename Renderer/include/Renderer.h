#pragma once
#include "Camera.h"
#include "SampleManager.h"

#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl2.h"

class Renderer
{
public:
	Camera camera;
	SampleManager _sampleManager;

	XMVECTOR MousePos;

	void Resize(int w, int h);

	void SetUp();

	void TearDown() const noexcept;

	void Run() noexcept;

	void drawSnowMan();

	void renderScene(void);

private:
	GLuint sphereDisplayList = 0;

	void DrawSphere(XMVECTOR center, float radius, int segments, const Color& col) noexcept;

	void Drawcuboid(XMVECTOR minBound, XMVECTOR maxBound, const Color& col) noexcept;

	void DrawcuboidBorder(XMVECTOR minBound, XMVECTOR maxBound, const Color& col) noexcept;

	void DrawAllGraphicsData() noexcept;

	//Time
	int prevTime, currentTime;
	float deltaTime;

	void InitializeDisplayList();

};
