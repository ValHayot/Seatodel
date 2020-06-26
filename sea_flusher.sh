#!/bin/bash


set -e

base_source=""
sources_arr=()
n_sources=0
conf_file=${SEA_HOME}/sea.ini
flush_file=${SEA_HOME}/.sea_flushlist

flush () {
    while :
    do
        
        re_flush=""
        for s in $sources_arr
        do
            while IFS="" read -r re || [ -n "$re" ]
            do
                if [[ $re_flush != "" ]]
                then
                    re_flush+=" "
                fi

                re_flush+=$sources_arr/$re

            done < $flush_file
        done

        if [[ $re_flush != "" ]]
        then
            tobe_flushed=$(ls -tu $(find ${re_flush} -type f -follow -print))


            for f in $tobe_flushed
            do
                for s in $sources_arr
                do
                    if [[ "$f" == "${s}/"* ]]
                    then
                        file_out=$(echo "${f/$s/$base_source}")
                        echo "flushing and evicting $f to $file_out"
                        mv $f ${file_out}
                        break
                    fi
                done
            done
        fi

        sleep 5
    done
}


get_sources () {

    # refers to the last index and not the total number of sources
    n_sources="$(($(awk -F "=" '/n_sources/ {print $2}' ${SEA_HOME}/sea.ini | tr -d ' ;') - 1))"

    for ((i=0;i<${n_sources};i++))
    do
        sources_arr+=($(cat ${conf_file} | grep "source_$i" | cut -d "=" -f 2 | tr -d ' ;'))
        echo source_$i ${sources_arr[$i]}
    done

    base_source="$(cat ${conf_file} | grep "source_$i" | cut -d "=" -f 2 | tr -d ' ;')"
    echo base dir ${base_source}

}

trap 'trap - SIGTERM && kill 0' SIGINT SIGTERM EXIT
get_sources
flush
