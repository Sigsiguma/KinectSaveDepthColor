#include "KinectManager.h"
#include "Utils.h"

#include <thread>
#include <chrono>

#include <ppl.h>

#include <yaml-cpp/yaml.h>

// Choose Resolution
#define COLOR
//#define DEPTH

// Constructor
Kinect::Kinect()
{
	// Initialize
	initialize();
}

// Destructor
Kinect::~Kinect()
{
	// Finalize
	finalize();
}

// Processing
void Kinect::run()
{
	// Update Data
	update();

	// Draw Data
	draw();

	// Save Data
	save();
}

// Initialize
void Kinect::initialize()
{
	cv::setUseOptimized(true);

	// Initialize Sensor
	initializeSensor();

	// Initialize Color
	initializeColor();

	// Initialize Depth
	initializeDepth();


	// Wait a Few Seconds until begins to Retrieve Data from Sensor ( about 2000-[ms] )
	std::this_thread::sleep_for(std::chrono::seconds(2));
}

// Initialize Sensor
inline void Kinect::initializeSensor()
{
	// Open Sensor
	ERROR_CHECK(GetDefaultKinectSensor(&kinect));

	ERROR_CHECK(kinect->Open());

	// Check Open
	BOOLEAN isOpen = FALSE;
	ERROR_CHECK(kinect->get_IsOpen(&isOpen));
	if (!isOpen) {
		throw std::runtime_error("failed IKinectSensor::get_IsOpen( &isOpen )");
	}

	// Retrieve Coordinate Mapper
	ERROR_CHECK(kinect->get_CoordinateMapper(&coordinateMapper));
}

// Initialize Color
inline void Kinect::initializeColor()
{
	// Open Color Reader
	Microsoft::WRL::ComPtr<IColorFrameSource> colorFrameSource;
	ERROR_CHECK(kinect->get_ColorFrameSource(&colorFrameSource));
	ERROR_CHECK(colorFrameSource->OpenReader(&colorFrameReader));

	// Retrieve Color Description
	Microsoft::WRL::ComPtr<IFrameDescription> colorFrameDescription;
	ERROR_CHECK(colorFrameSource->CreateFrameDescription(ColorImageFormat::ColorImageFormat_Bgra, &colorFrameDescription));
	ERROR_CHECK(colorFrameDescription->get_Width(&colorWidth)); // 1920
	ERROR_CHECK(colorFrameDescription->get_Height(&colorHeight)); // 1080
	ERROR_CHECK(colorFrameDescription->get_BytesPerPixel(&colorBytesPerPixel)); // 4

																				// Allocation Color Buffer
	colorBuffer.resize(colorWidth * colorHeight * colorBytesPerPixel);
}

// Initialize Depth
inline void Kinect::initializeDepth()
{
	// Open Depth Reader
	Microsoft::WRL::ComPtr<IDepthFrameSource> depthFrameSource;
	ERROR_CHECK(kinect->get_DepthFrameSource(&depthFrameSource));
	ERROR_CHECK(depthFrameSource->OpenReader(&depthFrameReader));

	// Retrieve Depth Description
	Microsoft::WRL::ComPtr<IFrameDescription> depthFrameDescription;
	ERROR_CHECK(depthFrameSource->get_FrameDescription(&depthFrameDescription));
	ERROR_CHECK(depthFrameDescription->get_Width(&depthWidth)); // 512
	ERROR_CHECK(depthFrameDescription->get_Height(&depthHeight)); // 424
	ERROR_CHECK(depthFrameDescription->get_BytesPerPixel(&depthBytesPerPixel)); // 2

																				// Allocation Depth Buffer
	depthBuffer.resize(depthWidth * depthHeight);
}

// Finalize
void Kinect::finalize()
{
	cv::destroyAllWindows();

	// Close Sensor
	if (kinect != nullptr) {
		kinect->Close();
	}
}

// Update Data
void Kinect::update()
{
	// Update Color
	updateColor();

	// Update Depth
	updateDepth();
}

// Update Color
inline void Kinect::updateColor()
{
	// Retrieve Color Frame
	Microsoft::WRL::ComPtr<IColorFrame> colorFrame;
	const HRESULT ret = colorFrameReader->AcquireLatestFrame(&colorFrame);
	if (FAILED(ret)) {
		return;
	}

	// Convert Format ( YUY2 -> BGRA )
	ERROR_CHECK(colorFrame->CopyConvertedFrameDataToArray(static_cast<UINT>(colorBuffer.size()), &colorBuffer[0], ColorImageFormat::ColorImageFormat_Bgra));
}

// Update Depth
inline void Kinect::updateDepth()
{
	// Retrieve Depth Frame
	Microsoft::WRL::ComPtr<IDepthFrame> depthFrame;
	const HRESULT ret = depthFrameReader->AcquireLatestFrame(&depthFrame);
	if (FAILED(ret)) {
		return;
	}

	// Retrieve Depth Data
	ERROR_CHECK(depthFrame->CopyFrameDataToArray(static_cast<UINT>(depthBuffer.size()), &depthBuffer[0]));
}

