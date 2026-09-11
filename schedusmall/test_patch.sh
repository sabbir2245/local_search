#!/bin/bash
set -e

echo "=== Cloning fresh xv6-riscv ==="
rm -rf test_final
git clone https://github.com/shuaibw/xv6-riscv.git test_final
cd test_final

echo "=== Applying patch ==="
git apply ../2205040.patch
echo "Patch applied successfully"

echo "=== Building ==="
make clean
make fs.img

echo "=== Build succeeded ==="
echo ""
echo "Run this to test:"
echo "  cd test_final && make qemu"
echo ""
echo "Then inside xv6 shell:"
echo "  testprocinfo"
echo "  dummyproc 30 1000000000 &"
echo "  testprocinfo"
echo ""
echo "You should see DEMO, PROMO, BOOST, LOTTERY logs."
