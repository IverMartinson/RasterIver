echo "[MAIN]     Building..."
gcc  -Wall -Wextra -D CL_TARGET_OPENCL_VERSION=120 $1 -o main.bin -lSDL2 -lm -lOpenCL
echo "[MAIN]     Built"
