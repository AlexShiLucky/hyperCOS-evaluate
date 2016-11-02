function cfg_gcc
{
	unset CLANG
}

function cfg_clang
{
	eval `cat Makefile | grep TARGET.*= | sed -e 's/[: ]//g'`
	a=`which $TARGET-gcc`
	a=${a%/*}
	a=${a%/*}
	b=`find $a -name "string.h" | grep -v "sys"`
	b=${b%/*}
	export CLANG="clang -target $TARGET -I$b"
	export CC=$CLANG
#	echo CC=\"$CC\"
}

function cfg_v5t
{
	export COPTS="-march=armv5t -msoft-float"
	echo "COPTS=$COPTS"
}

function cfg_v5tT
{
	export COPTS="-march=armv5t -mthumb -msoft-float"
	echo "COPTS=$COPTS"
}

function cfg_v7aT
{
	export COPTS="-march=armv7-a -mthumb -msoft-float"
	echo "COPTS=$COPTS"
}

function cfg_v7mTF_core
{
	export COPTS="-march=armv7-m -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -fsingle-precision-constant -Wdouble-promotion"
	echo "COPTS=$COPTS"
}

function cfg_v7mTF
{
	cfg_v7mTF_core
	export CONFIG="-DCFG_STACK_IRQ=0x4096 -DCFG_TICKLESS=0 -DCFG_CACHE_VMSA=0 -DCFG_IRQ_VECTS=1 -DCFG_HFLOAT=1"
	echo "CONFIG=\"-DCFG_STACK_IRQ=0x4096 -DCFG_TICKLESS=0 -DCFG_CACHE_VMSA=0 -DCFG_IRQ_VECTS=1 -DCFG_HFLOAT=1"
}

function cfg_v7mT_core
{
	export COPTS="-march=armv7-m -mthumb -mfloat-abi=soft "
	echo "COPTS=$COPTS"
}

function cfg_v7mT
{
	cfg_v7mT_core
	export CONFIG="-DCFG_STACK_IRQ=0x4096 -DCFG_TICKLESS=1 -DCFG_CACHE_VMSA=0 -DCFG_IRQ_VECTS=1 -DCFG_HFLOAT=0"
	echo "CONFIG=\"-DCFG_STACK_IRQ=0x4096 -DCFG_TICKLESS=1 -DCFG_CACHE_VMSA=0 -DCFG_IRQ_VECTS=1 -DCFG_HFLOAT=0"
}