// Draw Data
void Kinect::draw()
{
	// Draw Color
	drawColor();

	// Draw Depth
	drawDepth();
}

void calcColorBuffer(const cv::Mat& colorMat, std::vector<BYTE>& colorBuffer) {

	std::vector<BYTE> tmpVec(colorBuffer.size());
	for (int y = 0; y < colorMat.rows; ++y) {
		const cv::Vec3b* tmp = colorMat.ptr<cv::Vec3b>(y);
		for (int x = 0; x < colorMat.cols; ++x) {
			int colorIndex = (y * colorMat.cols + x) * 4;
			tmpVec[colorIndex] = tmp[x][0];
			tmpVec[colorIndex + 1] = tmp[x][1];
			tmpVec[colorIndex + 2] = tmp[x][2];
			tmpVec[colorIndex + 3] = 255;
		}
	}
	colorBuffer = tmpVec;
}

// Draw Color
inline void Kinect::drawColor()
{
#ifdef DEPTH
	// Retrieve Mapped Coordinates
	std::vector<ColorSpacePoint> colorSpacePoints(depthWidth * depthHeight);
	ERROR_CHECK(coordinateMapper->MapDepthFrameToColorSpace(depthBuffer.size(), &depthBuffer[0], colorSpacePoints.size(), &colorSpacePoints[0]));

	// Mapping Color to Depth Resolution
	std::vector<BYTE> buffer(depthWidth * depthHeight * colorBytesPerPixel);

	Concurrency::parallel_for(0, depthHeight, [&](const int depthY) {
		const unsigned int depthOffset = depthY * depthWidth;
		for (int depthX = 0; depthX < depthWidth; depthX++) {
			unsigned int depthIndex = depthOffset + depthX;
			const int colorX = static_cast<int>(colorSpacePoints[depthIndex].X + 0.5f);
			const int colorY = static_cast<int>(colorSpacePoints[depthIndex].Y + 0.5f);
			if ((0 <= colorX) && (colorX < colorWidth) && (0 <= colorY) && (colorY < colorHeight)) {
				const unsigned int colorIndex = (colorY * colorWidth + colorX) * colorBytesPerPixel;
				depthIndex = depthIndex * colorBytesPerPixel;
				buffer[depthIndex + 0] = colorBuffer[colorIndex + 0];
				buffer[depthIndex + 1] = colorBuffer[colorIndex + 1];
				buffer[depthIndex + 2] = colorBuffer[colorIndex + 2];
				buffer[depthIndex + 3] = colorBuffer[colorIndex + 3];
			}
		}
	});

	// Create cv::Mat from Coordinate Buffer
	colorMat = cv::Mat(depthHeight, depthWidth, CV_8UC4, &buffer[0]).clone();
#else
	// Create cv::Mat from Color Buffer
	colorMat = cv::Mat(colorHeight, colorWidth, CV_8UC4, &colorBuffer[0]);
#endif

}

// Draw Depth
inline void Kinect::drawDepth()
{
#ifdef COLOR
	// Retrieve Mapped Coordinates
	std::vector<DepthSpacePoint> depthSpacePoints(colorWidth * colorHeight);
	ERROR_CHECK(coordinateMapper->MapColorFrameToDepthSpace(depthBuffer.size(), &depthBuffer[0], depthSpacePoints.size(), &depthSpacePoints[0]));

	// Mapping Depth to Color Resolution
	std::vector<UINT16> buffer(colorWidth * colorHeight);

	Concurrency::parallel_for(0, colorHeight, [&](const int colorY) {
		const unsigned int colorOffset = colorY * colorWidth;
		for (int colorX = 0; colorX < colorWidth; colorX++) {
			const unsigned int colorIndex = colorOffset + colorX;
			const int depthX = static_cast<int>(depthSpacePoints[colorIndex].X + 0.5f);
			const int depthY = static_cast<int>(depthSpacePoints[colorIndex].Y + 0.5f);
			if ((0 <= depthX) && (depthX < depthWidth) && (0 <= depthY) && (depthY < depthHeight)) {
				const unsigned int depthIndex = depthY * depthWidth + depthX;
				buffer[colorIndex] = depthBuffer[depthIndex];
			}
		}
	});

	// Create cv::Mat from Coordinate Buffer
	depthMat = cv::Mat(colorHeight, colorWidth, CV_16UC1, &buffer[0]).clone();
#else
	// Create cv::Mat from Depth Buffer
	depthMat = cv::Mat(depthHeight, depthWidth, CV_16UC1, &depthBuffer[0]);
#endif
}

// Save Data
void Kinect::save() {
	saveColor();
	saveDepth();
}

// Save Color
inline void Kinect::saveColor() {
	static int count = 0;
	std::string fileName = "color" + std::to_string(count) + ".png";
	cv::imwrite(fileName, colorMat);
	++count;
}

// Save Depth
inline void Kinect::saveDepth() {
	static int count = 0;
	std::string fileName = "depth" + std::to_string(count) + ".png";
	cv::imwrite(fileName, depthMat);
	++count;
}