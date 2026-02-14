# Stage 1: Build stage
FROM ubuntu:latest AS build

# Install build-essential and cmake for compiling C++ code
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    && rm -rf /var/lib/apt/lists/*

# Sets the working directory inside the container to /app
WORKDIR /app

# Copies everything from the project into /app inside the container
COPY . .

# Configure and build the project
# PACKAGING_MAINTAINER_MODE disables dev-only tools (sanitizers, clang-tidy, etc.)
# that shouldn't be in a production build
RUN cmake -B build -Dcpp_template_redux_PACKAGING_MAINTAINER_MODE=ON \
    && cmake --build build

# Run tests
RUN ctest --test-dir build --output-on-failure

# Stage 2: Runtime stage
FROM ubuntu:latest

# Copy the built binary from the build stage
COPY --from=build /app/build/src/compiled_app /compiled_app

# Command to run the binary
CMD ["/compiled_app"]
