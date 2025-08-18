# Computer Network Project

A comprehensive client-server remote administration tool built with C++ that provides secure remote access and control capabilities over a network connection.

## 🚀 Features

### Remote System Management
- **Process Management**: View running processes with PID, name, memory usage, and CPU information
- **Process Control**: Terminate processes remotely by PID
- **Application Launcher**: Launch applications on the remote system

### File System Operations
- **File Explorer**: Browse remote file system directories
- **File Transfer**: Download files from the remote system
- **Directory Listing**: View contents of remote directories

### Monitoring & Security
- **Keylogger**: Capture and log keystrokes on the remote system
- **Screen Recording**: Record video of the remote desktop
- **Screenshot Capture**: Take screenshots of the remote desktop
- **Control Volumn**: Adjust system volume levels remotely

### Communication Features
- **Email Integration**: Send notifications and reports via email

## 🏗️ Architecture

The project follows a client-server architecture:

- **Server**: Runs on the target machine, handles incoming requests and executes system operations
- **Client**: Provides a GUI interface for connecting to and controlling remote servers

### Technology Stack
- **Language**: C++23
- **GUI Framework**: Dear ImGui
- **Networking**: Windows Sockets API (Winsock2)
- **Media**: Windows Media Foundation
- **Email**: libcurl for SMTP
- **Build System**: CMake

## 📋 Prerequisites

### Windows Requirements
- Windows 10 or later
- Visual Studio Build Tools or Visual Studio 2019+
- CMake 3.31 or later

### Dependencies
- **ImGui**: GUI framework (automatically downloaded)
- **GLFW**: Window management (automatically downloaded)
- **libcurl**: HTTP/Email functionality (automatically downloaded)
- **Windows SDK**: For system APIs
- **stb**: Image loading (automatically downloaded)

## 🔧 Installation & Setup

### 1. Clone the Repository
```bash
git clone https://github.com/yourusername/Computer-Network-Project.git
cd Computer-Network-Project
```

### 2. Build the Project
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### 3. Run the Applications

#### Server (Target Machine)
```bash
cd build/bin
./NetworkServer.exe
```

#### Client (Control Machine)
```bash
cd build/bin
./NetworkClient.exe
```

## 🖥️ Usage

### Starting the Server
1. Run `NetworkServer.exe` on the target machine
2. The server will start listening for incoming connections
3. Note the server's IP address for client connection

### Connecting with Client
1. Launch `NetworkClient.exe`
2. Enter the server's IP address and port
3. Click "Connect" to establish connection
4. Use the GUI interface to access remote features

### Feature Access
- **Process Manager**: View and manage running processes
- **File Explorer**: Browse and download files
- **Keylogger**: Start/stop keystroke logging
- **Screen Recorder**: Record desktop activity
- **Email Reports**: Configure and send status emails

## 📁 Project Structure

```
Computer-Network-Project/
├── CMakeLists.txt              # Main build configuration
├── client/                     # Client application
│   ├── main.cpp               # Client entry point
│   ├── include/               # Client headers
│   │   ├── UICore.hpp         # Main UI controller
│   │   ├── MainMenu.hpp       # Main menu interface
│   │   ├── HandleFeature.hpp  # Feature request handling
│   │   └── Mail.hpp           # Email functionality
│   └── src/                   # Client implementation
├── server/                     # Server application
│   ├── main.cpp               # Server entry point
│   ├── include/               # Server headers
│   │   ├── RequestHandler.hpp # Request processing
│   │   ├── Process.hpp        # Process management
│   │   ├── File.hpp           # File operations
│   │   ├── Keylogger.hpp      # Keystroke logging
│   │   └── VideoRecording.hpp # Screen recording
│   └── src/                   # Server implementation
└── cmake-build-debug/         # Build output directory
```

## 🔒 Security Considerations

⚠️ **Important Security Notice**

This tool is designed for **legitimate administrative purposes only**:
- Only use on systems you own or have explicit permission to access
- Ensure compliance with local laws and regulations
- Consider implementing additional authentication mechanisms
- Use secure network connections when possible

## 🐛 Troubleshooting

### Common Issues

1. **Connection Failed**
   - Check firewall settings
   - Verify server is running and accessible
   - Confirm IP address and port

2. **Build Errors**
   - Ensure all prerequisites are installed
   - Check CMake version compatibility
   - Verify Windows SDK installation

3. **Feature Not Working**
   - Check Windows permissions
   - Verify dependencies are properly linked
   - Review error logs

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## ⚠️ Disclaimer

This software is provided for educational and legitimate administrative purposes only. Users are responsible for ensuring compliance with applicable laws and regulations. The developers are not responsible for any misuse of this software.

## 📞 Support

For support and questions:
- Open an issue on GitHub
- Check the troubleshooting section
- Review the project documentation

---

**Built with ❤️ for network administration and system monitoring**

