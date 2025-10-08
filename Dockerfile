# ---- Crypto × RE project dependencies image ----
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    # Build + tooling
    build-essential make pkg-config cmake ninja-build \
    # OpenSSL libs/headers + CLI for keygen and BN
    libssl-dev openssl \
    # Python for attack scripts
    python3 python3-venv python3-pip \
    # Disassembly/inspection (CLI) - binutils includes objdump, strings, etc.
    binutils file radare2 \
    # RISC-V cross-compile + run via QEMU (optional but supported)
    gcc-riscv64-linux-gnu qemu-user qemu-user-static \
    # Helpful utilities
    ca-certificates git curl \
    # Additional reverse engineering tools (xxd is in vim-common)
    vim-common bsdextrautils \
    # Text processing tools (grep, sed are in coreutils, gawk provides awk)
    vim nano less gawk \
 && rm -rf /var/lib/apt/lists/*

# Non-root dev user (safer file perms when mounting your repo)
RUN useradd -m dev
USER dev
WORKDIR /workspace

# Copy project files (if building with context)
# Uncomment these lines if you want to include the project in the image
# COPY --chown=dev:dev . /workspace/

# Set up Python virtual environment for the dev user
RUN python3 -m venv /home/dev/venv
ENV PATH="/home/dev/venv/bin:$PATH"

# Install common Python packages for cryptography and reverse engineering
RUN pip install --no-cache-dir \
    pycryptodome \
    cryptography \
    numpy \
    matplotlib \
    jupyter \
    requests

# Default command drops you into a shell in /workspace (your project mount point)
CMD ["/bin/bash"]