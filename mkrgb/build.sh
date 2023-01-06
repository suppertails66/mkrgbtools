
echo "*******************************************************************************"
echo "Setting up environment..."
echo "*******************************************************************************"

set -o errexit

BASE_PWD=$PWD
PATH=".:./asm/bin/:$PATH"
INROM="mkrgb.gb"
OUTROM="mkrgb_en.gb"
WLADX="./wla-dx/binaries/wla-gb"
WLALINK="./wla-dx/binaries/wlalink"

cp "$INROM" "$OUTROM"

mkdir -p out

echo "*******************************************************************************"
echo "Building tools..."
echo "*******************************************************************************"

make blackt
make libgb
make

if [ ! -f $WLADX ]; then
  
  echo "********************************************************************************"
  echo "Building WLA-DX..."
  echo "********************************************************************************"
  
  cd wla-dx
    cmake -G "Unix Makefiles" .
    make
  cd $BASE_PWD
  
fi

echo "*******************************************************************************"
echo "Building font..."
echo "*******************************************************************************"

mkdir -p out/font
mkrgb_fontbuild "font/" "out/font/font.bin" "out/font/fontwidth.bin"
 
echo "*******************************************************************************"
echo "Building graphics..."
echo "*******************************************************************************"

mkdir -p out/maps
mkdir -p out/grp

for file in tilemappers/*; do
  tilemapper_gb "$file"
done

echo "*******************************************************************************"
echo "Building script..."
echo "*******************************************************************************"

mkdir -p out/script
mkdir -p out/scriptwrap

mkrgb_scriptwrap "script/script.txt" "out/scriptwrap/script.txt"
mkrgb_scriptwrap "script/script_credits.txt" "out/scriptwrap/script_credits.txt"
mkrgb_scriptwrap "script/script_fixedlen.txt" "out/scriptwrap/script_fixedlen.txt"
mkrgb_scriptwrap "script/script_misc.txt" "out/scriptwrap/script_misc.txt"
mkrgb_scriptwrap "script/script_places.txt" "out/scriptwrap/script_places.txt"

mkrgb_scriptbuild "out/scriptwrap/" "out/script/"

echo "********************************************************************************"
echo "Applying ASM patches..."
echo "********************************************************************************"

mkdir -p "out/asm"
cp "$OUTROM" "asm/mkrgb.gb"

cd asm
  # apply hacks
  ../$WLADX -I ".." -o "main.o" "main.s"
  ../$WLALINK -v -S linkfile mkrgb_patched.gb
cd "$BASE_PWD"

mv -f "asm/mkrgb_patched.gb" "$OUTROM"
mv -f "asm/mkrgb_patched.sym" "$(basename $OUTROM .gb).sym"
rm "asm/mkrgb.gb"
rm "asm/main.o"

echo "********************************************************************************"
echo "Fixing header checksum..."
echo "********************************************************************************"

fixheaderchksum_gb "$OUTROM" "$OUTROM"

echo "*******************************************************************************"
echo "Success!"
echo "Output file:" $OUTROM
echo "*******************************************************************************"
