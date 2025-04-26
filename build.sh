echo "[MAIN]     Building..."
gcc -g -Wall -Wextra -D CL_TARGET_OPENCL_VERSION=120 $1 -o main.bin -L. -lrasteriver -Wl,-rpath=.
echo "[MAIN]     Built"
