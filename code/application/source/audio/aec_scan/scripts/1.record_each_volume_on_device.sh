#!/bin/sh -e
DIR=$(cd ${0%/*}; pwd)

case_name=$1
root_dir=$DIR

min_play_volume=0
max_play_volume=100
play_volume_interval=10

min_mic_volume=10   # mic volume 0 is mute
max_mic_volume=100
mic_volume_interval=10

far_wav_1=audio_sample_1k_tone_16000.wav
far_wav_2=audio_sample_radio_16000.wav

if [ -z "$case_name" ];then
    echo "Please input folder name. It will generate '1k' and 'voice' directories."
    exit 1
fi

if [ "$#" -ge 7 ];then
    min_play_volume=$2
    max_play_volume=$3
    play_volume_interval=$4

    min_mic_volume=$5
    max_mic_volume=$6
    mic_volume_interval=$7
fi

if [ "$#" -ge 8 ];then
    far_wav_1=$8
fi
if [ "$#" -ge 9 ];then
    far_wav_2=$9
fi

print_help()
{
    echo -e "
    Please input the folder name for saving the results.  
    format: ./1.record_each_volume_on_device.sh NAME [min_play_volume max_play_volume play_volume_interval min_mic_volume max_mic_volume mic_volume_interval]
    e.g. ./1.record_each_volume_on_device.sh machine1_20220711 0 100 10 10 100 10"
    exit 0
}

calc() { awk "BEGIN{print $*}"; }

scan_each_volume(){
    local sub_dir=$1
    local far_wav=$2
    local aec_mode=$3
    local record_duration=$4

    echo "scan_each_volume+ sub_dir=$sub_dir, far_wav=$far_wav, aec_mode=$aec_mode, record_duration=$record_duration"

    local ns_switch="";
    if [ "$sub_dir" == "1k" ];then
        # turn off noise suppression
        ns_switch="-n -1";
    fi

    local play_volume=$max_play_volume
    local mic_volume=$max_mic_volume

    while [ $play_volume -ge $min_play_volume ]; do
        while [ $mic_volume -ge $min_mic_volume ]; do
            local target_dir=${root_dir}/${case_name}/${sub_dir}/play_$(printf "%03d\n" ${play_volume})_mic_$(printf "%03d\n" ${mic_volume})
            echo -e " ********************* $target_dir *********************"
            
            ${root_dir}/hd_audio_bidirect_with_apm -e $aec_mode -f ${root_dir}/${far_wav} -P ${play_volume} -M ${mic_volume} -t $record_duration $ns_switch -o $root_dir

            mkdir -p ${target_dir}
            if [ "$aec_mode" == "0" ];then
                mv ${root_dir}/cap.wav ${root_dir}/ref.wav $target_dir
            else
                mv ${root_dir}/cap.wav ${root_dir}/ref.wav ${root_dir}/aec.wav $target_dir
            fi

            mic_volume=$(calc $mic_volume-$mic_volume_interval)
        done
        play_volume=$(calc $play_volume-$play_volume_interval)
        mic_volume=$max_mic_volume
    done

    return 0
}

if [ -z "$case_name" ];then
    print_help
fi

aec_mode=0
record_duration=1000
scan_each_volume "1k" $far_wav_1 $aec_mode $record_duration

aec_mode=1 # also output AEC result (apm)
record_duration=8000
scan_each_volume "voice" $far_wav_2 $aec_mode $record_duration


# call analysis
$root_dir/2.analyze_on_device.sh $case_name