#!/bin/bash

date

echo "Input path："$1
# 检查是否提供了路径参数
if [ -z "$1" ]; then
	echo "Usage: $0 <file_path>"
	exit 1
fi

# 获取传入的路径参数
file_path="$1"

# 检查是否存在
if [ ! -f "$file_path" ]; then
	echo "Error: file_path '$file_path' does not exist."
	exit 2
fi

#rt_system.elf file path
echo 'creat sys.map'
nm -n $file_path >sys.map

echo 'delete $t/$d/U/w/B/b/A/a/D/d/__func__. in sys.map'
sed -i '/$d/d' sys.map
sed -i '/$t/d' sys.map
sed -i '/ U /d' sys.map
sed -i '/ w /d' sys.map
#sed -i '/ B /d' sys.map
#sed -i '/ b /d' sys.map
#sed -i '/ A /d' sys.map
#sed -i '/ a /d' sys.map
#sed -i '/ D /d' sys.map
#sed -i '/ d /d' sys.map
sed -i '/__func__./d' sys.map

echo 'kallsyms sys.map > symbol.S'
./kallsyms/kallsyms --all-symbols sys.map > ./symbol.S

echo 'fix rodata to xip_rodata in symbol.S'
sed -i 's/rodata/xip_rodata/g' ./symbol.S

echo 'fix symbol.S'
sed -i '1 s/</"/g' symbol.S
sed -i '1 s/>/"/g' symbol.S

echo 'successfully!'

