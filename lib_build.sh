echo "[LIB]      Building..."
gcc  -Wall -Wextra -D CL_TARGET_OPENCL_VERSION=120 -fPIC -shared -o librasteriver.so $1 -lc -lSDL2 -lm -lOpenCL
echo "[LIB]      Built"
