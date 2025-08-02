#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <vector>
#include <string>
#include "RequestHandler.hpp"

#include <opencv2/opencv.hpp>

#include "VideoRecording.hpp"

using namespace cv;


void FormBayer(BYTE* dest, const BYTE* rawData, int width, int height, int BytesPerPixel){
	for(int i =0; i< (width*height*BytesPerPixel); i+=2){
		uint16_t l = rawData[i];
		uint16_t h = rawData[i+1];
		uint16_t data = h*256 + l;
		dest[i/2] = BYTE((data & 0x3FF)>>2);
	}
}

struct BGRAPixel
{
	BYTE b;
	BYTE g;
	BYTE r;
	BYTE a;
};
void RGB24_to_BGRA32(BGRAPixel* destinationBuffer, const BYTE* sourceBuffer, DWORD  _width, DWORD  _height)
{
	if (destinationBuffer == nullptr || sourceBuffer == nullptr)
		return;
	UINT totalPixels = _width*_height;
	auto *sourceRGB24 = (RGBTRIPLE*)sourceBuffer;
	DWORD c2 = totalPixels - 1;
	for (DWORD c = 0; c <totalPixels; c++)
	{
		destinationBuffer[c].b = sourceRGB24[c2].rgbtBlue;
		destinationBuffer[c].g = sourceRGB24[c2].rgbtGreen;
		destinationBuffer[c].r = sourceRGB24[c2].rgbtRed;
		destinationBuffer[c].a = 255;
		c2--;
	}

}

__forceinline BYTE Clip(int clr)
{
	return (BYTE)(clr < 0 ? 0 : (clr > 255 ? 255 : clr));
}

BGRAPixel ConvertYCrCbToRGB(int  	y,
	int  	cr,
	int  	cb
)
{
	BGRAPixel rgbq{};

	int c = y - 16;
	int d = cb - 128;
	int e = cr - 128;

	rgbq.r = Clip((298 * c + 409 * e + 128) >> 8);
	rgbq.g = Clip((298 * c - 100 * d - 208 * e + 128) >> 8);
	rgbq.b = Clip((298 * c + 516 * d + 128) >> 8);

	return rgbq;
}

/// Note: this is taken from https://software.intel.com/sites/landingpage/mmsf/documentation/preview_8cpp.html#af3d9de67be8e955b824d4b497bba4c96
/// In the online version (from Intel) the comment below on Byte order was incorrect - although the code was fine (!). Never trust documentation..(?)
void TransformImage_YUY2(BYTE* pDest,
	LONG  	lDestStride,
	const BYTE* pSrc,
	LONG  	lSrcStride,
	DWORD  	dwWidthInPixels,
	DWORD  	dwHeightInPixels
)
{
	if (pDest == nullptr || pSrc == nullptr)
		return;

	for (DWORD y = 0; y < dwHeightInPixels; y++)
	{
		auto* pDestPel = (BGRAPixel*)pDest;
		WORD * pSrcPel = (WORD*)pSrc;

		for (DWORD x = 0; x < dwWidthInPixels; x += 2)
		{
			// Byte order is Y0 U0 Y1 V0  /// NOTE: On the Intel site this comment was wrong.

			int y0 = (int)LOBYTE(pSrcPel[x]);
			int u0 = (int)HIBYTE(pSrcPel[x]);
			int y1 = (int)LOBYTE(pSrcPel[x + 1]);
			int v0 = (int)HIBYTE(pSrcPel[x + 1]);

			pDestPel[x] = ConvertYCrCbToRGB(y0, v0, u0);
			pDestPel[x + 1] = ConvertYCrCbToRGB(y1, v0, u0);
		}

		pSrc +=  lSrcStride;
		pDest += lDestStride;
	}

}

