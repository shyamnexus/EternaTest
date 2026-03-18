#!/bin/sh

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" > /dev/null && pwd )"

root_dir=$1



result_file=${root_dir}/results.csv

print_help()
{
    echo "Please input folder path. It must contain '1k' and 'voice' directories."
    exit 0
}

if [ -z "$root_dir" ];then
    print_help
fi

if [ ! -f "$result_file" ];then
    echo "No $result_file"
    exit 1
fi

string_to_num() { 
    local str="$1"
    echo "$str" | sed "s/^0\+\([0-9]\)/\1/g"  #remove leading 0
 }

if [ "$(pip3 list | grep -E "numpy|scipy" | wc -l)" != "2" ];then
    pip3 install numpy scipy
fi

ls -1 $root_dir/1k | while IFS= read -r line; do 
    target_dir=$root_dir/1k/$line
    if [ ! -d "$target_dir" ];then
        continue
    fi
    temp="${line#*_}"

    play_vol="${temp%%_*}"
    mic_vol="${line##*_}"
    play_vol_num=$(string_to_num $play_vol)
    mic_vol_num=$(string_to_num $mic_vol)
    
    echo "python3 $DIR/thd/thd_analyzer_launcher.py  $target_dir/cap.wav "
    thdn=$(python3 $DIR/thd/thd_analyzer_launcher.py  $target_dir/cap.wav 2>/dev/null | grep -oP "THD\+N:\s+\K.*(?=%)")

    if [ -z "$thdn" ];then
        thdn="na"
    fi
    echo "==========  $line, THD+N=$thdn  =========="

    sed -i -E "s|^$play_vol_num,$mic_vol_num,(.*),THD\+N|$play_vol_num,$mic_vol_num,\1,$thdn|g" "$result_file"
done

echo Done