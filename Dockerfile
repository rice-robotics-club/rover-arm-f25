# Base image requested
FROM moveit/moveit2:jazzy-release

# 1. Basic Configuration
# Set the shell to bash to allow usage of "source"
SHELL ["/bin/bash", "-c"]

# Set the working directory for the container's workspace
WORKDIR /colcon_ws

# 2. Copy Source Code
# Copy your local directory (where this Dockerfile lives) into the container's src folder.
# NOTE: This assumes the Dockerfile is at the root of your package or repo.
COPY . src/my_package/

# 3. Install Dependencies
# Update apt repo and install dependencies defined in package.xml
USER root
RUN apt-get update && \
    rosdep update && \
    rosdep install --from-paths src --ignore-src -r -y && \
    rm -rf /var/lib/apt/lists/*

# 4. Build the Workspace
# We source the MoveIt installation (underlay) before building our overlay
RUN source /opt/ros/jazzy/setup.bash && \
    colcon build --cmake-args -DCMAKE_BUILD_TYPE=Release

# 5. Setup Entrypoint
# Create a script to source the workspace automatically when the container starts
COPY <<EOF /entrypoint.sh
#!/bin/bash
set -e
source /opt/ros/jazzy/setup.bash
source /colcon_ws/install/setup.bash
exec "\$@"
EOF

RUN chmod +x /entrypoint.sh

# Set the entrypoint
ENTRYPOINT ["/entrypoint.sh"]

# default command (optional)
CMD ["bash"]