void WindowMSGLoop() {
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_QUIT) {
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

int main(int argc, char *argv[])
{
    // WORD wVersionRequested = MAKEWORD(2, 2);
    // WSADATA wsaData;
    //
    // int wsaerr = WSAStartup(wVersionRequested, &wsaData);
    // if (wsaerr != 0)
    // {
    //
    //     std::cerr << "Failed to initialize Winsock: " << wsaerr << '\n';
    //     return 1;
    // }
    //
    // std::cout << "Winsock initialized successfully" << '\n';
    //
    //
    // const SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    // if (serverSocket == INVALID_SOCKET)
    // {
    //     std::cerr << "Cannot create socket" << '\n';
    //     WSACleanup();
    //     return 1;
    // }
    //
    // sockaddr_in serverAddr{};
    // serverAddr.sin_family = AF_INET;
    // serverAddr.sin_addr.s_addr = INADDR_ANY;
    // serverAddr.sin_port = htons(8080);
    //
    // if (bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
    // {
    //     std::cerr << "Bind failed!" << std::endl;
    //     closesocket(serverSocket);
    //     WSACleanup();
    //     return 1;
    // }
    //
    //
    // if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
    //     std::cerr << "Listen failed!" << std::endl;
    //     closesocket(serverSocket);
    //     WSACleanup();
    //     return 1;
    // }
    // std::cout << "Server is listening on port 8080..." << std::endl;
    //
    // std::vector<HANDLE> clients;
    // int idx = 0;
    // CreateThread(
    //     nullptr,
    //     0,
    //     LPTHREAD_START_ROUTINE(WindowMSGLoop),
    //     nullptr,
    //     0,
    //     nullptr
    // );
    //
    // while (true)
    // {
	   //  RequestHandler requestHandler;
	   //  sockaddr_in clientAddr{};
    //     int clientSize = sizeof(clientAddr);
    //     const SOCKET clientSocket = accept(serverSocket, reinterpret_cast<sockaddr *>(&clientAddr), &clientSize);
    //     if (clientSocket == INVALID_SOCKET)
    //     {
    //         continue;
    //     }
    //     HANDLE client_thread = CreateThread(
    //         nullptr,
    //         0,
    //         LPTHREAD_START_ROUTINE(requestHandler.ProcessClient),
    //         reinterpret_cast<LPVOID>(clientSocket),
    //         0,
    //         nullptr
    //     );
    //
    //     if (client_thread == nullptr)
    //     {
    //         closesocket(clientSocket);
    //         break;
    //     }
    //
    //     clients.push_back(client_thread);
    //     idx++;
    // }
    //
    //
    // for (HANDLE h : clients)
    // {
    //     CloseHandle(h);
    // }
    // closesocket(serverSocket);
    // WSACleanup();
	auto* m = new Media(1);
	m->CreateCaptureDevice();

	std::cout<<"Device Count = "<<m->deviceCount<<std::endl;
	// std::cout<<"Device name = "<<m->deviceName<<std::endl;
	std::cout<<"w = "<<m->width<<" h = "<<m->height<<" bpp = "<<m->bytePerPixel<<std::endl;

	BYTE* buffer = new BYTE[m->width*m->height*sizeof(BYTE)];
	auto *bgraBuffer = new BGRAPixel[m->width* m->height];
	MSG msg{ nullptr };
	while (msg.message != WM_QUIT)
	{
		if(m->videoFormat == MFVideoFormat_RGB24)
		{
			RGB24_to_BGRA32(bgraBuffer, m->raw, m->width, m->height);
		}
		else if (m->videoFormat == MFVideoFormat_YUY2)
		{
			// Dest stride in the following call is hardwired to 4 * width because it is RGBA (i.e. 4 bytes per pixel)
			//TransformImage_YUY2((BYTE*)(bgraBuffer),4* m->width, m->rawData, m->stride, m->width, m->height);

			// NOTE from Lafith Mattara: FormBayer is written for my own usecase of 10 bit camera input
			// Uncomment HowardsPlayPen's TransformImage_YUV2 for standard conversion.
			FormBayer(buffer, m->raw, m->width, m->height, m->bytePerPixel);
		}
		else if (m->videoFormat == MFVideoFormat_RGB32)
		{
			std::cerr << "Unsupported type MFVideoFormat_RGB32" << std::endl;
		}
		else if (m->videoFormat == MFVideoFormat_NV12)
		{
			std::cerr << "Unsupported type MFVideoFormat_NV12" << std::endl;
		}
		else
		{
			// Houston we have a problem...
			/// Note: people are welcome to paste the relevant function here to test
			std::cerr << "Unsupported video type" << std::endl;
		}

		Mat newImg = Mat(m->height, m->width, CV_8UC1, buffer);
		cvtColor(newImg, newImg, COLOR_BayerBG2RGB);
		imshow("Display", newImg);
		if(waitKey(5) == 'q')
		{
			destroyAllWindows();
			break;
		}
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			continue;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	m->Release();
	delete buffer;
    return 0;
}