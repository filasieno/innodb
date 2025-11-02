#!/bin/bash

declare source_dir
source_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
declare script_name=${BASH_SOURCE[0]}
script_name=${script_name##*/}


declare -a extra=()
declare -a targets=()
declare target=
declare preset=
declare preset_file="$source_dir/.preset"

help() {
	cat <<-EOF
	$script_name [--help|-h] [--preset PRESET] [TARGET ...]
	EOF
}

intro() {
	cat <<-EOF
	Current preset = $preset
	Targets = ${targets[@]}
	EOF
}

while (( $# > 0 )); do
	case $1 in
		--help|-h)
			help
			exit 0
			;;
		--preset|-p)
			preset=$2
			shift 2
			;;
		--)
			shift
			targets+=( "$@" )
			break
			;;
		-*)
			printf 'Invalid option "%s"\n' "$1" >&2
			help
			exit 1
			;;
		*)
			targets+=( "$1" )
			shift
		;;
	esac
done

if [[ $preset ]]; then
	extra+=(--preset "$preset")
	echo "$preset" > "$preset_file"
else
	preset=$(cat "$preset_file" 2> /dev/null)
	if [[ ! $preset ]]; then
		preset=debug
		echo "$preset" > "$preset_file"
	fi
	extra+=(--preset "$preset")
fi

for target in "${targets[@]}"; do
	extra+=(--target "$target")
done

intro


cd "$source_dir" || { printf 'Invalid source directory "%s"\n' "$source_dir" >&2 ;  exit 1; }

cmake --preset "$preset"
cmake --build -j "${extra[@]}"