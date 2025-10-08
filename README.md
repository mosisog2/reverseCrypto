# reverseCrypto
This is a group project for Cryptography class. This project focuses on the topic of Reverse Software Engineering.

## Quick Start with Docker

The easiest way to run this project is using Docker. This ensures you have all the necessary tools and dependencies without having to install them manually.

### Prerequisites
- [Docker](https://docs.docker.com/get-docker/)
- [Docker Compose](https://docs.docker.com/compose/install/) (usually included with Docker Desktop)

### Option 1: Using Docker Compose (Recommended)
```bash
# Clone the repository
git clone <your-repo-url>
cd reverseCrypto

# Build and run the container
docker-compose up -d

# Enter the container
docker-compose exec reversecrypto bash

# When done, stop the container
docker-compose down
```

### Option 2: Using Docker directly
```bash
# Build the image
docker build -t reversecrypto .

# Run the container with your project mounted
docker run -it -v $(pwd):/workspace reversecrypto

# Or on Windows PowerShell:
docker run -it -v ${PWD}:/workspace reversecrypto
```

### What's Included
The Docker container comes pre-installed with:
- **Build tools**: GCC, Make, CMake, Ninja
- **Cryptography**: OpenSSL library and CLI tools
- **Python environment**: Python 3 with virtual environment and crypto libraries
- **Reverse engineering tools**: Radare2, binutils, objdump, strings, hexdump
- **Cross-compilation**: RISC-V GCC toolchain and QEMU emulation
- **Utilities**: Git, curl, text editors, and more

### Running Your Code
Once inside the container:
```bash
# Your project files are available in /workspace
ls -la

# Use the Makefile (when implemented)
make

# Or run the shell script (when implemented)
./run.sh

# Access Python with crypto libraries
python3 -c "from Crypto.Cipher import AES; print('Crypto libraries ready!')"
```
