#!/bin/sh -e
DIR=$(cd ${0%/*}; pwd)

case_name=$1
root_dir=$DIR

if [ -z "$case_name" ];then
    echo "Please input folder name. It must contain '1k' and 'voice' directories."
    exit 1
fi

if [ ! -d "$root_dir/$case_name/1k" ];then
    echo "Cannot find $root_dir/$case_name/1k"
    exit 1
fi

if [ ! -d "$root_dir/$case_name/voice" ];then
    echo "Cannot find $root_dir/$case_name/voice"
    exit 1
fi

cd "$root_dir/$case_name/voice"

result_file=${root_dir}/$case_name/results.csv

if [ -f "$result_file" ];then
    read -p "Result file exists. Do you want to overwrite it? ($result_file) [Y/n]" confirm_overwrite
    if [ "${confirm_overwrite}" == "n" ];then
        exit 1
    fi
    rm -f "$result_file"
fi


calc() { awk "BEGIN{print $*}"; }
string_to_num() { 
    local str="$1"
    echo "$str" | sed "s/^0\+\([0-9]\)/\1/g"  #remove leading 0
 }

audiocap_volume_to_gain(){
    # ALC off
    local volume=$1
    local gain=20  #boost gain

    if [ $volume -le 0 ];then
        gain=-127
    elif [ $volume -le 11 ];then
        gain=$(calc $gain - 18)
    elif [ $volume -le 23 ];then
        gain=$(calc $gain - 12)
    elif [ $volume -le 35 ];then
        gain=$(calc $gain - 6)
    elif [ $volume -le 47 ];then
        gain=$(calc $gain + 0)
    elif [ $volume -le 59 ];then
        gain=$(calc $gain + 6)
    elif [ $volume -le 71 ];then
        gain=$(calc $gain + 12)
    elif [ $volume -le 83 ];then
        gain=$(calc $gain + 18)
    elif [ $volume -le 100 ];then
        gain=$(calc $gain + 24)
    elif [ $volume -le 160 ];then
        gain=$(calc $gain + 24 + \($volume-100\)*0.5)
    fi

    echo $gain
}

audioout_volume_to_gain(){
    local volume=$1

    if [ $volume -gt 100 ];then
        echo $(calc 6.5+\($volume-100\)*0.5)
        return
    fi

    case $volume in 
    0) echo -127;;
    1) echo -31;;
    2) echo -30.5;;
    3) echo -30;;
    4) echo -30;;
    5) echo -29.5;;
    6) echo -29;;
    7) echo -29;;
    8) echo -28;;
    9) echo -27.5;;
    10) echo -27.5;;
    11) echo -27;;
    12) echo -26.5;;
    13) echo -26.5;;
    14) echo -26;;
    15) echo -25;;
    16) echo -25;;
    17) echo -24.5;;
    18) echo -24;;
    19) echo -24;;
    20) echo -23.5;;
    21) echo -23;;
    22) echo -23;;
    23) echo -22;;
    24) echo -21.5;;
    25) echo -21.5;;
    26) echo -21;;
    27) echo -20.5;;
    28) echo -20.5;;
    29) echo -20;;
    30) echo -19;;
    31) echo -19;;
    32) echo -18.5;;
    33) echo -18;;
    34) echo -18;;
    35) echo -17.5;;
    36) echo -17;;
    37) echo -17;;
    38) echo -16;;
    39) echo -15.5;;
    40) echo -15.5;;
    41) echo -15;;
    42) echo -14.5;;
    43) echo -14.5;;
    44) echo -14;;
    45) echo -13;;
    46) echo -13;;
    47) echo -12.5;;
    48) echo -12;;
    49) echo -12;;
    50) echo -11.5;;
    51) echo -11;;
    52) echo -11;;
    53) echo -10;;
    54) echo -9.5;;
    55) echo -9.5;;
    56) echo -9;;
    57) echo -8.5;;
    58) echo -8.5;;
    59) echo -8;;
    60) echo -7;;
    61) echo -7;;
    62) echo -6.5;;
    63) echo -6;;
    64) echo -6;;
    65) echo -5.5;;
    66) echo -5;;
    67) echo -5;;
    68) echo -4;;
    69) echo -3.5;;
    70) echo -3.5;;
    71) echo -3;;
    72) echo -2.5;;
    73) echo -2.5;;
    74) echo -2;;
    75) echo -1;;
    76) echo -1;;
    77) echo -0.5;;
    78) echo 0;;
    79) echo 0;;
    80) echo 0.5;;
    81) echo 1;;
    82) echo 1;;
    83) echo 2;;
    84) echo 2.5;;
    85) echo 2.5;;
    86) echo 3;;
    87) echo 3.5;;
    88) echo 3.5;;
    89) echo 4;;
    90) echo 5;;
    91) echo 5;;
    92) echo 5.5;;
    93) echo 6;;
    94) echo 6;;
    95) echo 6.5;;
    96) echo 6.5;;
    97) echo 6.5;;
    98) echo 6.5;;
    99) echo 6.5;;
    100) echo 6.5;;
    esac
}


echo "play_vol,mic_vol,rec_tone_avg_db,rec_tone_peak_db,play_gain,mic_gain,ERL,THD+N,rec_avg_db,rec_peak_db,apm_avg_db,apm_peak_db,apm_sd,hdal_avg_db,hdal_peak_db,hdal_sd" >> "$result_file"

ls -1 "$root_dir/$case_name/voice" | while IFS= read -r line; do 
    if [ ! -d "$line" ];then
        continue
    fi
	cd $line
    temp="${line#*_}"

    play_vol="${temp%%_*}"
    mic_vol="${line##*_}"

    if [ ! -f "./aec_hdal.wav" ];then
        echo "Run hdal AEC on $line"
        ${root_dir}/aec_test ./ref.wav ./cap.wav ./aec_hdal.wav
    fi

    echo "Analyze 1k tone"
    result=$(${root_dir}/hd_audio_bidirect_with_apm -cf ../../1k/$line/cap.wav)
    rec_tone_avg_db=$(echo "$result" | grep average_db | sed 's/average_db=//g')
    rec_tone_peak_db=$(echo "$result" | grep peak_db | sed 's/peak_db=//g')

    echo "Analyze microphone input cap.wav"
    result=$(${root_dir}/hd_audio_bidirect_with_apm -cf ./cap.wav -s 2000)
    cap_average_db=$(echo "$result" | grep average_db | sed 's/average_db=//g')
    cap_peak_db=$(echo "$result" | grep peak_db | sed 's/peak_db=//g')
    cap_sd=$(echo "$result" | grep standard_deviation | sed 's/standard_deviation=//g')
    
    echo "Analyze apm aec.wav"
    result=$(${root_dir}/hd_audio_bidirect_with_apm -cf ./aec.wav -s 2000)
    aec_average_db=$(echo "$result" | grep average_db | sed 's/average_db=//g')
    aec_peak_db=$(echo "$result" | grep peak_db | sed 's/peak_db=//g')
    aec_sd=$(echo "$result" | grep standard_deviation | sed 's/standard_deviation=//g')

    echo "Analyze hdal aec_hdal.wav"
    result=$(${root_dir}/hd_audio_bidirect_with_apm -cf ./aec_hdal.wav -s 2000)
    hdal_average_db=$(echo "$result" | grep average_db | sed 's/average_db=//g')
    hdal_peak_db=$(echo "$result" | grep peak_db | sed 's/peak_db=//g')
    hdal_sd=$(echo "$result" | grep standard_deviation | sed 's/standard_deviation=//g')

    #echo "audioout_volume_to_gain $(string_to_num $play_vol)"
    play_vol_num=$(string_to_num $play_vol)
    play_gain=$(audioout_volume_to_gain $play_vol_num)
    #echo "audiocap_volume_to_gain $(string_to_num $mic_vol)"
    mic_vol_num=$(string_to_num $mic_vol)
    mic_gain=$(audiocap_volume_to_gain $mic_vol_num)

    ERL=$(calc $rec_tone_peak_db-\(-11+$play_gain+$mic_gain\))

    echo "result=$play_vol_num,$mic_vol_num,$rec_tone_avg_db,$rec_tone_peak_db,$play_gain,$mic_gain,$ERL,THD+N,$cap_average_db,$cap_peak_db,$aec_average_db,$aec_peak_db,$aec_sd,$hdal_average_db,$hdal_peak_db,$hdal_sd"
    echo "$play_vol_num,$mic_vol_num,$rec_tone_avg_db,$rec_tone_peak_db,$play_gain,$mic_gain,$ERL,THD+N,$cap_average_db,$cap_peak_db,$aec_average_db,$aec_peak_db,$aec_sd,$hdal_average_db,$hdal_peak_db,$hdal_sd" >> "$result_file"

	cd ..
done

echo Done
