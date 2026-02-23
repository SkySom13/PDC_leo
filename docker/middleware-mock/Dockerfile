FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive

# 1. Build deps
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    libboost-all-dev \
    libglib2.0-0 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# 2. Install CommonAPI + vsomeip from install_folder (local symlink)
COPY install_folder/include /usr/local/include
COPY install_folder/lib     /usr/local/lib
COPY install_folder/etc     /etc
RUN ldconfig

# 3. Copy generated CommonAPI code and mock sources (local symlinks/dirs)
COPY commonapi /pdc/app-commonapi
COPY src       /pdc/middleware-mock

WORKDIR /pdc/middleware-mock

# 4. Build mock server
RUN mkdir -p build && cd build && \
    cmake .. -G Ninja -DCMAKE_PREFIX_PATH=/usr/local && \
    ninja

# 5. vsomeip config from local file
COPY vsomeip-docker.json /etc/vsomeip/vsomeip-docker.json

ENV VSOMEIP_APPLICATION_NAME=vehicle_mock
ENV VSOMEIP_CONFIGURATION=/etc/vsomeip/vsomeip-docker.json

# 6. Entrypoint
CMD ["bash", "-lc", "cd /pdc/middleware-mock/build && ./vehicle_mock_server"]
