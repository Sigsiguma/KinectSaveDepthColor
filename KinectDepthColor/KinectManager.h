#pragma once
#include <Windows.h>
#include <Kinect.h>
#include <vector>
#include <wrl/client.h>

#include "include.hpp"

class Kinect
{
public:
	cv::Mat colorMat;
	cv::Mat depthMat;
private:
	// Sensor
	Microsoft::WRL::ComPtr<IKinectSensor> kinect;

	// Coordinate Mapper
	Microsoft::WRL::ComPtr<ICoordinateMapper> coordinateMapper;

	// Reader
	Microsoft::WRL::ComPtr<IColorFrameReader> colorFrameReader;
	Microsoft::WRL::ComPtr<IDepthFrameReader> depthFrameReader;

	// Color Buffer
	std::vector<BYTE> colorBuffer;
	int colorWidth;
	int colorHeight;
	unsigned int colorBytesPerPixel;

	// Depth Buffer
	std::vector<UINT16> depthBuffer;
	int depthWidth;
	int depthHeight;
	unsigned int depthBytesPerPixel;

public:
	// Constructor
	Kinect();

	// Destructor
	~Kinect();

	// Processing
	void run();

private:
	// Initialize
	void initialize();

	// Initialize Sensor
	inline void initializeSensor();

	// Initialize Color
	inline void initializeColor();

	// Initialize Depth
	inline void initializeDepth();

	// Finalize
	void finalize();

	// Update Data
	void update();

	// Update Color
	inline void updateColor();

	// Update Depth
	inline void updateDepth();

	// Draw Data
	void draw();

	// Draw Color
	inline void drawColor();

	// Draw Depth
	inline void drawDepth();

	// Save Data
	void save();

	// Save Color
	inline void saveColor();

	// Save Depth
	inline void saveDepth();

	// Show Data
	void show();

	// Show Color
	inline void showColor();

	// Show Depth
	inline void showDepth();
};
